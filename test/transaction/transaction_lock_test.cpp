/**
 * Copyright (c) 2026 Nikola Nedeljkovic
 * SPDX-License-Identifier: MIT
 */

#include "duck/transaction/lock_manager.hpp"
#include "duck/transaction/manager.hpp"
#include "duck/transaction/transaction.hpp"

#include <atomic>
#include <chrono>
#include <gtest/gtest.h>
#include <thread>

class LockManagerTest : public ::testing::Test {
protected:
    duck::LockManager lock_manager_;
    duck::TransactionManager txn_manager_{lock_manager_};
};

TEST_F(LockManagerTest, SharedLocksAreCompatible) {
    auto t1 = txn_manager_.begin();
    auto t2 = txn_manager_.begin();
    duck::RID rid{1, 0};

    EXPECT_TRUE(lock_manager_.lock_shared(t1.get(), rid));
    EXPECT_TRUE(lock_manager_.lock_shared(t2.get(), rid));

    txn_manager_.commit(t1.get());
    txn_manager_.commit(t2.get());
}

TEST_F(LockManagerTest, ExclusiveBlocksOtherExclusive) {
    auto t1 = txn_manager_.begin();
    auto t2 = txn_manager_.begin();
    duck::RID rid{1, 0};

    EXPECT_TRUE(lock_manager_.lock_exclusive(t1.get(), rid));

    std::atomic<bool> t2_acquired{false};
    std::thread t2_thread([&]() {
        if (lock_manager_.lock_exclusive(t2.get(), rid)) {
            t2_acquired = true;
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_FALSE(t2_acquired.load());

    txn_manager_.commit(t1.get());
    t2_thread.join();

    EXPECT_TRUE(t2_acquired.load());
    txn_manager_.commit(t2.get());
}

TEST_F(LockManagerTest, SameTransactionCanRelockSameRid) {
    auto t1 = txn_manager_.begin();
    duck::RID rid{1, 0};

    EXPECT_TRUE(lock_manager_.lock_shared(t1.get(), rid));
    EXPECT_TRUE(lock_manager_.lock_shared(t1.get(), rid));

    txn_manager_.commit(t1.get());
}

TEST_F(LockManagerTest, LockUpgradeSharedToExclusive) {
    auto t1 = txn_manager_.begin();
    duck::RID rid{1, 0};

    EXPECT_TRUE(lock_manager_.lock_shared(t1.get(), rid));
    EXPECT_TRUE(lock_manager_.lock_exclusive(t1.get(), rid));

    EXPECT_TRUE(t1->exclusive_locks_.contains(rid));
    EXPECT_FALSE(t1->shared_locks_.contains(rid));

    txn_manager_.commit(t1.get());
}

TEST_F(LockManagerTest, ExclusiveTimesOutWhenOtherHoldsShared) {
    auto t1 = txn_manager_.begin();
    auto t2 = txn_manager_.begin();
    duck::RID rid{1, 0};

    EXPECT_TRUE(lock_manager_.lock_shared(t1.get(), rid));
    EXPECT_FALSE(lock_manager_.lock_exclusive(t2.get(), rid));

    EXPECT_EQ(t2->state(), duck::TransactionState::ABORTED);

    txn_manager_.commit(t1.get());
}

TEST_F(LockManagerTest, UnlockAllReleasesEverything) {
    auto t1 = txn_manager_.begin();
    duck::RID rid_a{1, 0};
    duck::RID rid_b{1, 1};

    lock_manager_.lock_shared(t1.get(), rid_a);
    lock_manager_.lock_exclusive(t1.get(), rid_b);

    txn_manager_.commit(t1.get());

    auto t2 = txn_manager_.begin();
    EXPECT_TRUE(lock_manager_.lock_exclusive(t2.get(), rid_a));
    EXPECT_TRUE(lock_manager_.lock_shared(t2.get(), rid_b));

    txn_manager_.commit(t2.get());
}

class TransactionManagerTest : public ::testing::Test {
protected:
    duck::LockManager lock_manager_;
    duck::TransactionManager txn_manager_{lock_manager_};
};

TEST_F(TransactionManagerTest, BeginAssignsIncreasingIds) {
    auto t1 = txn_manager_.begin();
    auto t2 = txn_manager_.begin();

    EXPECT_LT(t1->id(), t2->id());

    txn_manager_.commit(t1.get());
    txn_manager_.commit(t2.get());
}

TEST_F(TransactionManagerTest, CommitSetsCommittedState) {
    auto t1 = txn_manager_.begin();
    txn_manager_.commit(t1.get());

    EXPECT_EQ(t1->state(), duck::TransactionState::COMMITTED);
}

TEST_F(TransactionManagerTest, AbortSetsAbortedState) {
    auto t1 = txn_manager_.begin();
    txn_manager_.abort(t1.get());

    EXPECT_EQ(t1->state(), duck::TransactionState::ABORTED);
}

TEST_F(TransactionManagerTest, ClientCanStillReadStateAfterInternalAbort) {
    // Exercises the exact reason for shared_ptr<Transaction>: the client holds
    // its own reference, and can safely observe ABORTED even after LockManager
    // has already removed the transaction from active_txs_ internally.
    auto t1 = txn_manager_.begin();
    auto t2 = txn_manager_.begin();
    duck::RID rid{1, 0};

    lock_manager_.lock_shared(t1.get(), rid);
    // t2's exclusive request times out after 1s and self-aborts + gets removed
    // from active_txs_ internally by LockManager -- but our shared_ptr keeps it alive.
    EXPECT_FALSE(lock_manager_.lock_exclusive(t2.get(), rid));

    EXPECT_EQ(t2->state(), duck::TransactionState::ABORTED); // client still safely reads this

    txn_manager_.commit(t1.get());
}
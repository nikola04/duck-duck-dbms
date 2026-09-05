/**
 * Copyright (c) 2026 Nikola Nedeljkovic
 * SPDX-License-Identifier: MIT
 */

#include "duck/database/database.hpp"
#include "duck/table/table.hpp"
#include "duck/tuple/column.hpp"
#include "duck/tuple/schema.hpp"
#include "duck/tuple/tuple.hpp"
#include "duck/tuple/value.hpp"

#include <cstdio>
#include <gtest/gtest.h>
#include <string>
#include <vector>

class TransactionRollbackTest : public ::testing::Test {
protected:
    static constexpr const char* kDBPath{"transaction_rollback_test.db"};

    void SetUp() override {
        std::remove(kDBPath);
    }

    void TearDown() override {
        std::remove(kDBPath);
    }

    static duck::Schema make_schema() {
        return duck::Schema{std::vector<duck::Column>{
            {"id", duck::TypeId::UINT32},
            {"name", duck::TypeId::VARCHAR, 128},
        }};
    }

    static duck::Tuple make_tuple(const duck::Schema& schema, std::uint32_t id, std::string name) {
        return duck::Tuple{std::vector<duck::Value>{
                               duck::Value::of(id),
                               duck::Value::of(std::move(name)),
                           },
                           schema};
    }
};

/*
 * CREATE TABLE
 */

TEST_F(TransactionRollbackTest, RollbackCreateTable) {
    duck::Database db{kDBPath};

    auto tx{db.begin_tx()};

    auto* table{db.create_table("users", make_schema(), tx.get())};

    ASSERT_NE(table, nullptr);
    ASSERT_TRUE(db.get_table("users").has_value());

    db.rollback_tx(tx.get());

    EXPECT_FALSE(db.get_table("users").has_value());
}

/*
 * INSERT
 */

TEST_F(TransactionRollbackTest, RollbackInsert) {
    duck::Database db{kDBPath};

    auto table{db.create_table("users", make_schema())};

    auto tx{db.begin_tx()};

    auto rid{table->insert_tuple(make_tuple(make_schema(), 1, "Nikola"), tx.get())};

    ASSERT_TRUE(rid.has_value());

    db.rollback_tx(tx.get());

    auto verify_tx{db.begin_tx()};

    EXPECT_FALSE(table->get_tuple(*rid, verify_tx.get()).has_value());

    db.commit_tx(verify_tx.get());
}

/*
 * DELETE
 */

TEST_F(TransactionRollbackTest, RollbackDelete) {
    duck::Database db{kDBPath};

    auto table{db.create_table("users", make_schema())};

    auto insert_tx{db.begin_tx()};

    auto rid{table->insert_tuple(make_tuple(make_schema(), 1, "Nikola"), insert_tx.get())};

    ASSERT_TRUE(rid.has_value());

    db.commit_tx(insert_tx.get());

    auto tx{db.begin_tx()};

    ASSERT_TRUE(table->delete_tuple(*rid, tx.get()));

    db.rollback_tx(tx.get());

    auto verify_tx{db.begin_tx()};

    auto restored{table->get_tuple(*rid, verify_tx.get())};

    ASSERT_TRUE(restored.has_value());
    EXPECT_EQ(restored->get(0).as_uint32(), 1);
    EXPECT_EQ(restored->get(1).as_string(), "Nikola");

    db.commit_tx(verify_tx.get());
}

/*
 * UPDATE IN PLACE
 */

TEST_F(TransactionRollbackTest, RollbackUpdateInPlace) {
    duck::Database db{kDBPath};

    auto table{db.create_table("users", make_schema())};

    auto insert_tx{db.begin_tx()};

    auto rid{table->insert_tuple(make_tuple(make_schema(), 1, "Nikola"), insert_tx.get())};

    ASSERT_TRUE(rid.has_value());

    db.commit_tx(insert_tx.get());

    auto tx{db.begin_tx()};

    ASSERT_TRUE(table->update_tuple(*rid, make_tuple(make_schema(), 1, "Marko"), tx.get()));

    db.rollback_tx(tx.get());

    auto verify_tx{db.begin_tx()};

    auto restored{table->get_tuple(*rid, verify_tx.get())};

    ASSERT_TRUE(restored.has_value());
    EXPECT_EQ(restored->get(0).as_uint32(), 1);
    EXPECT_EQ(restored->get(1).as_string(), "Nikola");

    db.commit_tx(verify_tx.get());
}

/*
 * UPDATE THAT MOVES THE TUPLE
 *
 * The new tuple is larger than the old one, so update should
 * delete the old RID and insert the new tuple at another RID.
 *
 * Rollback must remove the new tuple and restore the old one.
 */

TEST_F(TransactionRollbackTest, RollbackMovedUpdate) {
    duck::Database db{kDBPath};

    auto table{db.create_table("users", make_schema())};

    auto insert_tx{db.begin_tx()};

    auto old_rid{table->insert_tuple(make_tuple(make_schema(), 1, "Nikola"), insert_tx.get())};

    ASSERT_TRUE(old_rid.has_value());

    db.commit_tx(insert_tx.get());

    auto tx{db.begin_tx()};

    auto new_rid{table->update_tuple(*old_rid, make_tuple(make_schema(), 1, "Nikola Nedeljkovic"), tx.get())};

    ASSERT_TRUE(new_rid.has_value());

    db.rollback_tx(tx.get());

    auto verify_tx{db.begin_tx()};

    // Old RID must be restored.
    auto restored{table->get_tuple(*old_rid, verify_tx.get())};

    ASSERT_TRUE(restored.has_value());
    EXPECT_EQ(restored->get(0).as_uint32(), 1);
    EXPECT_EQ(restored->get(1).as_string(), "Nikola");

    // New RID must be gone if the update actually moved the tuple.
    if (*new_rid != *old_rid) {
        EXPECT_FALSE(table->get_tuple(*new_rid, verify_tx.get()).has_value());
    }

    db.commit_tx(verify_tx.get());
}

/*
 * MULTIPLE INSERTS
 *
 * Verifies that undo records are executed in reverse order.
 */

TEST_F(TransactionRollbackTest, RollbackMultipleInserts) {
    duck::Database db{kDBPath};

    auto table{db.create_table("users", make_schema())};

    auto tx{db.begin_tx()};

    auto rid1{table->insert_tuple(make_tuple(make_schema(), 1, "One"), tx.get())};

    auto rid2{table->insert_tuple(make_tuple(make_schema(), 2, "Two"), tx.get())};

    auto rid3{table->insert_tuple(make_tuple(make_schema(), 3, "Three"), tx.get())};

    ASSERT_TRUE(rid1.has_value());
    ASSERT_TRUE(rid2.has_value());
    ASSERT_TRUE(rid3.has_value());

    db.rollback_tx(tx.get());

    auto verify_tx{db.begin_tx()};

    EXPECT_FALSE(table->get_tuple(*rid1, verify_tx.get()).has_value());
    EXPECT_FALSE(table->get_tuple(*rid2, verify_tx.get()).has_value());
    EXPECT_FALSE(table->get_tuple(*rid3, verify_tx.get()).has_value());

    db.commit_tx(verify_tx.get());
}

/*
 * MIXED OPERATIONS
 *
 * A committed tuple is modified and then the transaction is aborted.
 * Everything done by the transaction must disappear, while the
 * previously committed state must remain intact.
 */

TEST_F(TransactionRollbackTest, RollbackMixedOperations) {
    duck::Database db{kDBPath};

    auto table{db.create_table("users", make_schema())};

    // Initial committed state.
    auto setup_tx{db.begin_tx()};

    auto existing_rid{table->insert_tuple(make_tuple(make_schema(), 1, "Existing"), setup_tx.get())};

    ASSERT_TRUE(existing_rid.has_value());

    db.commit_tx(setup_tx.get());

    // Transaction that will be rolled back.
    auto tx{db.begin_tx()};

    // Insert.
    auto inserted_rid{table->insert_tuple(make_tuple(make_schema(), 2, "Inserted"), tx.get())};

    ASSERT_TRUE(inserted_rid.has_value());

    // Update existing tuple.
    ASSERT_TRUE(table->update_tuple(*existing_rid, make_tuple(make_schema(), 1, "Updated"), tx.get()));

    // Roll everything back.
    db.rollback_tx(tx.get());

    auto verify_tx{db.begin_tx()};

    // Insert must be gone.
    EXPECT_FALSE(table->get_tuple(*inserted_rid, verify_tx.get()).has_value());

    // Existing tuple must have its original value.
    auto restored{table->get_tuple(*existing_rid, verify_tx.get())};

    ASSERT_TRUE(restored.has_value());
    EXPECT_EQ(restored->get(0).as_uint32(), 1);
    EXPECT_EQ(restored->get(1).as_string(), "Existing");

    db.commit_tx(verify_tx.get());
}

/*
 * COMMIT
 *
 * Sanity check: committed changes must NOT be undone.
 */

TEST_F(TransactionRollbackTest, CommitPreservesInsert) {
    duck::Database db{kDBPath};

    auto table{db.create_table("users", make_schema())};

    auto tx{db.begin_tx()};

    auto rid{table->insert_tuple(make_tuple(make_schema(), 1, "Nikola"), tx.get())};

    ASSERT_TRUE(rid.has_value());

    db.commit_tx(tx.get());

    auto verify_tx{db.begin_tx()};

    auto tuple{table->get_tuple(*rid, verify_tx.get())};

    ASSERT_TRUE(tuple.has_value());
    EXPECT_EQ(tuple->get(0).as_uint32(), 1);
    EXPECT_EQ(tuple->get(1).as_string(), "Nikola");

    db.commit_tx(verify_tx.get());
}
/**
 * Copyright (c) 2026 Nikola Nedeljkovic
 * SPDX-License-Identifier: MIT
 */

#include "duck/buffer/pool_manager.hpp"
#include "duck/storage/disk_manager.hpp"
#include "duck/table/table.hpp"
#include "duck/transaction/lock_manager.hpp"
#include "duck/transaction/manager.hpp"
#include "duck/tuple/column.hpp"
#include "duck/tuple/schema.hpp"

#include <cstdio>
#include <gtest/gtest.h>
#include <string>

namespace {

duck::Schema MakeSimpleSchema() {
    return duck::Schema(std::vector<duck::Column>{
        {"id", duck::TypeId::UINT32},
    });
}

} // namespace

class TableDropPagesTest : public ::testing::Test {
protected:
    std::string test_file_ = "table_drop_test.db";

    void SetUp() override {
        std::remove(test_file_.c_str());
    }

    void TearDown() override {
        std::remove(test_file_.c_str());
    }
};

TEST_F(TableDropPagesTest, DropPagesReturnsZeroWhenNothingPinned) {
    duck::DiskManager dm{test_file_};
    duck::BufferPoolManager bpm{dm, 10};
    duck::LockManager lock_manager;
    duck::TransactionManager txn_manager{lock_manager};

    duck::Schema schema = MakeSimpleSchema();
    duck::Table table{"t", duck::TableHeap::create(bpm), schema, lock_manager};

    auto txn = txn_manager.begin();

    std::string big_value(500, 'x');
    for (int i = 0; i < 20; ++i) {
        duck::Tuple row({duck::Value::of(static_cast<std::uint32_t>(i))}, schema);
        table.insert_tuple(row, txn.get());
    }

    txn_manager.commit(txn.get()); // pusti lock-ove pre nego što probaš da obrišeš stranice

    auto [failed_count, status] = table.drop_pages();
    EXPECT_EQ(status, duck::DropTableStatus::SUCCESS);
    EXPECT_EQ(failed_count, 0u);
}

TEST_F(TableDropPagesTest, DropPagesCountsPinnedPagesAsFailed) {
    duck::DiskManager dm{test_file_};
    duck::BufferPoolManager bpm{dm, 10};
    duck::LockManager lock_manager;
    duck::TransactionManager txn_manager{lock_manager};

    duck::Schema schema = MakeSimpleSchema();
    duck::Table table{"t", duck::TableHeap::create(bpm), schema, lock_manager};

    auto txn = txn_manager.begin();
    duck::Tuple row({duck::Value::of(static_cast<std::uint32_t>(1))}, schema);
    table.insert_tuple(row, txn.get());
    txn_manager.commit(txn.get());

    // Pin the only page manually to simulate a concurrent reader still holding it.
    duck::Page* page = bpm.fetch_page(table.table_heap()->first_page_id());
    ASSERT_NE(page, nullptr);

    auto [failed_count, status] = table.drop_pages();
    EXPECT_EQ(failed_count, 1u);

    bpm.unpin_page(table.table_heap()->first_page_id(), false);
}

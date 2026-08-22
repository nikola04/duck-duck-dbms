/**
 * Copyright (c) 2026 Nikola Nedeljkovic
 * SPDX-License-Identifier: MIT
 */

#include "duck/buffer/pool_manager.hpp"
#include "duck/catalog/catalog.hpp"
#include "duck/storage/disk_manager.hpp"
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

class CatalogDropTableTest : public ::testing::Test {
protected:
    std::string test_file_ = "catalog_drop_test.db";

    void SetUp() override {
        std::remove(test_file_.c_str());
    }

    void TearDown() override {
        std::remove(test_file_.c_str());
    }
};

TEST_F(CatalogDropTableTest, DropTableDoesNotThrowWhenPagesAreFree) {
    duck::DiskManager dm{test_file_};
    duck::BufferPoolManager bpm{dm, 10};
    duck::Catalog catalog{bpm, dm};

    catalog.create_table("users", MakeSimpleSchema());
    EXPECT_TRUE(catalog.drop_table("users"));
    EXPECT_FALSE(catalog.get_table("users").has_value());
}

TEST_F(CatalogDropTableTest, DropTableStillRemovesEntryEvenIfPagePinned) {
    duck::DiskManager dm{test_file_};
    duck::BufferPoolManager bpm{dm, 10};
    duck::Catalog catalog{bpm, dm};

    duck::Table* table = catalog.create_table("users", MakeSimpleSchema());
    duck::PageID first_page = table->table_heap()->first_page_id();

    // Simulate a concurrent reader holding the page pinned.
    duck::Page* page = bpm.fetch_page(first_page);
    ASSERT_NE(page, nullptr);

    // drop_table should still succeed logically (catalog entry removed),
    // even though the underlying page cannot be reclaimed right now.
    EXPECT_TRUE(catalog.drop_table("users"));
    EXPECT_FALSE(catalog.get_table("users").has_value());

    bpm.unpin_page(first_page, false);
}

TEST_F(CatalogDropTableTest, PagesAreReclaimedAndReusableAfterDrop) {
    duck::DiskManager dm{test_file_};
    duck::BufferPoolManager bpm{dm, 10};
    duck::Catalog catalog{bpm, dm};

    catalog.create_table("users", MakeSimpleSchema());
    duck::PageID users_first_page = catalog.get_table("users").value()->table_heap()->first_page_id();

    catalog.drop_table("users");

    // A newly created table should be able to reuse the reclaimed page_id
    // (DiskManager's free-list hands it back on next allocation).
    duck::Table* recreated = catalog.create_table("cars", MakeSimpleSchema());
    EXPECT_EQ(recreated->table_heap()->first_page_id(), users_first_page);
}
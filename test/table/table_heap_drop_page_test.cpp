/**
 * Copyright (c) 2026 Nikola Nedeljkovic
 * SPDX-License-Identifier: MIT
 */

#include "duck/buffer/pool_manager.hpp"
#include "duck/storage/disk_manager.hpp"
#include "duck/table/table_heap.hpp"

#include <algorithm>
#include <cstdio>
#include <gtest/gtest.h>
#include <string>
#include <vector>

class TableHeapDropPagesTest : public ::testing::Test {
protected:
    std::string test_file_ = "table_heap_drop_test.db";

    void SetUp() override {
        std::remove(test_file_.c_str());
    }

    void TearDown() override {
        std::remove(test_file_.c_str());
    }
};

TEST_F(TableHeapDropPagesTest, AllPagesReturnsSinglePageForFreshHeap) {
    duck::DiskManager dm{test_file_};
    duck::BufferPoolManager bpm{dm, 5};
    duck::TableHeap heap = duck::TableHeap::create(bpm);

    auto [pages, status] = heap.all_pages();
    EXPECT_EQ(status, duck::TableHeapFetchStatus::SUCCESS);
    ASSERT_EQ(pages.size(), 1u);
    EXPECT_EQ(pages[0], heap.first_page_id());
}

TEST_F(TableHeapDropPagesTest, AllPagesReturnsEveryPageInChain) {
    duck::DiskManager dm{test_file_};
    duck::BufferPoolManager bpm{dm, 10};
    duck::TableHeap heap = duck::TableHeap::create(bpm);

    // Force allocation of additional pages by inserting large tuples.
    std::string big_value(500, 'x');
    for (int i = 0; i < 20; ++i) {
        ASSERT_TRUE(heap.insert_tuple(std::as_bytes(std::span(big_value))).has_value());
    }

    auto [pages, status] = heap.all_pages();
    EXPECT_EQ(status, duck::TableHeapFetchStatus::SUCCESS);
    EXPECT_GT(pages.size(), 1u);

    // No duplicate page ids in the chain
    std::vector<duck::PageID> sorted_pages = pages;
    std::sort(sorted_pages.begin(), sorted_pages.end());
    EXPECT_EQ(std::adjacent_find(sorted_pages.begin(), sorted_pages.end()), sorted_pages.end());
}

TEST_F(TableHeapDropPagesTest, DropPageSucceedsWhenUnpinned) {
    duck::DiskManager dm{test_file_};
    duck::BufferPoolManager bpm{dm, 5};
    duck::TableHeap heap = duck::TableHeap::create(bpm);

    EXPECT_TRUE(heap.drop_page(heap.first_page_id()));
}

TEST_F(TableHeapDropPagesTest, DropPageFailsWhenPagePinned) {
    duck::DiskManager dm{test_file_};
    duck::BufferPoolManager bpm{dm, 5};
    duck::TableHeap heap = duck::TableHeap::create(bpm);

    // Manually pin the page without unpinning, to simulate concurrent in-flight use.
    duck::Page* page = bpm.fetch_page(heap.first_page_id());
    ASSERT_NE(page, nullptr);

    EXPECT_FALSE(heap.drop_page(heap.first_page_id()));

    bpm.unpin_page(heap.first_page_id(), false);
}
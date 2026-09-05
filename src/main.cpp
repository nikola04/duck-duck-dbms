/**
 * Copyright (c) 2026 Nikola Nedeljkovic
 * SPDX-License-Identifier: MIT
 */

#include "duck/buffer/pool_manager.hpp"
#include "duck/catalog/catalog.hpp"
#include "duck/storage/disk_manager.hpp"
#include "duck/table/table.hpp"
#include "duck/transaction/lock_manager.hpp"
#include "duck/transaction/manager.hpp"
#include "duck/transaction/transaction.hpp"
#include "duck/tuple/schema.hpp"
#include "duck/tuple/tuple.hpp"
#include <cstddef>
#include <exception>
#include <iostream>
#include <print>
#include <string>

int main() {
    try {
        duck::DiskManager disk_manager{"test.db"};
        duck::BufferPoolManager pool{disk_manager, 5};
        duck::LockManager lock_manager;
        duck::TransactionManager tx_manager{lock_manager};

        duck::Catalog catalog{pool, disk_manager, lock_manager};

        for (auto table : catalog.all_tables()) {
            std::println("{}\n{}\n", table->name(), table->schema().to_string());
        }

        auto _table{catalog.get_table("heap_test3")};
        if (!_table.has_value())
            std::println("Table not found!");
        duck::Table* table = _table.value();

        std::vector<duck::Column> columns{duck::Column{"id", duck::TypeId::UINT32},
                                          duck::Column{"username", duck::TypeId::VARCHAR, 3000}};
        duck::Schema schema{columns};
        std::vector<duck::Value> values{duck::Value::of((uint32_t)429967296), duck::Value::of(std::string("nikola"))};
        auto new_tuple{duck::Tuple{values, schema}};

        duck::Transaction* tx{tx_manager.begin()};
        table->insert_tuple(new_tuple, tx);
        tx_manager.commit(tx);

        duck::Table::Scan scan = table->scan();
        while (auto entry = scan.next()) {
            std::println("RID: {}/{}, {}", entry->first.page_id, entry->first.slot_num, entry->second.to_string());
        }

    } catch (std::exception& e) {
        std::cout << "Exception: " << e.what() << "\n";
    }
}
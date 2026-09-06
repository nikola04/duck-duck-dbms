/**
 * Copyright (c) 2026 Nikola Nedeljkovic
 * SPDX-License-Identifier: MIT
 */

#include "duck/database/database.hpp"
#include "duck/execution/executor.hpp"
#include "duck/execution/executor_context.hpp"
#include "duck/execution/operator/scan/seq_scan.hpp"
#include "duck/table/table.hpp"
#include "duck/tuple/schema.hpp"
#include "duck/tuple/tuple.hpp"
#include <cstddef>
#include <exception>
#include <iostream>
#include <memory>
#include <print>
#include <string>

int main() {
    try {
        duck::Database db{"test.db"};

        // auto _table{db.get_table("heap_test3")};
        // if (!_table.has_value())
        //     std::println("Table not found!");
        // duck::Table* table = _table.value();

        std::vector<duck::Column> columns{duck::Column{"id", duck::TypeId::UINT32},
                                          duck::Column{"username", duck::TypeId::VARCHAR, 3000}};
        duck::Schema schema{columns};
        std::vector<duck::Value> values{duck::Value::of((uint32_t)233), duck::Value::of(std::string("Sava"))};
        auto new_tuple{duck::Tuple{values, schema}};

        auto table{*db.get_table("test_table3")};
        auto tx{db.begin_tx()};

        duck::ExecutorContext context{tx.get()};
        // table->insert_tuple(new_tuple, tx.get());

        auto scan_op{std::make_unique<duck::SequentialScanOperator>(context, table)};
        duck::Executor executor{std::move(scan_op)};

        auto result{executor.execute()};
        while (auto entry = result.next()) {
            std::println("Entry: {} | {}", entry->get(0).to_string(), entry->get(1).to_string());
        }

        // table->update_tuple({2, 1}, new_tuple, tx.get());
        // table->delete_tuple({2, 1}, tx.get());
        // table->delete_tuple({2, 2}, tx.get());
        db.rollback_tx(tx.get());

        // for (auto table : db.all_tables()) {
        //     std::println("{}\n{}\n", table->name(), table->schema().to_string());
        // }

        // duck::Table::Scan scan = table->scan(tx.get());
        // while (auto entry = scan.next()) {
        //     std::println("Entry: {}", entry->second.get(1).to_string());
        // }

    } catch (std::exception& e) {
        std::cout << "Exception: " << e.what() << "\n";
    }
}
/**
 * Copyright (c) 2026 Nikola Nedeljkovic
 * SPDX-License-Identifier: MIT
 */

#include "duck/database/database.hpp"
#include "duck/expression/comparison.hpp"
#include "duck/expression/logical.hpp"
#include "duck/query/query_engine.hpp"
#include "duck/query/select_query.hpp"
#include "duck/tuple/schema.hpp"
#include "duck/tuple/value.hpp"
#include <iostream>
#include <print>

int main() {
    try {
        auto comp_g_exp{std::make_unique<duck::UnboundComparisonExpression>(
            std::make_unique<duck::UnboundConstantExpression>(duck::Value::of(std::string("test string!"))),
            duck::ComparisonOperator::GREATER_EQ,
            std::make_unique<duck::UnboundConstantExpression>(duck::Value::of(std::string("test string"))))};
        auto comp_l_exp{std::make_unique<duck::UnboundComparisonExpression>(
            std::make_unique<duck::UnboundColumnExpression>(duck::ColumnRef{"id"}), duck::ComparisonOperator::LESS,
            std::make_unique<duck::UnboundConstantExpression>(duck::Value::of(233.000000001)))};

        auto b_expr{std::make_unique<duck::UnboundBinaryExpression>(std::move(comp_g_exp), duck::BinaryOperator::AND,
                                                                    std::move(comp_l_exp))};

        auto query{duck::SelectQuery{}};
        query.columns({"username", "id"}).from("test_table3").where(std::move(b_expr)).limit(1);

        duck::Database db{"test.db"};

        auto tx{db.begin_tx()};

        duck::QueryContext context{tx.get()};
        duck::QueryEngine engine{db.catalog()};

        auto result{engine.execute(query, context)};

        std::println("Schema: {}", result.output_schema().to_string());
        while (auto entry = result.next()) {
            std::println("Entry: {} | {}", entry->get(0).to_string(), entry->get(1).to_string());
        }

    } catch (std::exception& e) {
        std::cout << "Exception: " << e.what() << "\n";
    }
}

/**
 * Copyright (c) 2026 Nikola Nedeljkovic
 * SPDX-License-Identifier: MIT
 */

#include "duck/database/database.hpp"
#include "duck/expression/comparison.hpp"
#include "duck/expression/logical.hpp"
#include "duck/query/expression.hpp"
#include "duck/query/join.hpp"
#include "duck/query/query_engine.hpp"
#include "duck/query/refs.hpp"
#include "duck/query/select_query.hpp"
#include "duck/tuple/column.hpp"
#include "duck/tuple/schema.hpp"
#include "duck/tuple/tuple.hpp"
#include "duck/tuple/value.hpp"
#include <cstdint>
#include <iostream>
#include <memory>
#include <print>
#include <string>

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

        duck::Database db{"test.db"};

        auto tx{db.begin_tx()};

        auto user_schema =
            duck::Schema{{duck::Column{"id", duck::TypeId::UINT64}, duck::Column{"username", duck::TypeId::VARCHAR},
                          duck::Column{"password", duck::TypeId::VARCHAR, 64}}};
        auto payments_schema =
            duck::Schema{{duck::Column{"id", duck::TypeId::UINT64}, duck::Column{"user_id", duck::TypeId::UINT64},
                          duck::Column{"amount", duck::TypeId::INT64}}};

        // auto users = db.get_table("users");
        // auto payments = db.get_table("payments");

        duck::Tuple user1 = duck::Tuple{{duck::Value::of((uint64_t)1), duck::Value::of(std::string{"nikola"}),
                                         duck::Value::of(std::string{"12345678!"})},
                                        user_schema};
        duck::Tuple user2 = duck::Tuple{{duck::Value::of((uint64_t)2), duck::Value::of(std::string{"sava"}),
                                         duck::Value::of(std::string{"321123"})},
                                        user_schema};
        duck::Tuple user3 = duck::Tuple{{duck::Value::of((uint64_t)3), duck::Value::of(std::string{"anastasija"}),
                                         duck::Value::of(std::string{"malakuca"})},
                                        user_schema};
        duck::Tuple pymnt1 =
            duck::Tuple{{duck::Value::of((uint64_t)1), duck::Value::of((uint64_t)2), duck::Value::of((int64_t)-5212)},
                        payments_schema};
        duck::Tuple pymnt2 =
            duck::Tuple{{duck::Value::of((uint64_t)2), duck::Value::of((uint64_t)2), duck::Value::of((int64_t)948)},
                        payments_schema};
        duck::Tuple pymnt3 =
            duck::Tuple{{duck::Value::of((uint64_t)3), duck::Value::of((uint64_t)1), duck::Value::of((int64_t)-465)},
                        payments_schema};
        duck::Tuple pymnt4 =
            duck::Tuple{{duck::Value::of((uint64_t)4), duck::Value::of((uint64_t)3), duck::Value::of((int64_t)312)},
                        payments_schema};

        // (*users)->insert_tuple(user1, tx.get());
        // (*users)->insert_tuple(user2, tx.get());
        // (*users)->insert_tuple(user3, tx.get());

        // (*payments)->insert_tuple(pymnt1, tx.get());
        // (*payments)->insert_tuple(pymnt2, tx.get());
        // (*payments)->insert_tuple(pymnt3, tx.get());
        // (*payments)->insert_tuple(pymnt4, tx.get());

        duck::QueryContext context{tx.get()};
        duck::QueryEngine engine{db.catalog()};

        auto query{duck::SelectQuery{}};
        // auto on{std::make_unique<duck::UnboundComparisonExpression>(
        //     std::make_unique<duck::UnboundConstantExpression>(duck::Value::of(1)), duck::ComparisonOperator::EQUAL,
        //     std::make_unique<duck::UnboundConstantExpression>(duck::Value::of(1)))};
        auto on{std::make_unique<duck::UnboundComparisonExpression>(
            std::make_unique<duck::UnboundColumnExpression>(duck::ColumnRef{{"u"}, {"id"}}),
            duck::ComparisonOperator::EQUAL,
            std::make_unique<duck::UnboundColumnExpression>(duck::ColumnRef{{"p"}, {"user_id"}}))};
        auto where{std::make_unique<duck::UnboundComparisonExpression>(
            std::make_unique<duck::UnboundColumnExpression>(duck::ColumnRef{{"p"}, {"amount"}}),
            duck::ComparisonOperator::GREATER,
            std::make_unique<duck::UnboundConstantExpression>(duck::Value::of(-3000)))};

        query.columns({{"*"}})
            .from({"users", "u"})
            .join({duck::TableRef{"payments", "p"}, std::move(on), duck::JoinType::INNER})
            .where(std::move(where))
            .limit(3);

        auto result{engine.execute(query, context)};

        std::println("Schema: {}", result.output_schema().to_string());
        while (auto entry = result.next()) {
            std::println("Entry: {}", entry->to_string());
        }

    } catch (std::exception& e) {
        std::cout << "Exception: " << e.what() << "\n";
    }
}

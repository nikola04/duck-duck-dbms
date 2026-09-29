/**
 * Copyright (c) 2026 Nikola Nedeljkovic
 * SPDX-License-Identifier: MIT
 */

#include "duck/database/database.hpp"
#include "duck/execution/executor_context.hpp"
#include "duck/query/expression.hpp"
#include "duck/query/query_engine.hpp"
#include "duck/query/select_query.hpp"
#include "duck/tuple/column.hpp"
#include "duck/tuple/schema.hpp"
#include "duck/tuple/tuple.hpp"
#include "duck/tuple/value.hpp"

#include <cstdio>
#include <gtest/gtest.h>
#include <memory>

namespace {

duck::Schema MakeSchema() {
    return duck::Schema(std::vector<duck::Column>{
        {"id", duck::TypeId::INT32},
        {"name", duck::TypeId::VARCHAR, 50},
    });
}

} // namespace

TEST(BinderTest, BindsColumnNamesInPredicate) {
    const std::string test_file = "binder_test.db";
    std::remove(test_file.c_str());

    {
        duck::Database db{test_file};
        auto write_tx = db.begin_tx();
        auto schema = MakeSchema();
        auto* table = db.create_table("people", schema, write_tx.get());

        for (int i = 0; i < 4; ++i) {
            table->insert_tuple(
                duck::Tuple({duck::Value::of(static_cast<std::int32_t>(i)),
                             duck::Value::of(std::string("person") + std::to_string(i))},
                            schema),
                write_tx.get());
        }
        db.commit_tx(write_tx.get());

        auto read_tx = db.begin_tx();
        duck::ExecutorContext context{read_tx.get()};

        duck::SelectQuery query{{"name"}};
        query.from("people").where(std::make_unique<duck::UnboundComparisonExpression>(
            std::make_unique<duck::UnboundColumnExpression>("id"), duck::ComparisonOperator::GREATER_EQ,
            std::make_unique<duck::UnboundConstantExpression>(duck::Value::of(std::int32_t{2}))));

        duck::QueryEngine engine{db.catalog()};
        auto result = engine.execute(query, context);

        std::vector<std::string> names;
        while (auto record = result.next())
            names.push_back(record->get(0).as_string());

        ASSERT_EQ(names, (std::vector<std::string>{"person2", "person3"}));
        db.commit_tx(read_tx.get());
    }

    std::remove(test_file.c_str());
}

/**
 * Copyright (c) 2026 Nikola Nedeljkovic
 * SPDX-License-Identifier: MIT
 */

#include "duck/database/database.hpp"
#include "duck/execution/executor.hpp"
#include "duck/execution/executor_context.hpp"
#include "duck/expression/column.hpp"
#include "duck/expression/comparison.hpp"
#include "duck/expression/constant.hpp"
#include "duck/plan/logical/filter.hpp"
#include "duck/plan/logical/projection.hpp"
#include "duck/plan/logical/scan.hpp"
#include "duck/plan/optimizer/optimizer.hpp"
#include "duck/tuple/column.hpp"
#include "duck/tuple/schema.hpp"

#include <cstdio>
#include <memory>
#include <gtest/gtest.h>

namespace {

duck::Schema MakeSchema() {
    return duck::Schema(std::vector<duck::Column>{
        {"id", duck::TypeId::INT32},
        {"name", duck::TypeId::VARCHAR, 50},
    });
}

} // namespace

class ExecutorIntegrationTest : public ::testing::Test {
protected:
    std::string test_file_ = "executor_integration_test.db";

    void SetUp() override {
        std::remove(test_file_.c_str());
    }

    void TearDown() override {
        std::remove(test_file_.c_str());
    }
};

TEST_F(ExecutorIntegrationTest, ScanFilterProjectionReturnsExpectedRows) {
    duck::Database db{test_file_};

    auto txn = db.begin_tx();
    duck::Schema schema = MakeSchema();
    duck::Table* table = db.create_table("people", schema, txn.get());

    for (int i = 0; i < 5; ++i) {
        duck::Tuple row(
            {duck::Value::of(static_cast<std::int32_t>(i)), duck::Value::of(std::string("person") + std::to_string(i))},
            schema);
        table->insert_tuple(row, txn.get());
    }
    db.commit_tx(txn.get());

    auto scan_txn = db.begin_tx();
    duck::ExecutorContext context{scan_txn.get()};

    // WHERE id >= 2
    auto filter_expr = std::make_unique<duck::ComparisonExpression>(
        std::make_unique<duck::ColumnExpression>(0), duck::ComparisonOperator::GREATER_EQ,
        std::make_unique<duck::ConstantExpression>(duck::Value::of(std::int32_t{2})));

    auto scan = std::make_unique<duck::LogicalScan>(*table);
    auto filter = std::make_unique<duck::LogicalFilterPlan>(std::move(scan), std::move(filter_expr));
    // Projection is above the filter, so the predicate still sees the original scan schema.
    auto projection = std::make_unique<duck::LogicalProjection>(std::move(filter), std::vector<std::size_t>{1});

    duck::Optimizer optimizer{db.catalog()};
    auto physical_plan = optimizer.optimize(std::move(projection));

    duck::Executor executor{context};
    auto result = executor.execute(*physical_plan);

    ASSERT_EQ(result.output_schema().column_count(), 1u);
    int count = 0;
    while (auto record = result.next()) {
        ASSERT_EQ(record->size(), 1u);
        EXPECT_EQ(record->get(0).as_string(), "person" + std::to_string(count + 2));
        ++count;
    }
    EXPECT_EQ(count, 3); // ids 2, 3, 4

    db.commit_tx(scan_txn.get());
}

TEST_F(ExecutorIntegrationTest, ProjectionReordersAndSubsetsColumns) {
    duck::Database db{test_file_};

    auto txn = db.begin_tx();
    duck::Schema schema = MakeSchema();
    duck::Table* table = db.create_table("people", schema, txn.get());

    duck::Tuple row({duck::Value::of(std::int32_t{7}), duck::Value::of(std::string("alice"))}, schema);
    table->insert_tuple(row, txn.get());
    db.commit_tx(txn.get());

    auto scan_txn = db.begin_tx();
    duck::ExecutorContext context{scan_txn.get()};

    auto scan = std::make_unique<duck::LogicalScan>(*table);
    // Project only column 1 (name), dropping column 0.
    auto projection = std::make_unique<duck::LogicalProjection>(std::move(scan), std::vector<std::size_t>{1});

    duck::Optimizer optimizer{db.catalog()};
    auto physical_plan = optimizer.optimize(std::move(projection));

    duck::Executor executor{context};
    auto result = executor.execute(*physical_plan);

    ASSERT_EQ(result.output_schema().column_count(), 1u);

    auto record = result.next();
    ASSERT_TRUE(record.has_value());
    EXPECT_EQ(record->size(), 1u);
    EXPECT_EQ(record->get(0).as_string(), "alice");

    db.commit_tx(scan_txn.get());
}

TEST_F(ExecutorIntegrationTest, FilterWithNoMatchesReturnsEmptyResult) {
    duck::Database db{test_file_};

    auto txn = db.begin_tx();
    duck::Schema schema = MakeSchema();
    duck::Table* table = db.create_table("people", schema, txn.get());

    duck::Tuple row({duck::Value::of(std::int32_t{1}), duck::Value::of(std::string("bob"))}, schema);
    table->insert_tuple(row, txn.get());
    db.commit_tx(txn.get());

    auto scan_txn = db.begin_tx();
    duck::ExecutorContext context{scan_txn.get()};

    auto filter_expr = std::make_unique<duck::ComparisonExpression>(
        std::make_unique<duck::ColumnExpression>(0), duck::ComparisonOperator::GREATER,
        std::make_unique<duck::ConstantExpression>(duck::Value::of(std::int32_t{999})));

    auto scan = std::make_unique<duck::LogicalScan>(*table);
    auto filter = std::make_unique<duck::LogicalFilterPlan>(std::move(scan), std::move(filter_expr));

    duck::Optimizer optimizer{db.catalog()};
    auto physical_plan = optimizer.optimize(std::move(filter));

    duck::Executor executor{context};
    auto result = executor.execute(*physical_plan);

    EXPECT_FALSE(result.next().has_value());

    db.commit_tx(scan_txn.get());
}

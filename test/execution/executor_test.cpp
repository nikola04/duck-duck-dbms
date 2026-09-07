/**
 * Copyright (c) 2026 Nikola Nedeljkovic
 * SPDX-License-Identifier: MIT
 */

#include "duck/database/database.hpp"
#include "duck/execution/executor.hpp"
#include "duck/execution/executor_context.hpp"
#include "duck/execution/expression/column.hpp"
#include "duck/execution/expression/comparison.hpp"
#include "duck/execution/expression/constant.hpp"
#include "duck/execution/operator/filter/filter.hpp"
#include "duck/execution/operator/projection/projection.hpp"
#include "duck/execution/operator/scan/seq_scan.hpp"
#include "duck/tuple/column.hpp"
#include "duck/tuple/schema.hpp"

#include <cstdio>
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

    auto scan_op = std::make_unique<duck::SequentialScanOperator>(table, context.tx);
    auto filter_op = std::make_unique<duck::FilterOperator>(std::move(scan_op), std::move(filter_expr));
    auto proj_op = std::make_unique<duck::ProjectionOperator>(std::move(filter_op), std::vector<std::size_t>{0, 1});

    duck::Executor executor{std::move(proj_op), context};
    auto result = executor.execute();

    int count = 0;
    while (auto record = result.next()) {
        EXPECT_GE(record->get(0).as_int32(), 2);
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

    auto scan_op = std::make_unique<duck::SequentialScanOperator>(table, context.tx);
    // Project only column 1 (name), dropping column 0
    auto proj_op = std::make_unique<duck::ProjectionOperator>(std::move(scan_op), std::vector<std::size_t>{1});

    duck::Executor executor{std::move(proj_op), context};
    auto result = executor.execute();

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

    auto scan_op = std::make_unique<duck::SequentialScanOperator>(table, context.tx);
    auto filter_op = std::make_unique<duck::FilterOperator>(std::move(scan_op), std::move(filter_expr));

    duck::Executor executor{std::move(filter_op), context};
    auto result = executor.execute();

    EXPECT_FALSE(result.next().has_value());

    db.commit_tx(scan_txn.get());
}
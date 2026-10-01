/**
 * Copyright (c) 2026 Nikola Nedeljkovic
 * SPDX-License-Identifier: MIT
 */

#include "duck/database/database.hpp"
#include "duck/expression/column.hpp"
#include "duck/expression/comparison.hpp"
#include "duck/expression/constant.hpp"
#include "duck/plan/logical/filter.hpp"
#include "duck/plan/logical/projection.hpp"
#include "duck/plan/logical/scan.hpp"
#include "duck/plan/optimizer/optimizer.hpp"
#include "duck/plan/physical/filter.hpp"
#include "duck/plan/physical/projection.hpp"
#include "duck/plan/physical/scan/seq_scan.hpp"
#include "duck/tuple/column.hpp"
#include "duck/tuple/schema.hpp"

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

class OptimizerTest : public ::testing::Test {
protected:
    std::string test_file_ = "optimizer_test.db";

    void SetUp() override {
        std::remove(test_file_.c_str());
    }

    void TearDown() override {
        std::remove(test_file_.c_str());
    }

    duck::Table* CreateTable(duck::Database& db, std::shared_ptr<duck::Transaction>& tx) {
        tx = db.begin_tx();
        auto* table = db.create_table("people", MakeSchema(), tx.get());
        db.commit_tx(tx.get());
        return table;
    }
};

TEST_F(OptimizerTest, OptimizesScanToSequentialScanPlan) {
    duck::Database db{test_file_};
    std::shared_ptr<duck::Transaction> tx;
    auto* table = CreateTable(db, tx);

    duck::Optimizer optimizer{db.catalog()};
    auto physical = optimizer.optimize(std::make_unique<duck::LogicalScan>(*table));

    auto* scan = dynamic_cast<duck::SequentialScanPlan*>(physical.get());
    ASSERT_NE(scan, nullptr);
    EXPECT_TRUE(scan->output_schema().compatible_with(table->schema()));
    EXPECT_EQ(scan->table().name(), "people");
}

TEST_F(OptimizerTest, OptimizesFilterAndPreservesChildAndSchema) {
    duck::Database db{test_file_};
    std::shared_ptr<duck::Transaction> tx;
    auto* table = CreateTable(db, tx);

    auto predicate = std::make_unique<duck::ComparisonExpression>(
        std::make_unique<duck::ColumnExpression>(0), duck::ComparisonOperator::GREATER,
        std::make_unique<duck::ConstantExpression>(duck::Value::of(std::int32_t{10})));
    auto scan = std::make_unique<duck::LogicalScan>(*table);
    auto logical = std::make_unique<duck::LogicalFilterPlan>(std::move(scan), std::move(predicate));

    duck::Optimizer optimizer{db.catalog()};
    auto physical = optimizer.optimize(std::move(logical));

    auto* filter = dynamic_cast<duck::PhysicalFilterPlan*>(physical.get());
    ASSERT_NE(filter, nullptr);
    EXPECT_TRUE(filter->output_schema().compatible_with(table->schema()));
    EXPECT_NE(dynamic_cast<const duck::SequentialScanPlan*>(&filter->child()), nullptr);
    EXPECT_NE(dynamic_cast<const duck::ComparisonExpression*>(&filter->predicate()), nullptr);
}

TEST_F(OptimizerTest, OptimizesProjectionWithProjectedSchema) {
    duck::Database db{test_file_};
    std::shared_ptr<duck::Transaction> tx;
    auto* table = CreateTable(db, tx);

    auto scan = std::make_unique<duck::LogicalScan>(*table);
    auto logical = std::make_unique<duck::LogicalProjection>(std::move(scan), std::vector<std::size_t>{1, 0});

    duck::Optimizer optimizer{db.catalog()};
    auto physical = optimizer.optimize(std::move(logical));

    auto* projection = dynamic_cast<duck::PhysicalProjectionPlan*>(physical.get());
    ASSERT_NE(projection, nullptr);
    ASSERT_EQ(projection->output_schema().column_count(), 2u);
    EXPECT_EQ(projection->output_schema().column(0).name(), "name");
    EXPECT_EQ(projection->output_schema().column(1).name(), "id");
}

TEST_F(OptimizerTest, OptimizesProjectionAboveFilter) {
    duck::Database db{test_file_};
    std::shared_ptr<duck::Transaction> tx;
    auto* table = CreateTable(db, tx);

    auto predicate = std::make_unique<duck::ComparisonExpression>(
        std::make_unique<duck::ColumnExpression>(0), duck::ComparisonOperator::GREATER_EQ,
        std::make_unique<duck::ConstantExpression>(duck::Value::of(std::int32_t{2})));
    auto scan = std::make_unique<duck::LogicalScan>(*table);
    auto filter = std::make_unique<duck::LogicalFilterPlan>(std::move(scan), std::move(predicate));
    auto logical = std::make_unique<duck::LogicalProjection>(std::move(filter), std::vector<std::size_t>{1});

    duck::Optimizer optimizer{db.catalog()};
    auto physical = optimizer.optimize(std::move(logical));

    auto* projection = dynamic_cast<duck::PhysicalProjectionPlan*>(physical.get());
    ASSERT_NE(projection, nullptr);
    ASSERT_EQ(projection->output_schema().column_count(), 1u);
    EXPECT_EQ(projection->output_schema().column(0).name(), "name");
}

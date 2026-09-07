/**
 * Copyright (c) 2026 Nikola Nedeljkovic
 * SPDX-License-Identifier: MIT
 */

#include "duck/execution/expression/column.hpp"
#include "duck/execution/expression/comparison.hpp"
#include "duck/execution/expression/constant.hpp"
#include "duck/execution/expression/logical.hpp"
#include "duck/execution/record.hpp"

#include <gtest/gtest.h>

namespace {

duck::Record MakeRecord(std::vector<duck::Value> values) {
    return duck::Record{std::move(values)};
}

} // namespace

// ---------- ConstantExpression ----------

TEST(ExpressionTest, ConstantAlwaysReturnsSameValue) {
    duck::ConstantExpression expr{duck::Value::of(std::int32_t{42})};
    duck::Record record = MakeRecord({});

    EXPECT_EQ(expr.evaluate(record).as_int32(), 42);
}

// ---------- ColumnExpression ----------

TEST(ExpressionTest, ColumnReturnsValueAtIndex) {
    duck::ColumnExpression expr{1};
    duck::Record record = MakeRecord({duck::Value::of(std::int32_t{1}), duck::Value::of(std::string("hello"))});

    EXPECT_EQ(expr.evaluate(record).as_string(), "hello");
}

// ---------- ComparisonExpression: numeric ----------

TEST(ComparisonExpressionTest, IntegerEqual) {
    duck::ComparisonExpression expr{
        std::make_unique<duck::ConstantExpression>(duck::Value::of(std::int32_t{5})),
        duck::ComparisonOperator::EQUAL,
        std::make_unique<duck::ConstantExpression>(duck::Value::of(std::int32_t{5})),
    };
    duck::Record record = MakeRecord({});

    EXPECT_TRUE(expr.evaluate(record).as_bool());
}

TEST(ComparisonExpressionTest, IntegerLessThan) {
    duck::ComparisonExpression expr{
        std::make_unique<duck::ConstantExpression>(duck::Value::of(std::int32_t{3})),
        duck::ComparisonOperator::LESS,
        std::make_unique<duck::ConstantExpression>(duck::Value::of(std::int32_t{5})),
    };
    duck::Record record = MakeRecord({});

    EXPECT_TRUE(expr.evaluate(record).as_bool());
}

TEST(ComparisonExpressionTest, MixedIntFloatComparison) {
    // int32 vs double — exercises the numeric/floating coercion path
    duck::ComparisonExpression expr{
        std::make_unique<duck::ConstantExpression>(duck::Value::of(std::int32_t{2})),
        duck::ComparisonOperator::LESS,
        std::make_unique<duck::ConstantExpression>(duck::Value::of(2.5)),
    };
    duck::Record record = MakeRecord({});

    EXPECT_TRUE(expr.evaluate(record).as_bool());
}

TEST(ComparisonExpressionTest, Uint64VsInt64Comparison) {
    duck::ComparisonExpression expr{
        std::make_unique<duck::ConstantExpression>(duck::Value::of(std::uint64_t{10})),
        duck::ComparisonOperator::GREATER,
        std::make_unique<duck::ConstantExpression>(duck::Value::of(std::int64_t{5})),
    };
    duck::Record record = MakeRecord({});

    EXPECT_TRUE(expr.evaluate(record).as_bool());
}

TEST(ComparisonExpressionTest, Uint64ExceedingInt64MaxIsGreater) {
    // Exercises the overflow-guard branch: uint64 > INT64_MAX must compare as GREATER
    // without undefined behavior from the int64 cast.
    std::uint64_t huge = static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max()) + 100;
    duck::ComparisonExpression expr{
        std::make_unique<duck::ConstantExpression>(duck::Value::of(huge)),
        duck::ComparisonOperator::GREATER,
        std::make_unique<duck::ConstantExpression>(duck::Value::of(std::int64_t{1})),
    };
    duck::Record record = MakeRecord({});

    EXPECT_TRUE(expr.evaluate(record).as_bool());
}

TEST(ComparisonExpressionTest, StringComparison) {
    duck::ComparisonExpression expr{
        std::make_unique<duck::ConstantExpression>(duck::Value::of(std::string("apple"))),
        duck::ComparisonOperator::LESS,
        std::make_unique<duck::ConstantExpression>(duck::Value::of(std::string("banana"))),
    };
    duck::Record record = MakeRecord({});

    EXPECT_TRUE(expr.evaluate(record).as_bool());
}

TEST(ComparisonExpressionTest, BoolComparison) {
    duck::ComparisonExpression expr{
        std::make_unique<duck::ConstantExpression>(duck::Value::of(false)),
        duck::ComparisonOperator::LESS,
        std::make_unique<duck::ConstantExpression>(duck::Value::of(true)),
    };
    duck::Record record = MakeRecord({});

    EXPECT_TRUE(expr.evaluate(record).as_bool());
}

// ---------- ComparisonExpression: NULL propagation ----------

TEST(ComparisonExpressionTest, NullLeftOperandReturnsNull) {
    duck::ComparisonExpression expr{
        std::make_unique<duck::ConstantExpression>(duck::Value::null(duck::ValueType::INT32)),
        duck::ComparisonOperator::EQUAL,
        std::make_unique<duck::ConstantExpression>(duck::Value::of(std::int32_t{5})),
    };
    duck::Record record = MakeRecord({});

    EXPECT_TRUE(expr.evaluate(record).is_null());
}

TEST(ComparisonExpressionTest, NullRightOperandReturnsNull) {
    duck::ComparisonExpression expr{
        std::make_unique<duck::ConstantExpression>(duck::Value::of(std::int32_t{5})),
        duck::ComparisonOperator::EQUAL,
        std::make_unique<duck::ConstantExpression>(duck::Value::null(duck::ValueType::INT32)),
    };
    duck::Record record = MakeRecord({});

    EXPECT_TRUE(expr.evaluate(record).is_null());
}

TEST(ComparisonExpressionTest, IncompatibleTypesReturnUnknownAsNull) {
    // e.g. comparing a string to a bool -- no defined ordering, should be NULL (UNKNOWN)
    duck::ComparisonExpression expr{
        std::make_unique<duck::ConstantExpression>(duck::Value::of(std::string("x"))),
        duck::ComparisonOperator::EQUAL,
        std::make_unique<duck::ConstantExpression>(duck::Value::of(true)),
    };
    duck::Record record = MakeRecord({});

    EXPECT_TRUE(expr.evaluate(record).is_null());
}

// ---------- BinaryExpression: AND (three-valued logic) ----------

TEST(BinaryExpressionTest, AndTrueTrue) {
    duck::BinaryExpression expr{
        std::make_unique<duck::ConstantExpression>(duck::Value::of(true)),
        duck::BinaryOperator::AND,
        std::make_unique<duck::ConstantExpression>(duck::Value::of(true)),
    };
    EXPECT_TRUE(expr.evaluate(MakeRecord({})).as_bool());
}

TEST(BinaryExpressionTest, AndFalseAnythingIsFalse) {
    // FALSE AND NULL must be FALSE (short-circuit on known-false), not NULL
    duck::BinaryExpression expr{
        std::make_unique<duck::ConstantExpression>(duck::Value::of(false)),
        duck::BinaryOperator::AND,
        std::make_unique<duck::ConstantExpression>(duck::Value::null(duck::ValueType::BOOL)),
    };
    auto result = expr.evaluate(MakeRecord({}));
    ASSERT_FALSE(result.is_null());
    EXPECT_FALSE(result.as_bool());
}

TEST(BinaryExpressionTest, AndTrueNullIsNull) {
    duck::BinaryExpression expr{
        std::make_unique<duck::ConstantExpression>(duck::Value::of(true)),
        duck::BinaryOperator::AND,
        std::make_unique<duck::ConstantExpression>(duck::Value::null(duck::ValueType::BOOL)),
    };
    EXPECT_TRUE(expr.evaluate(MakeRecord({})).is_null());
}

// ---------- BinaryExpression: OR (three-valued logic) ----------

TEST(BinaryExpressionTest, OrTrueAnythingIsTrue) {
    duck::BinaryExpression expr{
        std::make_unique<duck::ConstantExpression>(duck::Value::of(true)),
        duck::BinaryOperator::OR,
        std::make_unique<duck::ConstantExpression>(duck::Value::null(duck::ValueType::BOOL)),
    };
    auto result = expr.evaluate(MakeRecord({}));
    ASSERT_FALSE(result.is_null());
    EXPECT_TRUE(result.as_bool());
}

TEST(BinaryExpressionTest, OrFalseFalseIsFalse) {
    duck::BinaryExpression expr{
        std::make_unique<duck::ConstantExpression>(duck::Value::of(false)),
        duck::BinaryOperator::OR,
        std::make_unique<duck::ConstantExpression>(duck::Value::of(false)),
    };
    auto result = expr.evaluate(MakeRecord({}));
    ASSERT_FALSE(result.is_null());
    EXPECT_FALSE(result.as_bool());
}

TEST(BinaryExpressionTest, OrFalseNullIsNull) {
    duck::BinaryExpression expr{
        std::make_unique<duck::ConstantExpression>(duck::Value::of(false)),
        duck::BinaryOperator::OR,
        std::make_unique<duck::ConstantExpression>(duck::Value::null(duck::ValueType::BOOL)),
    };
    EXPECT_TRUE(expr.evaluate(MakeRecord({})).is_null());
}

// ---------- UnaryExpression: NOT ----------

TEST(UnaryExpressionTest, NotTrueIsFalse) {
    duck::UnaryExpression expr{std::make_unique<duck::ConstantExpression>(duck::Value::of(true)),
                               duck::UnaryOperator::NOT};
    auto result = expr.evaluate(MakeRecord({}));
    ASSERT_FALSE(result.is_null());
    EXPECT_FALSE(result.as_bool());
}

TEST(UnaryExpressionTest, NotNullIsNull) {
    duck::UnaryExpression expr{std::make_unique<duck::ConstantExpression>(duck::Value::null(duck::ValueType::BOOL)),
                               duck::UnaryOperator::NOT};
    EXPECT_TRUE(expr.evaluate(MakeRecord({})).is_null());
}

// ---------- ColumnExpression inside comparison, using a real record ----------

TEST(ComparisonExpressionTest, ColumnAgainstConstant) {
    duck::ComparisonExpression expr{
        std::make_unique<duck::ColumnExpression>(0),
        duck::ComparisonOperator::GREATER_EQ,
        std::make_unique<duck::ConstantExpression>(duck::Value::of(std::int32_t{10})),
    };
    duck::Record record = MakeRecord({duck::Value::of(std::int32_t{15})});

    EXPECT_TRUE(expr.evaluate(record).as_bool());
}
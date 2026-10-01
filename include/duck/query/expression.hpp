#pragma once

#include "duck/expression/comparison.hpp"
#include "duck/expression/logical.hpp"
#include "duck/query/refs.hpp"
#include "duck/tuple/value.hpp"
#include <memory>
#include <utility>

namespace duck {

enum class UnboundExpressionType { COLUMN, CONSTANT, COMPARISON, BINARY, UNARY };

class UnboundExpression {
public:
    virtual ~UnboundExpression() = default;
    virtual UnboundExpressionType type() const = 0;
};

class UnboundColumnExpression : public UnboundExpression {
public:
    explicit UnboundColumnExpression(ColumnRef ref) : ref_(std::move(ref)) {};

    const ColumnRef& ref() const {
        return ref_;
    }
    UnboundExpressionType type() const override {
        return UnboundExpressionType::COLUMN;
    }

private:
    ColumnRef ref_;
};

class UnboundConstantExpression : public UnboundExpression {
public:
    explicit UnboundConstantExpression(Value value) : value_(std::move(value)) {};

    const Value& value() const {
        return value_;
    }
    UnboundExpressionType type() const override {
        return UnboundExpressionType::CONSTANT;
    }

private:
    Value value_;
};

class UnboundComparisonExpression : public UnboundExpression {
public:
    UnboundComparisonExpression(std::unique_ptr<UnboundExpression> left, ComparisonOperator op,
                                std::unique_ptr<UnboundExpression> right)
        : left_(std::move(left)), op_(op), right_(std::move(right)) {};

    std::unique_ptr<UnboundExpression> take_left() {
        return std::move(left_);
    }
    std::unique_ptr<UnboundExpression> take_right() {
        return std::move(right_);
    }
    ComparisonOperator op() const {
        return op_;
    }
    UnboundExpressionType type() const override {
        return UnboundExpressionType::COMPARISON;
    }

private:
    std::unique_ptr<UnboundExpression> left_;
    ComparisonOperator op_;
    std::unique_ptr<UnboundExpression> right_;
};

class UnboundBinaryExpression : public UnboundExpression {
public:
    UnboundBinaryExpression(std::unique_ptr<UnboundExpression> left, BinaryOperator op,
                            std::unique_ptr<UnboundExpression> right)
        : left_(std::move(left)), op_(op), right_(std::move(right)) {};

    std::unique_ptr<UnboundExpression> take_left() {
        return std::move(left_);
    }
    std::unique_ptr<UnboundExpression> take_right() {
        return std::move(right_);
    }
    BinaryOperator op() const {
        return op_;
    }
    UnboundExpressionType type() const override {
        return UnboundExpressionType::BINARY;
    }

private:
    std::unique_ptr<UnboundExpression> left_;
    BinaryOperator op_;
    std::unique_ptr<UnboundExpression> right_;
};

class UnboundUnaryExpression : public UnboundExpression {
public:
    UnboundUnaryExpression(std::unique_ptr<UnboundExpression> expression, UnaryOperator op)
        : expression_(std::move(expression)), op_(op) {};

    std::unique_ptr<UnboundExpression> take_expression() {
        return std::move(expression_);
    }
    UnaryOperator op() const {
        return op_;
    }
    UnboundExpressionType type() const override {
        return UnboundExpressionType::UNARY;
    }

private:
    std::unique_ptr<UnboundExpression> expression_;
    UnaryOperator op_;
};

} // namespace duck

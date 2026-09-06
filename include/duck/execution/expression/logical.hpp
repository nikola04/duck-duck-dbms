#pragma once

#include "duck/execution/expression/expression.hpp"
#include "duck/tuple/value.hpp"
#include <memory>

namespace duck {

enum class BinaryOperator { AND, OR };

class BinaryExpression : public Expression {
public:
    BinaryExpression(std::unique_ptr<Expression> left_expr, BinaryOperator op, std::unique_ptr<Expression> right_expr)
        : left_(std::move(left_expr)), right_(std::move(right_expr)), op_(op) {};

    Value evaluate(const Record& record) const;

private:
    std::unique_ptr<Expression> left_;
    std::unique_ptr<Expression> right_;
    BinaryOperator op_;
};

enum class UnaryOperator { NOT };

class UnaryExpression : public Expression {
public:
    UnaryExpression(std::unique_ptr<Expression> expr, UnaryOperator op) : expr_(std::move(expr)), op_(op) {};

    Value evaluate(const Record& record) const;

private:
    std::unique_ptr<Expression> expr_;
    UnaryOperator op_;
};

} // namespace duck
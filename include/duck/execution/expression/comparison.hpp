#pragma once

#include "duck/execution/expression/expression.hpp"
#include "duck/tuple/value.hpp"
#include <memory>
namespace duck {

enum class ComparisonOperator { GREATER, LESS, GREATER_EQ, LESS_EQ, EQUAL, NOT_EQ };

enum class CompareResult { LESS, EQ, GREATER, UNKNOWN };

class ComparisonExpression : public Expression {
public:
    ComparisonExpression(std::unique_ptr<Expression> left_expr, ComparisonOperator op,
                         std::unique_ptr<Expression> right_expr)
        : left_(std::move(left_expr)), right_(std::move(right_expr)), op_(op) {};

    Value evaluate(const Record& record) const;

    static CompareResult compare(const Value& left, const Value& right);

private:
    std::unique_ptr<Expression> left_;
    std::unique_ptr<Expression> right_;
    ComparisonOperator op_;
};

} // namespace duck
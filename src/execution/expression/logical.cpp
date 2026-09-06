#include "duck/execution/expression/logical.hpp"
#include "duck/tuple/value.hpp"

namespace duck {

Value BinaryExpression::evaluate(const Record& record) const {
    Value left{left_->evaluate(record)};
    Value right{right_->evaluate(record)};

    if (left.is_null() && right.is_null())
        return Value::null(ValueType::BOOL);

    if (left.type() != ValueType::BOOL || right.type() != ValueType::BOOL)
        throw std::runtime_error("BinaryExpression expects boolean operands");

    switch (op_) {
    case BinaryOperator::AND:
        if (!left.is_null() && !left.as_bool())
            return Value::of(false);

        if (!right.is_null() && !right.as_bool())
            return Value::of(false);

        if (left.is_null() || right.is_null())
            return Value::null(ValueType::BOOL);

        return Value::of(true);

    case BinaryOperator::OR:
        if (!left.is_null() && left.as_bool())
            return Value::of(true);

        if (!right.is_null() && right.as_bool())
            return Value::of(true);

        if (left.is_null() || right.is_null())
            return Value::null(ValueType::BOOL);

        return Value::of(false);
    }

    throw std::runtime_error("BinaryExpression::evaluate: invalid binary operator");
}

Value UnaryExpression::evaluate(const Record& record) const {
    Value value{expr_->evaluate(record)};

    if (value.is_null())
        return Value::null(ValueType::BOOL);

    if (value.type() != ValueType::BOOL)
        throw std::runtime_error("UnaryExpression expects boolean operand");

    switch (op_) {
    case duck::UnaryOperator::NOT:
        return Value::of(!value.as_bool());
    }

    throw std::runtime_error("UnaryExpression::evaluate: invalid unary operator");
}

} // namespace duck
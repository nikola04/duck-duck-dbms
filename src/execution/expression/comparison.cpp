#include "duck/execution/expression/comparison.hpp"
#include "duck/execution/record.hpp"
#include "duck/tuple/value.hpp"

namespace duck {

/**
 * @brief Evaluates the comparison expression for a given record.
 *
 * If either the left or right operand evaluates to NULL, the function returns a NULL
 * value of the corresponding type. Otherwise, it compares the values and returns a boolean.
 *
 * @param record The database record used to evaluate the expression.
 * @return Value A boolean Value object, or a NULL Value if either operand is null.
 */
Value ComparisonExpression::evaluate(const Record& record) const {
    Value left{left_->evaluate(record)};
    Value right{right_->evaluate(record)};

    if (left.is_null() || right.is_null())
        return Value::null(left.type());

    switch (op_) {
    case duck::ComparisonOperator::EQUAL:
        return Value::of(left == right);
    case ComparisonOperator::GREATER:
        return Value::of(left.data() > right.data());
    case ComparisonOperator::LESS:
        return Value::of(left.data() < right.data());
    }
}

} // namespace duck
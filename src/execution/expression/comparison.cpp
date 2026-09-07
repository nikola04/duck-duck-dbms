#include "duck/execution/expression/comparison.hpp"
#include "duck/execution/record.hpp"
#include "duck/tuple/value.hpp"
#include <cmath>
#include <cstdint>
#include <limits>
#include <stdexcept>

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

    CompareResult cmp{compare(left, right)};
    if (cmp == CompareResult::UNKNOWN)
        return Value::null(ValueType::BOOL);

    switch (op_) {
    case duck::ComparisonOperator::EQUAL:
        return Value::of(cmp == CompareResult::EQ);
    case duck::ComparisonOperator::NOT_EQ:
        return Value::of(cmp != CompareResult::EQ);
    case ComparisonOperator::GREATER:
        return Value::of(cmp == CompareResult::GREATER);
    case ComparisonOperator::LESS:
        return Value::of(cmp == CompareResult::LESS);
    case ComparisonOperator::GREATER_EQ:
        return Value::of(cmp == CompareResult::GREATER || cmp == CompareResult::EQ);
    case ComparisonOperator::LESS_EQ:
        return Value::of(cmp == CompareResult::LESS || cmp == CompareResult::EQ);
    }

    throw std::runtime_error("ComparisonExpression::evaluate: unknow comparison operator");
}

template <typename T> CompareResult compare_generic(const T& left, const T& right) {
    if (left < right)
        return CompareResult::LESS;
    if (left > right)
        return CompareResult::GREATER;
    return CompareResult::EQ;
}

CompareResult ComparisonExpression::compare(const Value& left, const Value& right) {
    if (left.is_null() || right.is_null())
        return CompareResult::UNKNOWN;

    // DECIMAL
    if (left.is_floating() && right.is_floating()) {
        double l_val{left.type() == ValueType::FLOAT ? static_cast<double>(left.as_float()) : left.as_double()};
        double r_val{right.type() == ValueType::FLOAT ? static_cast<double>(right.as_float()) : right.as_double()};

        if (std::isnan(l_val) || std::isnan(r_val))
            return CompareResult::UNKNOWN;

        return compare_generic(l_val, r_val);
    } else if (left.is_floating() && right.is_numeric()) {
        double l_val{left.type() == ValueType::FLOAT ? static_cast<double>(left.as_float()) : left.as_double()};
        double r_val{static_cast<double>(right.type() == ValueType::UINT64 ? right.as_uint64() : right.as_integral())};

        if (std::isnan(l_val) || std::isnan(r_val))
            return CompareResult::UNKNOWN;

        return compare_generic(l_val, r_val);
    } else if (left.is_numeric() && right.is_floating()) {
        double l_val{static_cast<double>(left.type() == ValueType::UINT64 ? left.as_uint64() : left.as_integral())};
        double r_val{right.type() == ValueType::FLOAT ? static_cast<double>(right.as_float()) : right.as_double()};

        if (std::isnan(l_val) || std::isnan(r_val))
            return CompareResult::UNKNOWN;

        return compare_generic(l_val, r_val);
    }

    // INTEGRAL
    if (left.is_integral() && right.is_integral()) {
        if (left.type() == ValueType::UINT64 && right.type() == ValueType::UINT64) {
            std::uint64_t l_val{left.as_uint64()};
            std::uint64_t r_val{right.as_uint64()};

            return compare_generic(l_val, r_val);
        } else if (left.type() == ValueType::UINT64) {
            if (left.as_uint64() > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max()))
                return CompareResult::GREATER;

            return compare_generic(static_cast<std::int64_t>(left.as_uint64()), right.as_integral());
        } else if (right.type() == ValueType::UINT64) {
            if (right.as_uint64() > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max()))
                return CompareResult::LESS;

            return compare_generic(left.as_integral(), static_cast<std::int64_t>(right.as_uint64()));
        }

        return compare_generic(left.as_integral(), right.as_integral());
    }

    // BOOLEAN
    if (left.type() == ValueType::BOOL && right.type() == ValueType::BOOL)
        return compare_generic(left.as_bool(), right.as_bool());

    // STRING
    if (left.type() == ValueType::STRING && right.type() == ValueType::STRING)
        return compare_generic(left.as_string(), right.as_string());

    // BYTES
    if (left.type() == ValueType::BYTES && right.type() == ValueType::BYTES)
        return compare_generic(left.as_bytes(), right.as_bytes());

    return CompareResult::UNKNOWN;
}

} // namespace duck
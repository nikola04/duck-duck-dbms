#pragma once

#include "duck/execution/expression/expression.hpp"
#include "duck/tuple/value.hpp"
#include <cstddef>
namespace duck {

class ConstantExpression : public Expression {
public:
    ConstantExpression(Value value) : value_(value) {};

    Value evaluate(const Record& record) const override;

private:
    Value value_;
};

} // namespace duck
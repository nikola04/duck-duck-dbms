#pragma once

#include "duck/execution/expression/expression.hpp"
#include <cstddef>
namespace duck {

class ColumnExpression : public Expression {
public:
    ColumnExpression(std::size_t index) : index_(index) {};

    Value evaluate(const Record& record) const override;

private:
    std::size_t index_;
};

} // namespace duck
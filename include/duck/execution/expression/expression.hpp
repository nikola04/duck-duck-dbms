#pragma once

#include "duck/execution/record.hpp"
#include "duck/tuple/value.hpp"
namespace duck {

class Expression {
public:
    virtual ~Expression() = default;

    virtual Value evaluate(const Record& record) const = 0;
};

} // namespace duck
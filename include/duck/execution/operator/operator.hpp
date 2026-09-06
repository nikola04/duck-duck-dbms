#pragma once

#include "duck/execution/record.hpp"
#include <optional>

namespace duck {

class Operator {
public:
    virtual ~Operator() = default;

    virtual void init() = 0;
    virtual std::optional<Record> next() = 0;
};

} // namespace duck
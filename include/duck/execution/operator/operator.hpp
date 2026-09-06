#pragma once

#include "duck/execution/record.hpp"
#include "duck/tuple/schema.hpp"
#include <optional>

namespace duck {

class Operator {
public:
    virtual ~Operator() = default;

    virtual void init() = 0;
    virtual std::optional<Record> next() = 0;

    virtual const Schema& output_schema() const = 0;
};

} // namespace duck
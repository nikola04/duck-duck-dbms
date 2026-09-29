#pragma once

#include "duck/tuple/schema.hpp"
namespace duck {

class PhysicalPlanNode {
public:
    virtual ~PhysicalPlanNode() = default;

    virtual const Schema& output_schema() const = 0;
};

} // namespace duck
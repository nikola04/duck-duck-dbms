#pragma once

#include "duck/tuple/schema.hpp"
namespace duck {

class PhysicalPlanNode {
public:
    virtual ~PhysicalPlanNode() = 0;

    virtual const Schema& output_schema() const = 0;
};

} // namespace duck
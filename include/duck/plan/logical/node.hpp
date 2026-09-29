#pragma once

#include "duck/tuple/schema.hpp"
namespace duck {

enum class LogicalNodeType { SCAN, FILTER, PROJECTION, LIMIT };

class LogicalPlanNode {
public:
    virtual ~LogicalPlanNode() = default;

    virtual LogicalNodeType type() const = 0;
    virtual const Schema& output_schema() const = 0;
};

} // namespace duck
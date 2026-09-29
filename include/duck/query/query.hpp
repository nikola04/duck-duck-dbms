#pragma once

#include "duck/catalog/catalog.hpp"
#include "duck/plan/logical/node.hpp"
#include <memory>
namespace duck {

class Query {
public:
    virtual ~Query() = default;
    virtual std::unique_ptr<LogicalPlanNode> build_plan(Catalog& catalog) = 0;
};

} // namespace duck
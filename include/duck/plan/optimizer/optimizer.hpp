#pragma once

#include "duck/catalog/catalog.hpp"
#include "duck/plan/logical/node.hpp"
#include "duck/plan/physical/node.hpp"
#include <memory>
namespace duck {

class Optimizer {
public:
    Optimizer(Catalog& catalog);

    std::unique_ptr<PhysicalPlanNode> optimize(std::unique_ptr<LogicalPlanNode> logical_plan) const;

private:
    Catalog& catalog_;
};

} // namespace duck
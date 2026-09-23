#pragma once

#include "duck/plan/optimizer/optimizer.hpp"
#include "duck/catalog/catalog.hpp"
#include "duck/plan/logical/filter.hpp"
#include "duck/plan/logical/node.hpp"
#include "duck/plan/logical/scan.hpp"
#include "duck/plan/physical/filter.hpp"
#include "duck/plan/physical/node.hpp"
#include "duck/plan/physical/seq_scan.hpp"
#include <memory>
#include <stdexcept>
namespace duck {

Optimizer::Optimizer(Catalog& catalog) : catalog_(catalog) {};

std::unique_ptr<PhysicalPlanNode> Optimizer::optimize(std::unique_ptr<LogicalPlanNode> logical_plan) const {
    switch (logical_plan->type()) {
    case LogicalNodeType::SCAN: {
        const auto& scan{static_cast<LogicalScan&>(*logical_plan)};

        return std::make_unique<SequentialScanPlan>(scan.table());
    }
    case LogicalNodeType::FILTER: {
        auto& filter{static_cast<LogicalFilterPlan&>(*logical_plan)};

        auto child{optimize(filter.take_child())};

        return std::make_unique<PhysicalFilterPlan>(std::move(child), filter.take_predicate());
    }
    }
    throw std::runtime_error("Optimizer::optimize: LogicalNode type not found");
}

} // namespace duck
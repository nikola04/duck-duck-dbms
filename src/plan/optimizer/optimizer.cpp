#include "duck/plan/optimizer/optimizer.hpp"
#include "duck/catalog/catalog.hpp"
#include "duck/plan/logical/filter.hpp"
#include "duck/plan/logical/join.hpp"
#include "duck/plan/logical/limit.hpp"
#include "duck/plan/logical/node.hpp"
#include "duck/plan/logical/projection.hpp"
#include "duck/plan/logical/scan.hpp"
#include "duck/plan/physical/filter.hpp"
#include "duck/plan/physical/join/nested_loop_join.hpp"
#include "duck/plan/physical/limit.hpp"
#include "duck/plan/physical/node.hpp"
#include "duck/plan/physical/projection.hpp"
#include "duck/plan/physical/scan/seq_scan.hpp"
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
    case LogicalNodeType::JOIN: {
        auto& join{static_cast<LogicalJoin&>(*logical_plan)};

        auto left{optimize(join.take_left())};
        auto right{optimize(join.take_right())};

        return std::make_unique<NestedLoopJoinPlan>(std::move(left), std::move(right), join.take_predicate(),
                                                    join.take_schema());
    }
    case LogicalNodeType::FILTER: {
        auto& filter{static_cast<LogicalFilterPlan&>(*logical_plan)};

        auto child{optimize(filter.take_child())};

        return std::make_unique<PhysicalFilterPlan>(std::move(child), filter.take_predicate());
    }
    case duck::LogicalNodeType::PROJECTION: {
        auto& projection{static_cast<LogicalProjection&>(*logical_plan)};

        auto child{optimize(projection.take_child())};

        return std::make_unique<PhysicalProjectionPlan>(std::move(child), projection.take_columns(),
                                                        projection.take_schema());
    }
    case LogicalNodeType::LIMIT: {
        auto& limit{static_cast<LogicalLimit&>(*logical_plan)};

        auto child{optimize(limit.take_child())};

        return std::make_unique<PhysicalLimitPlan>(std::move(child), limit.limit());
    }
    }
    throw std::runtime_error("Optimizer::optimize: LogicalNode type not found");
}

} // namespace duck

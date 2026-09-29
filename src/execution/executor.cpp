#include "duck/execution/executor.hpp"
#include "duck/execution/operator/filter/filter.hpp"
#include "duck/execution/operator/scan/seq_scan.hpp"
#include "duck/execution/query_result.hpp"
#include "duck/plan/physical/filter.hpp"
#include "duck/plan/physical/seq_scan.hpp"
#include <stdexcept>

namespace duck {

QueryResult Executor::execute(PhysicalPlanNode& plan) {
    auto root{build(plan)};
    root->init();
    return QueryResult{std::move(root)};
}

std::unique_ptr<Operator> Executor::build(PhysicalPlanNode& plan) {
    if (auto* scan_plan{dynamic_cast<SequentialScanPlan*>(&plan)}; scan_plan != nullptr) {
        return std::make_unique<SequentialScanOperator>(&scan_plan->table(), context_.tx);
    }

    if (auto* filter_plan{dynamic_cast<PhysicalFilterPlan*>(&plan)}; filter_plan != nullptr) {
        auto child_plan{filter_plan->take_child()};
        auto child{build(*child_plan)};

        return std::make_unique<FilterOperator>(std::move(child), filter_plan->take_predicate());
    }

    throw std::runtime_error("Executor::build: unsupported physical plan node");
}

} // namespace duck

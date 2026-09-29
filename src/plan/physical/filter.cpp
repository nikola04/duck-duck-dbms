#include "duck/plan/physical/filter.hpp"
#include "duck/plan/physical/node.hpp"

namespace duck {

PhysicalFilterPlan::PhysicalFilterPlan(std::unique_ptr<PhysicalPlanNode> child, std::unique_ptr<Expression> predicate)
    : child_(std::move(child)), predicate_(std::move(predicate)) {};

const PhysicalPlanNode& PhysicalFilterPlan::child() const {
    return *child_;
}
const Expression& PhysicalFilterPlan::predicate() const {
    return *predicate_;
}

std::unique_ptr<PhysicalPlanNode> PhysicalFilterPlan::take_child() {
    return std::move(child_);
}

std::unique_ptr<Expression> PhysicalFilterPlan::take_predicate() {
    return std::move(predicate_);
}

const Schema& PhysicalFilterPlan::output_schema() const {
    return child_->output_schema();
}

} // namespace duck

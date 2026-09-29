#include "duck/plan/logical/filter.hpp"

namespace duck {

LogicalFilterPlan::LogicalFilterPlan(std::unique_ptr<LogicalPlanNode> child, std::unique_ptr<Expression> predicate)
    : child_(std::move(child)), predicate_(std::move(predicate)) {};

const LogicalPlanNode& LogicalFilterPlan::child() const {
    return *child_;
}
const Expression& LogicalFilterPlan::predicate() const {
    return *predicate_;
}

std::unique_ptr<LogicalPlanNode> LogicalFilterPlan::take_child() {
    return std::move(child_);
}
std::unique_ptr<Expression> LogicalFilterPlan::take_predicate() {
    return std::move(predicate_);
}

const Schema& LogicalFilterPlan::output_schema() const {
    return child_->output_schema();
}

} // namespace duck
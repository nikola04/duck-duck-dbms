#include "duck/plan/physical/join/nested_loop_join.hpp"
#include "duck/plan/physical/node.hpp"

namespace duck {

std::unique_ptr<PhysicalPlanNode> NestedLoopJoinPlan::take_left() {
    return std::move(left_);
}
std::unique_ptr<PhysicalPlanNode> NestedLoopJoinPlan::take_right() {
    return std::move(right_);
}
std::unique_ptr<Expression> NestedLoopJoinPlan::take_predicate() {
    return std::move(predicate_);
}
Schema NestedLoopJoinPlan::take_schema() {
    return std::move(schema_);
}

const Schema& NestedLoopJoinPlan::output_schema() const {
    return schema_;
}

} // namespace duck
#include "duck/plan/physical/limit.hpp"
#include <utility>
namespace duck {

PhysicalLimitPlan::PhysicalLimitPlan(std::unique_ptr<PhysicalPlanNode> child, std::size_t limit)
    : child_(std::move(child)), limit_(limit) {};

const PhysicalPlanNode& PhysicalLimitPlan::child() const {
    return *child_;
}
std::size_t PhysicalLimitPlan::limit() const {
    return limit_;
}

std::unique_ptr<PhysicalPlanNode> PhysicalLimitPlan::take_child() {
    return std::move(child_);
}

const Schema& PhysicalLimitPlan::output_schema() const {
    return child_->output_schema();
};

} // namespace duck
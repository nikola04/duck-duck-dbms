#include "duck/plan/logical/limit.hpp"
namespace duck {

LogicalLimit::LogicalLimit(std::unique_ptr<LogicalPlanNode> child, std::size_t limit)
    : child_(std::move(child)), limit_(limit) {};

const LogicalPlanNode& LogicalLimit::child() const {
    return *child_;
}
std::size_t LogicalLimit::limit() const {
    return limit_;
}

std::unique_ptr<LogicalPlanNode> LogicalLimit::take_child() {
    return std::move(child_);
}

const Schema& LogicalLimit::output_schema() const {
    return child_->output_schema();
}

} // namespace duck
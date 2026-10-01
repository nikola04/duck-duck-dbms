#include "duck/plan/logical/join.hpp"
namespace duck {

const LogicalPlanNode& LogicalJoin::left() const {
    return *left_;
}
const LogicalPlanNode& LogicalJoin::right() const {
    return *right_;
}
const Expression& LogicalJoin::predicate() const {
    return *predicate_;
}

std::unique_ptr<LogicalPlanNode> LogicalJoin::take_left() {
    return std::move(left_);
}
std::unique_ptr<LogicalPlanNode> LogicalJoin::take_right() {
    return std::move(right_);
}
std::unique_ptr<Expression> LogicalJoin::take_predicate() {
    return std::move(predicate_);
}
Schema LogicalJoin::take_schema() {
    return std::move(schema_);
}

const Schema& LogicalJoin::output_schema() const {
    return schema_;
}

} // namespace duck
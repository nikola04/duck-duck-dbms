#include "duck/plan/logical/projection.hpp"
#include "duck/tuple/schema.hpp"
#include <utility>
namespace duck {

LogicalProjection::LogicalProjection(std::unique_ptr<LogicalPlanNode> child, std::vector<std::size_t> columns)
    : child_(std::move(child)), columns_(std::move(columns)),
      schema_(Schema::projected(child_->output_schema(), columns_)) {};

std::unique_ptr<LogicalPlanNode> LogicalProjection::take_child() {
    return std::move(child_);
}

std::vector<std::size_t> LogicalProjection::take_columns() {
    return std::move(columns_);
}

Schema LogicalProjection::take_schema() {
    return std::move(schema_);
}

const Schema& LogicalProjection::output_schema() const {
    return schema_;
}

} // namespace duck

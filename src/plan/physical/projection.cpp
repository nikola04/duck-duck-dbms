#include "duck/plan/physical/projection.hpp"
#include "duck/plan/physical/node.hpp"
#include "duck/tuple/schema.hpp"
namespace duck {

PhysicalProjectionPlan::PhysicalProjectionPlan(std::unique_ptr<PhysicalPlanNode> child,
                                               std::vector<std::size_t> columns)
    : child_(std::move(child)), columns_(std::move(columns)),
      schema_(Schema::projected(child_->output_schema(), columns_)) {};

PhysicalProjectionPlan::PhysicalProjectionPlan(std::unique_ptr<PhysicalPlanNode> child,
                                               std::vector<std::size_t> columns, Schema output_schema)
    : child_(std::move(child)), columns_(std::move(columns)), schema_(std::move(output_schema)) {};

std::unique_ptr<PhysicalPlanNode> PhysicalProjectionPlan::take_child() {
    return std::move(child_);
}

std::vector<std::size_t> PhysicalProjectionPlan::take_columns() {
    return std::move(columns_);
}

Schema PhysicalProjectionPlan::take_schema() {
    return std::move(schema_);
}

const Schema& PhysicalProjectionPlan::output_schema() const {
    return schema_;
}

} // namespace duck

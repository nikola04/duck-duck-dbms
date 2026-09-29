#pragma once

#include "duck/plan/physical/node.hpp"
#include <cstddef>
#include <memory>
#include <vector>

namespace duck {

class PhysicalProjectionPlan : public PhysicalPlanNode {
public:
    PhysicalProjectionPlan(std::unique_ptr<PhysicalPlanNode> child, std::vector<std::size_t> columns);
    PhysicalProjectionPlan(std::unique_ptr<PhysicalPlanNode> child, std::vector<std::size_t> columns,
                           Schema output_schema);

    std::vector<std::size_t> take_columns();
    std::unique_ptr<PhysicalPlanNode> take_child();
    Schema take_schema();

    const Schema& output_schema() const override;

private:
    std::unique_ptr<PhysicalPlanNode> child_;
    std::vector<std::size_t> columns_;
    Schema schema_;
};

} // namespace duck

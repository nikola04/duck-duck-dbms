#pragma once

#include "duck/plan/physical/node.hpp"
#include <cstddef>
#include <memory>

namespace duck {

class PhysicalLimitPlan : public PhysicalPlanNode {
public:
    PhysicalLimitPlan(std::unique_ptr<PhysicalPlanNode> child, std::size_t limit);

    const PhysicalPlanNode& child() const;
    std::size_t limit() const;

    std::unique_ptr<PhysicalPlanNode> take_child();

    const Schema& output_schema() const override;

private:
    std::unique_ptr<PhysicalPlanNode> child_;
    std::size_t limit_;
};

} // namespace duck
#pragma once

#include "duck/expression/expression.hpp"
#include "duck/plan/physical/node.hpp"
#include <memory>

namespace duck {

class PhysicalFilterPlan : public PhysicalPlanNode {
public:
    PhysicalFilterPlan(std::unique_ptr<PhysicalPlanNode> child, std::unique_ptr<Expression> predicate);

    const PhysicalPlanNode& child() const;
    const Expression& predicate() const;

    std::unique_ptr<PhysicalPlanNode> take_child();
    std::unique_ptr<Expression> take_predicate();

    const Schema& output_schema() const override;

private:
    std::unique_ptr<PhysicalPlanNode> child_;
    std::unique_ptr<Expression> predicate_;
};

} // namespace duck

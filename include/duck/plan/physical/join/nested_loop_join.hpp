#pragma once

#include "duck/expression/expression.hpp"
#include "duck/plan/physical/node.hpp"
#include "duck/tuple/schema.hpp"
#include <memory>
namespace duck {

class NestedLoopJoinPlan : public PhysicalPlanNode {
public:
    explicit NestedLoopJoinPlan(std::unique_ptr<PhysicalPlanNode> left, std::unique_ptr<PhysicalPlanNode> right,
                                std::unique_ptr<Expression> predicate, Schema schema)
        : left_(std::move(left)), right_(std::move(right)), predicate_(std::move(predicate)),
          schema_(std::move(schema)) {};

    std::unique_ptr<PhysicalPlanNode> take_left();
    std::unique_ptr<PhysicalPlanNode> take_right();
    std::unique_ptr<Expression> take_predicate();
    Schema take_schema();

    const Schema& output_schema() const override;

private:
    std::unique_ptr<PhysicalPlanNode> left_;
    std::unique_ptr<PhysicalPlanNode> right_;
    std::unique_ptr<Expression> predicate_;
    Schema schema_;
};

} // namespace duck
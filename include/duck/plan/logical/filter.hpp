#pragma once

#include "duck/execution/expression/expression.hpp"
#include "duck/plan/logical/node.hpp"
#include <memory>

namespace duck {

class LogicalFilterPlan : public LogicalPlanNode {
public:
    LogicalFilterPlan(std::unique_ptr<LogicalPlanNode> child, std::unique_ptr<Expression> predicate);

    const LogicalPlanNode& child() const;
    const Expression& predicate() const;

    std::unique_ptr<LogicalPlanNode> take_child();
    std::unique_ptr<Expression> take_predicate();

    const Schema& output_schema() const override;
    LogicalNodeType type() const override {
        return LogicalNodeType::FILTER;
    };

private:
    std::unique_ptr<LogicalPlanNode> child_;
    std::unique_ptr<Expression> predicate_;
};

} // namespace duck
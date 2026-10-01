#pragma once

#include "duck/expression/expression.hpp"
#include "duck/plan/logical/node.hpp"
#include <memory>
namespace duck {

class LogicalJoin : public LogicalPlanNode {
public:
    explicit LogicalJoin(std::unique_ptr<LogicalPlanNode> left, std::unique_ptr<LogicalPlanNode> right,
                         std::unique_ptr<Expression> predicate)
        : left_(std::move(left)), right_(std::move(right)), predicate_(std::move(predicate)),
          schema_(left_->output_schema() + right_->output_schema()) {};

    const LogicalPlanNode& left() const;
    const LogicalPlanNode& right() const;
    const Expression& predicate() const;

    std::unique_ptr<LogicalPlanNode> take_left();
    std::unique_ptr<LogicalPlanNode> take_right();
    std::unique_ptr<Expression> take_predicate();
    Schema take_schema();

    const Schema& output_schema() const override;
    LogicalNodeType type() const override {
        return LogicalNodeType::JOIN;
    };

private:
    std::unique_ptr<LogicalPlanNode> left_;
    std::unique_ptr<LogicalPlanNode> right_;
    std::unique_ptr<Expression> predicate_;
    Schema schema_;
};

} // namespace duck
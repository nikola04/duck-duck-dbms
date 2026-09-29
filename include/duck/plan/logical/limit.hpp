#pragma once

#include "duck/plan/logical/node.hpp"
#include <cstddef>
#include <memory>

namespace duck {

class LogicalLimit : public LogicalPlanNode {
public:
    LogicalLimit(std::unique_ptr<LogicalPlanNode> child, std::size_t limit);

    const LogicalPlanNode& child() const;
    std::size_t limit() const;

    std::unique_ptr<LogicalPlanNode> take_child();

    const Schema& output_schema() const override;
    LogicalNodeType type() const override {
        return LogicalNodeType::LIMIT;
    };

private:
    std::unique_ptr<LogicalPlanNode> child_;
    std::size_t limit_;
};

} // namespace duck
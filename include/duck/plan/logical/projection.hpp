#pragma once

#include "duck/plan/logical/node.hpp"
#include <cstddef>
#include <memory>
#include <vector>
namespace duck {

class LogicalProjection : public LogicalPlanNode {
public:
    LogicalProjection(std::unique_ptr<LogicalPlanNode> child, std::vector<std::size_t> columns);

    std::unique_ptr<LogicalPlanNode> take_child();
    std::vector<std::size_t> take_columns();
    Schema take_schema();

    const Schema& output_schema() const override;
    LogicalNodeType type() const override {
        return LogicalNodeType::PROJECTION;
    };

private:
    std::unique_ptr<LogicalPlanNode> child_;
    std::vector<std::size_t> columns_;
    Schema schema_;
};

} // namespace duck

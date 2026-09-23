#pragma once

#include "duck/plan/logical/node.hpp"
#include "duck/table/table.hpp"

namespace duck {

class LogicalScan : public LogicalPlanNode {

public:
    explicit LogicalScan(Table& table);

    Table& table() const;

    const Schema& output_schema() const override;
    LogicalNodeType type() const override {
        return LogicalNodeType::SCAN;
    };

private:
    Table& table_;
};

} // namespace duck
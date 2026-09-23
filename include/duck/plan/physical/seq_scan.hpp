#pragma once

#include "duck/plan/physical/node.hpp"
#include "duck/table/table.hpp"
#include "duck/tuple/schema.hpp"

namespace duck {

class SequentialScanPlan : public PhysicalPlanNode {
public:
    explicit SequentialScanPlan(Table& table);

    Table& table() const;
    const Schema& output_schema() const override;

private:
    Table& table_;
};

} // namespace duck
#pragma once

#include "duck/execution/executor_context.hpp"
#include "duck/execution/operator/operator.hpp"
#include "duck/execution/record.hpp"
#include "duck/table/table.hpp"
#include <optional>

namespace duck {

class SequentialScanOperator : public Operator {
public:
    explicit SequentialScanOperator(ExecutorContext& context, Table* table);

    void init() override;
    std::optional<Record> next() override;

    const Schema& output_schema() const override;

private:
    Table* table_;
    ExecutorContext& context_;

    Table::Scan scanner_;
};

} // namespace duck
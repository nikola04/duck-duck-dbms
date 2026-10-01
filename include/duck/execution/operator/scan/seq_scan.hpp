#pragma once

#include "duck/execution/operator/operator.hpp"
#include "duck/record/record.hpp"
#include "duck/table/table.hpp"
#include "duck/transaction/transaction.hpp"
#include <memory>
#include <optional>

namespace duck {

class SequentialScanOperator : public Operator {
public:
    explicit SequentialScanOperator(Table* table, Transaction* tx);

    void init() override;
    void reset() override;
    std::optional<Record> next() override;

    const Schema& output_schema() const override;

private:
    Table* table_;
    Transaction* tx_;

    std::unique_ptr<Table::Scan> scanner_;
};

} // namespace duck
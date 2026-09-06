#pragma once

#include "duck/execution/operator/operator.hpp"
#include "duck/execution/record.hpp"
#include "duck/table/table.hpp"
#include "duck/transaction/transaction.hpp"
#include <optional>

namespace duck {

class SequentialScanOperator : public Operator {
public:
    explicit SequentialScanOperator(Table* table, Transaction* tx);

    void init() override;
    std::optional<Record> next() override;

    const Schema& output_schema() const override;

private:
    Table* table_;
    Transaction* tx_;

    Table::Scan scanner_;
};

} // namespace duck
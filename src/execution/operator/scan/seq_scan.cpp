#include "duck/execution/operator/scan/seq_scan.hpp"
#include "duck/execution/record.hpp"
#include <optional>

namespace duck {

SequentialScanOperator::SequentialScanOperator(ExecutorContext& context, Table* table)
    : table_(table), context_(context), scanner_(table_->scan(context_.tx)) {};

void SequentialScanOperator::init() {
}

std::optional<Record> SequentialScanOperator::next() {
    auto result{scanner_.next()};

    if (!result.has_value())
        return std::nullopt;

    return Record{std::move(result->second).take_values()};
}

const Schema& SequentialScanOperator::output_schema() const {
    return table_->schema();
}

} // namespace duck
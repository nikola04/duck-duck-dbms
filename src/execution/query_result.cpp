#include "duck/execution/query_result.hpp"

namespace duck {

QueryResult::QueryResult(std::unique_ptr<Operator> op) : root_(std::move(op)) {};

std::optional<Record> QueryResult::next() {
    return root_->next();
}

const Schema& QueryResult::output_schema() const {
    return root_->output_schema();
}

} // namespace duck
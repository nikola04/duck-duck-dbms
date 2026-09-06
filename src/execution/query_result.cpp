#include "duck/execution/query_result.hpp"

namespace duck {

std::optional<Record> QueryResult::next() {
    return root_->next();
}

const Schema& QueryResult::output_schema() const {
    return root_->output_schema();
}

} // namespace duck
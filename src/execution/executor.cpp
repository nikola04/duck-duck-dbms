#include "duck/execution/executor.hpp"
#include "duck/execution/query_result.hpp"

namespace duck {

QueryResult::QueryResult(std::unique_ptr<Operator> op) : root_(std::move(op)) {};

QueryResult Executor::execute() {
    root_->init();
    return QueryResult{std::move(root_)};
}

} // namespace duck
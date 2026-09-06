#include "duck/execution/executor.hpp"
#include "duck/execution/query_result.hpp"

namespace duck {

QueryResult Executor::execute() {
    root_->init();
    return QueryResult{*root_};
}

} // namespace duck
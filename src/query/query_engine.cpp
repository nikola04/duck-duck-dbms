#include "duck/query/query_engine.hpp"
#include "duck/execution/executor.hpp"
#include "duck/execution/executor_context.hpp"
#include "duck/query/select_query.hpp"

namespace duck {

QueryResult QueryEngine::execute(SelectQuery& query, QueryContext& query_context) {
    auto logical_plan{binder_.bind(query, query_context.tx)};
    auto optimized{optimizer_.optimize(std::move(logical_plan))};

    ExecutorContext exec_ctx{.tx = query_context.tx};
    Executor executor{exec_ctx};

    return executor.execute(*optimized);
}

} // namespace duck
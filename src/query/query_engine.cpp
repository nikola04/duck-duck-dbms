#include "duck/query/query_engine.hpp"
#include "duck/execution/executor.hpp"
#include "duck/query/select_query.hpp"

namespace duck {

QueryResult QueryEngine::execute(SelectQuery& query, ExecutorContext& context) {
    auto logical_plan{binder_.bind(query)};
    auto optimized{optimizer_.optimize(std::move(logical_plan))};

    Executor executor{context};
    return executor.execute(*optimized);
}

} // namespace duck
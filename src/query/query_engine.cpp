#include "duck/query/query_engine.hpp"
#include "duck/execution/executor.hpp"

namespace duck {

QueryResult QueryEngine::execute(std::unique_ptr<Query> query, ExecutorContext& context) {
    auto plan{optimizer_.optimize(query->build_plan(catalog_))};

    Executor executor{context};
    return executor.execute(*plan);
}

} // namespace duck
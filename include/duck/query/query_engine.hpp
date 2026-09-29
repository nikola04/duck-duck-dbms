#pragma once

#include "duck/catalog/catalog.hpp"
#include "duck/execution/executor_context.hpp"
#include "duck/execution/query_result.hpp"
#include "duck/plan/optimizer/optimizer.hpp"
#include "duck/query/binder.hpp"
#include "duck/query/select_query.hpp"

namespace duck {

class QueryEngine {
public:
    explicit QueryEngine(Catalog& catalog) : catalog_(catalog), binder_(catalog_), optimizer_(catalog_) {};

    QueryResult execute(SelectQuery& query, ExecutorContext& context);

private:
    Catalog& catalog_;
    Binder binder_;
    Optimizer optimizer_;
};

} // namespace duck
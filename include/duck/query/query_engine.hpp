#pragma once

#include "duck/catalog/catalog.hpp"
#include "duck/execution/executor_context.hpp"
#include "duck/execution/query_result.hpp"
#include "duck/plan/optimizer/optimizer.hpp"
#include "duck/query/query.hpp"
#include <memory>

namespace duck {

class QueryEngine {
public:
    explicit QueryEngine(Catalog& catalog) : catalog_(catalog), optimizer_(catalog_) {};

    QueryResult execute(std::unique_ptr<Query> query, ExecutorContext& context);

private:
    Catalog& catalog_;
    Optimizer optimizer_;
};

} // namespace duck
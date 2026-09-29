#pragma once

#include "duck/execution/executor_context.hpp"
#include "duck/execution/operator/operator.hpp"
#include "duck/execution/query_result.hpp"
#include "duck/plan/physical/node.hpp"
#include <memory>

namespace duck {

class Executor {
public:
    Executor(ExecutorContext& context) : context_(context) {};

    QueryResult execute(PhysicalPlanNode& plan);

private:
    ExecutorContext& context_;

    std::unique_ptr<Operator> build(PhysicalPlanNode& plan);
};

} // namespace duck
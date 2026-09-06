#pragma once

#include "duck/execution/executor_context.hpp"
#include "duck/execution/operator/operator.hpp"
#include "duck/execution/query_result.hpp"
#include <memory>

namespace duck {

class Executor {
public:
    Executor(std::unique_ptr<Operator> op, ExecutorContext& context) : context_(context), root_(std::move(op)) {};

    QueryResult execute();

private:
    ExecutorContext& context_;
    std::unique_ptr<Operator> root_;
};

} // namespace duck
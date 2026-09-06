#pragma once

#include "duck/execution/operator/operator.hpp"
#include "duck/execution/query_result.hpp"
#include <memory>

namespace duck {

class Executor {
public:
    Executor(std::unique_ptr<Operator> op) : root_(std::move(op)) {};

    QueryResult execute();

private:
    std::unique_ptr<Operator> root_;
};

} // namespace duck
#pragma once

#include "duck/execution/operator/operator.hpp"
#include <optional>

namespace duck {

class QueryResult {
public:
    explicit QueryResult(Operator& op) : root_(op) {};
    std::optional<Record> next();

private:
    Operator& root_;
};

} // namespace duck
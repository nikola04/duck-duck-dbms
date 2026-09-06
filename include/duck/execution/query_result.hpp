#pragma once

#include "duck/execution/operator/operator.hpp"
#include <memory>
#include <optional>

namespace duck {

class QueryResult {
public:
    explicit QueryResult(std::unique_ptr<Operator> op);
    std::optional<Record> next();

    const Schema& output_schema() const;

private:
    std::unique_ptr<Operator> root_;
};

} // namespace duck
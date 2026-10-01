#pragma once

#include "duck/query/expression.hpp"
#include "duck/query/refs.hpp"
#include <memory>

namespace duck {

enum class JoinType {
    INNER,
    // LEFT,
    // RIGHT,
    // OUTER,
};

struct JoinClause {
    TableRef table;
    std::unique_ptr<UnboundExpression> on;
    JoinType type;
};

} // namespace duck
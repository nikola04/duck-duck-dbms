#pragma once

#include "duck/query/expression.hpp"
#include "duck/query/join.hpp"
#include "duck/query/query.hpp"
#include "duck/query/refs.hpp"
#include <cstddef>
#include <memory>
#include <optional>
#include <vector>

namespace duck {

class SelectQuery : public Query {
public:
    SelectQuery() = default;
    SelectQuery(std::vector<ColumnRef> columns);

    SelectQuery& columns(std::vector<ColumnRef> columns);
    SelectQuery& from(TableRef table);
    SelectQuery& where(std::unique_ptr<UnboundExpression> predicate);
    SelectQuery& limit(std::size_t limit);
    SelectQuery& join(JoinClause clause);

    const std::optional<std::vector<ColumnRef>>& columns() const {
        return columns_;
    };
    const std::optional<TableRef>& table() const {
        return table_;
    };
    std::optional<std::size_t> limit() const {
        return limit_;
    }
    std::unique_ptr<UnboundExpression> take_predicate() {
        return std::move(predicate_);
    }
    std::vector<JoinClause> take_joins() {
        return std::move(joins_);
    }

private:
    std::optional<std::vector<ColumnRef>> columns_;
    std::optional<TableRef> table_;
    std::unique_ptr<UnboundExpression> predicate_;
    std::optional<std::size_t> limit_;
    std::vector<JoinClause> joins_;
};

} // namespace duck

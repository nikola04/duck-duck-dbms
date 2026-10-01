#include "duck/query/select_query.hpp"
#include "duck/query/join.hpp"
#include "duck/query/refs.hpp"
#include <memory>

namespace duck {

SelectQuery::SelectQuery(std::vector<ColumnRef> columns) {
    this->columns(std::move(columns));
}

SelectQuery& SelectQuery::columns(std::vector<ColumnRef> columns) {
    if (columns.size() == 1 && !columns.front().qualifier.has_value() && columns.front().name == "*") {
        columns_.reset();
        return *this;
    }

    columns_ = std::move(columns);
    return *this;
}

SelectQuery& SelectQuery::from(TableRef table) {
    table_ = table;
    return *this;
}

SelectQuery& SelectQuery::where(std::unique_ptr<UnboundExpression> predicate) {
    predicate_ = std::move(predicate);
    return *this;
}

SelectQuery& SelectQuery::limit(std::size_t limit) {
    limit_ = limit;
    return *this;
}

SelectQuery& SelectQuery::join(JoinClause clause) {
    joins_.push_back(std::move(clause));
    return *this;
}

} // namespace duck

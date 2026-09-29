#include "duck/query/select_query.hpp"
#include <memory>

namespace duck {

SelectQuery::SelectQuery() : columns_(0) {};

SelectQuery::SelectQuery(std::vector<std::string> columns) : columns_(std::move(columns)) {
}

SelectQuery& SelectQuery::from(std::string_view table) {
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

} // namespace duck

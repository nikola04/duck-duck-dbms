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

SelectQuery& SelectQuery::where(std::unique_ptr<Expression> predicate) {
    predicate_ = std::move(predicate);
    return *this;
}

} // namespace duck
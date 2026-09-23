#include "duck/query/select_query.hpp"

namespace duck {

SelectQuery& SelectQuery::select(std::vector<std::size_t> columns) {
    columns_ = columns;
    return *this;
}

SelectQuery& SelectQuery::from(std::string_view table) {
    table_ = table;
    return *this;
}

SelectQuery& SelectQuery::where(std::unique_ptr<Expression> predicate) {
    predicate_ = std::move(predicate);
    return *this;
}

const std::vector<std::size_t>& SelectQuery::columns() const {
    return columns_;
}
const std::string SelectQuery::table() const {
    return table_;
}
const Expression* SelectQuery::predicate() const {
    return predicate_.get();
}

} // namespace duck
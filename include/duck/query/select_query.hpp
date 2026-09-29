#pragma once

#include "duck/expression/expression.hpp"
#include "duck/query/query.hpp"
#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace duck {

class SelectQuery : public Query {
public:
    SelectQuery();
    SelectQuery(std::vector<std::string> columns);

    SelectQuery& from(std::string_view table);
    SelectQuery& where(std::unique_ptr<Expression> predicate);

    const std::vector<std::string>& columns() const {
        return columns_;
    };
    const std::string& table() const {
        return table_;
    };
    std::unique_ptr<Expression> take_predicate() {
        return std::move(predicate_);
    }

private:
    std::vector<std::string> columns_;
    std::string table_;
    std::unique_ptr<Expression> predicate_;
};

} // namespace duck
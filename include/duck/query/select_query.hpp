#pragma once

#include "duck/execution/expression/expression.hpp"
#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace duck {

class SelectQuery {
public:
    SelectQuery& select(std::vector<std::size_t> columns);
    SelectQuery& from(std::string_view table);
    SelectQuery& where(std::unique_ptr<Expression> predicate);

    const std::vector<std::size_t>& columns() const;
    const std::string table() const;
    const Expression* predicate() const;

private:
    std::vector<std::size_t> columns_;
    std::string table_;
    std::unique_ptr<Expression> predicate_;
};

} // namespace duck
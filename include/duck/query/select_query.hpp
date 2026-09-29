#pragma once

#include "duck/expression/expression.hpp"
#include "duck/query/query.hpp"
#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace duck {

class SelectQuery : public Query {
public:
    SelectQuery();
    SelectQuery(std::vector<std::size_t> columns);

    SelectQuery& from(std::string_view table);
    SelectQuery& where(std::unique_ptr<Expression> predicate);

    std::unique_ptr<LogicalPlanNode> build_plan(Catalog& catalog) override;

private:
    std::vector<std::size_t> columns_;
    std::string table_;
    std::unique_ptr<Expression> predicate_;
};

} // namespace duck
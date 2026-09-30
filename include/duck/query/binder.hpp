#pragma once

#include "duck/catalog/catalog.hpp"
#include "duck/expression/expression.hpp"
#include "duck/plan/logical/node.hpp"
#include "duck/query/expression.hpp"
#include "duck/query/select_query.hpp"
#include "duck/transaction/transaction.hpp"
#include <memory>

namespace duck {

class Binder {
public:
    Binder(Catalog& catalog) : catalog_(catalog) {};

    std::unique_ptr<LogicalPlanNode> bind(SelectQuery& select_query, Transaction* tx);

private:
    std::unique_ptr<Expression> bind_expression(std::unique_ptr<UnboundExpression> expression, const Schema& schema,
                                                std::string_view table_name);

    std::size_t resolve_column(const ColumnRef& column, const Schema& schema, std::string_view table_name) const;

    Catalog& catalog_;
};

} // namespace duck

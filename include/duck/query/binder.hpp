#pragma once

#include "duck/catalog/catalog.hpp"
#include "duck/expression/expression.hpp"
#include "duck/plan/logical/node.hpp"
#include "duck/query/expression.hpp"
#include "duck/query/select_query.hpp"
#include "duck/transaction/transaction.hpp"
#include "duck/tuple/schema.hpp"
#include <cstddef>
#include <memory>
#include <vector>

namespace duck {

struct TableBinding {
    std::string name;
    const Schema* schema;
    std::size_t offset;
};
using BindingContext = std::vector<TableBinding>;

class Binder {
public:
    Binder(Catalog& catalog) : catalog_(catalog) {};

    std::unique_ptr<LogicalPlanNode> bind(SelectQuery& select_query, Transaction* tx);

private:
    std::unique_ptr<Expression> bind_expression(std::unique_ptr<UnboundExpression> expression,
                                                const BindingContext& context);

    std::size_t resolve_column(const ColumnRef& column, const BindingContext& context) const;

    Catalog& catalog_;
};

} // namespace duck

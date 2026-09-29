#include "duck/query/select_query.hpp"
#include "duck/catalog/catalog.hpp"
#include "duck/plan/logical/filter.hpp"
#include "duck/plan/logical/node.hpp"
#include "duck/plan/logical/projection.hpp"
#include "duck/plan/logical/scan.hpp"
#include <memory>
#include <stdexcept>

namespace duck {

SelectQuery::SelectQuery() : columns_(0) {};

SelectQuery::SelectQuery(std::vector<std::size_t> columns) : columns_(std::move(columns)) {
}

SelectQuery& SelectQuery::from(std::string_view table) {
    table_ = table;
    return *this;
}

SelectQuery& SelectQuery::where(std::unique_ptr<Expression> predicate) {
    predicate_ = std::move(predicate);
    return *this;
}

std::unique_ptr<LogicalPlanNode> SelectQuery::build_plan(Catalog& catalog) {
    auto table{catalog.get_table(table_)};
    if (!table.has_value())
        throw std::runtime_error("SelectQuery::build_plan: table not found: " + table_);

    std::unique_ptr<LogicalPlanNode> plan{std::make_unique<duck::LogicalScan>(*table.value())};

    if (predicate_)
        plan = std::make_unique<duck::LogicalFilterPlan>(std::move(plan), std::move(predicate_));

    if (!columns_.empty())
        plan = std::make_unique<duck::LogicalProjection>(std::move(plan), std::move(columns_));

    return plan;
}

} // namespace duck
#include "duck/query/binder.hpp"
#include "duck/expression/column.hpp"
#include "duck/expression/comparison.hpp"
#include "duck/expression/constant.hpp"
#include "duck/expression/logical.hpp"
#include "duck/plan/logical/filter.hpp"
#include "duck/plan/logical/limit.hpp"
#include "duck/plan/logical/projection.hpp"
#include "duck/plan/logical/scan.hpp"
#include <cstddef>
#include <format>
#include <memory>
#include <stdexcept>
#include <vector>
namespace duck {

std::unique_ptr<LogicalPlanNode> Binder::bind(SelectQuery& select_query, Transaction* tx) {
    auto table{catalog_.get_table(select_query.table(), tx)};
    if (!table.has_value())
        throw std::runtime_error(std::format("Binder::bind: table {} doesn't exist", select_query.table()));

    std::unique_ptr<LogicalPlanNode> plan{std::make_unique<duck::LogicalScan>(*table.value())};

    const auto& schema{table.value()->schema()};

    // Filter
    if (auto predicate{select_query.take_predicate()}; predicate) {
        auto bound_predicate{bind_expression(std::move(predicate), schema, select_query.table())};
        plan = std::make_unique<duck::LogicalFilterPlan>(std::move(plan), std::move(bound_predicate));
    }

    // Projection
    if (const auto& columns{select_query.columns()}; columns.has_value()) {
        if (columns->empty())
            throw std::runtime_error("Binder::bind: columns not specified");

        std::vector<std::size_t> column_idxs;
        column_idxs.reserve(columns->size());

        for (const auto& column : *columns)
            column_idxs.push_back(resolve_column(column, schema, select_query.table()));

        plan = std::make_unique<duck::LogicalProjection>(std::move(plan), std::move(column_idxs));
    }

    // Limit
    if (auto limit{select_query.limit()}; limit.has_value()) {
        plan = std::make_unique<LogicalLimit>(std::move(plan), *limit);
    }

    return plan;
}

std::unique_ptr<Expression> Binder::bind_expression(std::unique_ptr<UnboundExpression> expression, const Schema& schema,
                                                    std::string_view table_name) {
    if (!expression)
        throw std::runtime_error("Binder::bind_expression: null expression");

    switch (expression->type()) {
    case UnboundExpressionType::COLUMN: {
        auto& column{static_cast<UnboundColumnExpression&>(*expression)};
        return std::make_unique<ColumnExpression>(resolve_column(column.ref(), schema, table_name));
    }
    case UnboundExpressionType::CONSTANT: {
        auto& constant{static_cast<UnboundConstantExpression&>(*expression)};
        return std::make_unique<ConstantExpression>(constant.value());
    }
    case UnboundExpressionType::COMPARISON: {
        auto& comparison{static_cast<UnboundComparisonExpression&>(*expression)};
        auto left{bind_expression(comparison.take_left(), schema, table_name)};
        auto right{bind_expression(comparison.take_right(), schema, table_name)};
        return std::make_unique<ComparisonExpression>(std::move(left), comparison.op(), std::move(right));
    }
    case UnboundExpressionType::BINARY: {
        auto& binary{static_cast<UnboundBinaryExpression&>(*expression)};
        auto left{bind_expression(binary.take_left(), schema, table_name)};
        auto right{bind_expression(binary.take_right(), schema, table_name)};
        return std::make_unique<BinaryExpression>(std::move(left), binary.op(), std::move(right));
    }
    case UnboundExpressionType::UNARY: {
        auto& unary{static_cast<UnboundUnaryExpression&>(*expression)};
        auto child{bind_expression(unary.take_expression(), schema, table_name)};
        return std::make_unique<UnaryExpression>(std::move(child), unary.op());
    }
    }

    throw std::runtime_error("Binder::bind_expression: unsupported expression type");
}

// should be vector<table_names> for joins
std::size_t Binder::resolve_column(const ColumnRef& column, const Schema& schema, std::string_view table_name) const {
    if (column.qualifier.has_value() && *column.qualifier != table_name)
        throw std::runtime_error(std::format("Binder::bind: table {} is not in the FROM clause", *column.qualifier));

    auto index{schema.column_index(column.name)};
    if (!index.has_value())
        throw std::runtime_error(std::format("Binder::bind: table {} has no column named {}", table_name, column.name));

    return *index;
}

} // namespace duck

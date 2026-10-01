#include "duck/query/binder.hpp"
#include "duck/expression/column.hpp"
#include "duck/expression/comparison.hpp"
#include "duck/expression/constant.hpp"
#include "duck/expression/logical.hpp"
#include "duck/plan/logical/filter.hpp"
#include "duck/plan/logical/join.hpp"
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
    if (!select_query.table().has_value())
        throw std::runtime_error("Binder::bind: FROM table not provided in select query");

    auto table{catalog_.get_table(select_query.table()->name, tx)};
    if (!table.has_value())
        throw std::runtime_error(std::format("Binder::bind: table {} doesn't exist", select_query.table()->name));

    std::unique_ptr<LogicalPlanNode> plan{std::make_unique<duck::LogicalScan>(*table.value())};

    const auto& schema{table.value()->schema()};
    BindingContext binding_context{
        TableBinding{select_query.table()->alias.value_or(select_query.table()->name), &schema, 0}};

    // Join
    if (auto joins{select_query.take_joins()}; joins.size() > 0) {
        for (auto& clause : joins) {
            auto join_table{catalog_.get_table(clause.table.name, tx)};
            if (!join_table.has_value())
                throw std::runtime_error(std::format("Binder::bind: join table {} doesn't exist", clause.table.name));

            std::unique_ptr<LogicalPlanNode> join_scan{std::make_unique<duck::LogicalScan>(*join_table.value())};

            binding_context.push_back({clause.table.alias.value_or(clause.table.name), &join_table.value()->schema(),
                                       plan->output_schema().column_count()});

            auto bound_predicate{bind_expression(std::move(clause.on), binding_context)};

            plan = std::make_unique<LogicalJoin>(std::move(plan), std::move(join_scan), std::move(bound_predicate));
        }
    }

    // Filter
    if (auto predicate{select_query.take_predicate()}; predicate) {
        auto bound_predicate{bind_expression(std::move(predicate), binding_context)};
        plan = std::make_unique<duck::LogicalFilterPlan>(std::move(plan), std::move(bound_predicate));
    }

    // Projection
    if (const auto& columns{select_query.columns()}; columns.has_value()) {
        if (columns->empty())
            throw std::runtime_error("Binder::bind: columns not specified");

        std::vector<std::size_t> column_idxs;
        column_idxs.reserve(columns->size());

        for (const auto& column : *columns)
            column_idxs.push_back(resolve_column(column, binding_context));

        plan = std::make_unique<duck::LogicalProjection>(std::move(plan), std::move(column_idxs));
    }

    // Limit
    if (auto limit{select_query.limit()}; limit.has_value()) {
        plan = std::make_unique<LogicalLimit>(std::move(plan), *limit);
    }

    return plan;
}

std::unique_ptr<Expression> Binder::bind_expression(std::unique_ptr<UnboundExpression> expression,
                                                    const BindingContext& context) {
    if (!expression)
        throw std::runtime_error("Binder::bind_expression: null expression");

    switch (expression->type()) {
    case UnboundExpressionType::COLUMN: {
        auto& column{static_cast<UnboundColumnExpression&>(*expression)};
        return std::make_unique<ColumnExpression>(resolve_column(column.ref(), context));
    }
    case UnboundExpressionType::CONSTANT: {
        auto& constant{static_cast<UnboundConstantExpression&>(*expression)};
        return std::make_unique<ConstantExpression>(constant.value());
    }
    case UnboundExpressionType::COMPARISON: {
        auto& comparison{static_cast<UnboundComparisonExpression&>(*expression)};
        auto left{bind_expression(comparison.take_left(), context)};
        auto right{bind_expression(comparison.take_right(), context)};
        return std::make_unique<ComparisonExpression>(std::move(left), comparison.op(), std::move(right));
    }
    case UnboundExpressionType::BINARY: {
        auto& binary{static_cast<UnboundBinaryExpression&>(*expression)};
        auto left{bind_expression(binary.take_left(), context)};
        auto right{bind_expression(binary.take_right(), context)};
        return std::make_unique<BinaryExpression>(std::move(left), binary.op(), std::move(right));
    }
    case UnboundExpressionType::UNARY: {
        auto& unary{static_cast<UnboundUnaryExpression&>(*expression)};
        auto child{bind_expression(unary.take_expression(), context)};
        return std::make_unique<UnaryExpression>(std::move(child), unary.op());
    }
    }

    throw std::runtime_error("Binder::bind_expression: unsupported expression type");
}

std::size_t Binder::resolve_column(const ColumnRef& column, const BindingContext& context) const {
    if (!column.qualifier.has_value() && context.size() > 1)
        throw std::runtime_error("Binder::resolve_column: column in multiple table query must have qualifer");

    std::size_t match{0};
    std::size_t matches{0};
    for (const auto& binding : context) {
        if (column.qualifier.has_value() && *column.qualifier != binding.name)
            continue;

        auto index{binding.schema->column_index(column.name)};
        if (!index.has_value())
            throw std::runtime_error(
                std::format("Binder::resolve_column: table {} has no column named {}", binding.name, column.name));

        if (matches > 0)
            throw std::runtime_error(std::format("Binder::resolve_column: ambiguous column named {}", column.name));

        match = binding.offset + *index;
        matches++;
    }

    if (matches == 0) {
        if (column.qualifier.has_value())
            throw std::runtime_error(std::format("Binder: table {} is not in the FROM clause", *column.qualifier));

        throw std::runtime_error(std::format("Binder: column {} doesn't exist", column.name));
    }

    return match;
}

} // namespace duck

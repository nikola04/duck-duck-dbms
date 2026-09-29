#include "duck/query/binder.hpp"
#include "duck/plan/logical/filter.hpp"
#include "duck/plan/logical/projection.hpp"
#include "duck/plan/logical/scan.hpp"
#include <cstddef>
#include <format>
#include <stdexcept>
#include <vector>
namespace duck {

std::unique_ptr<LogicalPlanNode> Binder::bind(SelectQuery& select_query) {
    auto table{catalog_.get_table(select_query.table())};
    if (!table.has_value())
        throw std::runtime_error(std::format("Binder::bind: table {} doesn't exist", select_query.table()));

    std::unique_ptr<LogicalPlanNode> plan{std::make_unique<duck::LogicalScan>(*table.value())};

    // Predicate, TODO: make abstraction of expressions to use column names instead of indexes
    if (auto predicate{select_query.take_predicate()}; predicate) {
        plan = std::make_unique<duck::LogicalFilterPlan>(std::move(plan), std::move(predicate));
    }

    // Projection
    if (auto columns{select_query.columns()}; columns.size() != 1 || columns[0] != "*") {
        if (columns.size() == 0)
            throw std::runtime_error("Binder::bind: columns not specified");

        std::vector<std::size_t> column_idxs;
        column_idxs.reserve(columns.size());

        const auto& schema{table.value()->schema()};
        for (const auto& column : columns) {
            auto idx{schema.column_index(column)};
            if (!idx.has_value())
                throw std::runtime_error(
                    std::format("Binder::bind: table {} has no column named {}", select_query.table(), column));
            column_idxs.push_back(*idx);
        }

        plan = std::make_unique<duck::LogicalProjection>(std::move(plan), std::move(column_idxs));
    }

    return plan;
}

} // namespace duck
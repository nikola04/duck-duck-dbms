#include "duck/execution/operator/projection/projection.hpp"
#include "duck/tuple/schema.hpp"
#include "duck/tuple/value.hpp"
#include <cstddef>
#include <optional>
#include <vector>

namespace duck {

ProjectionOperator::ProjectionOperator(std::unique_ptr<Operator> child, std::vector<std::size_t> columns)
    : child_(std::move(child)), columns_(std::move(columns)),
      schema_(Schema::projected(child_->output_schema(), columns_)) {};

void ProjectionOperator::init() {
    child_->init();
}
std::optional<Record> ProjectionOperator::next() {
    if (auto record{child_->next()}; record.has_value()) {
        std::vector<Value> values;
        values.reserve(columns_.size());

        for (std::size_t idx : columns_) {
            values.push_back(record->get(idx));
        }
        return Record{std::move(values)};
    }

    return std::nullopt;
}

const Schema& ProjectionOperator::output_schema() const {
    return schema_;
}

} // namespace duck
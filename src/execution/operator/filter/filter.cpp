#include "duck/execution/operator/filter/filter.hpp"
#include "duck/tuple/value.hpp"
#include <cassert>
#include <optional>

namespace duck {

void FilterOperator::init() {
    child_->init();
}

std::optional<Record> FilterOperator::next() {
    while (auto record{child_->next()}) {
        auto comp{comparator_->evaluate(*record)};
        assert(comp.type() == ValueType::BOOL);

        if (comp.is_null() || !comp.as_bool()) // skip unknwon and false
            continue;

        return record;
    }
    return std::nullopt;
}

const Schema& FilterOperator::output_schema() const {
    return child_->output_schema();
}

} // namespace duck
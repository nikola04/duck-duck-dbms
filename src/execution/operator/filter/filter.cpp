#include "duck/execution/operator/filter/filter.hpp"
#include <optional>

namespace duck {

void FilterOperator::init() {
    child_->init();
}

std::optional<Record> FilterOperator::next() {
    while (auto record{child_->next()}) {
        auto comp{comparator_->evaluate(*record)};
        if (comp.is_null() || !comp.as_bool()) // skip unknwon and false
            continue;

        return record;
    }
    return std::nullopt;
}

} // namespace duck
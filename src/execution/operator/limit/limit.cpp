#include "duck/execution/operator/limit/limit.hpp"
#include <optional>
namespace duck {

void LimitOperator::init() {
    child_->init();
}

void LimitOperator::reset() {
    child_->reset();
    produced_ = 0;
}

std::optional<Record> LimitOperator::next() {
    if (produced_ >= limit_)
        return std::nullopt;

    produced_++;
    return child_->next();
}

const Schema& LimitOperator::output_schema() const {
    return child_->output_schema();
}

} // namespace duck
#pragma once

#include "duck/execution/operator/operator.hpp"
#include <cstddef>
#include <memory>

namespace duck {

class LimitOperator : public Operator {
public:
    LimitOperator(std::unique_ptr<Operator> child, std::size_t limit)
        : child_(std::move(child)), limit_(limit), produced_(0) {};

    void init() override;
    void reset() override;
    std::optional<Record> next() override;

    const Schema& output_schema() const override;

private:
    std::unique_ptr<Operator> child_;
    std::size_t limit_;
    std::size_t produced_;
};

} // namespace duck
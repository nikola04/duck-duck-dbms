#pragma once

#include "duck/execution/expression/expression.hpp"
#include "duck/execution/operator/operator.hpp"
#include <memory>

namespace duck {

class FilterOperator : public Operator {
public:
    explicit FilterOperator(std::unique_ptr<Operator> child, std::unique_ptr<Expression> comp)
        : child_(std::move(child)), comparator_(std::move(comp)) {};

    void init() override;
    std::optional<Record> next() override;

private:
    std::unique_ptr<Operator> child_;
    std::unique_ptr<Expression> comparator_;
};

} // namespace duck
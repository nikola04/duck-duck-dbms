#pragma once

#include "duck/execution/operator/operator.hpp"
#include "duck/tuple/schema.hpp"
#include <cstddef>
#include <memory>
#include <vector>
namespace duck {

class ProjectionOperator : public Operator {
public:
    ProjectionOperator(std::unique_ptr<Operator> child, std::vector<std::size_t> columns);

    void init() override;
    std::optional<Record> next() override;

    const Schema& output_schema() const override;

private:
    std::unique_ptr<Operator> child_;
    std::vector<std::size_t> columns_;
    Schema schema_;
};

} // namespace duck
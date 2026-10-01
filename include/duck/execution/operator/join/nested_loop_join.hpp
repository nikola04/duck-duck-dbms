#pragma once

#include "duck/execution/operator/operator.hpp"
#include "duck/expression/expression.hpp"
#include "duck/record/record.hpp"
#include "duck/tuple/schema.hpp"
#include <cstddef>
#include <memory>
#include <optional>
#include <vector>
namespace duck {

class NestedLoopJoin : public Operator {
public:
    explicit NestedLoopJoin(std::unique_ptr<Operator> left, std::unique_ptr<Operator> right,
                            std::unique_ptr<Expression> predicate, Schema schema)
        : left_(std::move(left)), right_(std::move(right)), predicate_(std::move(predicate)),
          schema_(std::move(schema)) {};

    void init() override;
    void reset() override;
    std::optional<Record> next() override;

    const Schema& output_schema() const override;

private:
    std::unique_ptr<Operator> left_;
    std::unique_ptr<Operator> right_;
    std::unique_ptr<Expression> predicate_;
    Schema schema_;

    std::vector<Record> left_block_, right_block_;
    std::size_t left_it_{0}, right_it_{0};

    static std::vector<Record> fetch_block(Operator& op);
    static const std::size_t kBLOCK_SIZE{24};
};

} // namespace duck
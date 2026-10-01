#include "duck/execution/operator/join/nested_loop_join.hpp"
#include "duck/record/record.hpp"
#include <cassert>
#include <cstddef>
#include <optional>
#include <vector>
namespace duck {

void NestedLoopJoin::init() {
    left_->init();
    right_->init();
}

void NestedLoopJoin::reset() {
    left_->reset();
    right_->reset();
    left_block_.clear();
    right_block_.clear();
    left_it_ = 0;
    right_it_ = 0;
}

std::optional<Record> NestedLoopJoin::next() {
    while (true) {
        // Load one block from the left side. The right side must be reset
        // only once for this whole left block.
        if (left_block_.empty()) {
            left_block_ = fetch_block(*left_);
            if (left_block_.empty())
                return std::nullopt;

            left_it_ = 0;
            right_->reset();
            right_block_.clear();
            right_it_ = 0;
        }

        // Once the current right block is exhausted, fetch the next one.
        if (right_it_ >= right_block_.size()) {
            right_block_ = fetch_block(*right_);
            right_it_ = 0;

            // The right side is exhausted for the current left block.
            // Drop the left block so the next iteration loads a new one.
            if (right_block_.empty()) {
                left_block_.clear();
                continue;
            }
        }

        const auto& right_record{right_block_[right_it_]};

        // Compare this right-side record with every record in the left block.
        if (left_it_ < left_block_.size()) {
            const auto& left_record{left_block_[left_it_++]};
            auto record{left_record + right_record};

            auto comp{predicate_->evaluate(record)};
            assert(comp.type() == ValueType::BOOL);

            if (!comp.is_null() && comp.as_bool()) // skip unknown and false
                return record;

            continue;
        }

        // The current right record was compared with the whole left block.
        left_it_ = 0;
        ++right_it_;
    }
}

std::vector<Record> NestedLoopJoin::fetch_block(Operator& op) {
    std::vector<Record> block;
    block.reserve(kBLOCK_SIZE);

    for (std::size_t i{0}; i < kBLOCK_SIZE; ++i) {
        auto record{op.next()};
        if (!record.has_value())
            break;

        block.push_back(std::move(*record));
    }

    return block;
}

const Schema& NestedLoopJoin::output_schema() const {
    return schema_;
}

} // namespace duck

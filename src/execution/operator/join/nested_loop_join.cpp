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
    if (left_block_.empty()) {
        left_block_ = fetch_block(*left_);
        left_it_ = 0;
    }

    while (true) {
        if (left_block_.empty()) { // end
            break;
        }

        while (left_it_ < left_block_.size()) {
            auto l_record{left_block_[left_it_]};

            if (right_block_.empty()) {
                right_block_ = fetch_block(*right_);
                right_it_ = 0;
            }

            while (true) {
                if (right_block_.empty()) {
                    break;
                }

                while (right_it_ < right_block_.size()) {
                    auto r_record{right_block_[right_it_++]};

                    auto record{l_record + r_record};

                    auto comp{predicate_->evaluate(record)};
                    assert(comp.type() == ValueType::BOOL);

                    if (!comp.is_null() && comp.as_bool()) // skip unknwon and false
                        return record;
                }

                right_block_ = fetch_block(*right_);
                right_it_ = 0;
            }
            right_->reset();
            right_block_.clear();
            left_it_++;
        }
        left_block_ = fetch_block(*left_);
        left_it_ = 0;
    }

    return std::nullopt;
}

std::vector<Record> NestedLoopJoin::fetch_block(Operator& op) {
    std::vector<Record> block;
    block.reserve(kBLOCK_SIZE);

    for (std::size_t i{0}; i < kBLOCK_SIZE; ++i) {
        auto record{op.next()};
        if (!record.has_value())
            break;

        block.push_back(record.value());
    }

    return block;
}

const Schema& NestedLoopJoin::output_schema() const {
    return schema_;
}

} // namespace duck
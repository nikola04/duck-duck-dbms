#include "duck/execution/operator/join/nested_loop_join.hpp"
#include "duck/expression/column.hpp"
#include "duck/expression/comparison.hpp"
#include "duck/tuple/column.hpp"
#include "duck/tuple/schema.hpp"

#include <cstddef>
#include <cstdint>
#include <gtest/gtest.h>
#include <memory>
#include <optional>
#include <vector>

namespace {

class VectorOperator : public duck::Operator {
public:
    VectorOperator(std::vector<duck::Record> records, duck::Schema schema)
        : records_(std::move(records)), schema_(std::move(schema)) {}

    void init() override {
        current_ = 0;
    }

    void reset() override {
        current_ = 0;
        ++reset_count_;
    }

    std::optional<duck::Record> next() override {
        if (current_ >= records_.size())
            return std::nullopt;

        return records_[current_++];
    }

    const duck::Schema& output_schema() const override {
        return schema_;
    }

    std::size_t reset_count() const {
        return reset_count_;
    }

private:
    std::vector<duck::Record> records_;
    duck::Schema schema_;
    std::size_t current_{0};
    std::size_t reset_count_{0};
};

duck::Schema MakeJoinSchema(std::string name) {
    return duck::Schema({duck::Column{std::move(name), duck::TypeId::INT32}});
}

std::vector<duck::Record> MakeRecords(std::size_t count) {
    std::vector<duck::Record> records;
    records.reserve(count);

    for (std::size_t i{0}; i < count; ++i) {
        records.emplace_back(std::vector<duck::Value>{duck::Value::of(static_cast<std::int32_t>(i))});
    }

    return records;
}

std::unique_ptr<duck::Expression> MakeEqualityPredicate() {
    return std::make_unique<duck::ComparisonExpression>(
        std::make_unique<duck::ColumnExpression>(0), duck::ComparisonOperator::EQUAL,
        std::make_unique<duck::ColumnExpression>(1));
}

} // namespace

TEST(NestedLoopJoinTest, MatchesRowsAcrossMultipleBlocks) {
    auto left_schema = MakeJoinSchema("left_id");
    auto right_schema = MakeJoinSchema("right_id");
    auto output_schema = left_schema + right_schema;

    auto left = std::make_unique<VectorOperator>(MakeRecords(50), left_schema);
    auto right = std::make_unique<VectorOperator>(MakeRecords(50), right_schema);
    auto* right_operator = right.get();

    duck::NestedLoopJoin join{std::move(left), std::move(right), MakeEqualityPredicate(), std::move(output_schema)};
    join.init();

    std::size_t matches{0};
    while (auto record{join.next()}) {
        ASSERT_EQ(record->size(), 2u);
        EXPECT_EQ(record->get(0).as_int32(), record->get(1).as_int32());
        ++matches;
    }

    // 50 rows force multiple left and right blocks (the block size is 24).
    EXPECT_EQ(matches, 50u);
    // The right side is reset once for each left block, not once per left row.
    EXPECT_EQ(right_operator->reset_count(), 3u);
}

TEST(NestedLoopJoinTest, ResetStartsTheJoinFromTheBeginning) {
    auto left_schema = MakeJoinSchema("left_id");
    auto right_schema = MakeJoinSchema("right_id");
    auto output_schema = left_schema + right_schema;

    auto left = std::make_unique<VectorOperator>(MakeRecords(3), left_schema);
    auto right = std::make_unique<VectorOperator>(MakeRecords(3), right_schema);

    duck::NestedLoopJoin join{std::move(left), std::move(right), MakeEqualityPredicate(), std::move(output_schema)};
    join.init();

    ASSERT_TRUE(join.next().has_value());
    join.reset();

    std::size_t matches{0};
    while (auto record{join.next()}) {
        ASSERT_EQ(record->size(), 2u);
        EXPECT_EQ(record->get(0).as_int32(), record->get(1).as_int32());
        ++matches;
    }

    EXPECT_EQ(matches, 3u);
}

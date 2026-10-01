#pragma once

#include "duck/tuple/column.hpp"
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace duck {

// [column_count..2 bytes] [COLUMN...]
class Schema {
public:
    explicit Schema(std::vector<Column> columns);
    Schema(std::span<const std::byte> raw) : columns_(deserialize(raw)) {};

    const Column& column(size_t index) const {
        return columns_.at(index);
    }
    // when schemas are merged this method should not be used
    std::optional<size_t> column_index(std::string_view name) const;

    size_t column_count() const {
        return columns_.size();
    }

    std::uint16_t fixed_size_of(size_t index) const;
    bool compatible_with(const Schema& other) const;

    std::vector<std::byte> serialize() const;

    std::string to_string() const; // debug method

    Schema operator+(const Schema& schema) const;
    static Schema projected(const Schema& schema, const std::vector<std::size_t>& column_idxs);

private:
    std::vector<Column> columns_;

    static std::vector<Column> deserialize(std::span<const std::byte> raw);
};

} // namespace duck
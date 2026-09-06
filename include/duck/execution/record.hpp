#pragma once

#include "duck/tuple/value.hpp"
#include <cstddef>
#include <vector>
namespace duck {

class Record {
public:
    Record(std::vector<Value> values) : values_(std::move(values)) {};

    Value get(std::size_t index) const {
        return values_.at(index);
    }

    std::size_t size() const {
        return values_.size();
    }

private:
    std::vector<Value> values_;
};

} // namespace duck
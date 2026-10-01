#pragma once

#include "duck/tuple/value.hpp"
#include <cstddef>
#include <string>
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

    std::string to_string() const {
        std::string s;
        for (auto& v : values_) {
            s += " | " + v.to_string();
        }

        return s;
    }

    Record operator+(const Record& other) {
        auto values{std::vector{this->values_}};
        values.insert(values.end(), other.values_.begin(), other.values_.end());

        return Record{values};
    }

private:
    std::vector<Value> values_;
};

} // namespace duck
#pragma once

#include "types.hpp"
#include <cstddef>
#include <cstdint>
#include <functional>

namespace duck {

struct RID {
    PageID page_id;
    std::uint16_t slot_num;

    bool operator==(const RID&) const = default;
};

} // namespace duck

namespace std {
template <> struct hash<duck::RID> {
    size_t operator()(const duck::RID& rid) const noexcept {
        size_t h1{std::hash<duck::PageID>{}(rid.page_id)};
        size_t h2{std::hash<uint16_t>{}(rid.slot_num)};

        return h1 ^ (h2 << 1);
    }
};
} // namespace std
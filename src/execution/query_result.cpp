#include "duck/execution/query_result.hpp"

namespace duck {

std::optional<Record> QueryResult::next() {
    return root_.next();
}

} // namespace duck
#include "duck/expression/column.hpp"

namespace duck {

Value ColumnExpression::evaluate(const Record& record) const {
    return record.get(index_);
}

} // namespace duck
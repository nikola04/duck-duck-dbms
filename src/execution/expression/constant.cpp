#include "duck/execution/expression/constant.hpp"

namespace duck {

Value ConstantExpression::evaluate(const Record&) const {
    return value_;
}

} // namespace duck
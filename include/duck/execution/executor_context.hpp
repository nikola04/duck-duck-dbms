#pragma once

#include "duck/transaction/transaction.hpp"

namespace duck {

struct ExecutorContext {
    Transaction* tx;
};

} // namespace duck
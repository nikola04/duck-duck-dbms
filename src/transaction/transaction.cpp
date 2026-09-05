#include "duck/transaction/transaction.hpp"

namespace duck {

void Transaction::undo() {
    for (auto it{undo_log_.rbegin()}; it != undo_log_.rend(); ++it) {
        it->get()->undo();
    }
}

} // namespace duck
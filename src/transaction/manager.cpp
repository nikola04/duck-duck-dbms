#include "duck/transaction/manager.hpp"
#include "duck/common/types.hpp"
#include "duck/transaction/transaction.hpp"
#include <memory>
#include <mutex>
#include <utility>

namespace duck {

Transaction* TransactionManager::begin() {
    TransactionID id{next_tx_id_.fetch_add(1)};
    auto tx{std::make_unique<Transaction>(id)};
    Transaction* tx_ptr{tx.get()};

    std::unique_lock lock{latch_};
    active_txs_[id] = std::move(tx);

    return tx_ptr;
}

void TransactionManager::commit(Transaction* tx) {
}
void TransactionManager::abort(Transaction* tx) {
}

} // namespace duck
#include "duck/transaction/manager.hpp"
#include "duck/common/types.hpp"
#include "duck/transaction/transaction.hpp"
#include <cassert>
#include <memory>
#include <mutex>
#include <utility>

namespace duck {

std::shared_ptr<Transaction> TransactionManager::begin() {
    TransactionID id{next_tx_id_.fetch_add(1)};
    auto tx{std::make_shared<Transaction>(id)};

    std::unique_lock lock{latch_};
    active_txs_[id] = std::move(tx);

    return active_txs_[id];
}

void TransactionManager::commit(Transaction* tx) {
    assert(tx != nullptr);

    tx->set_state(TransactionState::SHRINKING);
    lock_manager_.unlock_all(tx);
    tx->set_state(TransactionState::COMMITTED);

    std::unique_lock lock{latch_};
    active_txs_.erase(tx->id());
}
void TransactionManager::abort(Transaction* tx) {
    assert(tx != nullptr);

    tx->set_state(TransactionState::SHRINKING);
    tx->undo();
    lock_manager_.unlock_all(tx);
    tx->set_state(TransactionState::ABORTED);

    std::unique_lock lock{latch_};
    active_txs_.erase(tx->id());
}

} // namespace duck
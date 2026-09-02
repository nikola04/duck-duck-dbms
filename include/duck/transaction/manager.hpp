#pragma once

#include "../common/types.hpp"
#include "lock_manager.hpp"
#include "transaction.hpp"
#include <atomic>
#include <memory>
#include <shared_mutex>
#include <unordered_map>

namespace duck {

class TransactionManager {
public:
    TransactionManager(LockManager& lock_manager) : lock_manager_(lock_manager) {
    }

    Transaction* begin();
    void commit(Transaction* tx);
    void abort(Transaction* tx);

private:
    LockManager& lock_manager_;

    std::atomic<TransactionID> next_tx_id_{0};
    std::unordered_map<TransactionID, std::unique_ptr<Transaction>> active_txs_;

    std::shared_mutex latch_;
};

} // namespace duck
#include "duck/transaction/lock_manager.hpp"
#include "duck/transaction/transaction.hpp"
#include <chrono>
#include <mutex>

namespace duck {

bool LockManager::lock_shared(Transaction* tx, RID rid) {
    std::unique_lock lock{latch_};

    if (tx == nullptr || tx->state() != TransactionState::GROWING)
        return false;

    if (tx->shared_locks_.contains(rid) || tx->exclusive_locks_.contains(rid))
        return true;

    auto& queue{lock_table_[rid]};
    LockRequest request{tx->id(), LockMode::SHARED, false};
    auto it{queue.requests.insert(queue.requests.end(), request)};

    if (!queue.cv.wait_for(lock, std::chrono::seconds(1), [&] { return can_grant(queue, request); })) {
        queue.requests.erase(it);
        tx->set_state(TransactionState::ABORTED);
        queue.cv.notify_all();
        return false;
    }

    it->granted = true;
    tx->shared_locks_.insert(rid);
    return true;
}

bool LockManager::lock_exclusive(Transaction* tx, RID rid) {
    std::unique_lock lock{latch_};

    if (tx == nullptr || tx->state() != TransactionState::GROWING)
        return false;

    if (tx->exclusive_locks_.contains(rid))
        return true;

    auto& queue{lock_table_[rid]};

    if (tx->shared_locks_.contains(rid)) { // lock upgrade
        std::erase_if(queue.requests, [&](const LockRequest& req) { return req.tx_id == tx->id(); });
        tx->shared_locks_.erase(rid);
    }

    LockRequest request{tx->id(), LockMode::EXCLUSIVE, false};
    auto it{queue.requests.insert(queue.requests.end(), request)};

    if (!queue.cv.wait_for(lock, std::chrono::seconds(1), [&] { return can_grant(queue, request); })) {
        queue.requests.erase(it);
        tx->set_state(TransactionState::ABORTED);
        queue.cv.notify_all();
        return false;
    }

    it->granted = true;
    tx->exclusive_locks_.insert(rid);
    return true;
}

void LockManager::unlock_all(Transaction* tx) {
    std::unique_lock lock{latch_};

    if (tx == nullptr)
        return;

    for (RID rid : tx->exclusive_locks_)
        remove_and_notify(rid, tx->id());

    for (RID rid : tx->shared_locks_)
        remove_and_notify(rid, tx->id());

    tx->exclusive_locks_.clear();
    tx->shared_locks_.clear();
}

void LockManager::remove_and_notify(RID rid, TransactionID tx_id) {
    auto it{lock_table_.find(rid)};
    if (it == lock_table_.end())
        return;

    auto& queue{it->second};
    std::erase_if(queue.requests, [&](const LockRequest& req) { return req.tx_id == tx_id; });

    queue.cv.notify_all();
}

bool LockManager::can_grant(const LockRequestQueue& queue, const LockRequest& request) {
    for (auto it{queue.requests.begin()}; it != queue.requests.end(); ++it) {
        if (!it->granted || it->tx_id == request.tx_id)
            continue;

        if (request.lock_mode == LockMode::EXCLUSIVE)
            return false;
        if (it->lock_mode == LockMode::EXCLUSIVE)
            return false;
    }
    return true;
}

} // namespace duck
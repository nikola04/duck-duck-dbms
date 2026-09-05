#pragma once

#include "transaction.hpp"
#include <condition_variable>
#include <list>
#include <mutex>
#include <unordered_map>
namespace duck {

enum class LockMode { SHARED, EXCLUSIVE };

class LockManager {
public:
    bool lock_shared(Transaction* tx, RID rid);
    bool lock_exclusive(Transaction* tx, RID rid);

    void unlock_all(Transaction* tx);

private:
    std::mutex latch_;

    struct LockRequest {
        TransactionID tx_id;
        LockMode lock_mode;
        bool granted;
    };

    struct LockRequestQueue {
        std::list<LockRequest> requests;
        std::condition_variable cv;
    };

    std::unordered_map<RID, LockRequestQueue> lock_table_;

    bool can_grant(const LockRequestQueue& queue, const LockRequest& request);
    void mark_granted(LockRequestQueue& queue, TransactionID tx_id);
    void remove_and_notify(RID rid, TransactionID tx_id);
};

} // namespace duck
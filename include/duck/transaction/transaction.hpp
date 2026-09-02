#pragma once

#include "../common/rid.hpp"
#include "../common/types.hpp"
#include <unordered_set>

namespace duck {

enum class TransactionState { GROWING, SHRINKING, COMMITED, ABORTED };

class Transaction {
public:
    Transaction(TransactionID id) : id_(id), state_(TransactionState::GROWING) {
    }

    TransactionID id() const {
        return id_;
    }
    TransactionState state() const {
        return state_;
    }

    void set_state(TransactionState state) {
        state_ = state;
    }

    std::unordered_set<RID> shared_locks_;
    std::unordered_set<RID> exclusive_locks_;

private:
    TransactionID id_;
    TransactionState state_;
};

} // namespace duck
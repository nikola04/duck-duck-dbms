#pragma once

#include "../common/rid.hpp"
#include "../common/types.hpp"
#include <cstddef>
#include <memory>
#include <unordered_set>
#include <vector>

namespace duck {

struct UndoRecord {
    virtual ~UndoRecord() = default;
    virtual void undo() = 0;
};

enum class TransactionState { GROWING, SHRINKING, COMMITTED, ABORTED };

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

    void push_undo(std::unique_ptr<UndoRecord> record) {
        undo_log_.push_back(std::move(record));
    }
    void undo();

    std::unordered_set<RID> shared_locks_;
    std::unordered_set<RID> exclusive_locks_;
    std::vector<std::unique_ptr<UndoRecord>> undo_log_;

private:
    TransactionID id_;
    TransactionState state_;
};

} // namespace duck
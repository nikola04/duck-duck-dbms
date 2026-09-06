#pragma once

#include "duck/common/rid.hpp"
#include "duck/table/table_heap.hpp"
#include "duck/transaction/lock_manager.hpp"
#include "duck/transaction/transaction.hpp"
#include "duck/tuple/schema.hpp"
#include "duck/tuple/tuple.hpp"
#include <cstddef>
#include <optional>
#include <string_view>
#include <utility>

namespace duck {

enum class DropTableStatus { READ_PAGES_FAILED, SUCCESS };

class Table {
public:
    explicit Table(std::string name, TableHeap table_heap, const Schema& schema, LockManager& lock_manager);

    std::optional<RID> insert_tuple(const Tuple& tuple, Transaction* tx = nullptr);
    std::optional<Tuple> get_tuple(RID rid, Transaction* tx = nullptr);
    std::optional<RID> update_tuple(RID rid, const Tuple& tuple, Transaction* tx = nullptr);
    bool delete_tuple(RID rid, Transaction* tx = nullptr);

    std::pair<size_t, DropTableStatus> drop_pages();

    std::string_view name() const {
        return name_;
    }
    const Schema& schema() const {
        return schema_;
    }
    TableHeap* table_heap() {
        return &table_heap_;
    }

    class Scan;
    Scan scan(Transaction* tx = nullptr) const;

private:
    std::string name_;

    TableHeap table_heap_;
    const Schema& schema_;

    LockManager& lock_manager_;
};

class Table::Scan {
public:
    Scan(TableHeap::Scan heap_scan, const Schema& schema, LockManager& lock_manager, Transaction* tx)
        : heap_scan_(std::move(heap_scan)), schema_(schema), lock_manager_(lock_manager), tx_(tx) {};

    std::optional<std::pair<RID, Tuple>> next();

private:
    TableHeap::Scan heap_scan_;
    const Schema& schema_;
    LockManager& lock_manager_;
    Transaction* tx_;
};

} // namespace duck

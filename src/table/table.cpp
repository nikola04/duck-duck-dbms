#include "duck/table/table.hpp"
#include "duck/common/types.hpp"
#include "duck/table/table_heap.hpp"
#include "duck/transaction/lock_manager.hpp"
#include "duck/transaction/undo.hpp"
#include "duck/tuple/tuple.hpp"
#include <cstddef>
#include <memory>
#include <optional>
#include <stdexcept>

namespace duck {

Table::Table(std::string name, TableHeap table_heap, const Schema& schema, LockManager& lock_manager)
    : name_(std::move(name)), table_heap_(std::move(table_heap)), schema_(schema), lock_manager_(lock_manager) {
}

std::optional<RID> Table::insert_tuple(const Tuple& tuple, Transaction* tx) {
    if (!schema_.compatible_with(tuple.schema()))
        throw std::runtime_error("Table::insert_tuple: schemas not compatible");

    auto rid{table_heap_.insert_tuple(tuple.serialize())};
    if (!rid.has_value())
        return std::nullopt;

    if (tx != nullptr && !lock_manager_.lock_exclusive(tx, *rid)) {
        table_heap_.delete_tuple(*rid); // rollback, but insert could be overwritten...
        return std::nullopt;
    }

    if (tx != nullptr)
        tx->push_undo(std::make_unique<DeleteUndoRecord>(this, *rid));

    return rid;
}

std::optional<Tuple> Table::get_tuple(RID rid, Transaction* tx) {
    if (tx != nullptr && !lock_manager_.lock_shared(tx, rid))
        return std::nullopt;

    auto bytes{table_heap_.get_tuple(rid)};
    if (!bytes.has_value())
        return std::nullopt;

    return Tuple{bytes.value(), schema_};
}

bool Table::delete_tuple(RID rid, Transaction* tx) {
    if (tx != nullptr && !lock_manager_.lock_exclusive(tx, rid))
        return false;

    auto bytes{table_heap_.get_tuple(rid)};
    if (!bytes.has_value())
        return false;

    if (!table_heap_.delete_tuple(rid))
        return false;

    if (tx != nullptr)
        tx->push_undo(std::make_unique<RestoreUndoRecord>(this, rid, *bytes));

    return true;
}

std::optional<RID> Table::update_tuple(RID rid, const Tuple& tuple, Transaction* tx) {
    if (!schema_.compatible_with(tuple.schema()))
        throw std::runtime_error("Table::update_tuple: schemas not compatible");

    if (tx != nullptr && !lock_manager_.lock_exclusive(tx, rid))
        return std::nullopt;

    auto old_tuple_bytes{table_heap_.get_tuple(rid)};
    if (!old_tuple_bytes.has_value()) // no data to be updated
        return std::nullopt;

    auto ret_rid{table_heap_.update_tuple(rid, tuple.serialize())};
    if (!ret_rid.has_value())
        return std::nullopt;

    if (*ret_rid != rid && tx != nullptr) {
        if (!lock_manager_.lock_exclusive(tx, *ret_rid)) // same issue as insert, rid is unknown
            return std::nullopt;
    }

    if (tx != nullptr) {
        tx->push_undo(std::make_unique<RestoreUndoRecord>(this, rid, *old_tuple_bytes));
        if (*ret_rid != rid)
            tx->push_undo(std::make_unique<DeleteUndoRecord>(this, *ret_rid));
        else
            tx->push_undo(std::make_unique<DeleteUndoRecord>(this, rid));
    }

    return ret_rid;
}

std::pair<size_t, DropTableStatus> Table::drop_pages() {
    size_t failed{0};

    auto [page_ids, status]{table_heap_.all_pages()};
    if (status == TableHeapFetchStatus::FAILED) // because we cannot be sure how many pages are left to fail we abort
        return {0, DropTableStatus::READ_PAGES_FAILED};

    for (PageID page_id : page_ids) {
        if (!table_heap_.drop_page(page_id))
            failed++;
    }

    return {failed, DropTableStatus::SUCCESS};
}

Table::Scan Table::scan() const {
    return Table::Scan{table_heap_.scan(), schema_};
}

std::optional<std::pair<RID, Tuple>> Table::Scan::next() {
    auto result{heap_scan_.next()};
    if (!result.has_value())
        return std::nullopt;

    return std::pair{result->first, Tuple{std::move(result->second), schema_}};
}

} // namespace duck

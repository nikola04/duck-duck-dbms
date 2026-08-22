#include "duck/table/table.hpp"
#include "duck/common/types.hpp"
#include "duck/table/table_heap.hpp"
#include "duck/tuple/tuple.hpp"
#include <cstddef>
#include <optional>
#include <stdexcept>

namespace duck {

Table::Table(std::string name, TableHeap table_heap, const Schema& schema)
    : name_(std::move(name)), table_heap_(std::move(table_heap)), schema_(schema) {
}

std::optional<RID> Table::insert_tuple(const Tuple& tuple) {
    if (!schema_.compatible_with(tuple.schema()))
        throw std::runtime_error("Table::insert_tuple: schemas not compatible");

    return table_heap_.insert_tuple(tuple.serialize());
}

std::optional<Tuple> Table::get_tuple(RID rid) {
    auto bytes{table_heap_.get_tuple(rid)};
    if (!bytes.has_value())
        return std::nullopt;

    return Tuple{bytes.value(), schema_};
}

bool Table::delete_tuple(RID rid) {
    return table_heap_.delete_tuple(rid);
}

std::optional<RID> Table::update_tuple(RID rid, const Tuple& tuple) {
    if (!schema_.compatible_with(tuple.schema()))
        throw std::runtime_error("Table::update_tuple: schemas not compatible");

    return table_heap_.update_tuple(rid, tuple.serialize());
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

    return std::pair{result->first, Tuple{result->second, schema_}};
}

} // namespace duck
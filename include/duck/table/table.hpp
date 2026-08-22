#pragma once

#include "duck/common/rid.hpp"
#include "duck/table/table_heap.hpp"
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
    explicit Table(std::string name, TableHeap table_heap, const Schema& schema);

    std::optional<RID> insert_tuple(const Tuple& tuple);
    std::optional<Tuple> get_tuple(RID rid);
    std::optional<RID> update_tuple(RID rid, const Tuple& tuple);
    bool delete_tuple(RID rid);

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
    Scan scan() const;

private:
    std::string name_;

    TableHeap table_heap_;
    const Schema& schema_;
};

class Table::Scan {
public:
    Scan(TableHeap::Scan heap_scan, const Schema& schema) : heap_scan_(std::move(heap_scan)), schema_(schema) {
    }

    std::optional<std::pair<RID, Tuple>> next();

private:
    TableHeap::Scan heap_scan_;
    const Schema& schema_;
};

} // namespace duck
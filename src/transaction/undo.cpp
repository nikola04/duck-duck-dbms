#include "duck/transaction/undo.hpp"
#include "duck/common/rid.hpp"
#include <cstddef>
#include <vector>

namespace duck {

// ----- Tables -----
DeleteUndoRecord::DeleteUndoRecord(Table* table, RID rid) : table_(table), rid_(rid) {
}
void DeleteUndoRecord::undo() {
    table_->table_heap()->delete_tuple(rid_); // skip transaction on table layer
}

RestoreUndoRecord::RestoreUndoRecord(Table* table, RID rid, std::vector<std::byte> tuple)
    : table_(table), rid_(rid), tuple_(std::move(tuple)) {
}
void RestoreUndoRecord::undo() {
    table_->table_heap()->restore_tuple(rid_, tuple_);
}

// ----- Catalog -----
DropTableUndoRecord::DropTableUndoRecord(Catalog* catalog, std::string name)
    : catalog_(catalog), name_(std::move(name)) {
}
void DropTableUndoRecord::undo() {
    catalog_->delete_table(name_);
}

} // namespace duck
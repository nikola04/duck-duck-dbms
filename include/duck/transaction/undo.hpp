#pragma once

#include "duck/catalog/catalog.hpp"
#include "duck/common/rid.hpp"
#include "duck/table/table.hpp"
#include "duck/transaction/transaction.hpp"
#include <cstddef>
#include <vector>

namespace duck {

class DeleteUndoRecord : public UndoRecord {
public:
    DeleteUndoRecord(Table* table, RID rid);

    void undo() override;

private:
    Table* table_;
    RID rid_;
};

class RestoreUndoRecord : public UndoRecord {
public:
    RestoreUndoRecord(Table* table, RID rid, std::vector<std::byte> tuple);

    void undo() override;

private:
    Table* table_;
    RID rid_;
    std::vector<std::byte> tuple_;
};

class DropTableUndoRecord : public UndoRecord {
public:
    DropTableUndoRecord(Catalog* catalog, std::string name);

    void undo() override;

private:
    Catalog* catalog_;
    std::string name_;
};

} // namespace duck
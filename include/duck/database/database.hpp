#pragma once

#include "duck/buffer/pool_manager.hpp"
#include "duck/catalog/catalog.hpp"
#include "duck/storage/disk_manager.hpp"
#include "duck/transaction/lock_manager.hpp"
#include "duck/transaction/manager.hpp"
#include "duck/transaction/transaction.hpp"
#include "duck/tuple/schema.hpp"
#include <cstddef>
#include <memory>
#include <optional>
#include <string>

namespace duck {

class Database {
public:
    explicit Database(const std::string& path, size_t pool_size = 128);

    std::shared_ptr<Transaction> begin_tx();
    void commit_tx(Transaction* tx);
    void rollback_tx(Transaction* tx);

    Table* create_table(const std::string& table_name, Schema schema, Transaction* tx = nullptr);
    bool drop_table(const std::string& table_name);
    std::optional<Table*> get_table(const std::string& table_name) const;
    std::vector<Table*> all_tables() const;

private:
    DiskManager disk_manager_;
    BufferPoolManager pool_;
    LockManager lock_manager_;
    TransactionManager tx_manager_;
    Catalog catalog_;
};

} // namespace duck
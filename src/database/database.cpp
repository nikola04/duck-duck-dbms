#include "duck/database/database.hpp"

namespace duck {

Database::Database(const std::string& path, size_t pool_size)
    : disk_manager_(path), pool_(disk_manager_, pool_size), tx_manager_(lock_manager_),
      catalog_(pool_, disk_manager_, lock_manager_) {
}

std::shared_ptr<Transaction> Database::begin_tx() {
    return tx_manager_.begin();
}
void Database::commit_tx(Transaction* tx) {
    return tx_manager_.commit(tx);
}
void Database::rollback_tx(Transaction* tx) {
    return tx_manager_.abort(tx);
}

Table* Database::create_table(const std::string& table_name, Schema schema, Transaction* tx) {
    return catalog_.create_table(table_name, std::move(schema), tx);
}
bool Database::drop_table(const std::string& table_name) {
    return catalog_.drop_table(table_name);
}
std::optional<Table*> Database::get_table(const std::string& table_name) const {
    return catalog_.get_table(table_name);
}
std::vector<Table*> Database::all_tables() const {
    return catalog_.all_tables();
}

} // namespace duck
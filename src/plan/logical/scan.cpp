#include "duck/plan/logical/scan.hpp"

namespace duck {

LogicalScan::LogicalScan(Table& table) : table_(table) {};

Table& LogicalScan::table() const {
    return table_;
}

const Schema& LogicalScan::output_schema() const {
    return table_.schema();
};

} // namespace duck
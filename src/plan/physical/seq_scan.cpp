#include "duck/plan/physical/seq_scan.hpp"

namespace duck {

SequentialScanPlan::SequentialScanPlan(Table& table) : table_(table) {};

Table& SequentialScanPlan::table() const {
    return table_;
}

const Schema& SequentialScanPlan::output_schema() const {
    return table_.schema();
};

} // namespace duck
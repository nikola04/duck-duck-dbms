#include "duck/plan/physical/join/nested_loop_join.hpp"

namespace duck {

const Schema& NestedLoopJoinPlan::output_schema() const {
    return schema_;
}

} // namespace duck
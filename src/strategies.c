#include "strategies.h"

StrategyFn get_strategy(int strategy_type) {
    if (strategy_type == 0) return handle_kill;
    if (strategy_type == 1) return handle_retry;
    return handle_rollback;
}

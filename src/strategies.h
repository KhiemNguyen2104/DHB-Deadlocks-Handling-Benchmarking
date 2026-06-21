#ifndef STRATEGIES_H
#define STRATEGIES_H

struct Simulator;

typedef void (*StrategyFn)(const char* pid, struct Simulator* sim);

void handle_kill(const char* pid, struct Simulator* sim);
void handle_retry(const char* pid, struct Simulator* sim);
void handle_rollback(const char* pid, struct Simulator* sim);
StrategyFn get_strategy(int strategy_type);

#endif

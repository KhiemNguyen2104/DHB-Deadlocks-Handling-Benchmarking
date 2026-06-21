#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "models.h"
#include "dataset.h"
#include "metrics.h"
#include "strategies.h"

typedef struct Simulator {
    int strategy_type;
    int timeout;
    int max_ticks;
    int retry_attempts;
    int current_time;
    Process processes[MAX_PROCS];
    int nprocs;
    Resource resources[MAX_RES];
    int nres;
    Metrics metrics;
    StrategyFn strategy_fn;
    Event* events;
    int nevents;
} Simulator;

Simulator* simulator_create(Event* events, int nevents, int strategy_type,
                            int timeout, int max_ticks, int retry_attempts);
void simulator_destroy(Simulator* sim);
void simulator_run(Simulator* sim);

Process*  sim_find_or_create_process(Simulator* sim, const char* pid);
Resource* sim_find_or_create_resource(Simulator* sim, const char* rid);
Process*  sim_find_process(Simulator* sim, const char* pid);
Resource* sim_find_resource(Simulator* sim, const char* rid);
void sim_grant_resource(Simulator* sim, const char* rid, const char* pid);
void sim_release_resource(Simulator* sim, const char* rid, const char* pid);

#endif

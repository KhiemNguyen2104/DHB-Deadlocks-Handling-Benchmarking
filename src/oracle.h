#ifndef ORACLE_H
#define ORACLE_H

#include "models.h"

#define MAX_WFG_EDGES MAX_PROCS

typedef struct {
    char from[ID_LEN];
    char to[ID_LEN];
} WFGEdge;

void build_wfg(Process* procs, int nprocs, Resource* res, int nres,
               WFGEdge* edges, int* edge_count);
int  has_cycle_dfs(WFGEdge* edges, int edge_count, const char* start);
int  classify_timeout(const char* pid, Process* procs, int nprocs,
                      Resource* res, int nres);

#endif

#include "oracle.h"
#include <string.h>

void build_wfg(Process* procs, int nprocs, Resource* res, int nres,
               WFGEdge* edges, int* edge_count) {
    int i, j;
    *edge_count = 0;
    for (i = 0; i < nprocs; i++) {
        if (procs[i].state != STATE_BLOCKED || !procs[i].waiting_for_valid) continue;
        for (j = 0; j < nres; j++) {
            if (strcmp(res[j].rid, procs[i].waiting_for) == 0) {
                if (res[j].owner_valid && *edge_count < MAX_WFG_EDGES) {
                    strncpy(edges[*edge_count].from, procs[i].pid, ID_LEN);
                    strncpy(edges[*edge_count].to, res[j].owner, ID_LEN);
                    (*edge_count)++;
                }
                break;
            }
        }
    }
}

static int dfs(WFGEdge* edges, int edge_count, const char* node,
               char visited[][ID_LEN], int* nvisited,
               char rec_stack[][ID_LEN], int* nrec) {
    int i;
    strncpy(visited[*nvisited], node, ID_LEN);
    (*nvisited)++;
    strncpy(rec_stack[*nrec], node, ID_LEN);
    (*nrec)++;

    const char* neighbor = NULL;
    for (i = 0; i < edge_count; i++) {
        if (strcmp(edges[i].from, node) == 0) { neighbor = edges[i].to; break; }
    }

    if (neighbor == NULL) {
        (*nrec)--;
        return 0;
    }

    int in_vis = 0, in_rec = 0;
    for (i = 0; i < *nvisited; i++)
        if (strcmp(visited[i], neighbor) == 0) { in_vis = 1; break; }
    for (i = 0; i < *nrec; i++)
        if (strcmp(rec_stack[i], neighbor) == 0) { in_rec = 1; break; }

    if (!in_vis) {
        if (dfs(edges, edge_count, neighbor, visited, nvisited, rec_stack, nrec))
            return 1;
    } else if (in_rec) {
        return 1;
    }
    (*nrec)--;
    return 0;
}

int has_cycle_dfs(WFGEdge* edges, int edge_count, const char* start) {
    char visited[MAX_WFG_EDGES][ID_LEN];
    char rec_stack[MAX_WFG_EDGES][ID_LEN];
    int nvisited = 0, nrec = 0;
    return dfs(edges, edge_count, start, visited, &nvisited, rec_stack, &nrec);
}

int classify_timeout(const char* pid, Process* procs, int nprocs,
                     Resource* res, int nres) {
    WFGEdge edges[MAX_WFG_EDGES];
    int edge_count = 0;
    build_wfg(procs, nprocs, res, nres, edges, &edge_count);
    return has_cycle_dfs(edges, edge_count, pid);
}

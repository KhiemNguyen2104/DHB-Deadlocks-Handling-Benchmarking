#include "simulator.h"
#include <string.h>

void handle_rollback(const char* pid, struct Simulator* sim) {
    Process* p = sim_find_process(sim, pid);
    if (!p) return;
    char contested_rid[ID_LEN];
    int has_contested = p->waiting_for_valid;
    if (has_contested) strncpy(contested_rid, p->waiting_for, ID_LEN);
    if (p->waiting_for_valid) {
        Resource* r = sim_find_resource(sim, p->waiting_for);
        if (r) waitqueue_remove(&r->wait_queue, pid);
    }
    p->retry_count++;
    if (p->retry_count > sim->retry_attempts) {
        int i;
        char held[MAX_HELD][ID_LEN];
        int nheld = p->held_resources.size;
        for (i = 0; i < nheld; i++)
            strncpy(held[i], p->held_resources.ids[i], ID_LEN);
        for (i = 0; i < nheld; i++)
            sim_release_resource(sim, held[i], pid);
        p->state = STATE_KILLED;
        p->remaining_time = 0;
        p->waiting_for_valid = 0;
        p->request_time = -1;
        sim->metrics.killed++;
        return;
    }
    if (!p->checkpoint_valid) {
        int i;
        char held[MAX_HELD][ID_LEN];
        int nheld = p->held_resources.size;
        for (i = 0; i < nheld; i++)
            strncpy(held[i], p->held_resources.ids[i], ID_LEN);
        for (i = 0; i < nheld; i++)
            sim_release_resource(sim, held[i], pid);
        p->remaining_time = 0;
        int delay = 3 * (1 << (p->retry_count - 1));
        if (delay > 16) delay = 16;
        p->backoff_until = sim->current_time + delay;
        p->state = STATE_BACKOFF;
        p->waiting_for_valid = 0;
        p->request_time = -1;
        return;
    }
    int i;
    char post_chk[MAX_HELD][ID_LEN];
    int npost = 0;
    for (i = 0; i < p->held_resources.size; i++) {
        if (!stringset_contains(&p->checkpoint, p->held_resources.ids[i]))
            strncpy(post_chk[npost++], p->held_resources.ids[i], ID_LEN);
    }
    for (i = 0; i < npost; i++)
        sim_release_resource(sim, post_chk[i], pid);
    p->remaining_time = p->checkpoint_remaining_time;
    int delay = 2 * p->retry_count;
    if (delay > 10) delay = 10;
    p->backoff_until = sim->current_time + delay;
    p->state = STATE_BACKOFF;
    p->waiting_for_valid = 0;
    p->request_time = -1;
    if (has_contested) {
        int found = 0;
        for (i = 0; i < p->history_size; i++)
            if (strcmp(p->history[i], contested_rid) == 0) { found = 1; break; }
        if (!found && p->history_size < MAX_HISTORY)
            strncpy(p->history[p->history_size++], contested_rid, ID_LEN);
        strncpy(p->rollback_target, contested_rid, ID_LEN);
        p->rollback_target_valid = 1;
    }
}

#include "simulator.h"
#include <string.h>

void handle_retry(const char* pid, struct Simulator* sim) {
    Process* p = sim_find_process(sim, pid);
    if (!p) return;
    if (p->waiting_for_valid) {
        Resource* r = sim_find_resource(sim, p->waiting_for);
        if (r) waitqueue_remove(&r->wait_queue, pid);
    }
    int i;
    char held[MAX_HELD][ID_LEN];
    int nheld = p->held_resources.size;
    for (i = 0; i < nheld; i++)
        strncpy(held[i], p->held_resources.ids[i], ID_LEN);
    for (i = 0; i < nheld; i++)
        sim_release_resource(sim, held[i], pid);
    p->retry_count++;
    if (p->retry_count > sim->retry_attempts) {
        p->state = STATE_KILLED;
        p->remaining_time = 0;
        p->waiting_for_valid = 0;
        p->request_time = -1;
        sim->metrics.killed++;
        return;
    }
    int delay = 3 * (1 << (p->retry_count - 1));
    if (delay > 16) delay = 16;
    p->backoff_until = sim->current_time + delay;
    p->state = STATE_BACKOFF;
    p->waiting_for_valid = 0;
    p->request_time = -1;
}

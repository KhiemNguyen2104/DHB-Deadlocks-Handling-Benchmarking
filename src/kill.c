#include "simulator.h"
#include <string.h>
#include <stdio.h>

void handle_kill(const char* pid, struct Simulator* sim) {
    Process* p = sim_find_process(sim, pid);
    if (!p) return;
    if (strcmp(pid, "P23") == 0) printf("KILLING P23 at %d\n", sim->current_time);
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
    p->state = STATE_KILLED;
    p->remaining_time = 0;
    p->waiting_for_valid = 0;
    p->request_time = -1;
    sim->metrics.killed++;
}

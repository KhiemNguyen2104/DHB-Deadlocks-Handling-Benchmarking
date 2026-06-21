#include "simulator.h"
#include "oracle.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static int is_terminal(ProcessState s) {
    return s == STATE_COMPLETED || s == STATE_KILLED;
}

Simulator* simulator_create(Event* events, int nevents, int strategy_type,
                            int timeout, int max_ticks, int retry_attempts) {
    int i;
    Simulator* sim = (Simulator*)calloc(1, sizeof(Simulator));
    sim->strategy_type  = strategy_type;
    sim->timeout        = timeout;
    sim->max_ticks      = max_ticks;
    sim->retry_attempts = retry_attempts;
    sim->current_time   = 0;
    sim->nprocs         = 0;
    sim->nres           = 0;
    sim->events         = events;
    sim->nevents        = nevents;
    sim->strategy_fn    = get_strategy(strategy_type);

    int total_pids = 0;
    char seen[MAX_PROCS][ID_LEN];
    int nseen = 0;
    for (i = 0; i < nevents; i++) {
        int found = 0, j;
        for (j = 0; j < nseen; j++)
            if (strcmp(seen[j], events[i].process_id) == 0) { found = 1; break; }
        if (!found && nseen < MAX_PROCS) {
            strncpy(seen[nseen++], events[i].process_id, ID_LEN);
            total_pids++;
        }
    }
    metrics_init(&sim->metrics, total_pids);
    return sim;
}

void simulator_destroy(Simulator* sim) {
    free(sim);
}

Process* sim_find_process(Simulator* sim, const char* pid) {
    int i;
    for (i = 0; i < sim->nprocs; i++)
        if (strcmp(sim->processes[i].pid, pid) == 0) return &sim->processes[i];
    return NULL;
}

Resource* sim_find_resource(Simulator* sim, const char* rid) {
    int i;
    for (i = 0; i < sim->nres; i++)
        if (strcmp(sim->resources[i].rid, rid) == 0) return &sim->resources[i];
    return NULL;
}

Process* sim_find_or_create_process(Simulator* sim, const char* pid) {
    Process* p = sim_find_process(sim, pid);
    if (p) return p;
    if (sim->nprocs >= MAX_PROCS) return NULL;
    p = &sim->processes[sim->nprocs++];
    memset(p, 0, sizeof(Process));
    strncpy(p->pid, pid, ID_LEN - 1);
    p->state = STATE_ACTIVE;
    p->waiting_for_valid  = 0;
    p->checkpoint_valid   = 0;
    p->rollback_target_valid = 0;
    p->backoff_until      = -1;
    p->request_time       = -1;
    return p;
}

Resource* sim_find_or_create_resource(Simulator* sim, const char* rid) {
    Resource* r = sim_find_resource(sim, rid);
    if (r) return r;
    if (sim->nres >= MAX_RES) return NULL;
    r = &sim->resources[sim->nres++];
    memset(r, 0, sizeof(Resource));
    strncpy(r->rid, rid, ID_LEN - 1);
    r->owner_valid = 0;
    return r;
}

void sim_grant_resource(Simulator* sim, const char* rid, const char* pid) {
    Resource* res = sim_find_resource(sim, rid);
    Process*  p   = sim_find_process(sim, pid);
    if (!res || !p) return;
    strncpy(res->owner, pid, ID_LEN);
    res->owner_valid = 1;
    stringset_add(&p->held_resources, rid);
    p->waiting_for_valid = 0;
    p->request_time = -1;
    if (p->state != STATE_ACTIVE) p->state = STATE_ACTIVE;
    if (!p->checkpoint_valid) {
        stringset_copy(&p->checkpoint, &p->held_resources);
        p->checkpoint_valid = 1;
        p->checkpoint_remaining_time = p->remaining_time;
    }
}

static void grant_next_in_queue(Simulator* sim, const char* rid);

void sim_release_resource(Simulator* sim, const char* rid, const char* pid) {
    Resource* res = sim_find_resource(sim, rid);
    Process*  p   = sim_find_process(sim, pid);
    if (!res) return;
    if (res->owner_valid && strcmp(res->owner, pid) == 0) res->owner_valid = 0;
    if (p) stringset_remove(&p->held_resources, rid);
    grant_next_in_queue(sim, rid);
}

static void grant_next_in_queue(Simulator* sim, const char* rid) {
    Resource* res = sim_find_resource(sim, rid);
    if (!res) return;
    while (!waitqueue_is_empty(&res->wait_queue)) {
        char next_pid[ID_LEN];
        waitqueue_pop_front(&res->wait_queue, next_pid);
        Process* np = sim_find_process(sim, next_pid);
        if (!np || is_terminal(np->state)) continue;
        sim_grant_resource(sim, rid, next_pid);
        int pending = np->pending_duration;
        if (np->remaining_time == 0)
            np->remaining_time = pending > 0 ? pending : 1;
        break;
    }
}

void dispatch_events(Simulator* sim) {
    int i;
    for (i = 0; i < sim->nevents; i++) {
        Event* ev = &sim->events[i];
        if (ev->time != sim->current_time) continue;
        const char* pid = ev->process_id;
        const char* rid = ev->resource_id;
        Process*  p = sim_find_or_create_process(sim, pid);
        Resource* r = sim_find_or_create_resource(sim, rid);
        if (!p || !r) continue;
        if (is_terminal(p->state) || p->state == STATE_BACKOFF) continue;
        if (p->history_size < MAX_HISTORY)
            strncpy(p->history[p->history_size++], rid, ID_LEN);
        if (!r->owner_valid) {
            sim_grant_resource(sim, rid, pid);
            if (ev->duration > 0 && p->remaining_time == 0)
                p->remaining_time = ev->duration;
            else if (ev->duration == 0)
                p->remaining_time = 1;
            if (strcmp(pid, "P23") == 0) printf("[%d] P23 AFTER GRANT p_rem=%d\n", sim->current_time, p->remaining_time);
        } else {
            if (!waitqueue_contains(&r->wait_queue, pid))
                waitqueue_push(&r->wait_queue, pid);
            p->state = STATE_BLOCKED;
            strncpy(p->waiting_for, rid, ID_LEN);
            p->waiting_for_valid = 1;
            p->request_time = sim->current_time;
            p->pending_duration = ev->duration;
        }
    }
}

static void wake_backoff(Simulator* sim) {
    int i;
    for (i = 0; i < sim->nprocs; i++) {
        Process* p = &sim->processes[i];
        if (p->state != STATE_BACKOFF || p->backoff_until < 0) continue;
        if (sim->current_time < p->backoff_until) continue;
        p->state = STATE_ACTIVE;
        p->backoff_until = -1;
        char target_rid[ID_LEN];
        int has_target = 0;
        if (p->rollback_target_valid) {
            strncpy(target_rid, p->rollback_target, ID_LEN);
            p->rollback_target_valid = 0;
            has_target = 1;
        } else if (p->history_size > 0) {
            strncpy(target_rid, p->history[0], ID_LEN);
            has_target = 1;
        }
        if (!has_target) continue;
        Resource* r = sim_find_resource(sim, target_rid);
        if (!r) continue;
        if (!r->owner_valid) {
            sim_grant_resource(sim, target_rid, p->pid);
            if (p->remaining_time == 0)
                p->remaining_time = p->pending_duration > 0 ? p->pending_duration : 1;
        } else if (!waitqueue_contains(&r->wait_queue, p->pid)) {
            waitqueue_push(&r->wait_queue, p->pid);
            p->state = STATE_BLOCKED;
            strncpy(p->waiting_for, target_rid, ID_LEN);
            p->waiting_for_valid = 1;
            p->request_time = sim->current_time;
        }
    }
}

static void tick_active(Simulator* sim) {
    int i, j;
    for (i = 0; i < sim->nprocs; i++) {
        Process* p = &sim->processes[i];
        if (p->state != STATE_ACTIVE || p->remaining_time <= 0) continue;
        p->remaining_time--;
        if (p->remaining_time == 0) {
            for (j = 0; j < p->held_resources.size; j++)
                sim_release_resource(sim, p->held_resources.ids[j], p->pid);
            p->held_resources.size = 0;
            p->state = STATE_COMPLETED;
            sim->metrics.completed++;
        }
    }
}

static void check_timeouts(Simulator* sim) {
    char timed_out[MAX_PROCS][ID_LEN];
    int nto = 0, i;
    for (i = 0; i < sim->nprocs; i++) {
        Process* p = &sim->processes[i];
        if (p->state == STATE_BLOCKED && p->request_time >= 0 &&
            (sim->current_time - p->request_time) >= sim->timeout) {
            strncpy(timed_out[nto++], p->pid, ID_LEN);
        }
    }
    for (i = 0; i < nto; i++) {
        Process* p = sim_find_process(sim, timed_out[i]);
        if (!p || p->state != STATE_BLOCKED) continue;
        int result = classify_timeout(timed_out[i], sim->processes, sim->nprocs,
                                       sim->resources, sim->nres);
        if (result) sim->metrics.resolved++;
        else        sim->metrics.false_positives++;
        sim->strategy_fn(timed_out[i], sim);
    }
}

static int all_terminal(Simulator* sim) {
    int i;
    for (i = 0; i < sim->nprocs; i++)
        if (!is_terminal(sim->processes[i].state)) return 0;
    return 1;
}

void simulator_run(Simulator* sim) {
    while (1) {
        dispatch_events(sim);
        wake_backoff(sim);
        tick_active(sim);
        check_timeouts(sim);
        if (all_terminal(sim)) break;
        if (sim->current_time >= sim->max_ticks) break;
        sim->current_time++;
    }
}

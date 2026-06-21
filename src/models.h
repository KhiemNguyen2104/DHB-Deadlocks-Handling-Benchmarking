#ifndef MODELS_H
#define MODELS_H

#include <string.h>

#define MAX_PROCS 1024
#define MAX_RES 1024
#define MAX_HISTORY 256
#define MAX_HELD 256
#define MAX_WAIT_QUEUE 1024
#define ID_LEN 16

typedef enum {
    STATE_ACTIVE,
    STATE_BLOCKED,
    STATE_BACKOFF,
    STATE_COMPLETED,
    STATE_KILLED
} ProcessState;

typedef struct {
    char ids[MAX_HELD][ID_LEN];
    int size;
} StringSet;

typedef struct {
    char ids[MAX_WAIT_QUEUE][ID_LEN];
    int size;
} WaitQueue;

typedef struct {
    char pid[ID_LEN];
    ProcessState state;
    int remaining_time;
    int pending_duration;
    StringSet held_resources;
    StringSet checkpoint;
    int checkpoint_valid;
    char waiting_for[ID_LEN];
    int waiting_for_valid;
    int request_time;
    int checkpoint_remaining_time;
    int retry_count;
    int backoff_until;
    char rollback_target[ID_LEN];
    int rollback_target_valid;
    char history[MAX_HISTORY][ID_LEN];
    int history_size;
} Process;

typedef struct {
    char rid[ID_LEN];
    char owner[ID_LEN];
    int owner_valid;
    WaitQueue wait_queue;
} Resource;

int  stringset_contains(const StringSet* s, const char* id);
void stringset_add(StringSet* s, const char* id);
void stringset_remove(StringSet* s, const char* id);
void stringset_copy(StringSet* dst, const StringSet* src);

int  waitqueue_is_empty(const WaitQueue* q);
int  waitqueue_contains(const WaitQueue* q, const char* id);
void waitqueue_push(WaitQueue* q, const char* id);
int  waitqueue_pop_front(WaitQueue* q, char* out);
void waitqueue_remove(WaitQueue* q, const char* id);

#endif

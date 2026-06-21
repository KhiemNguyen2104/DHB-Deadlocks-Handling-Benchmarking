#include "models.h"
#include <string.h>

int stringset_contains(const StringSet* s, const char* id) {
    int i;
    for (i = 0; i < s->size; i++)
        if (strcmp(s->ids[i], id) == 0) return 1;
    return 0;
}

void stringset_add(StringSet* s, const char* id) {
    if (stringset_contains(s, id)) return;
    if (s->size < MAX_HELD) {
        strncpy(s->ids[s->size], id, ID_LEN - 1);
        s->ids[s->size][ID_LEN - 1] = '\0';
        s->size++;
    }
}

void stringset_remove(StringSet* s, const char* id) {
    int i, j;
    for (i = 0; i < s->size; i++) {
        if (strcmp(s->ids[i], id) == 0) {
            for (j = i; j < s->size - 1; j++)
                strncpy(s->ids[j], s->ids[j + 1], ID_LEN);
            s->size--;
            return;
        }
    }
}

void stringset_copy(StringSet* dst, const StringSet* src) {
    int i;
    dst->size = src->size;
    for (i = 0; i < src->size; i++)
        strncpy(dst->ids[i], src->ids[i], ID_LEN);
}

int waitqueue_is_empty(const WaitQueue* q) {
    return q->size == 0;
}

int waitqueue_contains(const WaitQueue* q, const char* id) {
    int i;
    for (i = 0; i < q->size; i++)
        if (strcmp(q->ids[i], id) == 0) return 1;
    return 0;
}

void waitqueue_push(WaitQueue* q, const char* id) {
    if (q->size < MAX_WAIT_QUEUE) {
        strncpy(q->ids[q->size], id, ID_LEN - 1);
        q->ids[q->size][ID_LEN - 1] = '\0';
        q->size++;
    }
}

int waitqueue_pop_front(WaitQueue* q, char* out) {
    int i;
    if (q->size == 0) return 0;
    if (out) strncpy(out, q->ids[0], ID_LEN);
    for (i = 0; i < q->size - 1; i++)
        strncpy(q->ids[i], q->ids[i + 1], ID_LEN);
    q->size--;
    return 1;
}

void waitqueue_remove(WaitQueue* q, const char* id) {
    int i, j;
    for (i = 0; i < q->size; i++) {
        if (strcmp(q->ids[i], id) == 0) {
            for (j = i; j < q->size - 1; j++)
                strncpy(q->ids[j], q->ids[j + 1], ID_LEN);
            q->size--;
            return;
        }
    }
}

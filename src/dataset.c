#include "dataset.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int cmp_events(const void* a, const void* b) {
    return ((const Event*)a)->time - ((const Event*)b)->time;
}

Event* load_events(const char* path, int* count) {
    FILE* f = fopen(path, "r");
    if (!f) { *count = 0; return NULL; }

    int capacity = 512;
    Event* events = (Event*)malloc((size_t)capacity * sizeof(Event));
    *count = 0;

    char line[256];
    fgets(line, (int)sizeof(line), f);

    while (fgets(line, (int)sizeof(line), f)) {
        if (*count >= capacity) {
            capacity *= 2;
            events = (Event*)realloc(events, (size_t)capacity * sizeof(Event));
        }
        Event* e = &events[*count];
        if (sscanf(line, "%d,%15[^,],%15[^,],%15[^,],%d",
                   &e->time, e->process_id, e->action, e->resource_id, &e->duration) == 5)
            (*count)++;
    }
    fclose(f);
    // Dataset is already sorted by time, no need to qsort and risk unstable sorting!
    return events;
}

void free_events(Event* events) {
    free(events);
}

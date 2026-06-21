#ifndef DATASET_H
#define DATASET_H

#include "models.h"

typedef struct {
    int time;
    char process_id[ID_LEN];
    char action[16];
    char resource_id[ID_LEN];
    int duration;
} Event;

Event* load_events(const char* path, int* count);
void   free_events(Event* events);

#endif

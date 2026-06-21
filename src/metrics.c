#include "metrics.h"
#include <string.h>

void metrics_init(Metrics* m, int total) {
    memset(m, 0, sizeof(Metrics));
    m->total = total;
}

double metrics_throughput(const Metrics* m) {
    if (m->total == 0) return 0.0;
    return (double)m->completed / (double)m->total;
}

double metrics_fp_rate(const Metrics* m) {
    int denom = m->resolved + m->false_positives;
    if (denom == 0) return 0.0;
    return (double)m->false_positives / (double)denom;
}

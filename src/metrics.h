#ifndef METRICS_H
#define METRICS_H

typedef struct {
    int killed;
    int resolved;
    int false_positives;
    int completed;
    int total;
} Metrics;

void   metrics_init(Metrics* m, int total);
double metrics_throughput(const Metrics* m);
double metrics_fp_rate(const Metrics* m);

#endif

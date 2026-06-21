#ifndef BENCHMARK_H
#define BENCHMARK_H

#include "dataset.h"
#include "metrics.h"

#define MAX_BENCH_RESULTS 512

typedef struct {
    char strategy[16];
    int  timeout;
    int  max_ticks;
    int  killed;
    int  resolved;
    int  fp;
    int  completed;
    int  total;
    double throughput;
    double fp_rate;
} BenchmarkResult;

void run_grid_benchmark(Event* events, int nevents,
                        int* timeouts, int n_timeouts,
                        int* max_ticks_list, int n_max_ticks,
                        int retry_attempts,
                        BenchmarkResult* out, int* out_count);

void save_results_csv(BenchmarkResult* results, int count, const char* results_dir);
void print_summary_table(BenchmarkResult* results, int count, int max_ticks_filter);

#endif

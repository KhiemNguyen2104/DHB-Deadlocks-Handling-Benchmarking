#include "benchmark.h"
#include "simulator.h"
#include <stdio.h>
#include <string.h>

static const char* STRATEGY_NAMES[] = { "KILL", "RETRY", "ROLLBACK" };

void run_grid_benchmark(Event* events, int nevents,
                        int* timeouts, int n_timeouts,
                        int* max_ticks_list, int n_max_ticks,
                        int retry_attempts,
                        BenchmarkResult* out, int* out_count) {
    int s, t, m;
    *out_count = 0;
    for (m = 0; m < n_max_ticks; m++) {
        for (s = 0; s < 3; s++) {
            for (t = 0; t < n_timeouts; t++) {
                if (*out_count >= MAX_BENCH_RESULTS) return;
                Simulator* sim = simulator_create(events, nevents, s,
                                                  timeouts[t], max_ticks_list[m],
                                                  retry_attempts);
                simulator_run(sim);
                BenchmarkResult* r = &out[*out_count];
                strncpy(r->strategy, STRATEGY_NAMES[s], 15);
                r->timeout    = timeouts[t];
                r->max_ticks  = max_ticks_list[m];
                r->killed     = sim->metrics.killed;
                r->resolved   = sim->metrics.resolved;
                r->fp         = sim->metrics.false_positives;
                r->completed  = sim->metrics.completed;
                r->total      = sim->metrics.total;
                r->throughput = metrics_throughput(&sim->metrics);
                r->fp_rate    = metrics_fp_rate(&sim->metrics);
                simulator_destroy(sim);
                (*out_count)++;
            }
        }
    }
}

void save_results_csv(BenchmarkResult* results, int count, const char* results_dir) {
    int i;
    char path[512];

    snprintf(path, sizeof(path), "%s/grid_results.csv", results_dir);
    FILE* f = fopen(path, "w");
    if (f) {
        fprintf(f, "strategy,timeout,max_ticks,killed,resolved,false_positives,"
                   "completed,total,throughput,false_positive_rate\n");
        for (i = 0; i < count; i++) {
            BenchmarkResult* r = &results[i];
            fprintf(f, "%s,%d,%d,%d,%d,%d,%d,%d,%.10f,%.10f\n",
                    r->strategy, r->timeout, r->max_ticks, r->killed, r->resolved,
                    r->fp, r->completed, r->total, r->throughput, r->fp_rate);
        }
        fclose(f);
        printf("  Saved: %s\n", path);
    }

    snprintf(path, sizeof(path), "%s/results_maxticks800.csv", results_dir);
    f = fopen(path, "w");
    if (f) {
        fprintf(f, "strategy,timeout,max_ticks,killed,resolved,false_positives,"
                   "completed,total,throughput,false_positive_rate\n");
        for (i = 0; i < count; i++) {
            if (results[i].max_ticks != 800) continue;
            BenchmarkResult* r = &results[i];
            fprintf(f, "%s,%d,%d,%d,%d,%d,%d,%d,%.10f,%.10f\n",
                    r->strategy, r->timeout, r->max_ticks, r->killed, r->resolved,
                    r->fp, r->completed, r->total, r->throughput, r->fp_rate);
        }
        fclose(f);
        printf("  Saved: %s\n", path);
    }
}

void print_summary_table(BenchmarkResult* results, int count, int max_ticks_filter) {
    int i;
    printf("%-10s %7s %9s %6s %8s %15s %9s %5s %10s %7s\n",
           "Strategy", "Timeout", "MaxTicks", "Killed", "Resolved",
           "FalsePositives", "Completed", "Total", "Throughput", "FPRate");
    for (i = 0; i < count; i++) {
        BenchmarkResult* r = &results[i];
        if (r->max_ticks != max_ticks_filter) continue;
        printf("%-10s %7d %9d %6d %8d %15d %9d %5d %9.1f%% %6.1f%%\n",
               r->strategy, r->timeout, r->max_ticks, r->killed, r->resolved,
               r->fp, r->completed, r->total,
               r->throughput * 100.0, r->fp_rate * 100.0);
    }
}

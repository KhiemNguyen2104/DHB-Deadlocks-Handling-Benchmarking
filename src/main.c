#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dataset.h"
#include "benchmark.h"

#define DATA_PATH  "src/data/dataset_150_processes.csv"
#define RESULTS_DIR "results"
#define RETRY_ATTEMPTS 5

int main(void) {
    int timeouts[]      = { 2, 5, 10, 20, 30, 40, 50, 100 };
    int max_ticks_list[] = { 50, 100, 200, 400, 800 };
    int n_timeouts       = 8;
    int n_max_ticks      = 5;

    printf("Loading dataset...\n");
    int nevents = 0;
    Event* events = load_events(DATA_PATH, &nevents);
    if (!events) {
        fprintf(stderr, "Failed to load: %s\n", DATA_PATH);
        return 1;
    }
    printf("  %d events loaded\n\n", nevents);

    int total_runs = 3 * n_timeouts * n_max_ticks;
    printf("Running grid benchmark (3 strategies x %d timeouts x %d max_ticks = %d runs, retry_attempts=%d)...\n",
           n_timeouts, n_max_ticks, total_runs, RETRY_ATTEMPTS);

    BenchmarkResult* results = (BenchmarkResult*)malloc(MAX_BENCH_RESULTS * sizeof(BenchmarkResult));
    int out_count = 0;

    run_grid_benchmark(events, nevents, timeouts, n_timeouts,
                       max_ticks_list, n_max_ticks, RETRY_ATTEMPTS,
                       results, &out_count);
    printf("  Done (%d runs).\n\n", out_count);

    save_results_csv(results, out_count, RESULTS_DIR);
    printf("\n");
    print_summary_table(results, out_count, 800);

    free(results);
    free_events(events);
    return 0;
}

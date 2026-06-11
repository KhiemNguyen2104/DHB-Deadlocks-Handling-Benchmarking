import os
import sys

sys.path.insert(0, os.path.dirname(__file__))

from dataset import load_events
from benchmark import run_grid_benchmark
from visualizer import (
    plot_heatmaps,
    plot_fixed_timeout_line,
    plot_accuracy_per_strategy,
    plot_accuracy_profile,
    plot_throughput,
    save_csv,
)

DATA_PATH = os.path.join(os.path.dirname(__file__), "data", "dataset_150_processes.csv")
RESULTS_DIR = os.path.join(os.path.dirname(__file__), "..", "results")

TIMEOUTS = [2, 5, 10, 20, 30, 40, 50, 100]
MAX_TICKS_LIST = [50, 100, 200, 400, 800]
FIXED_TIMEOUTS = [10, 50, 100]
RETRY_ATTEMPTS = 5


def main():
    print("Loading dataset...")
    events = load_events(DATA_PATH)
    print(f"  {len(events)} events loaded from {DATA_PATH}\n")

    total_runs = len(TIMEOUTS) * len(MAX_TICKS_LIST) * 3
    print(f"Running grid benchmark (3 strategies × {len(TIMEOUTS)} timeouts × {len(MAX_TICKS_LIST)} max_ticks = {total_runs} runs, retry_attempts={RETRY_ATTEMPTS})...")
    results = run_grid_benchmark(events, TIMEOUTS, MAX_TICKS_LIST, retry_attempts=RETRY_ATTEMPTS)
    print("  Done.\n")

    print("Generating visualizations...")
    plot_heatmaps(results, RESULTS_DIR)
    for ft in FIXED_TIMEOUTS:
        plot_fixed_timeout_line(results, ft, RESULTS_DIR)
    plot_accuracy_per_strategy(results, MAX_TICKS_LIST[-1], RESULTS_DIR)
    plot_accuracy_profile(results, RESULTS_DIR)
    plot_throughput(results, RESULTS_DIR)
    save_csv(results, RESULTS_DIR, MAX_TICKS_LIST[-1])

    print_summary_table(results, MAX_TICKS_LIST[-1])


def print_summary_table(results, max_ticks_filter=800):
    filtered = [r for r in results if r["max_ticks"] == max_ticks_filter]
    header = f"{'Strategy':>10}  {'Timeout':>7}  {'Max Ticks':>9}  {'Killed':>6}  {'Resolved':>8}  {'False Positives':>15}  {'Completed':>9}  {'Total':>5} {'Throughput':>10} {'FP Rate':>7}"
    print(header)
    for r in filtered:
        print(
            f"{r['strategy']:>10}  {r['timeout']:>7}  {r['max_ticks']:>9}  "
            f"{r['killed']:>6}  {r['resolved']:>8}  {r['false_positives']:>15}  "
            f"{r['completed']:>9}  {r['total']:>5} "
            f"{r['throughput']*100:>9.1f}% "
            f"{r['false_positive_rate']*100:>6.1f}%"
        )


if __name__ == "__main__":
    main()

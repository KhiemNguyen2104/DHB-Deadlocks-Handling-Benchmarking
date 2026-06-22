# Deadlock Handling Simulator

A discrete-event simulator for benchmarking three deadlock handling strategies — **Kill**, **Retry**, and **Rollback** — under the timeout detection mechanism. The simulator includes a hidden WFG Oracle to classify each timeout event as a true deadlock or a false positive, enabling fair, quantitative comparison across strategies.

---

## Project Structure

```
deadlocks-handling-c/
├── Makefile                        # Build and run commands
├── run.sh                          # Alternative bash entry point
├── README.md                       # This file
├── results/                        # Generated CSVs (auto-created)
├── figures/                        # Generated PNG charts (auto-created)
└── src/
    ├── main.c                      # Entry point — configure parameters here
    ├── simulator.c                 # Discrete-event simulator core
    ├── benchmark.c                 # Grid-search benchmark runner
    ├── metrics.c                   # MetricsCollector
    ├── oracle.c                    # WFG builder + DFS cycle detector
    ├── dataset.c                   # CSV event loader
    ├── kill.c                      # Kill strategy
    ├── retry.c                     # Retry with exponential backoff
    ├── rollback.c                  # Checkpoint-based rollback
    ├── visualize.py                # Supplemental Python script for plots
    └── data/
        ├── scenario.csv            # Minimal circular deadlock scenario
        └── dataset_150_processes.csv  # Full 150-process benchmark dataset
```

The Python script `visualize.py` in `src/` is a supplemental tool to generate visualizations from the CSV results produced by the C simulator. It's not a part of the main program and do not affect the results, we just use it to generate some visualizations for reports because of the convenience of available Python plotting libraries, we can replace it by any visualization modules. 

---

## Requirements

To run the core simulator:

- `gcc` (with C99 support)
- `make`

To generate plots and visualizations:

- Python 3.8+
- `pandas`, `matplotlib`, `seaborn`

Install Python dependencies:

```bash
pip install pandas matplotlib seaborn
```

---

## How to Run

You have two independent options to build the C binary, run the simulation, and generate the visualizations. Both options handle the entire pipeline end-to-end. Firstly, you need to `cd` to the root directory of this project.

### Option 1 — Using Make

```bash
make run
```

### Option 2 — Using Bash Script

```bash
bash run.sh
```

### Clean output files

```bash
make clean
```

After running, the CSV files storing detailed results are generated in the `results/` directory.

### Visualization

You can run the visualization module independently after running the simulation. Just make sure you have Python 3.8+ and the required Python libraries installed.

```bash
python src/visualize.py
```

The visualization module will automatically look for CSV files in the `results/` directory and generate plots in the `figures/` directory.

Or, you can un-comment the script in `run.sh` and Makefile to run it automatically.

---

## Configuration

All experiment parameters are set in the `src/main.c` entry point:

| Parameter          | Default                              | Description                                |
| ------------------ | ------------------------------------ | ------------------------------------------ |
| `DATA_PATH`        | `src/data/dataset_150_processes.csv` | Input event dataset                        |
| `timeouts[]`       | `{2, 5, 10, 20, 30, 40, 50, 100}`    | Timeout thresholds to sweep                |
| `max_ticks_list[]` | `{50, 100, 200, 400, 800}`           | Simulation time limits to sweep            |
| `RETRY_ATTEMPTS`   | `5`                                  | Maximum retries before a process is killed |

---

## Input Format

Event CSV files must have these columns:

```
time, process_id, action, resource_id, duration
```

- **`time`** — logical clock tick when the event fires
- **`process_id`** — process identifier (e.g. `P1`)
- **`action`** — always `request`
- **`resource_id`** — resource identifier (e.g. `R1`)
- **`duration`** — how many ticks the process holds the resource after acquiring it (`0` = 1 tick minimum)

---

## Strategies

### Kill

When a timeout fires, the process is immediately terminated and all held resources are released. Simple and fast to break deadlocks, but prone to false-positive kills.

### Retry (Exponential Backoff)

When a timeout fires, the process releases all resources and enters a backoff sleep for `min(3 × 2^(k−1), 16)` ticks, where `k` is the retry count, then restarts from its first resource request. After `RETRY_ATTEMPTS` retries the process is killed.

### Rollback (Checkpoint)

When a timeout fires, the process is restored to its checkpoint state — the moment it first successfully acquired a resource. Only resources acquired _after_ the checkpoint are released. The execution timer is also restored. After `RETRY_ATTEMPTS` rollbacks the process is killed.

---

## Output

After a run, the generated results are saved into the `results/` and `figures/` directories:

| File                              | Description                                           |
| --------------------------------- | ----------------------------------------------------- |
| `heatmap_throughput.png`          | 3-panel heatmap: throughput (%) × timeout × MAX_TICKS |
| `throughput_vs_maxticks_t10.png`  | Line chart at TIMEOUT = 10                            |
| `throughput_vs_maxticks_t50.png`  | Line chart at TIMEOUT = 50                            |
| `throughput_vs_maxticks_t100.png` | Line chart at TIMEOUT = 100                           |
| `accuracy_kill.png`               | Resolved vs FP count for KILL                         |
| `accuracy_retry.png`              | Resolved vs FP count for RETRY                        |
| `accuracy_rollback.png`           | Resolved vs FP count for ROLLBACK                     |
| `accuracy_profile.png`            | FP Rate (%) vs timeout, all strategies                |
| `throughput_over_timeouts.png`    | Throughput (%) vs timeout, all strategies             |
| `grid_results.csv`                | Full 120-run result table                             |
| `results_maxticks800.csv`         | Snapshot at MAX_TICKS = 800                           |

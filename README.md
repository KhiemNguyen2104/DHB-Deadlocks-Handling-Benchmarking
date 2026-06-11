# Deadlock Handling Simulator

A discrete-event simulator for benchmarking three deadlock handling strategies — **Kill**, **Retry**, and **Rollback** — under the timeout detection mechanism. The simulator includes a hidden WFG Oracle to classify each timeout event as a true deadlock or a false positive, enabling fair, quantitative comparison across strategies.

---

## Project Structure

```
deadlocks-handling/
├── Makefile                        # make run | make clean
├── script.sh                       # Alternative bash entry point
├── results/                        # Generated figures and CSVs (auto-created)
└── src/
    ├── main.py                     # Entry point — configure parameters here
    ├── simulator.py                # Discrete-event simulator core
    ├── benchmark.py                # Grid-search benchmark runner
    ├── visualizer.py               # All plot and CSV export functions
    ├── models.py                   # Process and Resource data models
    ├── dataset.py                  # CSV event loader
    ├── metrics.py                  # MetricsCollector
    ├── oracle.py                   # WFG builder + DFS cycle detector
    ├── strategies/
    │   ├── __init__.py             # STRATEGY_MAP registry
    │   ├── kill.py                 # Kill strategy
    │   ├── retry.py                # Retry with exponential backoff
    │   └── rollback.py             # Checkpoint-based rollback
    └── data/
        ├── scenario.csv            # Minimal 3-process circular deadlock
        └── dataset_150_processes.csv  # Full 150-process benchmark dataset
```

---

## Requirements

- Python 3.8+
- `pandas`, `matplotlib`, `seaborn`, `numpy`

Install dependencies:

```bash
pip install pandas matplotlib seaborn numpy
```

---

## How to Run

### Option 1 — Makefile

```bash
make run
```

### Option 2 — Bash script

```bash
bash script.sh
```

### Option 3 — Direct Python

```bash
python3 src/main.py
```

### Clean output files

```bash
make clean
```

---

## Configuration

All experiment parameters are set at the top of `src/main.py`:

| Parameter        | Default                           | Description                                  |
| ---------------- | --------------------------------- | -------------------------------------------- |
| `DATA_PATH`      | `data/dataset_150_processes.csv`  | Input event dataset                          |
| `TIMEOUTS`       | `[2, 5, 10, 20, 30, 40, 50, 100]` | Timeout thresholds to sweep                  |
| `MAX_TICKS_LIST` | `[50, 100, 200, 400, 800]`        | Simulation time limits to sweep              |
| `FIXED_TIMEOUTS` | `[10, 50, 100]`                   | Fixed timeouts for per-MAX_TICKS line charts |
| `RETRY_ATTEMPTS` | `5`                               | Maximum retries before a process is killed   |

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

After a run, the `results/` directory contains:

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

---

## Dataset Generation

A dataset generator is provided at the project root:

```bash
python3 test-gen.py
```

The generator creates structured scenarios with:

- **True deadlocks** — circular WFG cycles of varying size and duration targeting specific MAX_TICKS thresholds
- **Convoys** — one heavy process blocking a large queue (false-positive traps)
- **Random background processes** — seeded random workloads for realistic noise

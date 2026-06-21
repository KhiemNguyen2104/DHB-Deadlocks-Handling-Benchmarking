# Deadlock Handling Simulator — C Implementation

A pure C99 port of the Python deadlock-handling simulator. The C core runs the full grid benchmark and writes results to CSV files. A supplemental Python script visualizes those results independently.

---

## Requirements

- `gcc` with C99 support
- `make`

For the visualization tool only: Python 3.8+, `pandas`, `matplotlib`, `seaborn`

---

## Project Structure

```
deadlocks-handling-c/
├── Makefile
├── run.sh
├── README.md
├── results/          CSV outputs (auto-created at runtime)
├── figures/          PNG outputs (auto-created by visualize.py)
└── src/
    ├── main.c
    ├── models.h / models.c
    ├── dataset.h / dataset.c
    ├── metrics.h / metrics.c
    ├── oracle.h / oracle.c
    ├── strategies.h / strategies.c
    ├── simulator.h / simulator.c
    ├── kill.c
    ├── retry.c
    ├── rollback.c
    ├── benchmark.h / benchmark.c
    ├── visualize.py
    └── data/
        ├── dataset_150_processes.csv
        └── scenario.csv
```

---

## C Core

### Build

```bash
make
```

### Run

```bash
./deadlock_sim
```

The binary must be executed from the project root so that relative paths to `src/data/` and `results/` resolve correctly.

### Clean

```bash
make clean
```

### Output

After running, `results/` contains:

| File | Description |
|---|---|
| `grid_results.csv` | All 120 benchmark runs |
| `results_maxticks800.csv` | Snapshot at MAX\_TICKS = 800 |

---

## Python Visualization Tool

> **Note:** The visualization script is a supplemental tool only. It is not part of the C implementation and is not required to run the simulator.

### Requirements

```bash
pip install pandas matplotlib seaborn
```

### Run

```bash
python3 src/visualize.py
```

The script reads `results/grid_results.csv` and saves plots to `figures/`.

---

## Run Everything

You have two independent options to build the C binary, run the simulation, and generate the visualizations. Both options handle the entire pipeline end-to-end.

**Option 1: Using Make**
```bash
make run
```

**Option 2: Using Bash Script**
```bash
bash run.sh
```

---

## Configuration

Edit `src/main.c` to change:

| Constant | Default | Description |
|---|---|---|
| `DATA_PATH` | `src/data/dataset_150_processes.csv` | Input dataset |
| `RETRY_ATTEMPTS` | `5` | Max retries before kill |
| `timeouts[]` | `{2,5,10,20,30,40,50,100}` | Timeout thresholds |
| `max_ticks_list[]` | `{50,100,200,400,800}` | Simulation time limits |

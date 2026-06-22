#!/usr/bin/env bash
set -e
cd "$(dirname "$0")"
gcc -std=c99 -Wall -Wextra -Isrc -o deadlock_sim \
    src/main.c src/models.c src/dataset.c src/metrics.c src/oracle.c \
    src/simulator.c src/strategies.c src/kill.c src/retry.c src/rollback.c \
    src/benchmark.c -lm
./deadlock_sim
# python3 src/visualize.py

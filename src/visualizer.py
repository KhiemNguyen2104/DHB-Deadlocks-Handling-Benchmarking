import os
import csv
from typing import List

import matplotlib.pyplot as plt
import matplotlib
import numpy as np
import seaborn as sns
import pandas as pd

matplotlib.use("Agg")


STRATEGIES = ["KILL", "RETRY", "ROLLBACK"]
PALETTE = {"KILL": "#e05c5c", "RETRY": "#5c9ee0", "ROLLBACK": "#5ce07a"}


def _ensure_dir(path: str):
    os.makedirs(path, exist_ok=True)


def _save(fig, path: str):
    fig.savefig(path, dpi=150, bbox_inches="tight", facecolor="white")
    plt.close(fig)
    print(f"  Saved: {path}")


def plot_heatmaps(results: List[dict], out_dir: str):
    _ensure_dir(out_dir)
    df = pd.DataFrame(results)

    fig, axes = plt.subplots(1, 3, figsize=(18, 5), facecolor="white")
    fig.suptitle("Throughput Heatmap by Strategy", fontsize=14, fontweight="bold")

    for ax, strategy in zip(axes, STRATEGIES):
        sub = df[df["strategy"] == strategy].copy()
        sub["throughput_pct"] = sub["throughput"] * 100
        pivot = sub.pivot(index="max_ticks", columns="timeout", values="throughput_pct")
        sns.heatmap(
            pivot,
            ax=ax,
            annot=True,
            fmt=".1f",
            cmap="YlGnBu",
            vmin=0,
            vmax=100,
            linewidths=0.5,
            cbar_kws={"label": "Throughput (%)"},
        )
        ax.set_title(strategy, fontsize=12, fontweight="bold")
        ax.set_xlabel("Timeout")
        ax.set_ylabel("Max Ticks")

    _save(fig, os.path.join(out_dir, "heatmap_throughput.png"))


def plot_fixed_timeout_line(results: List[dict], fixed_timeout: int, out_dir: str):
    _ensure_dir(out_dir)
    df = pd.DataFrame(results)
    sub = df[df["timeout"] == fixed_timeout].copy()
    sub["throughput_pct"] = sub["throughput"] * 100

    fig, ax = plt.subplots(figsize=(8, 5), facecolor="white")
    for strategy in STRATEGIES:
        s = sub[sub["strategy"] == strategy].sort_values("max_ticks")
        ax.plot(
            s["max_ticks"],
            s["throughput_pct"],
            marker="o",
            label=strategy,
            color=PALETTE[strategy],
        )

    ax.set_title(f"Throughput vs MAX_TICKS  (TIMEOUT = {fixed_timeout})", fontweight="bold")
    ax.set_xlabel("MAX_TICKS")
    ax.set_ylabel("Throughput (%)")
    ax.set_ylim(0, 105)
    ax.legend()
    ax.grid(True, alpha=0.3)

    _save(fig, os.path.join(out_dir, f"throughput_vs_maxticks_t{fixed_timeout}.png"))


def plot_accuracy_per_strategy(results: List[dict], max_ticks_filter: int, out_dir: str):
    _ensure_dir(out_dir)
    df = pd.DataFrame(results)
    sub = df[df["max_ticks"] == max_ticks_filter].copy()

    for strategy in STRATEGIES:
        s = sub[sub["strategy"] == strategy].sort_values("timeout")
        fig, ax = plt.subplots(figsize=(8, 5), facecolor="white")
        ax.plot(s["timeout"], s["resolved"], marker="o", label="Resolved", color="#2ecc71")
        ax.plot(s["timeout"], s["false_positives"], marker="s", label="False Positives", color="#e74c3c")
        ax.set_title(f"Accuracy Profile — {strategy}  (MAX_TICKS={max_ticks_filter})", fontweight="bold")
        ax.set_xlabel("Timeout")
        ax.set_ylabel("Count")
        ax.legend()
        ax.grid(True, alpha=0.3)
        _save(fig, os.path.join(out_dir, f"accuracy_{strategy.lower()}.png"))


def plot_accuracy_profile(results: List[dict], out_dir: str):
    _ensure_dir(out_dir)
    df = pd.DataFrame(results)
    max_ticks_filter = df["max_ticks"].max()
    sub = df[df["max_ticks"] == max_ticks_filter].copy()
    sub["fp_rate_pct"] = sub["false_positive_rate"] * 100

    fig, ax = plt.subplots(figsize=(9, 5), facecolor="white")
    for strategy in STRATEGIES:
        s = sub[sub["strategy"] == strategy].sort_values("timeout")
        ax.plot(s["timeout"], s["fp_rate_pct"], marker="o", label=strategy, color=PALETTE[strategy])

    ax.set_title(f"False Positive Rate vs Timeout  (MAX_TICKS={max_ticks_filter})", fontweight="bold")
    ax.set_xlabel("Timeout")
    ax.set_ylabel("FP Rate (%)")
    ax.set_ylim(0, 105)
    ax.legend()
    ax.grid(True, alpha=0.3)
    _save(fig, os.path.join(out_dir, "accuracy_profile.png"))


def plot_throughput(results: List[dict], out_dir: str):
    _ensure_dir(out_dir)
    df = pd.DataFrame(results)
    max_ticks_filter = df["max_ticks"].max()
    sub = df[df["max_ticks"] == max_ticks_filter].copy()
    sub["throughput_pct"] = sub["throughput"] * 100

    fig, ax = plt.subplots(figsize=(9, 5), facecolor="white")
    for strategy in STRATEGIES:
        s = sub[sub["strategy"] == strategy].sort_values("timeout")
        ax.plot(s["timeout"], s["throughput_pct"], marker="o", label=strategy, color=PALETTE[strategy])

    ax.set_title(f"Throughput vs Timeout  (MAX_TICKS={max_ticks_filter})", fontweight="bold")
    ax.set_xlabel("Timeout")
    ax.set_ylabel("Throughput (%)")
    ax.set_ylim(0, 105)
    ax.legend()
    ax.grid(True, alpha=0.3)
    _save(fig, os.path.join(out_dir, "throughput_over_timeouts.png"))


def save_csv(results: List[dict], out_dir: str, max_ticks_snapshot: int):
    _ensure_dir(out_dir)

    all_path = os.path.join(out_dir, "grid_results.csv")
    with open(all_path, "w", newline="") as f:
        fieldnames = ["strategy", "timeout", "max_ticks", "killed", "resolved",
                      "false_positives", "completed", "total", "throughput", "false_positive_rate"]
        writer = csv.DictWriter(f, fieldnames=fieldnames)
        writer.writeheader()
        for r in results:
            writer.writerow({k: r[k] for k in fieldnames})
    print(f"  Saved: {all_path}")

    snap = [r for r in results if r["max_ticks"] == max_ticks_snapshot]
    snap_path = os.path.join(out_dir, f"results_maxticks{max_ticks_snapshot}.csv")
    with open(snap_path, "w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=fieldnames)
        writer.writeheader()
        for r in snap:
            writer.writerow({k: r[k] for k in fieldnames})
    print(f"  Saved: {snap_path}")

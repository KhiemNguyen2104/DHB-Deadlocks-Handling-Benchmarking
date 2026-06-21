import os
import csv
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import pandas as pd
import seaborn as sns

RESULTS_DIR = os.path.join(os.path.dirname(__file__), "..", "results")
FIGURES_DIR = os.path.join(os.path.dirname(__file__), "..", "figures")
STRATEGIES  = ["KILL", "RETRY", "ROLLBACK"]
PALETTE     = {"KILL": "#e05c5c", "RETRY": "#5c9ee0", "ROLLBACK": "#5ce07a"}


def ensure_dir(path):
    os.makedirs(path, exist_ok=True)


def save(fig, path):
    fig.savefig(path, dpi=150, bbox_inches="tight", facecolor="white")
    plt.close(fig)
    print(f"  Saved: {path}")


def main():
    ensure_dir(FIGURES_DIR)
    csv_path = os.path.join(RESULTS_DIR, "grid_results.csv")
    if not os.path.exists(csv_path):
        print(f"No results file found at {csv_path}. Run the C binary first.")
        return

    df = pd.read_csv(csv_path)
    df["throughput_pct"]   = df["throughput"] * 100
    df["fp_rate_pct"]      = df["false_positive_rate"] * 100
    max_ticks_val          = df["max_ticks"].max()

    fig, axes = plt.subplots(1, 3, figsize=(18, 5), facecolor="white")
    fig.suptitle("Throughput Heatmap by Strategy", fontsize=14, fontweight="bold")
    for ax, strategy in zip(axes, STRATEGIES):
        sub = df[df["strategy"] == strategy]
        pivot = sub.pivot(index="max_ticks", columns="timeout", values="throughput_pct")
        sns.heatmap(pivot, ax=ax, annot=True, fmt=".1f", cmap="YlGnBu",
                    vmin=0, vmax=100, linewidths=0.5,
                    cbar_kws={"label": "Throughput (%)"})
        ax.set_title(strategy, fontsize=12, fontweight="bold")
        ax.set_xlabel("Timeout")
        ax.set_ylabel("Max Ticks")
    save(fig, os.path.join(FIGURES_DIR, "heatmap_throughput.png"))

    for ft in [10, 50, 100]:
        sub = df[df["timeout"] == ft]
        fig, ax = plt.subplots(figsize=(8, 5), facecolor="white")
        for strategy in STRATEGIES:
            s = sub[sub["strategy"] == strategy].sort_values("max_ticks")
            ax.plot(s["max_ticks"], s["throughput_pct"], marker="o",
                    label=strategy, color=PALETTE[strategy])
        ax.set_title(f"Throughput vs MAX_TICKS  (TIMEOUT = {ft})", fontweight="bold")
        ax.set_xlabel("MAX_TICKS")
        ax.set_ylabel("Throughput (%)")
        ax.set_ylim(0, 105)
        ax.legend()
        ax.grid(True, alpha=0.3)
        save(fig, os.path.join(FIGURES_DIR, f"throughput_vs_maxticks_t{ft}.png"))

    sub = df[df["max_ticks"] == max_ticks_val]
    for strategy in STRATEGIES:
        s = sub[sub["strategy"] == strategy].sort_values("timeout")
        fig, ax = plt.subplots(figsize=(8, 5), facecolor="white")
        ax.plot(s["timeout"], s["resolved"], marker="o", label="Resolved", color="#2ecc71")
        ax.plot(s["timeout"], s["false_positives"], marker="s", label="False Positives", color="#e74c3c")
        ax.set_title(f"Accuracy Profile — {strategy}  (MAX_TICKS={max_ticks_val})", fontweight="bold")
        ax.set_xlabel("Timeout")
        ax.set_ylabel("Count")
        ax.legend()
        ax.grid(True, alpha=0.3)
        save(fig, os.path.join(FIGURES_DIR, f"accuracy_{strategy.lower()}.png"))

    fig, ax = plt.subplots(figsize=(9, 5), facecolor="white")
    for strategy in STRATEGIES:
        s = sub[sub["strategy"] == strategy].sort_values("timeout")
        ax.plot(s["timeout"], s["fp_rate_pct"], marker="o",
                label=strategy, color=PALETTE[strategy])
    ax.set_title(f"False Positive Rate vs Timeout  (MAX_TICKS={max_ticks_val})", fontweight="bold")
    ax.set_xlabel("Timeout")
    ax.set_ylabel("FP Rate (%)")
    ax.set_ylim(0, 105)
    ax.legend()
    ax.grid(True, alpha=0.3)
    save(fig, os.path.join(FIGURES_DIR, "accuracy_profile.png"))

    fig, ax = plt.subplots(figsize=(9, 5), facecolor="white")
    for strategy in STRATEGIES:
        s = sub[sub["strategy"] == strategy].sort_values("timeout")
        ax.plot(s["timeout"], s["throughput_pct"], marker="o",
                label=strategy, color=PALETTE[strategy])
    ax.set_title(f"Throughput vs Timeout  (MAX_TICKS={max_ticks_val})", fontweight="bold")
    ax.set_xlabel("Timeout")
    ax.set_ylabel("Throughput (%)")
    ax.set_ylim(0, 105)
    ax.legend()
    ax.grid(True, alpha=0.3)
    save(fig, os.path.join(FIGURES_DIR, "throughput_over_timeouts.png"))

    print("Visualization complete.")


if __name__ == "__main__":
    main()

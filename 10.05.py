#!/usr/bin/env python3

from pathlib import Path
import matplotlib.pyplot as plt
import pandas as pd

DATA_FILE = "hash_collisions.csv"
PLOTS_FOLDER = "hash_plots"


def read_statistics(file_path: Path) -> pd.DataFrame:
    table = pd.read_csv(file_path)
    
    expected_fields = {
        "hash_name",
        "string_count",
        "series_index",
        "collision_count",
    }
    
    missing_fields = expected_fields - set(table.columns)
    
    if missing_fields:
        missing_list = ", ".join(sorted(missing_fields))
        raise ValueError(f"Missing columns: {missing_list}")
    
    return table


def aggregate_metrics(table: pd.DataFrame) -> pd.DataFrame:
    stats = (
        table.groupby("hash_name")["collision_count"]
        .agg(["mean", "median", "max", "sum"])
        .reset_index()
        .rename(
            columns={
                "mean": "avg_collisions",
                "median": "mid_collisions", 
                "max": "peak_collisions",
                "sum": "cumulative_collisions",
            }
        )
        .sort_values(
            by=[
                "avg_collisions",
                "peak_collisions", 
                "cumulative_collisions",
                "hash_name",
            ]
        )
        .reset_index(drop=True)
    )
    
    return stats


def write_stats_csv(stats: pd.DataFrame, target_dir: Path) -> None:
    stats.to_csv(target_dir / "statistics.csv", index=False)


def draw_average_plots(table: pd.DataFrame, target_dir: Path) -> None:
    avg_curves = (
        table.groupby(["hash_name", "string_count"])["collision_count"]
        .mean()
        .reset_index()
    )
    
    plt.figure(figsize=(12, 7))
    
    for algo in sorted(avg_curves["hash_name"].unique()):
        subset = avg_curves[avg_curves["hash_name"] == algo]
        plt.plot(
            subset["string_count"],
            subset["collision_count"],
            label=algo,
            linewidth=1.5,
        )
    
    plt.xlabel("Input size")
    plt.ylabel("Average conflicts")
    plt.title("Average collision rate by input size")
    plt.grid(True, alpha=0.3)
    plt.legend(loc='upper left', fontsize=8)
    plt.tight_layout()
    plt.savefig(target_dir / "average_conflicts.png", dpi=200)
    plt.close()


def draw_maximum_plots(table: pd.DataFrame, target_dir: Path) -> None:
    peak_curves = (
        table.groupby(["hash_name", "string_count"])["collision_count"]
        .max()
        .reset_index()
    )
    
    plt.figure(figsize=(12, 7))
    
    for algo in sorted(peak_curves["hash_name"].unique()):
        subset = peak_curves[peak_curves["hash_name"] == algo]
        plt.plot(
            subset["string_count"],
            subset["collision_count"],
            label=algo,
            linewidth=1.5,
        )
    
    plt.xlabel("Input size")
    plt.ylabel("Peak conflicts")
    plt.title("Maximum collisions across experiment runs")
    plt.grid(True, alpha=0.3)
    plt.legend(loc='upper left', fontsize=8)
    plt.tight_layout()
    plt.savefig(target_dir / "peak_conflicts.png", dpi=200)
    plt.close()


def draw_cumulative_barchart(stats: pd.DataFrame, target_dir: Path) -> None:
    plt.figure(figsize=(10, 6))
    
    bars = plt.bar(
        stats["hash_name"], 
        stats["cumulative_collisions"],
        color='steelblue',
        edgecolor='black',
        alpha=0.7
    )
    
    plt.xlabel("Hash algorithm")
    plt.ylabel("Total conflicts")
    plt.title("Aggregated collisions by hash function")
    plt.grid(True, axis='y', alpha=0.3)
    
    for bar, value in zip(bars, stats["cumulative_collisions"]):
        plt.text(
            bar.get_x() + bar.get_width() / 2,
            bar.get_height() + max(stats["cumulative_collisions"]) * 0.01,
            f'{int(value)}',
            ha='center',
            va='bottom',
            fontsize=8
        )
    
    plt.tight_layout()
    plt.savefig(target_dir / "cumulative_conflicts.png", dpi=200)
    plt.close()


def display_summary(stats: pd.DataFrame) -> None:
    print("\n=== Hash Function Performance Summary ===\n")
    print(stats[["hash_name", "avg_collisions", "peak_collisions", "cumulative_collisions"]].to_string(index=False))
    print("\n")



script_location = Path(__file__).resolve().parent
input_path = script_location / DATA_FILE
output_folder = script_location / PLOTS_FOLDER
output_folder.mkdir(exist_ok=True)

raw_data = read_statistics(input_path)
metrics = aggregate_metrics(raw_data)

display_summary(metrics)
write_stats_csv(metrics, output_folder)
draw_average_plots(raw_data, output_folder)
draw_maximum_plots(raw_data, output_folder)
draw_cumulative_barchart(metrics, output_folder)

print(f"✓ Charts saved to: {output_folder}")
print(f"✓ Statistics exported to: {output_folder / 'statistics.csv'}")



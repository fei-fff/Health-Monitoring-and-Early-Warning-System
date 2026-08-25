from __future__ import annotations

import argparse
import csv
from pathlib import Path

from ai_anomaly import HealthAnomalyDetector


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Analyze STM32 health CSV for anomalies")
    parser.add_argument("--csv", default="samples.csv", help="Input CSV file")
    parser.add_argument("--output", default="analysis_result.csv", help="Output CSV file")
    parser.add_argument("--window", type=int, default=30, help="Rolling window size")
    parser.add_argument("--threshold", type=float, default=3.5, help="Robust z-score threshold")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    input_path = Path(args.csv)

    if not input_path.exists():
        print(f"Input file not found: {input_path}")
        return 1

    detector = HealthAnomalyDetector(window_size=args.window, z_threshold=args.threshold)

    with input_path.open("r", newline="", encoding="utf-8") as fin, \
         Path(args.output).open("w", newline="", encoding="utf-8") as fout:
        reader = csv.DictReader(fin)
        writer = csv.writer(fout)
        writer.writerow(
            [
                "timestamp",
                "temp_c",
                "vdd_mv",
                "temp_z",
                "vdd_z",
                "temp_anomaly",
                "vdd_anomaly",
            ]
        )

        anomaly_count = 0

        for row in reader:
            try:
                temp_c = float(row["temp_c"])
                vdd_mv = int(float(row["vdd_mv"]))
            except (KeyError, ValueError):
                continue

            result = detector.update(temp_c, vdd_mv)
            temp_anomaly = int(result["temp_anomaly"])
            vdd_anomaly = int(result["vdd_anomaly"])

            writer.writerow(
                [
                    row.get("timestamp", ""),
                    temp_c,
                    vdd_mv,
                    round(result["temp_z"], 3),
                    round(result["vdd_z"], 3),
                    temp_anomaly,
                    vdd_anomaly,
                ]
            )

            if temp_anomaly or vdd_anomaly:
                anomaly_count += 1

        print(f"Analysis complete. Anomalous records: {anomaly_count}")
        print(f"Output written to: {Path(args.output).resolve()}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())

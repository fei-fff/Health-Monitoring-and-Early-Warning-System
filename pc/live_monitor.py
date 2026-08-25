from __future__ import annotations

import argparse
import csv
from datetime import datetime
from pathlib import Path

from ai_anomaly import HealthAnomalyDetector
from protocol import alarm_names, build_command, parse_ack_line, parse_sample_line

try:
    import serial
except ImportError:
    serial = None


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Live STM32 health monitor with AI anomaly detection")
    parser.add_argument("--port", required=True, help="Serial port, for example COM3")
    parser.add_argument("--baud", type=int, default=115200)
    parser.add_argument("--csv", default="samples.csv", help="CSV output file")
    parser.add_argument("--window", type=int, default=30, help="Anomaly detector window size")
    parser.add_argument("--threshold", type=float, default=3.5, help="Robust z-score threshold")
    parser.add_argument("--on-alarm", default="CMD,LED,1", help="Command sent when anomaly starts")
    parser.add_argument("--on-normal", default="CMD,LED,0", help="Command sent when state returns normal")
    return parser.parse_args()


def open_serial(port: str, baud: int):
    if serial is None:
        raise RuntimeError("pyserial is not installed. Run: python -m pip install pyserial")

    return serial.Serial(port=port, baudrate=baud, timeout=1.0)


def send_command(ser, command: str) -> None:
    line = command if command.endswith("\r\n") else command + "\r\n"
    ser.write(line.encode("ascii"))
    ser.flush()
    print(f"Sent: {command.strip()}")


def main() -> int:
    args = parse_args()

    try:
        ser = open_serial(args.port, args.baud)
    except Exception as exc:
        print(f"Failed to open {args.port}: {exc}")
        return 1

    detector = HealthAnomalyDetector(window_size=args.window, z_threshold=args.threshold)
    csv_path = Path(args.csv)
    csv_file = csv_path.open("a", newline="", encoding="utf-8")
    writer = csv.writer(csv_file)

    if csv_file.tell() == 0:
        writer.writerow(
            [
                "timestamp",
                "uptime_ms",
                "temp_c",
                "vdd_mv",
                "event_count",
                "alarm_flags",
                "alarm_names",
                "temp_z",
                "vdd_z",
                "ai_alarm",
            ]
        )

    ai_alarm_active = False

    print(f"Live monitoring on {args.port}. Ctrl+C to stop.")

    try:
        while True:
            raw = ser.readline()
            if not raw:
                continue

            line = raw.decode("ascii", errors="replace").strip()
            if not line:
                continue

            ack = parse_ack_line(line)
            if ack is not None:
                print(f"ACK {ack[0]} code={ack[1]}")
                continue

            sample = parse_sample_line(line)
            if sample is None:
                print(f"raw: {line}")
                continue

            result = detector.update(sample.temp_c, float(sample.vdd_mv))
            ai_alarm = bool(result["temp_anomaly"] or result["vdd_anomaly"])

            if ai_alarm and not ai_alarm_active:
                print(f"AI ALARM: {alarm_names(sample.alarm_flags)}")
                send_command(ser, args.on_alarm)
                ai_alarm_active = True
            elif not ai_alarm and ai_alarm_active:
                print("AI state returned to normal")
                send_command(ser, args.on_normal)
                ai_alarm_active = False

            timestamp = datetime.now().isoformat(timespec="seconds")
            writer.writerow(
                [
                    timestamp,
                    sample.uptime_ms,
                    sample.temp_c,
                    sample.vdd_mv,
                    sample.event_count,
                    sample.alarm_flags,
                    alarm_names(sample.alarm_flags),
                    round(result["temp_z"], 3),
                    round(result["vdd_z"], 3),
                    int(ai_alarm),
                ]
            )
            csv_file.flush()

            print(
                f"[{timestamp}] temp={sample.temp_c:.1f}C "
                f"vdd={sample.vdd_mv / 1000.0:.3f}V "
                f"z_temp={result['temp_z']:.2f} z_vdd={result['vdd_z']:.2f} "
                f"ai_alarm={ai_alarm}"
            )
    except KeyboardInterrupt:
        print("\nStopped.")
    finally:
        csv_file.close()
        ser.close()

    return 0


if __name__ == "__main__":
    raise SystemExit(main())

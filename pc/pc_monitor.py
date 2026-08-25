from __future__ import annotations

import argparse
import csv
import sys
from datetime import datetime
from pathlib import Path

from protocol import HealthSample, alarm_names, build_command, parse_ack_line, parse_sample_line

try:
    import serial
    from serial.tools import list_ports
except ImportError:
    serial = None


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="STM32 health monitor serial logger")
    parser.add_argument("--port", help="Serial port, for example COM3")
    parser.add_argument("--baud", type=int, default=115200)
    parser.add_argument("--csv", default="samples.csv", help="CSV output file")
    parser.add_argument("--command", help="Send one command then continue listening")
    parser.add_argument("--list-ports", action="store_true", help="List available serial ports")
    return parser.parse_args()


def print_available_ports() -> None:
    if serial is None:
        print("pyserial is not installed. Run: python -m pip install pyserial")
        return

    ports = list_ports.comports()
    if not ports:
        print("No serial ports found.")
        return

    for port in ports:
        print(f"{port.device}: {port.description}")


def open_serial(port: str, baud: int):
    if serial is None:
        raise RuntimeError("pyserial is not installed. Run: python -m pip install pyserial")

    ser = serial.Serial(port=port, baudrate=baud, timeout=1.0)
    return ser


def csv_writer(path: Path):
    file = path.open("a", newline="", encoding="utf-8")
    writer = csv.writer(file)

    if file.tell() == 0:
        writer.writerow(
            [
                "timestamp",
                "uptime_ms",
                "temp_c",
                "vdd_mv",
                "event_count",
                "alarm_flags",
                "alarm_names",
            ]
        )

    return file, writer


def main() -> int:
    args = parse_args()

    if args.list_ports:
        print_available_ports()
        return 0

    if not args.port:
        print("Missing --port. Use --list-ports to discover available ports.")
        return 2

    try:
        ser = open_serial(args.port, args.baud)
    except Exception as exc:
        print(f"Failed to open {args.port}: {exc}")
        return 1

    csv_path = Path(args.csv)
    file, writer = csv_writer(csv_path)

    print(f"Listening on {args.port} at {args.baud}. Ctrl+C to stop.")
    print(f"Logging to {csv_path.resolve()}")

    if args.command:
        command = args.command if args.command.endswith("\r\n") else args.command + "\r\n"
        ser.write(command.encode("ascii"))
        ser.flush()
        print(f"Sent: {command.strip()}")

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
                ]
            )
            file.flush()

            print(
                f"[{timestamp}] temp={sample.temp_c:.1f}C "
                f"vdd={sample.vdd_mv / 1000.0:.3f}V "
                f"events={sample.event_count} "
                f"alarm={alarm_names(sample.alarm_flags)}"
            )
    except KeyboardInterrupt:
        print("\nStopped.")
    finally:
        file.close()
        ser.close()

    return 0


if __name__ == "__main__":
    raise SystemExit(main())

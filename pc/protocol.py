from __future__ import annotations

from dataclasses import dataclass


ALARM_NONE = 0
ALARM_TEMP_HIGH = 1 << 0
ALARM_VDD_LOW = 1 << 1


@dataclass
class HealthSample:
    uptime_ms: int
    temp_c: float
    vdd_mv: int
    event_count: int
    alarm_flags: int


def parse_sample_line(line: str) -> HealthSample | None:
    """Parse one STM32 S frame.

    Frame format:
        S,<uptime_ms>,<temp_x10>,<vdd_mv>,<event_count>,<alarm_flags>\r\n
    """
    text = line.strip()
    parts = text.split(",")

    if len(parts) != 6 or parts[0] != "S":
        return None

    try:
        uptime_ms = int(parts[1])
        temp_c = int(parts[2]) / 10.0
        vdd_mv = int(parts[3])
        event_count = int(parts[4])
        alarm_flags = int(parts[5])
    except ValueError:
        return None

    return HealthSample(
        uptime_ms=uptime_ms,
        temp_c=temp_c,
        vdd_mv=vdd_mv,
        event_count=event_count,
        alarm_flags=alarm_flags,
    )


def parse_ack_line(line: str) -> tuple[str, int] | None:
    """Parse one STM32 ACK frame."""
    text = line.strip()
    parts = text.split(",")

    if len(parts) != 3 or parts[0] != "ACK":
        return None

    try:
        name = parts[1]
        code = int(parts[2])
    except ValueError:
        return None

    return name, code


def build_command(name: str, arg: int | str) -> str:
    """Build a command line for STM32."""
    name = name.upper()

    if name == "LED":
        return f"CMD,LED,{arg}\r\n"
    if name in {"TH_TEMP", "TEMP"}:
        return f"CMD,TH,TEMP,{arg}\r\n"
    if name in {"TH_VDD", "VDD"}:
        return f"CMD,TH,VDD,{arg}\r\n"
    if name == "PAGE":
        return f"CMD,PAGE,{arg}\r\n"

    raise ValueError(f"Unsupported command: {name}")


def alarm_names(alarm_flags: int) -> str:
    names: list[str] = []

    if alarm_flags & ALARM_TEMP_HIGH:
        names.append("TEMP_HIGH")
    if alarm_flags & ALARM_VDD_LOW:
        names.append("VDD_LOW")

    return "|".join(names) if names else "NORMAL"

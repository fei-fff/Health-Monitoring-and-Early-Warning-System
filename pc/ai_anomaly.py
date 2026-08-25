from __future__ import annotations

from collections import deque
from statistics import median
from typing import Any


class HealthAnomalyDetector:
    """Robust rolling anomaly detector for MCU temperature and VDD."""

    def __init__(self, window_size: int = 30, z_threshold: float = 3.5) -> None:
        if window_size < 5:
            raise ValueError("window_size must be at least 5")

        self.window_size = window_size
        self.z_threshold = z_threshold
        self.temp_values: deque[float] = deque(maxlen=window_size)
        self.vdd_values: deque[float] = deque(maxlen=window_size)

    @staticmethod
    def _mad(values: list[float]) -> float:
        med = median(values)
        deviations = [abs(v - med) for v in values]
        return median(deviations)

    @staticmethod
    def _robust_z(value: float, values: list[float]) -> float:
        med = median(values)
        mad = HealthAnomalyDetector._mad(values)

        if mad == 0.0:
            return 0.0

        return 0.6745 * (value - med) / mad

    def update(self, temp_c: float, vdd_mv: float) -> dict[str, Any]:
        self.temp_values.append(float(temp_c))
        self.vdd_values.append(float(vdd_mv))

        result: dict[str, Any] = {
            "warmup": len(self.temp_values) < self.window_size,
            "temp_z": 0.0,
            "vdd_z": 0.0,
            "temp_anomaly": False,
            "vdd_anomaly": False,
        }

        if result["warmup"]:
            return result

        temp_list = list(self.temp_values)
        vdd_list = list(self.vdd_values)

        temp_z = self._robust_z(float(temp_c), temp_list)
        vdd_z = self._robust_z(float(vdd_mv), vdd_list)

        result["temp_z"] = temp_z
        result["vdd_z"] = vdd_z
        result["temp_anomaly"] = temp_z > self.z_threshold
        result["vdd_anomaly"] = vdd_z < -self.z_threshold

        return result

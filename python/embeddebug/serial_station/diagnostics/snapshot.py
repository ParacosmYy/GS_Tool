"""性能快照数据结构：某时刻全部指标的只读切片。

被 :mod:`~embeddebug.serial_station.diagnostics.monitor` 在定时器触发或
手动调用时产生，通过 ``snapshot_ready`` 信号分发给 UI/日志订阅者。
仅依赖标准库与 :class:`PerfMetric`，不访问 transport/controller/UI。
"""

from __future__ import annotations

import time
from dataclasses import dataclass, field

from embeddebug.serial_station.diagnostics.metrics import PerfMetric

# 性能劣化阈值：fps 低于此值或延迟高于此值视为降级。
FPS_DEGRADED_THRESHOLD = 30.0
LATENCY_DEGRADED_THRESHOLD_MS = 100.0


@dataclass
class PerfSnapshot:
    """某时刻的性能快照。

    ``timestamp_ns`` 与 ``uptime_s`` 由 monitor 产生；
    ``metrics`` 为该时刻全部 ``PerfMetric`` 的浅拷贝集合。
    """

    timestamp_ns: int = field(default_factory=time.time_ns)
    uptime_s: float = 0.0
    metrics: dict[str, PerfMetric] = field(default_factory=dict)

    def format_text(self) -> str:
        """格式化为人类可读的多行文本。"""
        lines: list[str] = [
            f"[PerfSnapshot] uptime={self.uptime_s:.3f}s",
            f"  metrics: {len(self.metrics)}",
        ]
        for name in sorted(self.metrics):
            metric = self.metrics[name]
            unit = f"{metric.unit} " if metric.unit else ""
            lines.append(
                f"  - {name}: now={metric.value:.3f} {unit}"
                f"(min={metric.min:.3f} max={metric.max:.3f} "
                f"avg={metric.avg:.3f} n={metric.count})"
            )
        lines.append(f"  degraded: {self.is_degraded()}")
        return "\n".join(lines)

    def is_degraded(self) -> bool:
        """是否处于性能劣化状态（fps<30 或 latency>100ms）。"""
        fps_metric = self.metrics.get("fps")
        if fps_metric is not None and fps_metric.count > 0:
            if fps_metric.value < FPS_DEGRADED_THRESHOLD:
                return True
        latency_metric = self.metrics.get("latency_ms")
        if latency_metric is not None and latency_metric.count > 0:
            if latency_metric.value > LATENCY_DEGRADED_THRESHOLD_MS:
                return True
        return False

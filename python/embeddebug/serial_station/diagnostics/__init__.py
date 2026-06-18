"""性能诊断模块：指标采样、FPS 监控、定时快照。

公共入口：
- :class:`PerfMetric` —— 单指标运行态统计（min/max/avg/count）。
- :class:`PerfSnapshot` —— 某时刻全部指标的只读快照。
- :class:`PerfMonitor` —— ``QObject`` 聚合监控中心，定时广播快照。

约束：仅依赖 PyQt6 + 标准库，不访问 transport/protocol/UI。
"""

from __future__ import annotations

from embeddebug.serial_station.diagnostics.metrics import PerfMetric
from embeddebug.serial_station.diagnostics.monitor import PerfMonitor
from embeddebug.serial_station.diagnostics.snapshot import PerfSnapshot

__all__ = [
    "PerfMetric",
    "PerfMonitor",
    "PerfSnapshot",
]

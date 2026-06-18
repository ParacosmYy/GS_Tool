"""时序图模块。

提供逻辑信号的表示、边沿与电平查询、常见时序参数 (建立时间、保持时间、
传播延迟、周期、占空比) 的测量，以及多信号时序图的组装与文本导出。
"""

from __future__ import annotations

from embeddebug.serial_station.timing_diagram.diagram import TimingDiagram
from embeddebug.serial_station.timing_diagram.measurement import TimingMeasurement
from embeddebug.serial_station.timing_diagram.signal import (
    LogicSample,
    TimingEdge,
    TimingSignal,
)

__all__ = [
    "LogicSample",
    "TimingDiagram",
    "TimingEdge",
    "TimingMeasurement",
    "TimingSignal",
]

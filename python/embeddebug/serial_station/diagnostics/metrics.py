"""性能指标数据结构：单指标的运行态统计（最小/最大/均值/计数）。

本模块只依赖标准库，不访问 controller/transport/UI，便于在任意层被引用。
被 :mod:`~embeddebug.serial_station.diagnostics.monitor` 与
:mod:`~embeddebug.serial_station.diagnostics.snapshot` 复用。
"""

from __future__ import annotations

import time
from dataclasses import dataclass, field
from typing import Any


@dataclass
class PerfMetric:
    """单个性能指标的运行态统计。

    保存最近一次采样值，并维护最小/最大/累计均值/采样次数。
    用于跟踪 fps、吞吐量、延迟等指标随时间的波动。

    ``_sum`` 为内部累加字段，不进入构造参数与 repr，仅用于计算均值。
    """

    name: str
    value: float = 0.0
    unit: str = ""
    timestamp_ns: int = field(default_factory=time.time_ns)
    min: float = 0.0
    max: float = 0.0
    avg: float = 0.0
    count: int = 0
    _sum: float = field(default=0.0, init=False, repr=False)

    def update(self, value: float) -> None:
        """记录一次新采样并更新最小/最大/均值/计数与时间戳。"""
        sample = float(value)
        self.value = sample
        self.timestamp_ns = time.time_ns()
        if self.count == 0:
            self.min = sample
            self.max = sample
        else:
            if sample < self.min:
                self.min = sample
            if sample > self.max:
                self.max = sample
        self._sum += sample
        self.count += 1
        self.avg = self._sum / self.count

    def reset(self) -> None:
        """清空全部统计，恢复到未采样状态。"""
        self.value = 0.0
        self.timestamp_ns = time.time_ns()
        self.min = 0.0
        self.max = 0.0
        self.avg = 0.0
        self.count = 0
        self._sum = 0.0

    def to_dict(self) -> dict[str, Any]:
        """导出为可序列化字典（供快照、日志、UI 展示使用）。"""
        return {
            "name": self.name,
            "value": self.value,
            "unit": self.unit,
            "timestamp_ns": self.timestamp_ns,
            "min": self.min,
            "max": self.max,
            "avg": self.avg,
            "count": self.count,
        }

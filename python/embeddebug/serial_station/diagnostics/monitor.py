"""性能监控中枢：聚合指标采样、按帧计算 FPS、定时产生快照。

遵循 serial_station 分层：本模块只依赖 PyQt6 + 标准库 + 同目录
metrics/snapshot，不访问 transport/protocol/UI，可被 controller/workers
以依赖注入方式持有。后台定时采样通过 ``snapshot_ready`` 信号分发，
遵循约束 §三（跨线程只走信号/队列）。
"""

from __future__ import annotations

import time

from PyQt6.QtCore import QObject, QTimer, pyqtSignal

from embeddebug.serial_station.diagnostics.metrics import PerfMetric
from embeddebug.serial_station.diagnostics.snapshot import PerfSnapshot

# 默认快照周期（毫秒）。UI/日志订阅者按此频率收到一次汇总。
DEFAULT_SNAPSHOT_INTERVAL_MS = 1000
# 默认指标名 -> 单位映射，保证不同订阅者读到一致命名。
DEFAULT_METRIC_UNITS: dict[str, str] = {
    "fps": "Hz",
    "throughput_bps": "Bps",
    "latency_ms": "ms",
    "memory_mb": "MB",
    "cpu_percent": "%",
}


class PerfMonitor(QObject):
    """性能监控中心对象。

    - :meth:`record_metric` 记录任意命名指标的最新采样；
    - :meth:`tick_fps` 在每个 GUI/渲染帧调用，按帧间隔换算 FPS；
    - :meth:`snapshot` 取当前全部指标快照；
    - ``snapshot_ready`` 信号在 ``QTimer`` 周期触发，携带最新快照。
    """

    snapshot_ready = pyqtSignal(object)  # PerfSnapshot

    def __init__(
        self,
        snapshot_interval_ms: int = DEFAULT_SNAPSHOT_INTERVAL_MS,
        parent: QObject | None = None,
    ) -> None:
        super().__init__(parent)
        self._metrics: dict[str, PerfMetric] = {}
        self._start_ns = time.time_ns()
        self._last_tick_ns: int | None = None
        self._timer = QTimer(self)
        self._timer.setInterval(max(1, int(snapshot_interval_ms)))
        self._timer.timeout.connect(self._emit_periodic_snapshot)

    @property
    def start_ns(self) -> int:
        """监控启动时间戳（纳秒）。"""
        return self._start_ns

    @property
    def is_running(self) -> bool:
        """定时快照是否启用。"""
        return self._timer.isActive()

    def _get_or_create(self, name: str, unit: str = "") -> PerfMetric:
        metric = self._metrics.get(name)
        if metric is None:
            unit = unit or DEFAULT_METRIC_UNITS.get(name, "")
            metric = PerfMetric(name=name, unit=unit)
            self._metrics[name] = metric
        return metric

    def record_metric(self, name: str, value: float, unit: str = "") -> None:
        """记录一次任意命名指标的采样。"""
        self._get_or_create(name, unit).update(value)

    def tick_fps(self) -> float:
        """在每帧调用：由相邻帧间隔换算瞬时 FPS 并写入 fps 指标。

        返回本次换算得到的 FPS；首帧无前一帧时返回 0.0。
        """
        now_ns = time.time_ns()
        fps_metric = self._get_or_create("fps")
        if self._last_tick_ns is None:
            self._last_tick_ns = now_ns
            return 0.0
        delta_ns = now_ns - self._last_tick_ns
        self._last_tick_ns = now_ns
        if delta_ns <= 0:
            return 0.0
        fps = 1e9 / delta_ns
        fps_metric.update(fps)
        return fps

    def uptime_s(self) -> float:
        """从启动到现在的秒数。"""
        return (time.time_ns() - self._start_ns) / 1e9

    def snapshot(self) -> PerfSnapshot:
        """生成当前全部指标的只读快照。"""
        return PerfSnapshot(
            timestamp_ns=time.time_ns(),
            uptime_s=self.uptime_s(),
            metrics=dict(self._metrics),
        )

    def start(self) -> None:
        """启动定时快照。"""
        if not self._timer.isActive():
            self._timer.start()

    def stop(self) -> None:
        """停止定时快照。"""
        self._timer.stop()

    def set_snapshot_interval(self, interval_ms: int) -> None:
        """调整快照周期；运行中修改会立即生效。"""
        self._timer.setInterval(max(1, int(interval_ms)))

    def reset(self) -> None:
        """清空全部指标统计与 FPS 帧间状态。"""
        for metric in self._metrics.values():
            metric.reset()
        self._last_tick_ns = None
        self._start_ns = time.time_ns()

    def _emit_periodic_snapshot(self) -> None:
        """定时器槽：产生并广播快照。"""
        self.snapshot_ready.emit(self.snapshot())

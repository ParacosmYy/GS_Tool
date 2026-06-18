"""波形热路径性能优化（对齐 VOFA+ 高频数据不卡顿）。

- ``RefreshThrottle``：按 Hz 节流 UI 刷新，避免逐批次 setData 卡 UI。
- ``BatchAccumulator``：累积多个测量批次，定时一次性 flush，减少信号次数。

遵循 serial_station_architecture §5.4 VOFA+ parity target：
热路径必须用 typed batch、批量信号、定时刷新，不允许逐点信号或无界列表。

约束：本模块只依赖 PyQt6 + numpy + 标准库，不访问 controller/transport。
"""

from __future__ import annotations

import time
from collections.abc import Callable

import numpy as np
from PyQt6.QtCore import QObject, QTimer, pyqtSignal


class RefreshThrottle:
    """按目标 Hz 节流刷新调用。

    多次 ``maybe_refresh`` 在一个间隔窗口内只触发一次真实刷新。
    用 ``QTimer`` 兜底，保证最后一次刷新不丢失。
    """

    def __init__(
        self,
        callback: Callable[[], None],
        target_hz: int = 60,
        parent: QObject | None = None,
    ) -> None:
        self._callback = callback
        self._interval_ms = max(1, int(1000 / max(1, target_hz)))
        self._timer = QTimer(parent)
        self._timer.setSingleShot(True)
        self._timer.setInterval(self._interval_ms)
        self._timer.timeout.connect(self._fire)
        self._pending = False
        self._last_fire = 0.0

    @property
    def target_hz(self) -> int:
        return int(1000 / self._interval_ms)

    def maybe_refresh(self) -> None:
        """请求一次刷新；若在间隔内则延迟到窗口结束。"""

        now = time.monotonic()
        elapsed_ms = (now - self._last_fire) * 1000.0
        if elapsed_ms >= self._interval_ms and not self._timer.isActive():
            self._fire()
            return
        # 在窗口内，安排一次延迟刷新（兜底最后一次）。
        if not self._timer.isActive():
            self._timer.start()

    def _fire(self) -> None:
        self._last_fire = time.monotonic()
        self._callback()

    def stop(self) -> None:
        self._timer.stop()


class BatchAccumulator(QObject):
    """累积测量批次，定时 flush 为单个合并批次。

    用 ``QTimer`` 按 flush_interval_ms 触发 ``flush_signal``，把窗口内
    所有批次拼接成一个 ``ChannelBatch``，减少下游 setData 次数。
    """

    flush_signal = pyqtSignal(object)  # 合并后的 ChannelBatch

    def __init__(
        self,
        flush_interval_ms: int = 50,
        max_batches: int = 64,
        parent: QObject | None = None,
    ) -> None:
        super().__init__(parent)
        self._flush_interval_ms = max(1, flush_interval_ms)
        self._max_batches = max(1, max_batches)
        self._pending: list[np.ndarray] = []
        self._channel_names: tuple[str, ...] = ()
        self._dt_ns: int = 0
        self._timer = QTimer(self)
        self._timer.setInterval(self._flush_interval_ms)
        self._timer.timeout.connect(self.flush)

    def start(self) -> None:
        self._timer.start()

    def stop(self) -> None:
        self._timer.stop()
        self.flush()

    def push(self, values: np.ndarray, channel_names: tuple[str, ...], dt_ns: int) -> None:
        """累积一个测量批次。"""

        if values.size == 0:
            return
        self._pending.append(np.asarray(values, dtype=np.float32))
        self._channel_names = channel_names
        self._dt_ns = dt_ns
        if len(self._pending) >= self._max_batches:
            self.flush()

    def flush(self) -> None:
        """把累积批次合并为一个并发出 flush_signal。"""

        if not self._pending:
            return
        from embeddebug.serial_station.core import ChannelBatch

        merged = np.vstack(self._pending) if len(self._pending) > 1 else self._pending[0]
        self._pending.clear()
        self.flush_signal.emit(
            ChannelBatch(
                channel_names=self._channel_names,
                values=merged,
                dt_ns=self._dt_ns,
            )
        )

    @property
    def pending_count(self) -> int:
        return len(self._pending)

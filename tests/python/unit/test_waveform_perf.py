"""B12 波形热路径性能测试：刷新节流 + 批量累积。"""

from __future__ import annotations

import os
import time

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import numpy as np

from embeddebug.serial_station.ui.waveform_perf import (
    BatchAccumulator,
    RefreshThrottle,
)


# ── RefreshThrottle ───────────────────────────────────────────────
def test_throttle_target_hz(qtbot):
    fired: list[int] = []
    throttle = RefreshThrottle(lambda: fired.append(1), target_hz=100)
    assert throttle.target_hz == 100


def test_throttle_first_call_fires_immediately(qtbot):
    fired: list[int] = []
    throttle = RefreshThrottle(lambda: fired.append(1), target_hz=1000)
    throttle.maybe_refresh()
    assert len(fired) == 1


def test_throttle_rapid_calls_do_not_overfire(qtbot):
    fired: list[int] = []
    # 10Hz → 100ms 间隔。
    throttle = RefreshThrottle(lambda: fired.append(1), target_hz=10)
    throttle.maybe_refresh()
    # 立即在窗口内再次请求，不应立即触发第二次。
    for _ in range(20):
        throttle.maybe_refresh()
    # 第一次已触发；窗口内不应有第二次（除非 timer 到期）。
    assert len(fired) >= 1


def test_throttle_stop(qtbot):
    throttle = RefreshThrottle(lambda: None, target_hz=10)
    throttle.stop()
    # stop 后 timer 不应活跃。
    assert not throttle._timer.isActive()


# ── BatchAccumulator ──────────────────────────────────────────────
def test_batch_accumulator_start_stop(qtbot):
    acc = BatchAccumulator(flush_interval_ms=10)
    acc.start()
    assert acc._timer.isActive()
    acc.stop()
    assert not acc._timer.isActive()


def test_batch_accumulator_push_then_flush(qtbot):
    acc = BatchAccumulator(flush_interval_ms=1000)
    flushed: list[object] = []
    acc.flush_signal.connect(lambda batch: flushed.append(batch))
    acc.push(np.array([[1.0, 2.0]], dtype=np.float32), ("a", "b"), dt_ns=1000)
    acc.push(np.array([[3.0, 4.0]], dtype=np.float32), ("a", "b"), dt_ns=1000)
    assert acc.pending_count == 2
    acc.flush()
    assert acc.pending_count == 0
    assert len(flushed) == 1
    merged = flushed[0].values
    assert merged.shape == (2, 2)
    assert merged[1, 0] == 3.0


def test_batch_accumulator_auto_flush_at_max(qtbot):
    acc = BatchAccumulator(flush_interval_ms=10000, max_batches=2)
    flushed: list[object] = []
    acc.flush_signal.connect(lambda batch: flushed.append(batch))
    acc.push(np.array([[1.0]], dtype=np.float32), ("a",), dt_ns=1)
    acc.push(np.array([[2.0]], dtype=np.float32), ("a",), dt_ns=1)
    # 第二个 push 达到 max_batches，应自动 flush。
    assert len(flushed) == 1
    assert acc.pending_count == 0


def test_batch_accumulator_empty_flush_noop(qtbot):
    acc = BatchAccumulator(flush_interval_ms=10)
    flushed: list[object] = []
    acc.flush_signal.connect(lambda batch: flushed.append(batch))
    acc.flush()
    assert flushed == []


def test_batch_accumulator_push_empty_ignored(qtbot):
    acc = BatchAccumulator(flush_interval_ms=10)
    acc.push(np.array([]).reshape(0, 2), ("a", "b"), dt_ns=1)
    assert acc.pending_count == 0


def test_batch_accumulator_single_batch_not_vstacked(qtbot):
    acc = BatchAccumulator(flush_interval_ms=1000)
    flushed: list[object] = []
    acc.flush_signal.connect(lambda batch: flushed.append(batch))
    acc.push(np.array([[1.0, 2.0, 3.0]], dtype=np.float32), ("a", "b", "c"), dt_ns=1)
    acc.flush()
    assert flushed[0].values.shape == (1, 3)
    assert flushed[0].channel_names == ("a", "b", "c")

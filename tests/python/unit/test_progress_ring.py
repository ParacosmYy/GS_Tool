"""ProgressRing 环形进度条单元测试。

覆盖：
- objectName / 默认量程 / setValue clamp / setRange。
- setValue 启动值动画并最终收敛到目标值。
- setIndeterminate 开启/关闭旋转动画（loopCount == -1、停止后非 Running）。
- setValue 与 indeterminate 互斥。
- paintEvent 在确定/不确定/零量程三种模式下均不抛异常。
- sizeHint 为正方形。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtCore import QAbstractAnimation

from embeddebug.serial_station.ui.controls.progress_ring import ProgressRing


def _make_ring(qtbot, minimum: int = 0, maximum: int = 100, value: int = 0) -> ProgressRing:
    """构造一个 64x64 的 ProgressRing 并注册到 qtbot。"""

    ring = ProgressRing(minimum, maximum, value)
    ring.resize(64, 64)
    qtbot.addWidget(ring)
    return ring


# ── 基础属性 ──────────────────────────────────────────────────────
def test_ring_objectname(qtbot):
    ring = _make_ring(qtbot)
    assert ring.objectName() == "serialStationProgressRing"


def test_ring_default_range(qtbot):
    ring = _make_ring(qtbot)
    assert ring.minimum() == 0
    assert ring.maximum() == 100
    assert ring.value() == 0


def test_ring_set_value_clamps(qtbot):
    ring = _make_ring(qtbot, 0, 100, 0)
    ring.setValue(-5)
    assert ring.value() == 0
    ring.setValue(999)
    assert ring.value() == 100


def test_ring_set_range(qtbot):
    ring = _make_ring(qtbot, 0, 100, 0)
    ring.setRange(10, 20)
    assert ring.minimum() == 10
    assert ring.maximum() == 20


# ── 动画行为 ──────────────────────────────────────────────────────
def test_ring_set_value_starts_animation(qtbot):
    ring = _make_ring(qtbot, 0, 100, 0)
    ring.setValue(50)
    assert ring._value_anim is not None, "setValue should create _value_anim"
    # 推进动画直到结束。
    qtbot.waitUntil(
        lambda: ring._value_anim is None
        or ring._value_anim.state() == QAbstractAnimation.State.Stopped,
        timeout=2000,
    )
    assert ring.displayValue == 50.0


def test_ring_set_indeterminate_starts_spinning(qtbot):
    ring = _make_ring(qtbot)
    ring.setIndeterminate(True)
    assert ring._indeterminate_anim is not None
    assert ring._indeterminate_anim.loopCount() == -1
    assert ring.isIndeterminate() is True


def test_ring_set_indeterminate_false_stops_spinning(qtbot):
    ring = _make_ring(qtbot)
    ring.setIndeterminate(True)
    assert ring._indeterminate_anim is not None
    ring.setIndeterminate(False)
    # 停止后引用被清空，或仍存在但非 Running。
    anim = ring._indeterminate_anim
    assert anim is None or anim.state() != QAbstractAnimation.State.Running
    assert ring.isIndeterminate() is False


def test_ring_set_value_disables_indeterminate(qtbot):
    ring = _make_ring(qtbot)
    ring.setIndeterminate(True)
    assert ring.isIndeterminate() is True
    ring.setValue(50)
    assert ring.isIndeterminate() is False
    assert ring.value() == 50


# ── paintEvent 安全性 ────────────────────────────────────────────
def test_ring_paint_does_not_raise(qtbot):
    ring = _make_ring(qtbot, 0, 100, 30)
    # 确定模式：通过 grab 触发实际绘制。
    ring.grab()
    # 不确定模式：开启旋转后也应可绘制。
    ring.setIndeterminate(True)
    ring.grab()
    ring.setIndeterminate(False)


def test_ring_size_hint(qtbot):
    ring = _make_ring(qtbot)
    hint = ring.sizeHint()
    assert hint.width() > 0 and hint.height() > 0
    assert hint.width() == hint.height(), "sizeHint should be square"


def test_ring_zero_maximum_safe(qtbot):
    """maximum == minimum 时 paintEvent 不应因除零崩溃。"""

    ring = _make_ring(qtbot, 0, 100, 0)
    ring.setRange(0, 0)
    ring.setValue(0)
    # 触发绘制，仅验证不抛异常。
    ring.grab()
    assert ring.minimum() == 0
    assert ring.maximum() == 0

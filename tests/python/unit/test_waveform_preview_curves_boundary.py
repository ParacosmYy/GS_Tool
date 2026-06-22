"""waveform_preview SafePlotWidget + _ensure_curves + _update_legend 边界测试。

覆盖 SafePlotWidget resizeEvent/paintEvent 安全覆盖（offscreen 短路 + RuntimeError 兜底）+
_ensure_curves 曲线创建数 + 多余曲线隐藏 + curve 可见性。

覆盖：
1. SafePlotWidget resizeEvent offscreen 短路（event.accept）。
2. SafePlotWidget paintEvent offscreen 短路（event.accept）。
3. SafePlotWidget resizeEvent None event 安全。
4. _ensure_curves 创建曲线数 == channel_names 长度。
5. _ensure_curves 通道减少时多余曲线 setVisible(False)。
6. _ensure_curves 通道增加时新增曲线。
7. _ensure_curves 空 channel_names 不创建曲线。
8. _update_legend 不崩溃（空 batch / 有 batch）。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import numpy as np

from embeddebug.serial_station.core import ChannelBatch
from embeddebug.serial_station.ui.waveform_preview import (
    SafePlotWidget,
    SerialWaveformPreview,
)


class _FakeEvent:
    """模拟 QResizeEvent/QPaintEvent（有 accept 方法）。"""

    def __init__(self) -> None:
        self.accepted = False

    def accept(self) -> None:
        self.accepted = True


# ── SafePlotWidget resizeEvent ───────────────────────────────────
def test_resize_event_offscreen_accepts(qtbot):
    """offscreen 模式 resizeEvent 短路 accept（不调 super）。"""

    plot = SafePlotWidget()
    qtbot.addWidget(plot)
    event = _FakeEvent()
    plot.resizeEvent(event)
    assert event.accepted is True


def test_paint_event_offscreen_accepts(qtbot):
    """offscreen 模式 paintEvent 短路 accept。"""

    plot = SafePlotWidget()
    qtbot.addWidget(plot)
    event = _FakeEvent()
    plot.paintEvent(event)
    assert event.accepted is True


def test_resize_event_none_event_safe(qtbot):
    """resizeEvent(None) 不崩溃（offscreen 短路前 None 守卫）。"""

    plot = SafePlotWidget()
    qtbot.addWidget(plot)
    plot.resizeEvent(None)  # 不抛


# ── _ensure_curves 曲线创建 ──────────────────────────────────────
def test_ensure_curves_creates_correct_count(qtbot):
    """_ensure_curves 创建曲线数 == channel_names 长度。"""

    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    preview._ensure_curves(("ch1", "ch2", "ch3"))
    assert len(preview._curves) == 3


def test_ensure_curves_empty_names_no_curves(qtbot):
    """空 channel_names 不创建曲线。"""

    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    preview._ensure_curves(())
    assert len(preview._curves) == 0


def test_ensure_curves_increments_on_add(qtbot):
    """通道增加时新增曲线（不重建已有）。"""

    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    preview._ensure_curves(("ch1",))
    assert len(preview._curves) == 1
    preview._ensure_curves(("ch1", "ch2", "ch3"))
    assert len(preview._curves) == 3


def test_ensure_curves_hides_excess_on_reduce(qtbot):
    """通道减少时多余曲线 setVisible(False)。"""

    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    preview._ensure_curves(("ch1", "ch2", "ch3"))
    # 记录第 3 条曲线引用。
    excess_curve = preview._curves[2]
    preview._ensure_curves(("ch1",))  # 减到 1 通道
    assert excess_curve.isVisible() is False


def test_ensure_curves_keeps_visible_for_active(qtbot):
    """活跃通道曲线保持可见。"""

    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    preview._ensure_curves(("ch1", "ch2"))
    assert preview._curves[0].isVisible() is True
    assert preview._curves[1].isVisible() is True


def test_ensure_curves_reactivate_after_reduce(qtbot):
    """减少后重新增加通道 → 被隐藏的曲线恢复可见。"""

    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    preview._ensure_curves(("ch1", "ch2", "ch3"))
    preview._ensure_curves(("ch1",))  # ch2/ch3 隐藏
    preview._ensure_curves(("ch1", "ch2", "ch3"))  # 恢复
    assert preview._curves[1].isVisible() is True
    assert preview._curves[2].isVisible() is True


# ── _update_legend ───────────────────────────────────────────────
def test_update_legend_empty_batch_no_crash(qtbot):
    """_update_legend 空 batch 不崩溃。"""

    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    preview._update_legend(ChannelBatch(channel_names=(), values=np.empty((0, 0), dtype=np.float32)))


def test_update_legend_with_channels_no_crash(qtbot):
    """_update_legend 有通道 batch 不崩溃。"""

    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    batch = ChannelBatch(
        channel_names=("temp", "volt"),
        values=np.array([[24.5, 3.3]], dtype=np.float32),
    )
    preview._update_legend(batch)

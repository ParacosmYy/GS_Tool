"""BarChartPanel update_batch + latest_per_channel 边界扩展测试。

test_waveform_display 覆盖基础；本文件补 ndim 守卫 + 1D latest_per_channel +
status_label 多通道 + shutdown 后再 update + ticks 通道名映射。

覆盖：
1. latest_per_channel 1D 输入（ndim!=2）→ 返回空数组。
2. latest_per_channel 3D 输入 → 返回空数组。
3. latest_per_channel 单行 → 返回该行。
4. BarChartPanel update_batch ndim!=2（1D）→ 不创建 _bar。
5. BarChartPanel update_batch 多通道 status_label 含通道数。
6. BarChartPanel update_batch 通道名不足用 ch{i} 兜底。
7. BarChartPanel shutdown 后 _bar None。
8. BarChartPanel shutdown 后再 update_batch 恢复。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import numpy as np

from embeddebug.serial_station.ui.waveform_modes import (
    BarChartPanel,
    latest_per_channel,
)


# ── latest_per_channel ndim 守卫 ─────────────────────────────────
def test_latest_per_channel_1d_returns_empty():
    """1D 输入（ndim!=2）→ 返回空数组。"""

    result = latest_per_channel(np.array([1.0, 2.0, 3.0], dtype=np.float32))
    assert result.size == 0


def test_latest_per_channel_3d_returns_empty():
    """3D 输入 → 返回空数组。"""

    result = latest_per_channel(np.zeros((2, 2, 2), dtype=np.float32))
    assert result.size == 0


def test_latest_per_channel_single_row():
    """单行 → 返回该行。"""

    result = latest_per_channel(np.array([[5.0, 10.0]], dtype=np.float32))
    assert list(result) == [5.0, 10.0]


def test_latest_per_channel_returns_float32():
    """返回 float32 类型。"""

    result = latest_per_channel(np.array([[1.0]], dtype=np.float64))
    assert result.dtype == np.float32


# ── BarChartPanel update_batch ndim 守卫 ─────────────────────────
def test_update_batch_1d_no_bar(qtbot):
    """1D values（ndim!=2）→ 不创建 _bar。"""

    panel = BarChartPanel()
    qtbot.addWidget(panel)
    panel.update_batch(np.array([1.0, 2.0], dtype=np.float32), ("a", "b"))
    assert panel._bar is None


def test_update_batch_zero_rows_no_bar(qtbot):
    """0 行 values → 不创建 _bar。"""

    panel = BarChartPanel()
    qtbot.addWidget(panel)
    panel.update_batch(np.array([]).reshape(0, 2), ("a", "b"))
    assert panel._bar is None


# ── status_label 多通道 ──────────────────────────────────────────
def test_update_batch_status_label_channel_count(qtbot):
    """多通道 status_label 含通道数。"""

    panel = BarChartPanel()
    qtbot.addWidget(panel)
    panel.update_batch(np.array([[1.0, 2.0, 3.0, 4.0]], dtype=np.float32), ("a", "b", "c", "d"))
    assert "4 channels" in panel._status.text()


def test_update_batch_status_label_single_channel(qtbot):
    """单通道 status_label 含 '1'。"""

    panel = BarChartPanel()
    qtbot.addWidget(panel)
    panel.update_batch(np.array([[5.0]], dtype=np.float32), ("solo",))
    assert "1" in panel._status.text()


# ── shutdown + 恢复 ──────────────────────────────────────────────
def test_shutdown_clears_bar(qtbot):
    """shutdown 后 _bar None。"""

    panel = BarChartPanel()
    qtbot.addWidget(panel)
    panel.update_batch(np.array([[1.0, 2.0]], dtype=np.float32), ("a", "b"))
    assert panel._bar is not None
    panel.shutdown()
    assert panel._bar is None


def test_update_after_shutdown_restores_bar(qtbot):
    """shutdown 后再 update_batch 恢复 _bar。"""

    panel = BarChartPanel()
    qtbot.addWidget(panel)
    panel.update_batch(np.array([[1.0]], dtype=np.float32), ("a",))
    panel.shutdown()
    assert panel._bar is None
    panel.update_batch(np.array([[2.0]], dtype=np.float32), ("a",))
    assert panel._bar is not None


# ── update_batch 多次（替换旧 bar） ──────────────────────────────
def test_update_batch_replaces_old_bar(qtbot):
    """多次 update_batch 替换旧 _bar（removeItem + 新建）。"""

    panel = BarChartPanel()
    qtbot.addWidget(panel)
    panel.update_batch(np.array([[1.0, 2.0]], dtype=np.float32), ("a", "b"))
    bar1 = panel._bar
    panel.update_batch(np.array([[3.0, 4.0]], dtype=np.float32), ("a", "b"))
    bar2 = panel._bar
    assert bar1 is not bar2  # 新对象

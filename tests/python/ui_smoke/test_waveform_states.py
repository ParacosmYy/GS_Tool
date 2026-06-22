"""B49-3 ui_smoke: 波形区空态 + 连接加载态切换。

覆盖：
- 空态：无 batch 时显示 EmptyStateWidget（activity 图标 + 「等待波形数据」）。
- 加载态：set_connecting(True) 显示 ProgressRing 覆盖层；set_connecting(False) 隐藏。
- 切换：首个 batch submit 后空态淡出；连接中空态被加载态替代。

submit_batch 是异步（BatchAccumulator 50ms flush），用 update_batch 直接触发
同步刷新以确定性验证空态隐藏。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import numpy as np
from embeddebug.serial_station.core import ChannelBatch
from embeddebug.serial_station.ui.waveform_preview import SerialWaveformPreview


def _make_preview(qtbot):
    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    preview.resize(400, 300)
    preview.show()
    qtbot.waitUntil(lambda: preview.isVisible(), timeout=1000)
    return preview


def test_waveform_empty_state_visible_on_startup(qtbot):
    """启动时波形面板应显示空态占位（activity 图标 + 标题）。"""

    preview = _make_preview(qtbot)
    empty = preview._empty_overlay
    assert empty is not None
    assert not empty.isHidden()
    # 空态标题应包含「等待波形数据」。
    title = empty._title_label.text()
    assert "等待波形数据" in title or "waveform" in title.lower() or "sample" in title.lower()


def test_waveform_loading_overlay_hidden_initially(qtbot):
    """连接加载态覆盖层初始隐藏。"""

    preview = _make_preview(qtbot)
    assert preview._loading_overlay.isHidden()


def test_waveform_set_connecting_shows_loading(qtbot):
    """set_connecting(True) 显示加载 overlay，隐藏空态。"""

    preview = _make_preview(qtbot)
    preview.set_connecting(True)
    assert not preview._loading_overlay.isHidden()
    # 空态在连接中应被隐藏（避免与 loading overlay 重叠）。
    assert preview._empty_overlay.isHidden()


def test_waveform_set_connecting_false_restores_empty(qtbot):
    """set_connecting(False) 且无 batch 时，重新显示空态。"""

    preview = _make_preview(qtbot)
    preview.set_connecting(True)
    preview.set_connecting(False)
    # 无 batch 到达 → 空态重新显示。
    assert not preview._empty_overlay.isHidden()
    assert preview._loading_overlay.isHidden()


def test_waveform_first_batch_hides_empty(qtbot):
    """首个 batch 到达后空态淡出（update_batch 同步路径）。"""

    preview = _make_preview(qtbot)
    values = np.random.rand(20, 1).astype(np.float32) * 5.0
    batch = ChannelBatch(channel_names=("ch0",), values=values)
    preview.update_batch(batch)
    # 空态应已隐藏（update_batch 是真实刷新，空态不再需要）。
    qtbot.waitUntil(lambda: preview._empty_overlay.isHidden(), timeout=2000)


def test_waveform_set_connecting_idempotent(qtbot):
    """重复 set_connecting 同值不应崩溃（防御性）。"""

    preview = _make_preview(qtbot)
    preview.set_connecting(True)
    preview.set_connecting(True)
    preview.set_connecting(False)
    preview.set_connecting(False)
    assert preview._loading_overlay.isHidden()

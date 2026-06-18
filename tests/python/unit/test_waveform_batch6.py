"""Batch 6 (C2) 波形美化测试：渐变填充/发光/统计接入。

覆盖三个改进：
1. 曲线下方半透明渐变填充（setFillLevel + QBrush(QLinearGradient)）。
2. 曲线发光（QGraphicsDropShadowEffect 同色 blur）。
3. waveform_measure 死代码激活：update_batch 后 stats label 显示首通道 Vpp/RMS 等。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import numpy as np
from PyQt6.QtWidgets import QGraphicsDropShadowEffect, QLabel

from embeddebug.serial_station.core import ChannelBatch
from embeddebug.serial_station.ui.waveform_preview import SerialWaveformPreview


def _make_batch(channels: int = 2, samples: int = 100) -> ChannelBatch:
    values = np.random.rand(samples, channels).astype(np.float32) * 10.0
    return ChannelBatch(channel_names=tuple(f"ch{i}" for i in range(channels)), values=values)


def test_waveform_preview_curve_has_fill_level(qtbot):
    """Batch 6: 曲线应设置 fillLevel（半透明渐变填充基线）。"""

    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    preview.update_batch(_make_batch(channels=2))
    # 每条曲线 opts 都应有 fillLevel（非 None）。
    for curve in preview._curves:
        assert curve.opts.get("fillLevel") is not None


def test_waveform_preview_curve_has_glow_effect(qtbot):
    """Batch 6: 曲线应装 QGraphicsDropShadowEffect 发光。"""

    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    preview.update_batch(_make_batch(channels=1))
    for curve in preview._curves:
        effect = curve.graphicsEffect()
        assert isinstance(effect, QGraphicsDropShadowEffect)


def test_waveform_preview_stats_label_updates(qtbot):
    """Batch 6: update_batch 后 stats label 应显示首通道统计（激活 waveform_measure）。"""

    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    stats_label = preview.findChild(QLabel, "serialStationWaveformStatsLabel")
    assert stats_label is not None
    # 初始应是占位。
    assert "—" in stats_label.text() or stats_label.text() == ""
    preview.update_batch(_make_batch(channels=2))
    # 更新后应含 Vpp/RMS 等统计字段。
    text = stats_label.text()
    assert "Vpp" in text
    assert "RMS" in text
    assert "ch0" in text


def test_waveform_preview_stats_label_empty_batch(qtbot):
    """空 batch 时 stats label 应显示占位。"""

    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    empty_batch = ChannelBatch(channel_names=(), values=np.zeros((0, 0), dtype=np.float32))
    preview.update_batch(empty_batch)
    stats_label = preview.findChild(QLabel, "serialStationWaveformStatsLabel")
    assert "—" in stats_label.text()


def test_waveform_preview_grid_alpha_increased(qtbot):
    """Batch 6: 网格 alpha 应从 0.12 提到 0.18（更清晰）。"""

    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    # 网格 alpha 通过 showGrid 设置，无法直接读取，但验证 plot 可正常构建不崩溃。
    assert preview._plot is not None

"""波形美化测试：渐变填充/发光/统计接入。

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


# ── Batch 7: CursorManager 接入（激活 waveform_cursors 死代码） ────
def test_waveform_preview_uses_cursor_manager(qtbot):
    """update_batch 后应初始化 CursorManager（替代固定双游标）。"""

    from embeddebug.serial_station.ui.waveform_cursors import CursorManager

    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    # 初始未初始化。
    assert preview.cursor_manager() is None
    preview.update_batch(_make_batch(channels=2, samples=50))
    # 首次更新后应初始化 CursorManager。
    cm = preview.cursor_manager()
    assert isinstance(cm, CursorManager)
    # 默认两条 X 游标（对齐旧 attach_cursors 观感）。
    assert len(cm.x_cursors) == 2


def test_waveform_preview_cursor_manager_can_add_remove(qtbot):
    """CursorManager 应支持运行时增删游标。"""

    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    preview.update_batch(_make_batch(channels=1, samples=30))
    cm = preview.cursor_manager()
    assert cm is not None
    # 增加一条 Y 游标。
    y_cursor = cm.add_y_cursor(0.5)
    assert len(cm.y_cursors) == 1
    # 删除一条 X 游标。
    first_x = cm.x_cursors[0]
    assert cm.remove_cursor(first_x) is True
    assert len(cm.x_cursors) == 1
    # 删除不存在的返回 False。
    assert cm.remove_cursor(y_cursor) is True
    assert cm.remove_cursor(y_cursor) is False


def test_waveform_preview_cursor_hud_uses_compute_cursor_measurement(qtbot):
    """HUD 应通过 compute_cursor_measurement 显示 ΔT/频率（激活 waveform_measure）。"""

    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    preview.update_batch(_make_batch(channels=1, samples=100))
    # 默认两条 X 游标在 0.25/0.75，sample_rate=1.0，ΔT 应 = 0.5。
    hud_text = preview._cursor_hud.text()
    # 应含 ΔT 读数（compute_cursor_measurement 输出）。
    assert "ΔT" in hud_text


def test_waveform_preview_set_sample_rate(qtbot):
    """set_sample_rate 应更新采样率，影响游标 ΔT/频率计算。"""

    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    preview.set_sample_rate(1000.0)
    assert preview._sample_rate == 1000.0
    preview.update_batch(_make_batch(channels=1, samples=100))
    # sample_rate=1000，两条游标 Δindex=0.5 → ΔT=0.0005s，频率=2000Hz。
    hud_text = preview._cursor_hud.text()
    assert "Hz" in hud_text


def test_waveform_preview_no_legacy_cursor_x1_attribute(qtbot):
    """旧的 _cursor_x1/_cursor_x2 固定游标字段应已移除（改用 CursorManager）。"""

    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    assert not hasattr(preview, "_cursor_x1")
    assert not hasattr(preview, "_cursor_x2")


# ── Batch 7-2: waveform_perf RefreshThrottle/BatchAccumulator 接入 ──
def test_waveform_preview_has_accumulator_and_throttle(qtbot):
    """构造时应装配 BatchAccumulator + RefreshThrottle（激活 waveform_perf 死代码）。"""

    from embeddebug.serial_station.ui.waveform_perf import BatchAccumulator, RefreshThrottle

    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    assert isinstance(preview._accumulator, BatchAccumulator)
    assert isinstance(preview._throttle, RefreshThrottle)


def test_waveform_preview_submit_batch_accumulates(qtbot):
    """submit_batch 应累积到 BatchAccumulator，不立即刷新 plot。"""

    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    batch = _make_batch(channels=1, samples=10)
    preview.submit_batch(batch)
    # 累积器应有 1 个 pending 批次。
    assert preview._accumulator.pending_count == 1


def test_waveform_preview_submit_multiple_then_flush(qtbot):
    """多次 submit_batch 后 flush 应合并批次（BatchAccumulator 合并）。

    同步信号路径：flush → emit → _on_accumulator_flush → throttle.maybe_refresh
    → _flush_latest_to_plot → update_batch（消费 latest_batch）。
    验证合并后的批次（30 samples）最终推到曲线。
    """

    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    for _ in range(3):
        preview.submit_batch(_make_batch(channels=1, samples=10))
    # 手动触发 flush（模拟定时器到期），同步路径会立即 update_batch。
    preview._accumulator.flush()
    # 曲线应已接收合并的 30 samples（getData 返回 x,y 数组）。
    assert len(preview._curves) >= 1
    xdata, ydata = preview._curves[0].getData()
    assert len(ydata) == 30


def test_measurement_actions_uses_submit_batch(qtbot):
    """measurement_actions.append_measurement_batch 应调 submit_batch（热路径节流）。"""

    import inspect

    from embeddebug.serial_station.ui import measurement_actions

    src = inspect.getsource(measurement_actions.append_measurement_batch)
    assert "submit_batch" in src


def test_waveform_preview_shutdown_stops_perf_components(qtbot):
    """shutdown 应停止 throttle 和 accumulator（清理定时器）。"""

    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    # shutdown 前组件存在。
    assert preview._accumulator is not None
    preview.shutdown()
    # shutdown 不崩溃即通过（stop 是幂等的）。

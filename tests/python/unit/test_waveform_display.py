"""波形预览美化 + CursorManager/性能组件接入 + 李萨如/条形图/独立缩放测试。"""

from __future__ import annotations

import os
os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import inspect
import numpy as np
import pyqtgraph as pg
from PyQt6.QtWidgets import QGraphicsDropShadowEffect, QLabel

from embeddebug.serial_station.core import ChannelBatch
from embeddebug.serial_station.ui import waveform_lissajous, waveform_modes
from embeddebug.serial_station.ui.waveform_preview import SerialWaveformPreview

def _make_batch(channels: int = 2, samples: int = 100) -> ChannelBatch:
    values = np.random.rand(samples, channels).astype(np.float32) * 10.0
    return ChannelBatch(channel_names=tuple(f"ch{i}" for i in range(channels)), values=values)

def test_waveform_preview_curve_has_fill_level(qtbot):
    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    preview.update_batch(_make_batch(channels=2))
    for curve in preview._curves:
        assert curve.opts.get("fillLevel") is not None

def test_waveform_preview_curve_has_glow_effect(qtbot):
    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    preview.update_batch(_make_batch(channels=1))
    for curve in preview._curves:
        effect = curve.graphicsEffect()
        assert isinstance(effect, QGraphicsDropShadowEffect)

def test_waveform_preview_stats_label_updates(qtbot):
    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    stats_label = preview.findChild(QLabel, "serialStationWaveformStatsLabel")
    assert stats_label is not None
    assert "—" in stats_label.text() or stats_label.text() == ""
    preview.update_batch(_make_batch(channels=2))
    text = stats_label.text()
    assert "Vpp" in text and "RMS" in text and "ch0" in text

def test_waveform_preview_stats_label_empty_batch(qtbot):
    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    empty_batch = ChannelBatch(channel_names=(), values=np.zeros((0, 0), dtype=np.float32))
    preview.update_batch(empty_batch)
    stats_label = preview.findChild(QLabel, "serialStationWaveformStatsLabel")
    assert "—" in stats_label.text()

def test_waveform_preview_grid_alpha_increased(qtbot):
    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    assert preview._plot is not None

def test_waveform_preview_uses_cursor_manager(qtbot):
    from embeddebug.serial_station.ui.waveform_cursors import CursorManager
    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    assert preview.cursor_manager() is None
    preview.update_batch(_make_batch(channels=2, samples=50))
    cm = preview.cursor_manager()
    assert isinstance(cm, CursorManager)
    assert len(cm.x_cursors) == 2

def test_waveform_preview_cursor_manager_can_add_remove(qtbot):
    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    preview.update_batch(_make_batch(channels=1, samples=30))
    cm = preview.cursor_manager()
    y_cursor = cm.add_y_cursor(0.5)
    assert len(cm.y_cursors) == 1
    first_x = cm.x_cursors[0]
    assert cm.remove_cursor(first_x) is True
    assert len(cm.x_cursors) == 1
    assert cm.remove_cursor(y_cursor) is True
    assert cm.remove_cursor(y_cursor) is False

def test_waveform_preview_cursor_hud_uses_compute_cursor_measurement(qtbot):
    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    preview.update_batch(_make_batch(channels=1, samples=100))
    assert "ΔT" in preview._cursor_hud.text()

def test_waveform_preview_set_sample_rate(qtbot):
    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    preview.set_sample_rate(1000.0)
    assert preview._sample_rate == 1000.0
    preview.update_batch(_make_batch(channels=1, samples=100))
    assert "Hz" in preview._cursor_hud.text()

def test_waveform_preview_no_legacy_cursor_x1_attribute(qtbot):
    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    assert not hasattr(preview, "_cursor_x1")
    assert not hasattr(preview, "_cursor_x2")

def test_waveform_preview_has_accumulator_and_throttle(qtbot):
    from embeddebug.serial_station.ui.waveform_perf import BatchAccumulator, RefreshThrottle
    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    assert isinstance(preview._accumulator, BatchAccumulator)
    assert isinstance(preview._throttle, RefreshThrottle)

def test_waveform_preview_submit_batch_accumulates(qtbot):
    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    batch = _make_batch(channels=1, samples=10)
    preview.submit_batch(batch)
    assert preview._accumulator.pending_count == 1

def test_waveform_preview_submit_multiple_then_flush(qtbot):
    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    for _ in range(3):
        preview.submit_batch(_make_batch(channels=1, samples=10))
    preview._accumulator.flush()
    assert len(preview._curves) >= 1
    _xdata, ydata = preview._curves[0].getData()
    assert len(ydata) == 30

def test_measurement_actions_uses_submit_batch(qtbot):
    from embeddebug.serial_station.ui import measurement_actions
    src = inspect.getsource(measurement_actions.append_measurement_batch)
    assert "submit_batch" in src

def test_waveform_preview_shutdown_stops_perf_components(qtbot):
    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    assert preview._accumulator is not None
    preview.shutdown()

def test_lissajous_panel_has_objectnames(qtbot):
    panel = waveform_lissajous.LissajousPanel()
    qtbot.addWidget(panel)
    assert panel.objectName() == "serialStationLissajousPanel"
    assert panel.findChild(pg.PlotWidget, "serialStationLissajousPlot") is not None

def test_lissajous_set_channels(qtbot):
    panel = waveform_lissajous.LissajousPanel()
    qtbot.addWidget(panel)
    panel.set_channels(2, 3)
    assert panel._x_channel == 2
    assert panel._y_channel == 3

def test_lissajous_update_batch_draws_curve(qtbot):
    panel = waveform_lissajous.LissajousPanel()
    qtbot.addWidget(panel)
    panel.set_channels(0, 1)
    t = np.linspace(0, 2 * np.pi, 200)
    values = np.stack([np.sin(t), np.cos(t)], axis=1).astype(np.float32)
    panel.update_batch(values, ("sin", "cos"))
    assert panel._curve is not None
    assert "sin" in panel._status.text() and "cos" in panel._status.text()

def test_lissajous_update_empty_does_nothing(qtbot):
    panel = waveform_lissajous.LissajousPanel()
    qtbot.addWidget(panel)
    panel.update_batch(np.array([]).reshape(0, 2), ("a", "b"))
    assert panel._curve is None

def test_lissajous_clamps_channel_index(qtbot):
    panel = waveform_lissajous.LissajousPanel()
    qtbot.addWidget(panel)
    panel.set_channels(99, 99)
    panel.update_batch(np.ones((10, 2), dtype=np.float32), ("a", "b"))

def test_lissajous_shutdown_clears(qtbot):
    panel = waveform_lissajous.LissajousPanel()
    qtbot.addWidget(panel)
    t = np.linspace(0, 1, 50)
    values = np.stack([t, t * 2], axis=1).astype(np.float32)
    panel.update_batch(values, ("a", "b"))
    panel.shutdown()
    assert panel._curve is None

def test_bar_chart_panel_has_objectnames(qtbot):
    panel = waveform_modes.BarChartPanel()
    qtbot.addWidget(panel)
    assert panel.objectName() == "serialStationBarChartPanel"
    assert panel.findChild(pg.PlotWidget, "serialStationBarChartPlot") is not None

def test_bar_chart_update_batch_draws_bars(qtbot):
    panel = waveform_modes.BarChartPanel()
    qtbot.addWidget(panel)
    values = np.array([[1.0, 2.0, 3.0], [4.0, 5.0, 6.0]], dtype=np.float32)
    panel.update_batch(values, ("a", "b", "c"))
    assert panel._bar is not None
    assert "3 channels" in panel._status.text()

def test_bar_chart_update_empty_does_nothing(qtbot):
    panel = waveform_modes.BarChartPanel()
    qtbot.addWidget(panel)
    panel.update_batch(np.array([]).reshape(0, 3), ("a", "b", "c"))
    assert panel._bar is None

def test_bar_chart_shutdown_clears(qtbot):
    panel = waveform_modes.BarChartPanel()
    qtbot.addWidget(panel)
    panel.update_batch(np.array([[1.0, 2.0]], dtype=np.float32), ("a", "b"))
    panel.shutdown()
    assert panel._bar is None

def test_latest_per_channel_returns_last_row(qtbot):
    values = np.array([[1.0, 2.0], [3.0, 4.0]], dtype=np.float32)
    latest = waveform_modes.latest_per_channel(values)
    assert list(latest) == [3.0, 4.0]

def test_latest_per_channel_empty(qtbot):
    latest = waveform_modes.latest_per_channel(np.array([]).reshape(0, 2))
    assert latest.size == 0

def test_zoom_mode_both_enables_xy(qtbot):
    plot = pg.PlotWidget()
    qtbot.addWidget(plot)
    waveform_modes.apply_zoom_mode(plot, waveform_modes.ZoomMode.BOTH)
    view = plot.getViewBox()
    assert view.state["mouseEnabled"][0] is True
    assert view.state["mouseEnabled"][1] is True

def test_zoom_mode_x_only_disables_y(qtbot):
    plot = pg.PlotWidget()
    qtbot.addWidget(plot)
    waveform_modes.apply_zoom_mode(plot, waveform_modes.ZoomMode.X_ONLY)
    view = plot.getViewBox()
    assert view.state["mouseEnabled"][0] is True
    assert view.state["mouseEnabled"][1] is False

def test_zoom_mode_y_only_disables_x(qtbot):
    plot = pg.PlotWidget()
    qtbot.addWidget(plot)
    waveform_modes.apply_zoom_mode(plot, waveform_modes.ZoomMode.Y_ONLY)
    view = plot.getViewBox()
    assert view.state["mouseEnabled"][0] is False
    assert view.state["mouseEnabled"][1] is True

def test_zoom_mode_enum_values():
    assert waveform_modes.ZoomMode.BOTH.value == "both"
    assert waveform_modes.ZoomMode.X_ONLY.value == "x_only"
    assert waveform_modes.ZoomMode.Y_ONLY.value == "y_only"


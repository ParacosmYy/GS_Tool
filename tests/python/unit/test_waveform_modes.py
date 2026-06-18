"""B5 波形进阶测试：李萨如 / 条形图 / 独立缩放。"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import numpy as np
import pyqtgraph as pg

from embeddebug.serial_station.ui import waveform_lissajous
from embeddebug.serial_station.ui import waveform_modes


# ── 李萨如 ────────────────────────────────────────────────────────
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
    assert "sin" in panel._status.text()
    assert "cos" in panel._status.text()


def test_lissajous_update_empty_does_nothing(qtbot):
    panel = waveform_lissajous.LissajousPanel()
    qtbot.addWidget(panel)
    panel.update_batch(np.array([]).reshape(0, 2), ("a", "b"))
    assert panel._curve is None


def test_lissajous_clamps_channel_index(qtbot):
    panel = waveform_lissajous.LissajousPanel()
    qtbot.addWidget(panel)
    panel.set_channels(99, 99)
    values = np.ones((10, 2), dtype=np.float32)
    panel.update_batch(values, ("a", "b"))
    # 不崩溃即通过（内部 clamp 到 0/1）。


def test_lissajous_shutdown_clears(qtbot):
    panel = waveform_lissajous.LissajousPanel()
    qtbot.addWidget(panel)
    t = np.linspace(0, 1, 50)
    values = np.stack([t, t * 2], axis=1).astype(np.float32)
    panel.update_batch(values, ("a", "b"))
    panel.shutdown()
    assert panel._curve is None


# ── 条形图 ────────────────────────────────────────────────────────
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
    values = np.array([[1.0, 2.0]], dtype=np.float32)
    panel.update_batch(values, ("a", "b"))
    panel.shutdown()
    assert panel._bar is None


def test_latest_per_channel_returns_last_row(qtbot):
    values = np.array([[1.0, 2.0], [3.0, 4.0]], dtype=np.float32)
    latest = waveform_modes.latest_per_channel(values)
    assert list(latest) == [3.0, 4.0]


def test_latest_per_channel_empty(qtbot):
    latest = waveform_modes.latest_per_channel(np.array([]).reshape(0, 2))
    assert latest.size == 0


# ── 独立缩放 ──────────────────────────────────────────────────────
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

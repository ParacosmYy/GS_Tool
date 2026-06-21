"""波形游标 HUD/图例、面板动画、响应式布局 + overlays 死代码守护 + 模式集成测试。"""
from __future__ import annotations
import os
os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")
import inspect
import re
import numpy as np
import pyqtgraph as pg
from PyQt6.QtWidgets import QLabel, QSplitter, QWidget
from embeddebug.serial_station.ui import (
    panel_animations, responsive_layout, waveform_lissajous, waveform_modes, waveform_overlays,
)
from embeddebug.serial_station.ui.waveform_engine import apply_curve_perf, configure_high_performance_plot
from embeddebug.serial_station.ui.waveform_fft import WINDOW_HANN, compute_fft
from embeddebug.serial_station.ui.waveform_histogram import compute_histogram
from embeddebug.serial_station.ui.waveform_measure import (
    compute_channel_stats, compute_cursor_measurement, format_cursor_measurement, format_stats,
)
from embeddebug.serial_station.ui.waveform_preview import SafePlotWidget
def _make_test_batch(channels: int = 2, samples: int = 256) -> np.ndarray:
    t = np.linspace(0, 1, samples)
    cols = [np.sin(2 * np.pi * (i + 1) * t) for i in range(channels)]
    return np.stack(cols, axis=1).astype(np.float32)
def _make_three_zone_splitter(qtbot) -> QSplitter:
    splitter = QSplitter()
    qtbot.addWidget(splitter)
    for name in ("serialStationLeftZone", "serialStationCenterZone", "serialStationRightZone"):
        zone = QWidget(splitter)
        zone.setObjectName(name)
    splitter.setSizes([220, 560, 220])
    return splitter
def test_build_cursor_hud_has_expected_objectname(qtbot):
    parent = QWidget()
    qtbot.addWidget(parent)
    hud = waveform_overlays.build_cursor_hud(parent)
    assert hud.objectName() == "serialStationWaveformCursorHud"
    assert "ΔX" in hud.text()
def test_waveform_legend_has_objectname(qtbot):
    legend = waveform_overlays.WaveformLegend()
    qtbot.addWidget(legend)
    assert legend.objectName() == "serialStationWaveformLegend"
def test_waveform_legend_updates_channels(qtbot):
    legend = waveform_overlays.WaveformLegend()
    qtbot.addWidget(legend)
    legend.update_channels(("ch0", "ch1"), (1.5, 2.5))
    chips = legend.findChildren(QLabel, "serialStationWaveformLegendChip")
    assert len(chips) == 2
def test_waveform_legend_clears_old_chips(qtbot):
    legend = waveform_overlays.WaveformLegend()
    qtbot.addWidget(legend)
    legend.update_channels(("a", "b", "c"), (1.0, 2.0, 3.0))
    legend.update_channels(("x",), (9.0,))
    chips = legend.findChildren(QLabel, "serialStationWaveformLegendChip")
    assert len(chips) == 1
def test_fade_in_returns_animation_with_opacity_target(qtbot):
    widget = QWidget()
    qtbot.addWidget(widget)
    anim = panel_animations.fade_in(widget)
    assert anim.duration() == panel_animations.DURATION_NORMAL
    assert anim.startValue() == 0.0
    assert anim.endValue() == 1.0
def test_fade_out_hides_widget_on_finish(qtbot):
    widget = QWidget()
    qtbot.addWidget(widget)
    widget.show()
    anim = panel_animations.fade_out(widget)
    anim.finished.emit()
    assert not widget.isVisible()
def test_slide_in_sets_start_below_target(qtbot):
    widget = QWidget()
    qtbot.addWidget(widget)
    target = widget.pos()
    anim = panel_animations.slide_in(widget)
    start = anim.startValue()
    assert start.y() == target.y() + panel_animations.SLIDE_PIXELS
    assert anim.endValue() == target
def test_card_enter_returns_two_animations(qtbot):
    widget = QWidget()
    qtbot.addWidget(widget)
    assert len(panel_animations.card_enter(widget)) == 2
def test_stagger_returns_animation_per_card(qtbot):
    cards = [QWidget() for _ in range(3)]
    anims = panel_animations.stagger_fade(cards)
    assert len(anims) == len(cards)
def test_responsive_layout_starts_expanded(qtbot):
    splitter = _make_three_zone_splitter(qtbot)
    rl = responsive_layout.ResponsiveLayout(splitter)
    assert rl.is_collapsed is False
def test_responsive_layout_collapses_below_breakpoint(qtbot):
    splitter = _make_three_zone_splitter(qtbot)
    rl = responsive_layout.ResponsiveLayout(splitter)
    collapsed_signals: list[bool] = []
    rl.sidebar_collapsed.connect(lambda: collapsed_signals.append(True))
    rl.on_window_resized(800)
    assert rl.is_collapsed is True
    assert collapsed_signals == [True]
    assert splitter.sizes()[0] == responsive_layout.COLLAPSED_WIDTH
def test_responsive_layout_expands_above_breakpoint(qtbot):
    splitter = _make_three_zone_splitter(qtbot)
    rl = responsive_layout.ResponsiveLayout(splitter)
    rl.on_window_resized(800)
    assert rl.is_collapsed is True
    expanded_signals: list[bool] = []
    rl.sidebar_expanded.connect(lambda: expanded_signals.append(True))
    rl.on_window_resized(1000)
    assert rl.is_collapsed is False
    assert expanded_signals == [True]
def test_responsive_layout_hysteresis_avoids_jitter(qtbot):
    splitter = _make_three_zone_splitter(qtbot)
    rl = responsive_layout.ResponsiveLayout(splitter)
    rl.on_window_resized(850)
    assert rl.is_collapsed is True
    rl.on_window_resized(920)
    assert rl.is_collapsed is True
    rl.on_window_resized(990)
    assert rl.is_collapsed is False
def test_attach_cursors_removed():
    assert not hasattr(waveform_overlays, "attach_cursors")
def test_cursor_readout_removed():
    assert not hasattr(waveform_overlays, "cursor_readout")
def test_make_cursor_removed():
    assert not hasattr(waveform_overlays, "_make_cursor")
def test_sample_y_removed():
    assert not hasattr(waveform_overlays, "_sample_y")
def test_waveform_legend_still_present():
    from embeddebug.serial_station.ui.waveform_overlays import WaveformLegend
    assert WaveformLegend is not None
def test_build_cursor_hud_still_present():
    assert callable(waveform_overlays.build_cursor_hud)
def test_waveform_measure_all_functions_used():
    from embeddebug.serial_station.ui import waveform_preview
    src = inspect.getsource(waveform_preview)
    for func in ("compute_channel_stats", "compute_cursor_measurement", "format_stats", "format_cursor_measurement"):
        assert f"waveform_measure.{func}" in src
def test_waveform_preview_uses_cursor_manager_not_attach_cursors():
    from embeddebug.serial_station.ui import waveform_preview
    src = inspect.getsource(waveform_preview)
    assert "CursorManager" in src
    assert re.findall(r"\battach_cursors\s*\(", src) == []
def test_waveform_overlays_imports_clean(qtbot):
    from embeddebug.serial_station.ui.waveform_overlays import WaveformLegend, build_cursor_hud
    legend = WaveformLegend()
    qtbot.addWidget(legend)
    hud = build_cursor_hud(legend)
    assert hud.objectName() == "serialStationWaveformCursorHud"
def test_lissajous_with_zoom_mode_both_compose(qtbot):
    lissajous = waveform_lissajous.LissajousPanel()
    qtbot.addWidget(lissajous)
    plot = pg.PlotWidget()
    qtbot.addWidget(plot)
    waveform_modes.apply_zoom_mode(plot, waveform_modes.ZoomMode.Y_ONLY)
    values = _make_test_batch(2, 128)
    lissajous.update_batch(values, ("sin1", "sin2"))
    assert lissajous._curve is not None
    assert plot.getViewBox().state["mouseEnabled"][0] is False
def test_bar_chart_with_high_performance_plot(qtbot):
    bar = waveform_modes.BarChartPanel()
    qtbot.addWidget(bar)
    configure_high_performance_plot(bar._plot)
    values = _make_test_batch(3, 50)
    bar.update_batch(values, ("ch0", "ch1", "ch2"))
    assert bar._bar is not None
def test_lissajous_channel_switch_redraws(qtbot):
    panel = waveform_lissajous.LissajousPanel()
    qtbot.addWidget(panel)
    values = _make_test_batch(3, 100)
    panel.set_channels(0, 1)
    panel.update_batch(values, ("a", "b", "c"))
    first_curve = panel._curve
    panel.set_channels(1, 2)
    panel.update_batch(values, ("a", "b", "c"))
    assert panel._curve is first_curve
    assert "b" in panel._status.text() and "c" in panel._status.text()
def test_zoom_mode_cycles_through_all(qtbot):
    plot = pg.PlotWidget()
    qtbot.addWidget(plot)
    view = plot.getViewBox()
    for mode in waveform_modes.ZoomMode:
        waveform_modes.apply_zoom_mode(plot, mode)
        x_expected = mode in (waveform_modes.ZoomMode.BOTH, waveform_modes.ZoomMode.X_ONLY)
        y_expected = mode in (waveform_modes.ZoomMode.BOTH, waveform_modes.ZoomMode.Y_ONLY)
        assert view.state["mouseEnabled"][0] is x_expected
        assert view.state["mouseEnabled"][1] is y_expected
def test_bar_chart_axis_shows_channel_names(qtbot):
    bar = waveform_modes.BarChartPanel()
    qtbot.addWidget(bar)
    values = np.array([[1.0, 2.0, 3.0]], dtype=np.float32)
    bar.update_batch(values, ("voltage", "current", "temp"))
    bar._plot.getAxis("bottom").tickValues(0, 3, 100)
    assert bar._names == ("voltage", "current", "temp")
def test_waveform_pipeline_fft_then_stats(qtbot):
    values = _make_test_batch(1, 512)[:, 0]
    fft_result = compute_fft(values, sample_rate=512.0, points=512, window=WINDOW_HANN)
    stats = compute_channel_stats(values)
    assert fft_result.frequencies.size > 0
    assert stats.vpp > 0
    assert 0.9 < stats.maximum <= 1.1
def test_waveform_pipeline_histogram_then_measure(qtbot):
    values = _make_test_batch(1, 256)[:, 0]
    hist = compute_histogram(values, bin_count=16)
    m = compute_cursor_measurement(x_cursor_values=[0.0, 128.0], y_cursor_values=[-1.0, 1.0], sample_rate=256.0)
    assert hist.counts.shape == (16,)
    assert abs(m.delta_t - 0.5) < 1e-5
    assert abs(m.delta_y - 2.0) < 1e-5
def test_apply_curve_perf_does_not_crash(qtbot):
    plot = pg.PlotWidget()
    qtbot.addWidget(plot)
    curve = plot.plot([1, 2, 3, 4])
    apply_curve_perf(curve)
def test_format_outputs_are_human_readable(qtbot):
    stats = compute_channel_stats(np.array([1.0, 2.0, 3.0], dtype=np.float32))
    stats_text = format_stats(stats)
    m = compute_cursor_measurement([0, 100], [1, 2], 1000.0)
    measure_text = format_cursor_measurement(m)
    assert "Vpp" in stats_text and "RMS" in stats_text
    assert "ΔT" in measure_text and "Freq" in measure_text
def test_all_b5_panels_shutdown_cleanly(qtbot):
    lissajous = waveform_lissajous.LissajousPanel()
    qtbot.addWidget(lissajous)
    bar = waveform_modes.BarChartPanel()
    qtbot.addWidget(bar)
    values = _make_test_batch(2, 50)
    lissajous.update_batch(values, ("a", "b"))
    bar.update_batch(values, ("a", "b"))
    lissajous.shutdown()
    bar.shutdown()
    assert lissajous._curve is None
    assert bar._bar is None
def test_safe_plot_widget_guards_resize_events_during_offscreen_teardown(qtbot):
    assert "resizeEvent" in SafePlotWidget.__dict__
    widget = SafePlotWidget()
    qtbot.addWidget(widget)
    widget.resizeEvent(None)

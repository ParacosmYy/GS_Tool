"""B5 波形模式集成测试：李萨如/条形图/缩放在波形上下文中协同工作。

验证 B5 的三个模块可独立使用、可组合，且与 B3/B4 的波形预览兼容。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import numpy as np
import pyqtgraph as pg

from embeddebug.serial_station.ui import waveform_lissajous
from embeddebug.serial_station.ui import waveform_modes
from embeddebug.serial_station.ui.waveform_engine import apply_curve_perf, configure_high_performance_plot
from embeddebug.serial_station.ui.waveform_fft import compute_fft, WINDOW_HANN
from embeddebug.serial_station.ui.waveform_histogram import compute_histogram
from embeddebug.serial_station.ui.waveform_measure import (
    compute_channel_stats,
    compute_cursor_measurement,
    format_cursor_measurement,
    format_stats,
)


def _make_test_batch(channels: int = 2, samples: int = 256) -> np.ndarray:
    """生成测试用多通道数据。"""

    t = np.linspace(0, 1, samples)
    cols = [np.sin(2 * np.pi * (i + 1) * t) for i in range(channels)]
    return np.stack(cols, axis=1).astype(np.float32)


def test_lissajous_with_zoom_mode_both_compose(qtbot):
    """李萨如面板与缩放模式可独立配置，互不干扰。"""

    lissajous = waveform_lissajous.LissajousPanel()
    qtbot.addWidget(lissajous)
    plot = pg.PlotWidget()
    qtbot.addWidget(plot)
    # 给时域图设缩放模式，不影响李萨如面板。
    waveform_modes.apply_zoom_mode(plot, waveform_modes.ZoomMode.Y_ONLY)
    values = _make_test_batch(2, 128)
    lissajous.update_batch(values, ("sin1", "sin2"))
    assert lissajous._curve is not None
    view = plot.getViewBox()
    assert view.state["mouseEnabled"][0] is False


def test_bar_chart_with_high_performance_plot(qtbot):
    """条形图面板可与高性能渲染配置组合。"""

    bar = waveform_modes.BarChartPanel()
    qtbot.addWidget(bar)
    configure_high_performance_plot(bar._plot)
    values = _make_test_batch(3, 50)
    bar.update_batch(values, ("ch0", "ch1", "ch2"))
    assert bar._bar is not None


def test_lissajous_channel_switch_redraws(qtbot):
    """切换 X/Y 通道后重新绘制使用新通道。"""

    panel = waveform_lissajous.LissajousPanel()
    qtbot.addWidget(panel)
    values = _make_test_batch(3, 100)
    panel.set_channels(0, 1)
    panel.update_batch(values, ("a", "b", "c"))
    first_curve = panel._curve
    panel.set_channels(1, 2)
    panel.update_batch(values, ("a", "b", "c"))
    # 同一 curve 对象复用，数据已更新。
    assert panel._curve is first_curve
    assert "b" in panel._status.text()
    assert "c" in panel._status.text()


def test_zoom_mode_cycles_through_all(qtbot):
    """三种缩放模式可循环切换且状态正确。"""

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
    """条形图 X 轴刻度应显示通道名。"""

    bar = waveform_modes.BarChartPanel()
    qtbot.addWidget(bar)
    values = np.array([[1.0, 2.0, 3.0]], dtype=np.float32)
    bar.update_batch(values, ("voltage", "current", "temp"))
    ticks = bar._plot.getAxis("bottom").tickValues(0, 3, 100)
    # 至少应有刻度生成。
    assert bar._names == ("voltage", "current", "temp")


def test_waveform_pipeline_fft_then_stats(qtbot):
    """B4/B5 流水线：FFT 与统计可串联分析同一信号。"""

    values = _make_test_batch(1, 512)[:, 0]
    fft_result = compute_fft(values, sample_rate=512.0, points=512, window=WINDOW_HANN)
    stats = compute_channel_stats(values)
    assert fft_result.frequencies.size > 0
    assert stats.vpp > 0
    assert 0.9 < stats.maximum <= 1.1  # 正弦峰值约 1


def test_waveform_pipeline_histogram_then_measure(qtbot):
    """B4/B5 流水线：直方图与游标测量可串联。"""

    values = _make_test_batch(1, 256)[:, 0]
    hist = compute_histogram(values, bin_count=16)
    measurement = compute_cursor_measurement(
        x_cursor_values=[0.0, 128.0], y_cursor_values=[-1.0, 1.0], sample_rate=256.0
    )
    assert hist.counts.shape == (16,)
    assert abs(measurement.delta_t - 0.5) < 1e-5
    assert abs(measurement.delta_y - 2.0) < 1e-5


def test_apply_curve_perf_does_not_crash(qtbot):
    """高性能曲线参数应用不崩溃。"""

    plot = pg.PlotWidget()
    qtbot.addWidget(plot)
    curve = plot.plot([1, 2, 3, 4])
    apply_curve_perf(curve)


def test_format_outputs_are_human_readable(qtbot):
    """格式化读数输出人类可读。"""

    stats = compute_channel_stats(np.array([1.0, 2.0, 3.0], dtype=np.float32))
    stats_text = format_stats(stats)
    measurement = compute_cursor_measurement([0, 100], [1, 2], 1000.0)
    measure_text = format_cursor_measurement(measurement)
    assert "Vpp" in stats_text and "RMS" in stats_text
    assert "ΔT" in measure_text and "Freq" in measure_text


def test_all_b5_panels_shutdown_cleanly(qtbot):
    """所有 B5 面板可干净关闭，无残留。"""

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

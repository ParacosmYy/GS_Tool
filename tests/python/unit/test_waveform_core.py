"""波形引擎核心：FFT / 直方图 / 富游标 / 测量 + 性能 RefreshThrottle/BatchAccumulator。"""
from __future__ import annotations
import os
os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")
import numpy as np
import pyqtgraph as pg
from embeddebug.serial_station.ui import waveform_cursors, waveform_fft, waveform_histogram, waveform_measure
from embeddebug.serial_station.ui.waveform_perf import BatchAccumulator, RefreshThrottle
def test_fft_supported_points_are_powers_of_two():
    for p in waveform_fft.SUPPORTED_FFT_POINTS:
        assert (p & (p - 1)) == 0, f"{p} not power of two"
def test_fft_window_coefficients_shape():
    for window in waveform_fft.SUPPORTED_WINDOWS:
        coeff = waveform_fft.window_coefficients(512, window)
        assert coeff.shape == (512,)
        assert np.all(np.isfinite(coeff))
def test_fft_rect_window_is_all_ones():
    assert np.all(waveform_fft.window_coefficients(256, waveform_fft.WINDOW_RECT) == 1.0)
def test_fft_detects_known_frequency():
    sample_rate = 1000.0
    t = np.arange(1024) / sample_rate
    signal = np.sin(2 * np.pi * 50.0 * t).astype(np.float32)
    result = waveform_fft.compute_fft(signal, sample_rate, points=1024, window=waveform_fft.WINDOW_HANN)
    peak_idx = int(np.argmax(result.magnitudes))
    peak_freq = float(result.frequencies[peak_idx])
    assert 45.0 <= peak_freq <= 55.0
def test_fft_handles_empty_signal():
    result = waveform_fft.compute_fft(np.array([]), 1000.0)
    assert result.frequencies.size == 0
    assert result.magnitudes.size == 0
def test_fft_pads_short_signal():
    short = np.ones(64, dtype=np.float32)
    result = waveform_fft.compute_fft(short, 1000.0, points=512)
    assert result.frequencies.shape[0] == 257
def test_fft_returns_db_magnitudes():
    signal = np.sin(2 * np.pi * 10 * np.arange(512) / 1000.0).astype(np.float32)
    result = waveform_fft.compute_fft(signal, 1000.0, points=512)
    assert result.magnitudes_db.shape == result.magnitudes.shape
    assert np.all(np.isfinite(result.magnitudes_db))
def test_fft_invalid_points_returns_empty():
    result = waveform_fft.compute_fft(np.ones(100), 1000.0, points=0)
    assert result.frequencies.size == 0
def test_histogram_returns_correct_bin_count():
    signal = np.random.randn(1000).astype(np.float32)
    result = waveform_histogram.compute_histogram(signal, bin_count=32)
    assert result.counts.shape == (32,)
    assert result.centers.shape == (32,)
    assert result.bin_edges.shape == (33,)
def test_histogram_clamps_bin_count_to_range():
    signal = np.ones(100, dtype=np.float32)
    result = waveform_histogram.compute_histogram(signal, bin_count=2)
    assert result.counts.shape[0] >= waveform_histogram.MIN_BIN_COUNT
    result_big = waveform_histogram.compute_histogram(signal, bin_count=99999)
    assert result_big.counts.shape[0] <= waveform_histogram.MAX_BIN_COUNT
def test_histogram_empty_signal_returns_empty():
    assert waveform_histogram.compute_histogram(np.array([])).counts.size == 0
def test_histogram_centers_are_midpoints():
    signal = np.arange(100, dtype=np.float32)
    result = waveform_histogram.compute_histogram(signal, bin_count=10)
    for i in range(10):
        expected = (result.bin_edges[i] + result.bin_edges[i + 1]) / 2.0
        assert abs(result.centers[i] - expected) < 1e-5
def test_histogram_custom_range():
    signal = np.array([0.0, 5.0, 10.0, 15.0, 20.0], dtype=np.float32)
    result = waveform_histogram.compute_histogram(signal, bin_count=4, value_range=(0.0, 20.0))
    assert float(result.bin_edges[0]) == 0.0
    assert float(result.bin_edges[-1]) == 20.0
def test_cursor_manager_add_x_cursor(qtbot):
    plot = pg.PlotWidget()
    qtbot.addWidget(plot)
    manager = waveform_cursors.CursorManager(plot)
    cursor = manager.add_x_cursor(5.0)
    assert cursor in plot.items()
    assert len(manager.x_cursors) == 1
def test_cursor_manager_add_y_cursor(qtbot):
    plot = pg.PlotWidget()
    qtbot.addWidget(plot)
    manager = waveform_cursors.CursorManager(plot)
    cursor = manager.add_y_cursor(3.0)
    assert cursor in plot.items()
    assert len(manager.y_cursors) == 1
def test_cursor_manager_remove_cursor(qtbot):
    plot = pg.PlotWidget()
    qtbot.addWidget(plot)
    manager = waveform_cursors.CursorManager(plot)
    cursor = manager.add_x_cursor(1.0)
    assert manager.remove_cursor(cursor) is True
    assert len(manager.x_cursors) == 0
def test_cursor_manager_remove_unknown_returns_false(qtbot):
    plot = pg.PlotWidget()
    qtbot.addWidget(plot)
    manager = waveform_cursors.CursorManager(plot)
    foreign = waveform_cursors.make_x_cursor(0.0, 99)
    assert manager.remove_cursor(foreign) is False
def test_cursor_manager_clear(qtbot):
    plot = pg.PlotWidget()
    qtbot.addWidget(plot)
    manager = waveform_cursors.CursorManager(plot)
    manager.add_x_cursor(1.0)
    manager.add_x_cursor(2.0)
    manager.add_y_cursor(3.0)
    manager.clear()
    assert len(manager.x_cursors) == 0
    assert len(manager.y_cursors) == 0
def test_cursor_manager_cursor_values(qtbot):
    plot = pg.PlotWidget()
    qtbot.addWidget(plot)
    manager = waveform_cursors.CursorManager(plot)
    manager.add_x_cursor(10.0)
    manager.add_x_cursor(20.0)
    manager.add_y_cursor(5.0)
    xs, ys = manager.cursor_values()
    assert xs == [10.0, 20.0]
    assert ys == [5.0]
def test_cursor_objectnames(qtbot):
    plot = pg.PlotWidget()
    qtbot.addWidget(plot)
    x_cursor = waveform_cursors.make_x_cursor(0.0, 1)
    y_cursor = waveform_cursors.make_y_cursor(0.0, 1)
    assert x_cursor.objectName() == "serialStationWaveformCursorX"
    assert y_cursor.objectName() == "serialStationWaveformCursorY"
def test_channel_stats_basic():
    signal = np.array([1.0, 2.0, 3.0, 4.0, 5.0], dtype=np.float32)
    stats = waveform_measure.compute_channel_stats(signal)
    assert stats.maximum == 5.0
    assert stats.minimum == 1.0
    assert stats.vpp == 4.0
    assert abs(stats.mean - 3.0) < 1e-5
def test_channel_stats_empty():
    stats = waveform_measure.compute_channel_stats(np.array([]))
    assert stats.vpp == 0.0
    assert stats.mean == 0.0
def test_channel_stats_rms():
    signal = np.array([3.0, 4.0], dtype=np.float32)
    stats = waveform_measure.compute_channel_stats(signal)
    assert abs(stats.rms - 3.5355) < 1e-3
def test_cursor_measurement_time_and_frequency():
    m = waveform_measure.compute_cursor_measurement(
        x_cursor_values=[0.0, 100.0], y_cursor_values=[], sample_rate=1000.0)
    assert m.delta_t == 0.1
    assert abs(m.frequency - 10.0) < 1e-5
    assert m.delta_y is None
def test_cursor_measurement_y_delta():
    m = waveform_measure.compute_cursor_measurement(
        x_cursor_values=[], y_cursor_values=[1.0, 4.0], sample_rate=1000.0)
    assert m.delta_y == 3.0
    assert m.delta_t is None
def test_cursor_measurement_insufficient_cursors():
    m = waveform_measure.compute_cursor_measurement(
        x_cursor_values=[5.0], y_cursor_values=[], sample_rate=1000.0)
    assert m.delta_t is None
    assert m.frequency is None
def test_cursor_measurement_zero_sample_rate():
    m = waveform_measure.compute_cursor_measurement(
        x_cursor_values=[0.0, 100.0], y_cursor_values=[], sample_rate=0.0)
    assert m.delta_t is None
def test_format_stats_contains_all_metrics():
    stats = waveform_measure.ChannelStats(vpp=1.0, mean=2.0, maximum=3.0, minimum=1.0, std=0.5, rms=2.1)
    text = waveform_measure.format_stats(stats)
    for label in ("Vpp", "Mean", "Max", "Min", "Std", "RMS"):
        assert label in text
def test_format_cursor_measurement_empty():
    m = waveform_measure.CursorMeasurement(delta_t=None, frequency=None, delta_y=None)
    assert "ΔT" in waveform_measure.format_cursor_measurement(m)
def test_throttle_target_hz(qtbot):
    fired: list[int] = []
    throttle = RefreshThrottle(lambda: fired.append(1), target_hz=100)
    assert throttle.target_hz == 100
def test_throttle_first_call_fires_immediately(qtbot):
    fired: list[int] = []
    throttle = RefreshThrottle(lambda: fired.append(1), target_hz=1000)
    throttle.maybe_refresh()
    assert len(fired) == 1
def test_throttle_rapid_calls_do_not_overfire(qtbot):
    fired: list[int] = []
    throttle = RefreshThrottle(lambda: fired.append(1), target_hz=10)
    throttle.maybe_refresh()
    for _ in range(20):
        throttle.maybe_refresh()
    assert len(fired) >= 1
def test_throttle_stop(qtbot):
    throttle = RefreshThrottle(lambda: None, target_hz=10)
    throttle.stop()
    assert not throttle._timer.isActive()
def test_batch_accumulator_start_stop(qtbot):
    acc = BatchAccumulator(flush_interval_ms=10)
    acc.start()
    assert acc._timer.isActive()
    acc.stop()
    assert not acc._timer.isActive()
def test_batch_accumulator_push_then_flush(qtbot):
    acc = BatchAccumulator(flush_interval_ms=1000)
    flushed: list[object] = []
    acc.flush_signal.connect(lambda batch: flushed.append(batch))
    acc.push(np.array([[1.0, 2.0]], dtype=np.float32), ("a", "b"), dt_ns=1000)
    acc.push(np.array([[3.0, 4.0]], dtype=np.float32), ("a", "b"), dt_ns=1000)
    assert acc.pending_count == 2
    acc.flush()
    assert acc.pending_count == 0
    assert len(flushed) == 1
    merged = flushed[0].values
    assert merged.shape == (2, 2)
    assert merged[1, 0] == 3.0
def test_batch_accumulator_auto_flush_at_max(qtbot):
    acc = BatchAccumulator(flush_interval_ms=10000, max_batches=2)
    flushed: list[object] = []
    acc.flush_signal.connect(lambda batch: flushed.append(batch))
    acc.push(np.array([[1.0]], dtype=np.float32), ("a",), dt_ns=1)
    acc.push(np.array([[2.0]], dtype=np.float32), ("a",), dt_ns=1)
    assert len(flushed) == 1
    assert acc.pending_count == 0
def test_batch_accumulator_empty_flush_noop(qtbot):
    acc = BatchAccumulator(flush_interval_ms=10)
    flushed: list[object] = []
    acc.flush_signal.connect(lambda batch: flushed.append(batch))
    acc.flush()
    assert flushed == []
def test_batch_accumulator_push_empty_ignored(qtbot):
    acc = BatchAccumulator(flush_interval_ms=10)
    acc.push(np.array([]).reshape(0, 2), ("a", "b"), dt_ns=1)
    assert acc.pending_count == 0
def test_batch_accumulator_single_batch_not_vstacked(qtbot):
    acc = BatchAccumulator(flush_interval_ms=1000)
    flushed: list[object] = []
    acc.flush_signal.connect(lambda batch: flushed.append(batch))
    acc.push(np.array([[1.0, 2.0, 3.0]], dtype=np.float32), ("a", "b", "c"), dt_ns=1)
    acc.flush()
    assert flushed[0].values.shape == (1, 3)
    assert flushed[0].channel_names == ("a", "b", "c")

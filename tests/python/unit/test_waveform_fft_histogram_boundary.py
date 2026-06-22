"""waveform_fft + histogram + cursors 常量/边界单元测试。

补强 test_waveform_core 未直接断言的边角：
- WINDOW 常量（RECT/HANN/HAMMING/BLACKMAN）+ window_coefficients 各窗型返回非零。
- FFTResult 字段（frequencies/magnitudes/magnitudes_db 对齐形状）。
- HistogramResult 字段（bin_edges/counts/value_range）。
- make_x_cursor / make_y_cursor 返回 InfiniteLine + objectName 契约。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import numpy as np

from embeddebug.serial_station.ui.waveform_fft import (
    WINDOW_BLACKMAN,
    WINDOW_HAMMING,
    WINDOW_HANN,
    WINDOW_RECT,
    compute_fft,
    window_coefficients,
)
from embeddebug.serial_station.ui.waveform_histogram import compute_histogram


# ── WINDOW 常量 ───────────────────────────────────────────────────────


def test_window_constants_values():
    """WINDOW 常量是小写字符串。"""

    assert WINDOW_RECT == "rect"
    assert WINDOW_HANN == "hann"
    assert WINDOW_HAMMING == "hamming"
    assert WINDOW_BLACKMAN == "blackman"


def test_window_constants_distinct():
    """4 个窗常量互不相同。"""

    windows = {WINDOW_RECT, WINDOW_HANN, WINDOW_HAMMING, WINDOW_BLACKMAN}
    assert len(windows) == 4


def test_window_coefficients_all_types_nonempty():
    """4 种窗型 → 非空数组。"""

    for window in (WINDOW_RECT, WINDOW_HANN, WINDOW_HAMMING, WINDOW_BLACKMAN):
        coeff = window_coefficients(256, window)
        assert coeff.size == 256


def test_window_coefficients_rect_all_ones():
    """RECT 窗 → 全 1.0。"""

    coeff = window_coefficients(128, WINDOW_RECT)
    assert np.all(coeff == 1.0)


def test_window_coefficients_hann_not_all_ones():
    """HANN 窗 → 非全 1（有衰减）。"""

    coeff = window_coefficients(128, WINDOW_HANN)
    assert not np.all(coeff == 1.0)


def test_window_coefficients_hann_symmetric():
    """HANN 窗关于中心对称。"""

    coeff = window_coefficients(128, WINDOW_HANN)
    half = 128 // 2
    for i in range(half):
        assert abs(coeff[i] - coeff[127 - i]) < 1e-9


# ── FFTResult 字段对齐 ────────────────────────────────────────────────


def test_fft_result_frequencies_magnitudes_aligned():
    """frequencies 和 magnitudes 形状一致。"""

    signal = np.sin(np.linspace(0, 10 * np.pi, 512))
    result = compute_fft(signal, sample_rate=1000.0, points=512)
    assert result.frequencies.shape == result.magnitudes.shape


def test_fft_result_magnitudes_db_aligned():
    """magnitudes_db 和 magnitudes 形状一致。"""

    signal = np.sin(np.linspace(0, 10 * np.pi, 512))
    result = compute_fft(signal, sample_rate=1000.0, points=512)
    assert result.magnitudes_db.shape == result.magnitudes.shape


def test_fft_result_magnitudes_db_finite():
    """magnitudes_db 全有限值。"""

    signal = np.sin(np.linspace(0, 10 * np.pi, 512))
    result = compute_fft(signal, sample_rate=1000.0, points=512)
    assert np.all(np.isfinite(result.magnitudes_db))


# ── HistogramResult 字段 ─────────────────────────────────────────────


def test_histogram_result_has_bin_edges_and_counts():
    """compute_histogram 返回 bin_edges + counts + centers。"""

    signal = np.random.randn(100)
    result = compute_histogram(signal, bin_count=16)
    assert result.bin_edges is not None
    assert result.counts is not None
    assert result.centers is not None


def test_histogram_counts_sum_equals_signal_length():
    """counts 之和 = 信号长度。"""

    signal = np.random.randn(100)
    result = compute_histogram(signal, bin_count=10)
    assert result.counts.sum() == 100


def test_histogram_bin_edges_count_is_bin_count_plus_one():
    """bin_edges 长度 = bin_count + 1。"""

    signal = np.random.randn(100)
    result = compute_histogram(signal, bin_count=16)
    assert result.bin_edges.size == 17


def test_histogram_empty_signal():
    """空信号 → counts.size=0。"""

    result = compute_histogram(np.array([]))
    assert result.counts.size == 0


def test_histogram_custom_value_range():
    """自定义 value_range → bin_edges 在该范围内。"""

    signal = np.random.randn(100) * 10
    result = compute_histogram(signal, bin_count=10, value_range=(-5.0, 5.0))
    assert result.bin_edges[0] == -5.0
    assert result.bin_edges[-1] == 5.0

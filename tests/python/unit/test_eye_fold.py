"""眼图折叠单测（Wave 60，纯 numpy，无 qtbot）。

覆盖位周期折叠、密度网格构建、相位重采样、空输入与退化保护。
指标测量在 test_eye_metrics.py，模板测试在 test_eye_mask.py。
对齐 test_waveform_math.py 头约定。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import numpy as np
import pytest

from embeddebug.serial_station.eye_diagram import EyeDiagram, fold_eye


def _alternating_bits(n_bits: int, samples_per_bit: int, high: float = 1.0, low: float = -1.0) -> np.ndarray:
    """生成 0101... 交替位序列（每 bit samples_per_bit 个采样）。"""

    pattern = [high, low] * n_bits
    samples = []
    for level in pattern:
        samples.extend([level] * samples_per_bit)
    return np.asarray(samples, dtype=np.float32)


def test_fold_basic_period_count():
    samples = _alternating_bits(n_bits=4, samples_per_bit=8)
    diagram = fold_eye(samples, samples_per_bit=8)
    assert isinstance(diagram, EyeDiagram)
    assert diagram.period_count == 8  # 4 bits * 2 transitions
    assert diagram.phase_bins == 8


def test_fold_density_matrix_shape():
    samples = _alternating_bits(n_bits=4, samples_per_bit=8)
    diagram = fold_eye(samples, samples_per_bit=8, level_bins=64)
    # density = [level_bins, phase_bins]
    assert diagram.density.shape == (64, 8)
    assert diagram.levels.shape == (64,)


def test_fold_has_density_true_when_samples_present():
    samples = _alternating_bits(n_bits=2, samples_per_bit=4)
    diagram = fold_eye(samples, samples_per_bit=4)
    assert diagram.has_density is True
    assert float(diagram.density.sum()) > 0


def test_fold_phase_axis_normalized_zero_to_one():
    samples = _alternating_bits(n_bits=2, samples_per_bit=4)
    diagram = fold_eye(samples, samples_per_bit=4)
    assert abs(float(diagram.phase_axis[0]) - 0.0) < 1e-6
    assert abs(float(diagram.phase_axis[-1]) - 1.0) < 1e-6


def test_fold_overlay_length_matches_periods():
    samples = _alternating_bits(n_bits=3, samples_per_bit=10)
    diagram = fold_eye(samples, samples_per_bit=10)
    assert diagram.overlay.size == diagram.period_count * diagram.phase_bins


def test_fold_phase_resample_different_bins():
    samples = _alternating_bits(n_bits=2, samples_per_bit=8)
    # 请求 phase_bins=16（重采样到更细相位）。
    diagram = fold_eye(samples, samples_per_bit=8, phase_bins=16)
    assert diagram.phase_bins == 16
    assert diagram.density.shape[1] == 16


def test_fold_truncates_partial_period():
    # 不足一个完整 UI 的尾部丢弃。
    samples = _alternating_bits(n_bits=2, samples_per_bit=8)
    samples = samples[:-3]  # 砍掉 3 个尾样本
    diagram = fold_eye(samples, samples_per_bit=8)
    assert diagram.period_count == samples.size // 8


def test_fold_empty_samples_returns_empty_grid():
    diagram = fold_eye(np.array([], dtype=np.float32), samples_per_bit=8)
    assert diagram.period_count == 0
    assert diagram.density.size == 0
    assert diagram.has_density is False


def test_fold_too_few_samples_returns_empty():
    diagram = fold_eye(np.array([0.0, 1.0], dtype=np.float32), samples_per_bit=8)
    assert diagram.period_count == 0


def test_fold_invalid_samples_per_bit_raises():
    with pytest.raises(ValueError):
        fold_eye(np.array([1.0]), samples_per_bit=0)


def test_fold_constant_signal_uses_symmetric_range():
    # 恒定信号（全 1.0）触发退化保护，应给 ±0.5 范围不报错。
    samples = np.ones(16, dtype=np.float32)
    diagram = fold_eye(samples, samples_per_bit=8)
    assert diagram.period_count == 2
    assert diagram.levels.shape[0] > 0


def test_fold_custom_level_range():
    samples = _alternating_bits(n_bits=2, samples_per_bit=4)
    diagram = fold_eye(samples, samples_per_bit=4, level_range=(-2.0, 2.0))
    assert abs(float(diagram.levels[0]) - (-2.0)) < 1e-5
    assert abs(float(diagram.levels[-1]) - 2.0) < 1e-5

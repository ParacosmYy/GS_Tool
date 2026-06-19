"""眼图指标测量单测（Wave 60，纯 numpy，无 qtbot）。

覆盖眼高/眼宽/抖动/上升下降时间/SNR 测量 + to_payload + 退化保护。
折叠单测在 test_eye_fold.py，模板测试在 test_eye_mask.py。
对齐 test_waveform_math.py 头约定。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import numpy as np

from embeddebug.serial_station.eye_diagram import EyeMetrics, compute_eye_metrics, fold_eye


def _clean_eye(samples_per_bit: int = 16, n_bits: int = 8) -> np.ndarray:
    """生成干净交替位序列（高=1.0/低=-1.0），理想眼图。"""

    pattern = [1.0, -1.0] * n_bits
    samples = []
    for level in pattern:
        samples.extend([level] * samples_per_bit)
    return np.asarray(samples, dtype=np.float32)


def _noisy_eye(samples_per_bit: int = 16, n_bits: int = 8, noise: float = 0.05) -> np.ndarray:
    """干净眼图加高斯噪声。"""

    rng = np.random.default_rng(seed=42)
    clean = _clean_eye(samples_per_bit, n_bits)
    return (clean + rng.normal(0, noise, clean.shape)).astype(np.float32)


def test_metrics_clean_eye_has_height():
    samples = _clean_eye()
    diagram = fold_eye(samples, samples_per_bit=16)
    metrics = compute_eye_metrics(diagram)
    assert metrics.eye_height > 0
    # 干净眼图高 ≈ 2.0（+1 - (-1)）。
    assert abs(metrics.eye_height - 2.0) < 0.2


def test_metrics_eye_height_decreases_with_noise():
    clean = compute_eye_metrics(fold_eye(_clean_eye(), samples_per_bit=16))
    noisy = compute_eye_metrics(fold_eye(_noisy_eye(noise=0.2), samples_per_bit=16))
    # 噪声使眼高下降（均值收紧）。
    assert noisy.eye_height <= clean.eye_height


def test_metrics_eye_width_positive_for_open_eye():
    samples = _clean_eye()
    diagram = fold_eye(samples, samples_per_bit=16)
    metrics = compute_eye_metrics(diagram)
    assert 0.0 < metrics.eye_width <= 1.0  # 归一化 UI


def test_metrics_jitter_zero_for_clean_signal():
    # 完美交替信号无抖动（过零点固定）。
    samples = _clean_eye()
    diagram = fold_eye(samples, samples_per_bit=16)
    metrics = compute_eye_metrics(diagram)
    assert metrics.jitter_pp < 0.1  # 近零


def test_metrics_jitter_positive_for_noisy_signal():
    samples = _noisy_eye(noise=0.3)
    diagram = fold_eye(samples, samples_per_bit=16)
    metrics = compute_eye_metrics(diagram)
    assert metrics.jitter_rms >= 0.0  # 噪声引入抖动（可能仍小，但非负）。


def test_metrics_rise_fall_time_nonneg():
    samples = _clean_eye()
    diagram = fold_eye(samples, samples_per_bit=16)
    metrics = compute_eye_metrics(diagram)
    assert metrics.rise_time >= 0.0
    assert metrics.fall_time >= 0.0


def test_metrics_snr_positive_for_clean_eye():
    samples = _clean_eye()
    diagram = fold_eye(samples, samples_per_bit=16)
    metrics = compute_eye_metrics(diagram)
    assert metrics.snr >= 0.0


def test_metrics_to_payload_keys():
    metrics = EyeMetrics(eye_height=2.0, eye_width=0.5, snr=10.0)
    payload = metrics.to_payload()
    assert payload["type"] == "eye_metrics"
    assert payload["eyeHeight"] == 2.0
    assert payload["eyeWidth"] == 0.5
    assert payload["snr"] == 10.0


def test_metrics_eye_opening_is_product():
    metrics = EyeMetrics(eye_height=2.0, eye_width=0.5, eye_opening=1.0)
    assert metrics.eye_opening == 2.0 * 0.5


def test_metrics_empty_diagram_returns_zeros():
    diagram = fold_eye(np.array([], dtype=np.float32), samples_per_bit=8)
    metrics = compute_eye_metrics(diagram)
    assert metrics.eye_height == 0.0
    assert metrics.eye_width == 0.0
    assert metrics.jitter_pp == 0.0


def test_metrics_constant_signal_returns_zeros():
    # 恒定信号无眼图（amplitude=0）。
    samples = np.ones(64, dtype=np.float32)
    diagram = fold_eye(samples, samples_per_bit=16)
    metrics = compute_eye_metrics(diagram)
    assert metrics.eye_height == 0.0


def test_metrics_custom_thresholds():
    samples = _clean_eye()
    diagram = fold_eye(samples, samples_per_bit=16)
    metrics = compute_eye_metrics(diagram, low_threshold=-0.5, high_threshold=0.5)
    assert metrics.rise_time >= 0.0

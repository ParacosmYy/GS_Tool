"""眼图模板测试单测（Wave 60，纯 numpy，无 qtbot）。

覆盖点在多边形判定、掩码违规检测、裕量估计 + PolyMask 校验。
折叠/指标单测在 test_eye_fold.py / test_eye_metrics.py。
对齐 test_waveform_math.py 头约定。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import numpy as np
import pytest

from embeddebug.serial_station.eye_diagram import (
    EyeMetrics,
    PolyMask,
    evaluate_mask,
    fold_eye,
    mask_margin,
    mask_violation_points,
    point_in_polygon,
)


def _clean_eye(samples_per_bit: int = 16, n_bits: int = 8) -> np.ndarray:
    pattern = [1.0, -1.0] * n_bits
    samples = []
    for level in pattern:
        samples.extend([level] * samples_per_bit)
    return np.asarray(samples, dtype=np.float32)


def _square_mask(half_width: float, half_height: float) -> PolyMask:
    """居中在 (0.5, 0) 的矩形掩码（phase/level）。"""

    verts = np.array([
        [0.5 - half_width, -half_height],
        [0.5 + half_width, -half_height],
        [0.5 + half_width, half_height],
        [0.5 - half_width, half_height],
    ], dtype=np.float32)
    return PolyMask(vertices=verts, name="square")


# ── 点在多边形 ──────────────────────────────────────────────────────
def test_point_inside_square():
    square = _square_mask(0.2, 0.2)
    assert point_in_polygon((0.5, 0.0), square.vertices) is True


def test_point_outside_square():
    square = _square_mask(0.2, 0.2)
    assert point_in_polygon((0.9, 0.9), square.vertices) is False


def test_point_on_boundary_tolerance():
    # 边界点：射线法对边界行为不确定，只测明确内/外的点。
    square = _square_mask(0.25, 0.25)
    assert point_in_polygon((0.5, 0.1), square.vertices) is True
    assert point_in_polygon((0.1, 0.5), square.vertices) is False


# ── PolyMask 校验 ───────────────────────────────────────────────────
def test_polymask_rejects_bad_shape():
    with pytest.raises(ValueError):
        PolyMask(vertices=np.array([1.0, 2.0, 3.0]))  # 1D 非法。


def test_polymask_accepts_valid_polygon():
    mask = PolyMask(vertices=np.array([[0, 0], [1, 0], [1, 1], [0, 1]], dtype=np.float32))
    assert mask.vertices.shape == (4, 2)


# ── 掩码违规检测 ────────────────────────────────────────────────────
def test_small_mask_no_violation_on_clean_eye():
    # 干净眼图开口大（±1.0），小掩码（±0.1）不违规。
    samples = _clean_eye()
    diagram = fold_eye(samples, samples_per_bit=16)
    mask = _square_mask(0.1, 0.1)
    # 清洁信号稳定电平 ±1.0 远离掩码（±0.1），应无违规。
    violations = mask_violation_points(diagram, mask)
    assert violations.shape[0] == 0
    assert evaluate_mask(diagram, mask) is False


def test_large_mask_violation_on_clean_eye():
    # 大掩码（±1.5）覆盖稳定电平 → 违规。
    samples = _clean_eye()
    diagram = fold_eye(samples, samples_per_bit=16)
    mask = _square_mask(0.3, 1.5)
    assert evaluate_mask(diagram, mask) is True


def test_violation_points_returned_as_phase_level():
    samples = _clean_eye()
    diagram = fold_eye(samples, samples_per_bit=16)
    mask = _square_mask(0.3, 1.5)
    violations = mask_violation_points(diagram, mask)
    assert violations.ndim == 2
    assert violations.shape[1] == 2  # [phase, level]
    assert violations.shape[0] > 0


def test_mask_empty_diagram_no_violation():
    diagram = fold_eye(np.array([], dtype=np.float32), samples_per_bit=8)
    mask = _square_mask(0.2, 0.2)
    assert evaluate_mask(diagram, mask) is False
    assert mask_violation_points(diagram, mask).shape[0] == 0


# ── 裕量估计 ────────────────────────────────────────────────────────
def test_mask_margin_positive_when_eye_taller_than_mask():
    samples = _clean_eye()
    diagram = fold_eye(samples, samples_per_bit=16)
    mask = _square_mask(0.1, 0.1)  # 掩码跨度 0.2，眼高 ≈2.0 → 正裕量。
    margin = mask_margin(diagram, mask)
    assert margin > 0.0


def test_mask_margin_with_explicit_metrics():
    diagram = fold_eye(_clean_eye(), samples_per_bit=16)
    mask = _square_mask(0.1, 0.5)
    metrics = EyeMetrics(eye_height=2.0)
    margin = mask_margin(diagram, mask, metrics=metrics)
    # (2.0 - 1.0) / 1.0 = 1.0
    assert abs(margin - 1.0) < 0.01


def test_mask_margin_zero_span_returns_zero():
    # 掩码跨度为 0（所有顶点同 level）→ 除零保护返回 0。
    diagram = fold_eye(_clean_eye(), samples_per_bit=16)
    mask = PolyMask(vertices=np.array([[0.4, 0.0], [0.6, 0.0]], dtype=np.float32))
    assert mask_margin(diagram, mask) == 0.0

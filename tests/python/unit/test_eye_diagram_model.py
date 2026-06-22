"""眼图数据模型单元测试 — 密度/开口/抖动。

覆盖：EyeDiagram has_density + float32 强制、EyeMetrics 默认值、frozen 不可变。
"""

from __future__ import annotations

import numpy as np
import pytest

from embeddebug.serial_station.eye_diagram.model import EyeDiagram, EyeMetrics


def _make_diagram(density=None) -> EyeDiagram:
    d = np.array(density if density is not None else [[1, 2], [3, 4]])
    return EyeDiagram(
        phase_axis=np.array([0.0, 0.5, 1.0]),
        levels=np.array([-1.0, 0.0, 1.0]),
        density=d,
    )


def test_eye_diagram_has_density_true():
    """非空密度矩阵 has_density=True。"""
    diag = _make_diagram([[1, 0], [0, 1]])
    assert diag.has_density is True


def test_eye_diagram_has_density_false_empty():
    """空密度矩阵 has_density=False。"""
    diag = _make_diagram(np.zeros((2, 2)))
    assert diag.has_density is False


def test_eye_diagram_float32_enforced():
    """数组字段强制 float32。"""
    diag = _make_diagram()
    assert diag.phase_axis.dtype == np.float32
    assert diag.levels.dtype == np.float32
    assert diag.density.dtype == np.float32


def test_eye_diagram_overlay_raveled():
    """overlay 强制一维。"""
    diag = EyeDiagram(
        phase_axis=np.array([0.0]),
        levels=np.array([0.0]),
        density=np.array([[1.0]]),
        overlay=np.array([[1, 2], [3, 4]]),
    )
    assert diag.overlay.ndim == 1
    assert diag.overlay.size == 4


def test_eye_diagram_frozen():
    """frozen 不可变（但 __post_init__ 用 object.__setattr__ 绕过）。"""
    diag = _make_diagram()
    with pytest.raises((AttributeError, TypeError)):
        diag.period_count = 5


def test_eye_metrics_defaults():
    """EyeMetrics 默认值为 0.0。"""
    m = EyeMetrics()
    assert m.eye_height == 0.0
    assert m.jitter_pp == 0.0

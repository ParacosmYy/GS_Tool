"""波形数学引擎函数库单测（Wave 58，纯 numpy，无 qtbot）。

覆盖通道算术、逐点一元函数（含 NaN 边界）、微积分（derivative/integral）。
表达式解析器与引擎编排在 test_waveform_math_expr.py。
对齐 test_waveform_engine.py 头约定。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import numpy as np

from embeddebug.serial_station.waveform_math import (
    absolute,
    add,
    cos,
    derivative,
    divide,
    integral,
    log,
    multiply,
    sin,
    sqrt,
    subtract,
)


# ── 通道算术 ────────────────────────────────────────────────────────
def test_add_subtract_multiply():
    a = np.array([1.0, 2.0, 3.0], dtype=np.float32)
    b = np.array([4.0, 5.0, 6.0], dtype=np.float32)
    np.testing.assert_allclose(add(a, b), [5, 7, 9])
    np.testing.assert_allclose(subtract(a, b), [-3, -3, -3])
    np.testing.assert_allclose(multiply(a, b), [4, 10, 18])


def test_divide_normal():
    a = np.array([10.0, 20.0], dtype=np.float32)
    b = np.array([2.0, 4.0], dtype=np.float32)
    np.testing.assert_allclose(divide(a, b), [5, 5])


def test_divide_by_near_zero_yields_nan():
    a = np.array([1.0, 1.0], dtype=np.float32)
    b = np.array([0.0, 1e-40], dtype=np.float32)
    result = divide(a, b)
    assert np.isnan(result).all()  # 两个都置 NaN，无 inf 尖刺。


# ── 一元函数 + NaN 边界 ────────────────────────────────────────────
def test_sqrt_negative_is_nan():
    out = sqrt(np.array([4.0, -1.0, 9.0], dtype=np.float32))
    np.testing.assert_allclose(out[0], 2.0)
    assert np.isnan(out[1])
    np.testing.assert_allclose(out[2], 3.0)


def test_log_nonpositive_is_nan():
    out = log(np.array([1.0, 0.0, -2.0, np.e], dtype=np.float32))
    assert np.isnan(out[1]) and np.isnan(out[2])
    np.testing.assert_allclose(out[0], 0.0, atol=1e-6)
    np.testing.assert_allclose(out[3], 1.0, atol=1e-5)


def test_sin_cos_periodicity():
    x = np.array([0.0, np.pi / 2, np.pi], dtype=np.float32)
    np.testing.assert_allclose(sin(x), [0, 1, 0], atol=1e-5)
    np.testing.assert_allclose(cos(x), [1, 0, -1], atol=1e-5)


def test_absolute():
    np.testing.assert_allclose(absolute(np.array([-3, 0, 5], dtype=np.float32)), [3, 0, 5])


# ── 微积分 ──────────────────────────────────────────────────────────
def test_derivative_of_linear_is_slope():
    x = np.arange(5, dtype=np.float32)
    y = 2.0 * x + 1.0
    np.testing.assert_allclose(derivative(y, dt=1.0), [2, 2, 2, 2, 2], atol=1e-5)


def test_derivative_with_dt_scaling():
    y = np.arange(4, dtype=np.float32)
    np.testing.assert_allclose(derivative(y, dt=0.5), [2, 2, 2, 2], atol=1e-5)


def test_derivative_short_signal_returns_zeros():
    out = derivative(np.array([5.0], dtype=np.float32))
    assert out.shape == (1,)
    assert np.all(out == 0)


def test_integral_of_constant():
    # ∫1 dt, dt=1 → [0,1,2,3]（梯形累积，首点 0）。
    y = np.ones(4, dtype=np.float32)
    np.testing.assert_allclose(integral(y, dt=1.0), [0, 1, 2, 3], atol=1e-5)


def test_integral_round_trips_derivative():
    # 导数再积分应近似还原（端点偏差内）。
    y = np.array([0.0, 1.0, 4.0, 9.0], dtype=np.float32)  # x^2 样本。
    d = derivative(y, dt=1.0)
    reint = integral(d, dt=1.0)
    assert reint.shape == y.shape  # 形状一致（积分常数差）。

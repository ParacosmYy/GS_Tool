"""波形数学引擎函数库 + 表达式解析器 + 高级多轴/散点/瀑布图测试。"""

from __future__ import annotations

import os
os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import numpy as np
import pytest

from embeddebug.serial_station.core.measurements import ChannelBatch
from embeddebug.serial_station.waveform_math import (
    ExpressionError, MathResult, VirtualChannel,
    absolute, add, build_scope, cos, derivative, divide,
    evaluate, evaluate_channel, evaluate_channels, integral,
    log, multiply, sin, sqrt, subtract,
)
from embeddebug.serial_station.waveform_advanced import (
    MultiAxisPlot, ScatterPlot, SpectrumWaterfall, WaterfallPlot,
)

def _batch(values: list[list[float]], names: tuple[str, ...] | None = None) -> ChannelBatch:
    arr = np.asarray(values, dtype=np.float32)
    if arr.ndim == 1:
        arr = arr.reshape(-1, 1)
    if names is None:
        names = tuple(f"ch{i + 1}" for i in range(arr.shape[1]))
    return ChannelBatch(channel_names=names, values=arr, dt_ns=1_000_000)

# ── 通道算术 ──
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
    assert np.isnan(result).all()

# ── 一元函数 + NaN 边界 ──
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

# ── 微积分 ──
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
    y = np.ones(4, dtype=np.float32)
    np.testing.assert_allclose(integral(y, dt=1.0), [0, 1, 2, 3], atol=1e-5)

def test_integral_round_trips_derivative():
    y = np.array([0.0, 1.0, 4.0, 9.0], dtype=np.float32)
    d = derivative(y, dt=1.0)
    reint = integral(d, dt=1.0)
    assert reint.shape == y.shape

# ── 表达式解析器 ──
def test_evaluate_binary_expression():
    scope = {"ch1": np.array([1.0, 2.0]), "ch2": np.array([3.0, 4.0])}
    np.testing.assert_allclose(evaluate("ch1 + ch2", scope), [4, 6])
    np.testing.assert_allclose(evaluate("ch1 * ch2 - ch1", scope), [2, 6])

def test_evaluate_with_constants_and_unary():
    scope = {"ch1": np.array([1.0, 4.0])}
    np.testing.assert_allclose(evaluate("0.5 * ch1", scope), [0.5, 2.0])
    np.testing.assert_allclose(evaluate("-ch1", scope), [-1, -4])

def test_evaluate_function_call():
    scope = {"ch1": np.array([4.0, 9.0])}
    np.testing.assert_allclose(evaluate("sqrt(ch1)", scope), [2, 3])

def test_evaluate_derivative_with_positional_dt():
    scope = {"ch1": np.arange(5, dtype=np.float64)}
    out = evaluate("derivative(ch1, 1.0)", scope)
    np.testing.assert_allclose(out, [1, 1, 1, 1, 1], atol=1e-5)

def test_evaluate_nested_expression():
    scope = {"ch1": np.array([3.0, 4.0]), "ch2": np.array([1.0, 1.0])}
    np.testing.assert_allclose(evaluate("abs(ch1 - ch2) + cos(0)", scope), [3, 4], atol=1e-5)

@pytest.mark.parametrize("expr", [
    "__import__('os')", "ch1.__class__", "ch1[0]",
    "ch1 if 1 else ch2", "lambda x: x", "open('x')", "ch1; import os",
])
def test_evaluate_rejects_dangerous_input(expr):
    with pytest.raises(ExpressionError):
        evaluate(expr, {"ch1": np.array([1.0])})

def test_evaluate_unknown_channel_name():
    with pytest.raises(ExpressionError):
        evaluate("chX + 1", {"ch1": np.array([1.0])})

def test_evaluate_syntax_error():
    with pytest.raises(ExpressionError):
        evaluate("ch1 +", {"ch1": np.array([1.0])})

def test_evaluate_rejects_keyword_args():
    with pytest.raises(ExpressionError):
        evaluate("sqrt(ch1, bogus=1)", {"ch1": np.array([1.0])})

# ── 引擎编排 ──
def test_virtual_channel_validation():
    with pytest.raises(ValueError):
        VirtualChannel(name="", expression="ch1")
    with pytest.raises(ValueError):
        VirtualChannel(name="x", expression="   ")

def test_build_scope_maps_columns():
    batch = _batch([[1, 10], [2, 20]], names=("a", "b"))
    scope = build_scope(batch)
    np.testing.assert_allclose(scope["a"], [1, 2])
    np.testing.assert_allclose(scope["b"], [10, 20])

def test_evaluate_channel_ok():
    batch = _batch([[1, 10], [2, 20], [3, 30]], names=("a", "b"))
    ch = VirtualChannel(name="sum", expression="a + b")
    result = evaluate_channel(ch, batch)
    assert result.ok
    assert result.error is None
    np.testing.assert_allclose(result.values, [11, 22, 33])

def test_evaluate_channel_reports_error():
    batch = _batch([[1], [2]], names=("a",))
    ch = VirtualChannel(name="bad", expression="a + unknown")
    result = evaluate_channel(ch, batch)
    assert not result.ok
    assert "unknown" in result.error
    assert result.sample_count == 0

def test_evaluate_channel_empty_batch():
    batch = ChannelBatch(channel_names=("a",), values=np.empty((0, 1), dtype=np.float32))
    ch = VirtualChannel(name="x", expression="a * 2")
    result = evaluate_channel(ch, batch)
    assert result.ok
    assert result.sample_count == 0

def test_evaluate_channels_batch_independent():
    batch = _batch([[4, 9]], names=("a", "b"))
    channels = [
        VirtualChannel(name="r1", expression="sqrt(a)"),
        VirtualChannel(name="r2", expression="sqrt(b)"),
        VirtualChannel(name="bad", expression="sqrt(missing)"),
    ]
    results = evaluate_channels(channels, batch)
    assert len(results) == 3
    np.testing.assert_allclose(results[0].values, [2])
    np.testing.assert_allclose(results[1].values, [3])
    assert not results[2].ok

def test_math_result_coerces_to_1d_float32():
    r = MathResult(values=np.array([[1], [2]], dtype=np.float32))
    assert r.values.dtype == np.float32
    assert r.values.ndim == 1
    assert r.sample_count == 2

# ── 高级：MultiAxis / Scatter / Waterfall / SpectrumWaterfall ──
def test_multi_axis_add_and_plot(qtbot):
    plot = MultiAxisPlot()
    qtbot.addWidget(plot)
    assert plot.add_axis("ch0") == 0
    assert plot.add_axis("ch1") == 1
    plot.plot_channel(0, np.arange(10), np.random.rand(10))
    plot.shutdown()

def test_multi_axis_bad_index(qtbot):
    plot = MultiAxisPlot()
    qtbot.addWidget(plot)
    plot.add_axis("x")
    plot.plot_channel(99, np.arange(5), np.arange(5))

def test_scatter_points(qtbot):
    sp = ScatterPlot()
    qtbot.addWidget(sp)
    sp.set_points(np.random.rand(20), np.random.rand(20), np.random.rand(20))
    assert sp._scatter is not None

def test_scatter_empty_no_crash(qtbot):
    sp = ScatterPlot()
    qtbot.addWidget(sp)
    sp.set_points(np.array([]), np.array([]))
    assert sp._scatter is None

def test_waterfall_append_and_scroll(qtbot):
    wf = WaterfallPlot(max_rows=3)
    qtbot.addWidget(wf)
    for _ in range(5):
        wf.append_row(np.random.rand(8))
    assert wf.row_count == 3

def test_waterfall_clear(qtbot):
    wf = WaterfallPlot()
    qtbot.addWidget(wf)
    wf.append_row(np.random.rand(8))
    wf.clear()
    assert wf.row_count == 0

def test_spectrum_waterfall(qtbot):
    sw = SpectrumWaterfall(freq_count=16, max_rows=3)
    qtbot.addWidget(sw)
    sw.append_spectrum(np.arange(16), np.random.rand(16))
    assert sw.row_count == 1

def test_spectrum_waterfall_empty(qtbot):
    sw = SpectrumWaterfall(freq_count=16)
    qtbot.addWidget(sw)
    sw.append_spectrum(np.arange(16), np.array([]))
    assert sw.row_count == 0


"""波形数学引擎：表达式解析器 + 引擎编排单测（Wave 58，纯 numpy，无 qtbot）。

覆盖 ast 安全表达式求值（白名单 + 危险输入拒绝 + 语法/语义错误）、
VirtualChannel 校验、engine 编排（ChannelBatch → MathResult，逐通道独立）。
函数库单测在 test_waveform_math.py。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import numpy as np
import pytest

from embeddebug.serial_station.core.measurements import ChannelBatch
from embeddebug.serial_station.waveform_math import (
    ExpressionError,
    MathResult,
    VirtualChannel,
    build_scope,
    evaluate,
    evaluate_channel,
    evaluate_channels,
)


def _batch(values: list[list[float]], names: tuple[str, ...] | None = None) -> ChannelBatch:
    """便捷构造 ChannelBatch（2D float32）。"""

    arr = np.asarray(values, dtype=np.float32)
    if arr.ndim == 1:
        arr = arr.reshape(-1, 1)
    if names is None:
        names = tuple(f"ch{i + 1}" for i in range(arr.shape[1]))
    return ChannelBatch(channel_names=names, values=arr, dt_ns=1_000_000)


# ── 表达式解析器（安全） ──────────────────────────────────────────
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
    scope = {
        "ch1": np.array([3.0, 4.0]),
        "ch2": np.array([1.0, 1.0]),
    }
    # abs(ch1 - ch2) + cos(0) → [3, 4]。
    np.testing.assert_allclose(evaluate("abs(ch1 - ch2) + cos(0)", scope), [3, 4], atol=1e-5)


@pytest.mark.parametrize(
    "expr",
    [
        "__import__('os')",     # 调用任意函数 → 拒。
        "ch1.__class__",        # 属性访问 → 拒。
        "ch1[0]",               # 下标 → 拒。
        "ch1 if 1 else ch2",    # IfExp → 拒。
        "lambda x: x",          # lambda → 拒。
        "open('x')",            # 未知函数 → 拒。
        "ch1; import os",       # 多语句 → 语法/节点拒。
    ],
)
def test_evaluate_rejects_dangerous_input(expr):
    scope = {"ch1": np.array([1.0])}
    with pytest.raises(ExpressionError):
        evaluate(expr, scope)


def test_evaluate_unknown_channel_name():
    with pytest.raises(ExpressionError):
        evaluate("chX + 1", {"ch1": np.array([1.0])})


def test_evaluate_syntax_error():
    with pytest.raises(ExpressionError):
        evaluate("ch1 +", {"ch1": np.array([1.0])})


def test_evaluate_rejects_keyword_args():
    with pytest.raises(ExpressionError):
        evaluate("sqrt(ch1, bogus=1)", {"ch1": np.array([1.0])})


# ── 引擎编排 ────────────────────────────────────────────────────────
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
    assert result.error is not None
    assert "unknown" in result.error
    assert result.sample_count == 0


def test_evaluate_channel_empty_batch():
    batch = ChannelBatch(
        channel_names=("a",), values=np.empty((0, 1), dtype=np.float32)
    )
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

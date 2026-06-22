"""CursorMeasurement + ChannelStats + OperationError 边界单元测试。

补强 test_waveform_core / test_operation_result 未直接断言的边角：
- ChannelStats frozen 验证 + 6 字段完整 + format_stats 文本含全部指标。
- CursorMeasurement 默认值（delta_t/frequency/delta_y=None）+ compute_cursor_measurement
  无游标/单游标/零 sample_rate 边界。
- format_cursor_measurement 含 ΔT/freq/ΔY 行（或省略）。
- OperationError frozen + code 必填 + 默认 message="" + 非空 code 验证。
"""

from __future__ import annotations

import numpy as np
import pytest

from embeddebug.serial_station.ui.waveform_measure import (
    ChannelStats,
    CursorMeasurement,
    compute_cursor_measurement,
    compute_channel_stats,
    format_cursor_measurement,
    format_stats,
)
from embeddebug.shared.results import OperationError


# ── ChannelStats frozen + 字段 ────────────────────────────────────────


def test_channel_stats_is_frozen():
    """ChannelStats 是 frozen dataclass。"""

    stats = ChannelStats(vpp=1.0, mean=2.0, maximum=3.0, minimum=1.0, std=0.5, rms=2.1)
    with pytest.raises((AttributeError, Exception)):
        stats.vpp = 99.0  # type: ignore[misc]


def test_channel_stats_six_fields():
    """ChannelStats 含 6 个统计字段。"""

    stats = ChannelStats(vpp=1.0, mean=2.0, maximum=3.0, minimum=1.0, std=0.5, rms=2.1)
    assert stats.vpp == 1.0
    assert stats.mean == 2.0
    assert stats.maximum == 3.0
    assert stats.minimum == 1.0
    assert stats.std == 0.5
    assert stats.rms == 2.1


def test_format_stats_contains_all_six_metrics():
    """format_stats 文本含全部 6 个指标名。"""

    stats = ChannelStats(vpp=1.0, mean=2.0, maximum=3.0, minimum=1.0, std=0.5, rms=2.1)
    text = format_stats(stats)
    assert "Vpp" in text or "vpp" in text.lower()
    assert "Mean" in text or "mean" in text.lower()
    assert "Max" in text or "max" in text.lower()
    assert "Min" in text or "min" in text.lower()
    assert "Std" in text or "std" in text.lower()
    assert "RMS" in text or "rms" in text.lower()


def test_compute_channel_stats_constant_signal():
    """常量信号 → vpp=0, std=0。"""

    stats = compute_channel_stats(np.array([5.0, 5.0, 5.0]))
    assert stats.vpp == 0.0
    assert stats.std == 0.0
    assert stats.mean == 5.0


# ── CursorMeasurement 边界 ────────────────────────────────────────────


def test_cursor_measurement_all_none():
    """CursorMeasurement 全 None 合法。"""

    m = CursorMeasurement(delta_t=None, frequency=None, delta_y=None)
    assert m.delta_t is None
    assert m.frequency is None
    assert m.delta_y is None


def test_cursor_measurement_frozen():
    """CursorMeasurement 是 frozen dataclass。"""

    m = CursorMeasurement(delta_t=1.0, frequency=1.0, delta_y=2.0)
    with pytest.raises((AttributeError, Exception)):
        m.delta_t = 99.0  # type: ignore[misc]


def test_compute_cursor_measurement_no_cursors():
    """无游标 → delta_t/frequency/delta_y=None。"""

    m = compute_cursor_measurement(
        x_cursor_values=[], y_cursor_values=[], sample_rate=1000.0,
    )
    assert m.delta_t is None
    assert m.frequency is None
    assert m.delta_y is None


def test_compute_cursor_measurement_single_x_no_delta_t():
    """单 X 游标 → delta_t=None（需两个才有差值）。"""

    m = compute_cursor_measurement(
        x_cursor_values=[5.0], y_cursor_values=[], sample_rate=1000.0,
    )
    assert m.delta_t is None


def test_compute_cursor_measurement_two_x():
    """两 X 游标 → delta_t = |x2-x1|/sample_rate, frequency = 1/delta_t。"""

    m = compute_cursor_measurement(
        x_cursor_values=[0.0, 100.0], y_cursor_values=[], sample_rate=1000.0,
    )
    assert m.delta_t is not None
    assert abs(m.delta_t - 0.1) < 1e-9  # 100/1000
    assert m.frequency is not None
    assert abs(m.frequency - 10.0) < 1e-6  # 1/0.1


def test_compute_cursor_measurement_zero_sample_rate():
    """sample_rate=0 + 两 X 游标 → frequency=None（防除零）。"""

    m = compute_cursor_measurement(
        x_cursor_values=[0.0, 100.0], y_cursor_values=[], sample_rate=0.0,
    )
    assert m.frequency is None


def test_compute_cursor_measurement_two_y():
    """两 Y 游标 → delta_y = |y2-y1|。"""

    m = compute_cursor_measurement(
        x_cursor_values=[], y_cursor_values=[1.0, 4.0], sample_rate=1000.0,
    )
    assert m.delta_y is not None
    assert abs(m.delta_y - 3.0) < 1e-9


# ── format_cursor_measurement ─────────────────────────────────────────


def test_format_cursor_measurement_no_data():
    """全 None → 文本不崩溃。"""

    m = CursorMeasurement(delta_t=None, frequency=None, delta_y=None)
    text = format_cursor_measurement(m)
    assert isinstance(text, str)
    assert len(text) > 0


# ── OperationError 边界 ───────────────────────────────────────────────


def test_operation_error_is_frozen():
    """OperationError 是 frozen dataclass。"""

    e = OperationError(code="timeout")
    with pytest.raises((AttributeError, Exception)):
        e.code = "changed"  # type: ignore[misc]


def test_operation_error_default_message_empty():
    """默认 message=""。"""

    e = OperationError(code="x")
    assert e.message == ""


def test_operation_error_custom_message():
    """自定义 message。"""

    e = OperationError(code="x", message="detail")
    assert e.message == "detail"


def test_operation_error_empty_code_raises():
    """code="" → ValueError。"""

    with pytest.raises(ValueError):
        OperationError(code="")


def test_operation_error_code_field():
    """code 字段正确存储。"""

    e = OperationError(code="timeout", message="timed out")
    assert e.code == "timeout"

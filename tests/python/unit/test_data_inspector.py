"""数据检查器单元测试。"""
from __future__ import annotations
import struct
import numpy as np
from embeddebug.serial_station.data_inspector import DataInspector, HexFormatter

def test_hex_format():
    lines = HexFormatter.format_bytes(b"Hi\x00")
    assert "48 69 00" in lines[0] and "|Hi.|" in lines[0]

def test_hex_empty():
    assert HexFormatter.format_bytes(b"") == []

def test_int_le():
    assert HexFormatter.format_int(b"\x01\x00", signed=False) == 1

def test_float_le():
    assert HexFormatter.format_float(struct.pack("<f", 1.5)) == 1.5

def test_ascii():
    assert HexFormatter.format_ascii(b"A\x00") == "A."

def test_stats():
    r = DataInspector().inspect(np.array([[1.0], [2.0], [3.0]], dtype=np.float32), ["x"])
    assert r.channel_stats["x"]["min"] == 1.0 and r.channel_stats["x"]["max"] == 3.0

def test_outlier():
    r = DataInspector(outlier_threshold=2.0).inspect(np.array([[0], [0], [0], [0], [0], [100]], dtype=np.float32), ["x"])
    assert len(r.outliers) == 1 and r.outliers[0]["index"] == 5

def test_no_outlier_constant():
    r = DataInspector(outlier_threshold=1.0).inspect(np.array([[5], [5], [5]], dtype=np.float32), ["c"])
    assert r.outliers == []

def test_correlation_positive():
    col = np.array([1, 2, 3])
    r = DataInspector().inspect(np.column_stack([col, col]), ["a", "b"])
    assert abs(r.correlations[("a", "b")] - 1.0) < 1e-6

def test_correlation_negative():
    r = DataInspector().inspect(np.array([[1, 10], [2, 8], [3, 6]], dtype=np.float32), ["a", "b"])
    assert abs(r.correlations[("a", "b")] - (-1.0)) < 1e-6

def test_correlation_constant():
    r = DataInspector().inspect(np.array([[1, 5], [2, 5], [3, 5]], dtype=np.float32), ["a", "b"])
    assert r.correlations[("a", "b")] == 0.0

def test_default_names():
    r = DataInspector().inspect(np.array([[1, 2]], dtype=np.float32))
    assert "ch0" in r.channel_stats and "ch1" in r.channel_stats

def test_reject_1d():
    import pytest
    with pytest.raises(ValueError):
        DataInspector().inspect(np.array([1, 2, 3]))

def test_to_dict():
    r = DataInspector().inspect(np.array([[1, 2], [3, 4]], dtype=np.float32), ["a", "b"])
    d = r.to_dict()
    assert "a|b" in d["correlations"] and d["threshold"] > 0


# ---- Batch 135: InspectionResult.format_text + to_dict 边界 ----


def test_format_text_contains_channel_stats():
    """format_text 输出 channel 名 + min/max/mean/std 四统计。"""
    from embeddebug.serial_station.data_inspector import InspectionResult

    result = InspectionResult(
        channel_stats={
            "temp": {"min": 10.0, "max": 90.0, "mean": 50.0, "std": 20.0},
        },
    )
    text = result.format_text()
    assert "[InspectionResult]" in text
    assert "channels: 1" in text
    assert "temp" in text
    assert "min=10.000" in text
    assert "max=90.000" in text
    assert "mean=50.000" in text
    assert "std=20.000" in text


def test_format_text_outliers_count_line():
    """format_text 含 outliers 计数行。"""
    from embeddebug.serial_station.data_inspector import InspectionResult

    result = InspectionResult(
        channel_stats={"x": {"min": 0, "max": 1, "mean": 0.5, "std": 0.5}},
        outliers=[{"index": 5}, {"index": 9}],
    )
    text = result.format_text()
    assert "outliers: 2" in text


def test_format_text_sorts_channels_alphabetically():
    """format_text 按字母序输出 channels。"""
    from embeddebug.serial_station.data_inspector import InspectionResult

    stats = {"min": 0, "max": 1, "mean": 0.5, "std": 0.1}
    result = InspectionResult(channel_stats={
        "zebra": stats, "alpha": stats, "mid": stats,
    })
    text = result.format_text()
    alpha_pos = text.index("alpha")
    mid_pos = text.index("mid")
    zebra_pos = text.index("zebra")
    assert alpha_pos < mid_pos < zebra_pos


def test_format_text_empty_channels():
    """无 channel 时 format_text 仍工作（channels: 0）。"""
    from embeddebug.serial_station.data_inspector import InspectionResult

    text = InspectionResult().format_text()
    assert "channels: 0" in text
    assert "outliers: 0" in text


def test_to_dict_correlations_key_format():
    """to_dict 把 (a, b) tuple key 序列化为 'a|b' 字符串。"""
    from embeddebug.serial_station.data_inspector import InspectionResult

    result = InspectionResult(correlations={("a", "b"): 0.95, ("x", "y"): -0.3})
    d = result.to_dict()
    assert d["correlations"]["a|b"] == 0.95
    assert d["correlations"]["x|y"] == -0.3


def test_to_dict_preserves_outliers_and_threshold():
    """to_dict 保留 outliers 列表和 threshold。"""
    from embeddebug.serial_station.data_inspector import InspectionResult

    result = InspectionResult(
        outliers=[{"index": 1, "value": 99.0}],
        threshold=2.5,
    )
    d = result.to_dict()
    assert d["outliers"] == [{"index": 1, "value": 99.0}]
    assert d["threshold"] == 2.5


def test_to_dict_default_empty_state():
    """默认 InspectionResult 的 to_dict 返回空结构。"""
    from embeddebug.serial_station.data_inspector import InspectionResult

    d = InspectionResult().to_dict()
    assert d["channel_stats"] == {}
    assert d["correlations"] == {}
    assert d["outliers"] == []
    assert d["threshold"] == 0.0
    assert d["timestamp_ns"] > 0


def test_inspection_result_timestamp_defaults_to_recent():
    """InspectionResult 默认 timestamp_ns 落在构造前后。"""
    import time
    from embeddebug.serial_station.data_inspector import InspectionResult

    before = time.time_ns()
    result = InspectionResult()
    after = time.time_ns()
    assert before <= result.timestamp_ns <= after


def test_inspect_with_custom_threshold_records_threshold():
    """inspect 把 outlier_threshold 记录到 result.threshold。"""
    r = DataInspector(outlier_threshold=3.5).inspect(
        np.array([[1.0], [2.0]], dtype=np.float32), ["x"]
    )
    assert r.threshold == 3.5

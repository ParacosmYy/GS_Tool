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

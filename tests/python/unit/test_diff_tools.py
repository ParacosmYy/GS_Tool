"""数据对比工具单元测试。"""
from __future__ import annotations
import numpy as np
import pytest
from embeddebug.serial_station.diff_tools import DataDiffer, DiffConfig

def test_identical():
    a = np.array([[1.0, 2.0]])
    r = DataDiffer().compare(a, a.copy(), ["x", "y"], ["x", "y"])
    assert r.is_identical and r.summary["matching_cells"] == 2

def test_strict():
    r = DataDiffer().compare(np.array([[1.0]]), np.array([[1.0 + 1e-9]]), ["x"], ["x"])
    assert not r.is_identical

def test_tolerance():
    r = DataDiffer().compare(np.array([[10.0]]), np.array([[10.5]]), ["x"], ["x"], DiffConfig(tolerance=0.5))
    assert r.is_identical

def test_ignore_column():
    a = np.array([[1.0, 99.0]])
    b = np.array([[1.0, 77.0]])
    r = DataDiffer().compare(a, b, ["x", "y"], ["x", "y"], DiffConfig(ignore_columns=["y"]))
    assert r.is_identical and r.summary["compared_columns"] == ["x"]

def test_row_diff_count():
    a = np.array([[1.0, 2.0]])
    b = np.array([[1.0, 9.0]])
    r = DataDiffer().compare(a, b, ["x", "y"], ["x", "y"])
    assert len(r.row_diffs) == 1 and r.row_diffs[0]["column"] == "y"

def test_different_rows():
    r = DataDiffer().compare(np.array([[1.0], [2.0], [3.0]]), np.array([[1.0]]), ["x"], ["x"])
    assert r.summary["a_rows"] == 3
    assert r.summary["b_rows"] == 1
    assert not r.is_identical

def test_column_reorder():
    a = np.array([[1.0, 2.0]])
    b = np.array([[2.0, 1.0]])
    r = DataDiffer().compare(a, b, ["x", "y"], ["y", "x"])
    assert r.is_identical

def test_max_display_rows():
    a = np.random.rand(50, 1)
    b = np.random.rand(50, 1)
    r = DataDiffer().compare(a, b, ["x"], ["x"], DiffConfig(max_display_rows=5))
    assert len(r.row_diffs) <= 5

def test_config_validation():
    with pytest.raises(ValueError):
        DiffConfig(tolerance=-1).validate()

def test_no_common_columns():
    r = DataDiffer().compare(np.array([[1.0]]), np.array([[1.0]]), ["x"], ["y"])
    assert r.summary["total_cells"] == 0

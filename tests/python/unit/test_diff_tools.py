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

def test_diff_config_defaults():
    cfg = DiffConfig()
    assert cfg.tolerance == 0.0 and cfg.ignore_columns == []
    assert cfg.align_by_timestamp is False and cfg.max_display_rows == 100

def test_diff_config_validate_boundaries():
    DiffConfig(tolerance=0.0).validate()
    DiffConfig(max_display_rows=1).validate()
    with pytest.raises(ValueError, match="max_display_rows"):
        DiffConfig(max_display_rows=0).validate()
    with pytest.raises(ValueError, match="tolerance"):
        DiffConfig(tolerance=-0.1).validate()

def test_diff_result_format_text_and_to_dict():
    from embeddebug.serial_station.diff_tools.result import DiffResult
    empty = DiffResult(summary={}, is_identical=False)
    assert "matching=0" in empty.format_text()
    result = DiffResult(summary={"matching_cells": 5, "differing_cells": 2, "max_diff": 1.5},
                        row_diffs=[{"column": "x"}], is_identical=False)
    text = result.format_text()
    assert "matching=5" in text and "differing=2" in text and "max_diff=1.5" in text
    assert result.to_dict()["row_diffs"] == [{"column": "x"}]

def test_diff_result_empty_defaults():
    from embeddebug.serial_station.diff_tools.result import DiffResult
    result = DiffResult()
    assert result.summary == {} and result.row_diffs == [] and result.is_identical is False

def test_pairs_intersection_indices_and_ignore():
    pairs = DataDiffer._pairs(["a", "b", "c"], ["b", "c", "d"], ignore=[])
    assert [p[0] for p in pairs] == ["b", "c"]
    assert next(p for p in DataDiffer._pairs(["x", "y"], ["y", "x"], ignore=[]) if p[0] == "x") == ("x", 0, 1)
    assert [p[0] for p in DataDiffer._pairs(["a", "b"], ["a", "b"], ignore=["b"])] == ["a"]
    assert DataDiffer._pairs(["a"], ["b"], ignore=[]) == []

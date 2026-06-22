"""DiffConfig + DiffResult + DataDiffer._pairs 边界扩展单元测试。

补强 test_diff_tools.py 未直接断言的边角：
- DiffConfig：默认值 + tolerance=0/max_display_rows=1 边界 + 负值/零值 ValueError。
- DiffResult：format_text 缺字段兜底 0 + 完整 summary + to_dict round-trip + 空默认。
- DataDiffer._pairs：共有名配对 + a/b 索引 + ignore 过滤 + 无交集空 + ignore=[] 全返回。
"""

from __future__ import annotations

import pytest

from embeddebug.serial_station.diff_tools import DataDiffer, DiffConfig
from embeddebug.serial_station.diff_tools.result import DiffResult


# ── DiffConfig 默认值 + 验证 ─────────────────────────────────────────────


def test_diff_config_defaults():
    """DiffConfig 默认值：tolerance=0 / ignore_columns=[] / align_by_timestamp=False / max_display_rows=100。"""

    cfg = DiffConfig()
    assert cfg.tolerance == 0.0
    assert cfg.ignore_columns == []
    assert cfg.align_by_timestamp is False
    assert cfg.max_display_rows == 100


def test_diff_config_validate_tolerance_zero_ok():
    """tolerance=0 合法（边界，不 < 0）。"""

    DiffConfig(tolerance=0.0).validate()  # 不抛


def test_diff_config_validate_max_display_rows_one_ok():
    """max_display_rows=1 合法（边界，不 < 1）。"""

    DiffConfig(max_display_rows=1).validate()  # 不抛


def test_diff_config_validate_max_display_rows_zero_raises():
    """max_display_rows=0 → ValueError。"""

    with pytest.raises(ValueError, match="max_display_rows"):
        DiffConfig(max_display_rows=0).validate()


def test_diff_config_validate_negative_tolerance_raises():
    """tolerance=-0.1 → ValueError。"""

    with pytest.raises(ValueError, match="tolerance"):
        DiffConfig(tolerance=-0.1).validate()


# ── DiffResult format_text / to_dict ─────────────────────────────────────


def test_diff_result_format_text_missing_fields_defaults_zero():
    """format_text 缺字段（matching/differing/max_diff）兜底 0。"""

    result = DiffResult(summary={}, is_identical=False)
    text = result.format_text()
    assert "identical=False" in text
    assert "matching=0" in text
    assert "differing=0" in text


def test_diff_result_format_text_with_values():
    """format_text 含完整 summary。"""

    result = DiffResult(summary={"matching_cells": 5, "differing_cells": 2, "max_diff": 1.5})
    text = result.format_text()
    assert "matching=5" in text
    assert "differing=2" in text
    assert "max_diff=1.5" in text


def test_diff_result_to_dict_round_trip():
    """to_dict 返回可序列化 dict（含 summary/row_diffs/is_identical）。"""

    result = DiffResult(
        summary={"matching_cells": 1},
        row_diffs=[{"column": "x", "a": 1.0, "b": 2.0}],
        is_identical=False,
    )
    d = result.to_dict()
    assert d["summary"] == {"matching_cells": 1}
    assert d["row_diffs"] == [{"column": "x", "a": 1.0, "b": 2.0}]
    assert d["is_identical"] is False


def test_diff_result_empty_defaults():
    """空 DiffResult 默认 summary={} / row_diffs=[] / is_identical=False。"""

    result = DiffResult()
    assert result.summary == {}
    assert result.row_diffs == []
    assert result.is_identical is False


# ── DataDiffer._pairs 静态助手 ───────────────────────────────────────────


def test_pairs_intersects_common_names():
    """_pairs 返回两侧共有名字的 (name, a_idx, b_idx) 三元组。"""

    pairs = DataDiffer._pairs(["a", "b", "c"], ["b", "c", "d"], ignore=[])
    names = [p[0] for p in pairs]
    assert "b" in names
    assert "c" in names
    assert "a" not in names  # a 不在 b_names
    assert "d" not in names  # d 不在 a_names


def test_pairs_returns_indices():
    """_pairs 三元组含 a/b 索引（供 compare 定位列）。"""

    pairs = DataDiffer._pairs(["x", "y"], ["y", "x"], ignore=[])
    # x 在 a_idx=0, 在 b_names 中 idx=1
    x_pair = next(p for p in pairs if p[0] == "x")
    assert x_pair == ("x", 0, 1)


def test_pairs_respects_ignore_list():
    """_pairs 过滤 ignore 列表中的名字。"""

    pairs = list(DataDiffer._pairs(["a", "b"], ["a", "b"], ignore=["b"]))
    names = [p[0] for p in pairs]
    assert "a" in names
    assert "b" not in names


def test_pairs_empty_when_no_intersection():
    """无交集 → 空配对列表。"""

    pairs = list(DataDiffer._pairs(["a"], ["b"], ignore=[]))
    assert pairs == []


def test_pairs_empty_ignore_returns_all_common():
    """ignore=[] → 返回全部共有名字。"""

    pairs = list(DataDiffer._pairs(["a", "b"], ["a", "b"], ignore=[]))
    assert len(pairs) == 2

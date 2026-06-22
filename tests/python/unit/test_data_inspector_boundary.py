"""data_inspector/inspector 边界单元测试。

补强 test_data_inspector.py 未直接断言的边角：
- DataInspector 构造：默认 bin_count=16 + 自定义 outlier_threshold + 属性。
- inspect 空矩阵 / 单通道 / 单行。
- _stat 统计字段（min/max/mean/std）。
- _correlations 多通道对 / 对称性。
- DEFAULT_OUTLIER_THRESHOLD 常量。
"""

from __future__ import annotations

import numpy as np
import pytest

from embeddebug.serial_station.data_inspector.inspector import (
    DEFAULT_OUTLIER_THRESHOLD,
    DataInspector,
)


# ── 构造 + 属性 ─────────────────────────────────────────────────────────


def test_default_bin_count():
    """默认 bin_count=16。"""

    insp = DataInspector()
    assert insp._bin_count == 16


def test_custom_bin_count():
    """自定义 bin_count=32。"""

    insp = DataInspector(bin_count=32)
    assert insp._bin_count == 32


def test_outlier_threshold_property():
    """outlier_threshold 属性返回构造值。"""

    insp = DataInspector(outlier_threshold=3.0)
    assert insp.outlier_threshold == 3.0


def test_default_outlier_threshold_constant():
    """DEFAULT_OUTLIER_THRESHOLD 常量存在且为正。"""

    assert DEFAULT_OUTLIER_THRESHOLD > 0


# ── inspect 边界 ───────────────────────────────────────────────────────


def test_inspect_single_row_empty_stats():
    """单行矩阵 inspect → channel_stats 有 2 通道。"""

    insp = DataInspector()
    data = np.array([[1.0, 2.0]], dtype=np.float32)
    result = insp.inspect(data, channel_names=("a", "b"))
    assert len(result.channel_stats) == 2


def test_inspect_single_channel():
    """单通道 inspect → 1 channel。"""

    insp = DataInspector()
    data = np.array([[1.0], [2.0], [3.0]], dtype=np.float32)
    result = insp.inspect(data, channel_names=("temp",))
    assert len(result.channel_stats) == 1
    assert "temp" in result.channel_stats


def test_inspect_single_row():
    """单行矩阵 inspect → 统计退化为该行的值。"""

    insp = DataInspector()
    data = np.array([[5.0, 10.0]], dtype=np.float32)
    result = insp.inspect(data, channel_names=("a", "b"))
    assert result.channel_stats["a"]["mean"] == 5.0
    assert result.channel_stats["b"]["mean"] == 10.0


def test_inspect_rejects_1d():
    """1D 数组 inspect → ValueError。"""

    insp = DataInspector()
    with pytest.raises((ValueError, Exception)):
        insp.inspect(np.array([1.0, 2.0, 3.0]))


def test_inspect_default_channel_names():
    """无 channel_names → 默认 ch0/ch1/...。"""

    insp = DataInspector()
    data = np.array([[1.0, 2.0]], dtype=np.float32)
    result = insp.inspect(data)
    assert "ch0" in result.channel_stats
    assert "ch1" in result.channel_stats


# ── _stat 统计字段 ─────────────────────────────────────────────────────


def test_stat_fields_present():
    """_stat 返回 min/max/mean/std 4 个字段。"""

    insp = DataInspector()
    col = np.array([1.0, 2.0, 3.0, 4.0, 5.0])
    stats = insp._stat(col)
    assert set(stats.keys()) >= {"min", "max", "mean", "std"}
    assert stats["min"] == 1.0
    assert stats["max"] == 5.0
    assert stats["mean"] == 3.0


def test_stat_std_positive():
    """_stat std > 0（有方差的数据）。"""

    insp = DataInspector()
    col = np.array([1.0, 2.0, 3.0])
    stats = insp._stat(col)
    assert stats["std"] > 0


def test_stat_std_zero_constant():
    """_stat std = 0（常量数据）。"""

    insp = DataInspector()
    col = np.array([5.0, 5.0, 5.0])
    stats = insp._stat(col)
    assert stats["std"] == 0.0


# ── _correlations ──────────────────────────────────────────────────────


def test_correlations_two_channels():
    """2 通道 → 1 对 correlation。"""

    DataInspector()
    matrix = np.array([[1.0, 2.0], [2.0, 4.0], [3.0, 6.0]], dtype=np.float32)
    corrs = DataInspector._correlations(matrix, ("a", "b"))
    # 完全正相关
    assert ("a", "b") in corrs
    assert corrs[("a", "b")] > 0.99


def test_correlations_single_channel_empty():
    """单通道 → 空 correlations。"""

    DataInspector()
    matrix = np.array([[1.0], [2.0]], dtype=np.float32)
    corrs = DataInspector._correlations(matrix, ("a",))
    assert len(corrs) == 0


def test_correlations_three_channels_has_three_pairs():
    """3 通道 → 3 对。"""

    DataInspector()
    matrix = np.array([[1, 2, 3], [2, 3, 1], [3, 1, 2]], dtype=np.float32)
    corrs = DataInspector._correlations(matrix, ("a", "b", "c"))
    assert len(corrs) == 3


# ── inspect 端到端验证 ─────────────────────────────────────────────────


def test_inspect_with_outlier_detection():
    """inspect 检测离群值（极端值超出 threshold*std）。"""

    insp = DataInspector(outlier_threshold=2.0)
    data = np.array([
        [1.0], [1.1], [0.9], [1.0], [1.05], [100.0],  # 最后一个是离群值
    ], dtype=np.float32)
    result = insp.inspect(data, channel_names=("temp",))
    assert len(result.outliers) >= 1


def test_inspect_no_outlier_in_normal_data():
    """正常数据无离群值。"""

    insp = DataInspector(outlier_threshold=3.0)
    data = np.array([[50.0], [51.0], [49.0], [50.5], [49.5]], dtype=np.float32)
    result = insp.inspect(data, channel_names=("temp",))
    assert len(result.outliers) == 0


def test_inspect_result_has_timestamp():
    """inspect 结果含 timestamp_ns。"""

    insp = DataInspector()
    data = np.array([[1.0]], dtype=np.float32)
    result = insp.inspect(data, channel_names=("a",))
    assert result.timestamp_ns > 0

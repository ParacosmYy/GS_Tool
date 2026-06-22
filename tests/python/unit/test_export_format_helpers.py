"""export/format 纯 helper 单元测试（ExportFormat delimiter + ExportConfig validation）。

补强 test_export.py 未直接断言的边角：
- ExportFormat.delimiter：CSV=, / TSV=tab / JSON/NUMPY 也返回 tab（非 CSV 兜底）。
- ExportFormat.from_extension：4 格式 + 大小写 + 无扩展名 + 未知扩展名。
- ExportConfig.for_format：便捷工厂。
- ExportConfig frozen：不可变。
- ExportConfig validation：空 channels / 非 ExportFormat / channels tuple 强制转换。
"""

from __future__ import annotations

import pytest

from embeddebug.serial_station.export.format import ExportConfig, ExportFormat


# ── ExportFormat.delimiter ───────────────────────────────────────────────


def test_delimiter_csv_is_comma():
    """CSV 格式分隔符 = 逗号。"""

    assert ExportFormat.CSV.delimiter == ","


def test_delimiter_tsv_is_tab():
    """TSV 格式分隔符 = 制表符。"""

    assert ExportFormat.TSV.delimiter == "\t"


def test_delimiter_json_falls_back_to_tab():
    """JSON 格式（非 CSV）→ tab 兜底（delimiter 仅 CSV/TSV 有意义）。"""

    assert ExportFormat.JSON.delimiter == "\t"


def test_delimiter_numpy_falls_back_to_tab():
    """NUMPY 格式 → tab 兜底。"""

    assert ExportFormat.NUMPY.delimiter == "\t"


# ── ExportFormat.from_extension ──────────────────────────────────────────


def test_from_extension_csv():
    assert ExportFormat.from_extension("data.csv") == ExportFormat.CSV


def test_from_extension_tsv():
    assert ExportFormat.from_extension("data.tsv") == ExportFormat.TSV


def test_from_extension_json():
    assert ExportFormat.from_extension("out.json") == ExportFormat.JSON


def test_from_extension_numpy():
    """NUMPY 扩展名 = .numpy（成员值）。"""

    assert ExportFormat.from_extension("arr.numpy") == ExportFormat.NUMPY


def test_from_extension_uppercase():
    """大写扩展名 normalize 为小写。"""

    assert ExportFormat.from_extension("DATA.CSV") == ExportFormat.CSV


def test_from_extension_with_path_object():
    """Path 对象也可解析。"""

    from pathlib import Path

    assert ExportFormat.from_extension(Path("dir/data.json")) == ExportFormat.JSON


def test_from_extension_unknown_raises():
    """未知扩展名 → ValueError。"""

    with pytest.raises(ValueError, match="无法识别"):
        ExportFormat.from_extension("data.txt")


def test_from_extension_no_extension_raises():
    """无扩展名 → ValueError。"""

    with pytest.raises(ValueError):
        ExportFormat.from_extension("noext")


# ── ExportConfig.for_format ──────────────────────────────────────────────


def test_for_format_creates_default_config():
    """for_format 返回该格式的默认配置（无 time_range/channels 过滤）。"""

    cfg = ExportConfig.for_format(ExportFormat.CSV)
    assert cfg.format == ExportFormat.CSV
    assert cfg.time_range is None
    assert cfg.channels is None
    assert cfg.include_header is True
    assert cfg.decimal_places == 6


def test_for_format_all_formats():
    """4 种格式都能 for_format。"""

    for fmt in ExportFormat:
        cfg = ExportConfig.for_format(fmt)
        assert cfg.format == fmt


# ── ExportConfig frozen 不可变 ───────────────────────────────────────────


def test_export_config_is_frozen():
    """ExportConfig 是 frozen dataclass（不可变）。"""

    cfg = ExportConfig(format=ExportFormat.CSV)
    with pytest.raises((AttributeError, Exception)):
        cfg.decimal_places = 99  # type: ignore[misc]


# ── ExportConfig validation 边角 ─────────────────────────────────────────


def test_export_config_empty_channels_raises():
    """空 channels tuple → ValueError（应传 None 表示全部）。"""

    with pytest.raises(ValueError, match="channels"):
        ExportConfig(format=ExportFormat.CSV, channels=())


def test_export_config_non_enum_format_raises():
    """format 非 ExportFormat 枚举 → ValueError。"""

    with pytest.raises(ValueError, match="format"):
        ExportConfig(format="csv")  # type: ignore[arg-type]


def test_export_config_channels_list_converted_to_tuple():
    """channels 传 list → __post_init__ 转为 tuple（frozen 要求）。"""

    cfg = ExportConfig(format=ExportFormat.CSV, channels=["a", "b"])
    assert isinstance(cfg.channels, tuple)
    assert cfg.channels == ("a", "b")


def test_export_config_time_range_equal_start_end_ok():
    """time_range start == end 合法（边界，不 >）。"""

    cfg = ExportConfig(format=ExportFormat.CSV, time_range=(1.0, 1.0))
    assert cfg.time_range == (1.0, 1.0)


def test_export_config_decimal_places_zero_ok():
    """decimal_places=0 合法（边界，不 < 0）。"""

    cfg = ExportConfig(format=ExportFormat.CSV, decimal_places=0)
    assert cfg.decimal_places == 0


def test_export_config_defaults():
    """ExportConfig 默认：time_range=None / channels=None / include_header=True / decimal_places=6。"""

    cfg = ExportConfig(format=ExportFormat.JSON)
    assert cfg.time_range is None
    assert cfg.channels is None
    assert cfg.include_header is True
    assert cfg.decimal_places == 6


# ── ExportFormat 枚举完备性 ──────────────────────────────────────────────


def test_export_format_has_four_members():
    """ExportFormat 含 CSV/TSV/JSON/NUMPY 4 成员。"""

    members = {m.value for m in ExportFormat}
    assert members == {"csv", "tsv", "json", "numpy"}


def test_export_format_values_distinct():
    """4 个枚举值互不相同。"""

    values = [m.value for m in ExportFormat]
    assert len(set(values)) == len(values)

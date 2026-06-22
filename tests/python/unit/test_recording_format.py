"""录制格式枚举 + RecordingHeader 单元测试。

覆盖：RecordingFormat.from_extension 解析、RecordingHeader to_dict/from_dict
round-trip、channel_count 属性、DEFAULT_DT_NS 默认。
"""

from __future__ import annotations

import pytest

from embeddebug.serial_station.recording.format import (
    DEFAULT_DT_NS,
    RecordingFormat,
    RecordingHeader,
)


def test_from_extension_csv():
    assert RecordingFormat.from_extension("data.csv") == RecordingFormat.CSV


def test_from_extension_jsonl():
    assert RecordingFormat.from_extension("log.jsonl") == RecordingFormat.JSONL


def test_from_extension_uppercase():
    assert RecordingFormat.from_extension("DATA.CSV") == RecordingFormat.CSV


def test_from_extension_invalid():
    with pytest.raises(ValueError):
        RecordingFormat.from_extension("data.txt")


def test_from_extension_no_extension():
    with pytest.raises(ValueError):
        RecordingFormat.from_extension("noext")


def test_default_dt_ns():
    assert DEFAULT_DT_NS == 10_000_000


def test_header_defaults():
    h = RecordingHeader(start_time_ns=1000)
    assert h.dt_ns == DEFAULT_DT_NS
    assert h.channel_names == ()
    assert h.channel_count == 0


def test_header_channel_count():
    h = RecordingHeader(start_time_ns=0, channel_names=("ch0", "ch1", "ch2"))
    assert h.channel_count == 3


def test_header_to_dict():
    h = RecordingHeader(start_time_ns=42, dt_ns=5000, channel_names=("a", "b"))
    d = h.to_dict()
    assert d["start_time_ns"] == 42
    assert d["dt_ns"] == 5000
    assert d["channel_names"] == ["a", "b"]


def test_header_from_dict():
    d = {"start_time_ns": 100, "dt_ns": 2000, "channel_names": ["x", "y"]}
    h = RecordingHeader.from_dict(d)
    assert h.start_time_ns == 100
    assert h.dt_ns == 2000
    assert h.channel_names == ("x", "y")


def test_header_round_trip():
    original = RecordingHeader(start_time_ns=999, dt_ns=33333, channel_names=("alpha", "beta"))
    restored = RecordingHeader.from_dict(original.to_dict())
    assert restored.start_time_ns == original.start_time_ns
    assert restored.dt_ns == original.dt_ns
    assert restored.channel_names == original.channel_names


def test_header_from_dict_defaults():
    """from_dict 缺字段用默认值。"""
    h = RecordingHeader.from_dict({"start_time_ns": 0})
    assert h.dt_ns == DEFAULT_DT_NS
    assert h.channel_names == ()

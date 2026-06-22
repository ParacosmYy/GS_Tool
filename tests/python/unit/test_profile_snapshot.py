"""profile_snapshot 单元测试 — 快照构建 + transport 序列化。

覆盖：build_profile_snapshot 输出结构、config=None 默认值、
transport 子 dict 字段完整性、commandHistory list 转换。
"""

from __future__ import annotations

from embeddebug.serial_station.controllers.profile_snapshot import (
    _transport_snapshot,
    build_profile_snapshot,
)
from embeddebug.serial_station.drivers import SerialPortConfig


def _config() -> SerialPortConfig:
    return SerialPortConfig(
        port_name="COM3",
        baud_rate=115200,
        data_bits=8,
        parity="none",
        stop_bits="1",
        flow_control="none",
    )


def test_build_profile_snapshot_structure():
    snap = build_profile_snapshot(
        name="test",
        mode="serial",
        config=_config(),
        is_connected=True,
        protocol="ascii",
        command_history=("AT", "ATZ"),
    )
    assert snap["name"] == "test"
    assert snap["protocol"] == "ascii"
    assert snap["commandHistory"] == ["AT", "ATZ"]
    assert snap["transport"]["mode"] == "serial"


def test_transport_snapshot_with_config():
    ts = _transport_snapshot("serial", _config(), True)
    assert ts["portName"] == "COM3"
    assert ts["baudRate"] == 115200
    assert ts["connected"] is True
    assert ts["dataBits"] == 8
    assert ts["parity"] == "none"


def test_transport_snapshot_none_config_defaults():
    """config=None 时用默认值。"""
    ts = _transport_snapshot("fake", None, False)
    assert ts["portName"] == ""
    assert ts["baudRate"] == 0
    assert ts["dataBits"] == 8
    assert ts["connected"] is False


def test_build_profile_snapshot_empty_history():
    """空 command_history → 空 list。"""
    snap = build_profile_snapshot("x", "fake", None, False, "ascii", ())
    assert snap["commandHistory"] == []


def test_build_profile_snapshot_tuple_to_list():
    """tuple command_history 转 list（JSON 兼容）。"""
    snap = build_profile_snapshot("x", "fake", None, False, "ascii", ("a", "b", "c"))
    assert isinstance(snap["commandHistory"], list)
    assert snap["commandHistory"] == ["a", "b", "c"]

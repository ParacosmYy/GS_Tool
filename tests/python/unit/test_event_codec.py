"""event_codec + plugins/discovery 单元测试。

覆盖：event_to_record/event_from_record round-trip 序列化、
plugin discovery 基础发现逻辑。
"""

from __future__ import annotations

from embeddebug.serial_station.protocols.base import ProtocolEvent
from embeddebug.serial_station.services.event_codec import (
    event_from_record,
    event_to_record,
)


def test_event_to_record_basic():
    event = ProtocolEvent(type="frame", protocol_name="modbus", payload={"addr": 1}, raw=b"\x01\x02")
    record = event_to_record(event)
    assert record["type"] == "frame"
    assert record["protocolName"] == "modbus"
    assert record["payload"] == {"addr": 1}
    assert record["rawHex"] == "0102"


def test_event_from_record_basic():
    record = {"type": "byte", "protocolName": "ascii", "payload": {"char": "A"}, "rawHex": "41"}
    event = event_from_record(record)
    assert event.type == "byte"
    assert event.protocol_name == "ascii"
    assert event.payload == {"char": "A"}
    assert event.raw == b"\x41"


def test_event_round_trip():
    """to_record → from_record round-trip 保持等价。"""
    original = ProtocolEvent(type="pkt", protocol_name="can", payload={"id": 0x123}, raw=b"\xaa\xbb")
    record = event_to_record(original)
    restored = event_from_record(record)
    assert restored.type == original.type
    assert restored.protocol_name == original.protocol_name
    assert restored.payload == original.payload
    assert restored.raw == original.raw


def test_event_from_record_missing_payload_defaults_empty():
    """缺 payload 字段时默认空 dict。"""
    record = {"type": "x", "protocolName": "test", "rawHex": ""}
    event = event_from_record(record)
    assert event.payload == {}


def test_event_from_record_missing_raw_defaults_empty():
    """缺 rawHex 字段时默认空 bytes。"""
    record = {"type": "x", "protocolName": "test"}
    event = event_from_record(record)
    assert event.raw == b""


def test_event_to_record_empty_raw():
    """空 raw → 空字符串。"""
    event = ProtocolEvent(type="x", protocol_name="p", raw=b"")
    record = event_to_record(event)
    assert record["rawHex"] == ""

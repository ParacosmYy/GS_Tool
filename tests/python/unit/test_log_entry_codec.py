"""log_entry_codec 单元测试 — ProtocolEvent <-> SerialWorkbenchLogEntry 转换器。

覆盖 entry_from_event 的 4 类已知 direction、payload.direction 优先级、
text 回退到 raw、_direction_from_type 推断；以及 event_from_entry 的
type 选择（tx→tx / 其他→frame）、raw 透传、round-trip 稳定性。
"""

from __future__ import annotations

from embeddebug.serial_station.controllers.log_entry import SerialWorkbenchLogEntry
from embeddebug.serial_station.controllers.log_entry_codec import (
    entry_from_event,
    event_from_entry,
)
from embeddebug.serial_station.protocols.base import ProtocolEvent


# ----------------------- entry_from_event -----------------------


def test_entry_from_event_uses_payload_text_when_present():
    """payload.text 优先；raw 不参与 text。"""
    event = ProtocolEvent(
        type="frame",
        protocol_name="raw_data",
        payload={"text": "hello", "direction": "rx"},
        raw=b"ignored",
    )
    entry = entry_from_event(event)
    assert entry.text == "hello"
    assert entry.direction == "rx"
    assert entry.raw == b"ignored"


def test_entry_from_event_falls_back_to_raw_decoded_as_utf8_when_no_text():
    """无 payload.text 时使用 raw.decode(utf-8, replace)。"""
    event = ProtocolEvent(
        type="frame",
        protocol_name="raw_data",
        payload={"direction": "rx"},
        raw=b"OK",
    )
    entry = entry_from_event(event)
    assert entry.text == "OK"


def test_entry_from_event_raw_with_invalid_utf8_uses_replace():
    """无效 utf-8 字节用 errors=replace 代替（不抛异常）。"""
    event = ProtocolEvent(
        type="frame",
        protocol_name="raw_data",
        payload={"direction": "rx"},
        raw=b"\xff\xfe",
    )
    entry = entry_from_event(event)
    # 替换字符存在，但不应抛异常
    assert "\ufffd" in entry.text


def test_entry_from_event_payload_direction_takes_priority_over_type_inference():
    """payload.direction 在 _KNOWN_DIRECTIONS 集合中时优先于 _direction_from_type。"""
    event = ProtocolEvent(
        type="tx",  # type 指向 tx（按推断会是 tx）
        protocol_name="raw_data",
        payload={"direction": "system"},  # 但 payload 显式给 system，应优先
        raw=b"x",
    )
    entry = entry_from_event(event)
    assert entry.direction == "system"


def test_entry_from_event_invalid_payload_direction_falls_back_to_type_inference():
    """payload.direction 不在 _KNOWN_DIRECTIONS 中时回退到 _direction_from_type。"""
    event = ProtocolEvent(
        type="tx",
        protocol_name="raw_data",
        payload={"direction": "weird_value"},  # 非 tx/rx/system/error
        raw=b"x",
    )
    entry = entry_from_event(event)
    assert entry.direction == "tx"  # 来自 _direction_from_type("tx")


def test_entry_from_event_frame_type_infers_rx_direction():
    """frame/measurement/其他类型通过 _direction_from_type 推断为 rx。"""
    event = ProtocolEvent(
        type="frame",
        protocol_name="raw_data",
        payload={},  # 无 direction，需推断
        raw=b"data",
    )
    entry = entry_from_event(event)
    assert entry.direction == "rx"


def test_entry_from_event_measurement_type_infers_rx_direction():
    event = ProtocolEvent(
        type="measurement",
        protocol_name="just_float",
        payload={"values": [1.0]},
        raw=b"1",
    )
    entry = entry_from_event(event)
    assert entry.direction == "rx"


# ----------------------- event_from_entry -----------------------


def test_event_from_entry_tx_direction_produces_tx_event_type():
    """direction=tx 时 event.type 也是 tx。"""
    entry = SerialWorkbenchLogEntry(direction="tx", text="AT", raw=b"AT")
    event = event_from_entry(entry, "raw_data")
    assert event.type == "tx"
    assert event.protocol_name == "raw_data"
    assert event.payload["text"] == "AT"
    assert event.payload["direction"] == "tx"
    assert event.raw == b"AT"


def test_event_from_entry_non_tx_direction_produces_frame_event_type():
    """非 tx direction（rx/system/error）一律映射为 frame 类型。"""
    for direction in ("rx", "system", "error"):
        entry = SerialWorkbenchLogEntry(direction=direction, text="msg", raw=b"msg")
        event = event_from_entry(entry, "raw_data")
        assert event.type == "frame"
        assert event.payload["direction"] == direction


def test_event_from_entry_empty_raw_preserved():
    entry = SerialWorkbenchLogEntry(direction="system", text="", raw=b"")
    event = event_from_entry(entry, "raw_data")
    assert event.raw == b""
    assert event.payload["text"] == ""


# ----------------------- Round-trip -----------------------


def test_round_trip_entry_to_event_to_entry_preserves_direction_and_text():
    """tx 与 rx 两种 direction 经过 event 化再转回 entry 时保持稳定。"""
    for direction in ("tx", "rx"):
        original = SerialWorkbenchLogEntry(direction=direction, text="payload", raw=b"payload")
        event = event_from_entry(original, "raw_data")
        # event.payload.direction 是已知的，entry_from_event 会优先使用它
        restored = entry_from_event(event)
        assert restored.direction == direction
        assert restored.text == "payload"
        assert restored.raw == b"payload"


def test_round_trip_system_direction_via_payload_direction():
    """system direction 通过 event_from_entry→entry_from_event 保持稳定。

    event_from_entry 把 system 映射为 type=frame；entry_from_event 看到
    payload.direction=system（在 _KNOWN_DIRECTIONS 中）→ 保留 system。
    """
    original = SerialWorkbenchLogEntry(direction="system", text="connected", raw=b"connected")
    event = event_from_entry(original, "raw_data")
    restored = entry_from_event(event)
    assert restored.direction == "system"
    assert restored.text == "connected"

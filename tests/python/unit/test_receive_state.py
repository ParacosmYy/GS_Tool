"""controller_receive_state 单元测试 — 收字节 + 错误处理。

用 fake dispatcher 测 handle_received_bytes + handle_error。
"""

from __future__ import annotations

from embeddebug.serial_station.controllers.controller_receive_state import (
    ReceiveState,
    handle_error,
    handle_received_bytes,
)
from embeddebug.serial_station.protocols.base import ProtocolEvent


class _FakeDispatcher:
    """可控事件的假分发器。"""

    def __init__(self, events: list[ProtocolEvent]) -> None:
        self._events = list(events)

    def feed(self, data: bytes) -> list[ProtocolEvent]:
        return list(self._events)


def test_handle_received_bytes_no_events():
    """分发器无事件 → 不追加日志，state 不变。"""
    state = ReceiveState()
    dispatcher = _FakeDispatcher([])
    entries = []
    new_state = handle_received_bytes(
        state=state, data=b"", dispatcher=dispatcher,
        entries=entries, log_callbacks=[], measurement_callbacks=[],
    )
    assert len(entries) == 0
    assert new_state.measurement_ring is None


def test_handle_received_bytes_byte_event():
    """byte 事件追加日志条目。"""
    event = ProtocolEvent(type="byte", protocol_name="ascii", payload={"data": "41"})
    state = ReceiveState()
    dispatcher = _FakeDispatcher([event])
    entries = []
    handle_received_bytes(
        state=state, data=b"\x41", dispatcher=dispatcher,
        entries=entries, log_callbacks=[], measurement_callbacks=[],
    )
    assert len(entries) == 1


def test_handle_received_bytes_measurement_creates_ring():
    """measurement 事件创建 ring + 触发回调。"""
    event = ProtocolEvent(type="measurement", protocol_name="x", payload={"values": [1.0]})
    state = ReceiveState()
    dispatcher = _FakeDispatcher([event])
    batches = []
    new_state = handle_received_bytes(
        state=state, data=b"x", dispatcher=dispatcher,
        entries=[], log_callbacks=[], measurement_callbacks=[lambda b: batches.append(b)],
    )
    assert new_state.measurement_ring is not None
    assert len(batches) == 1


def test_handle_error_adds_entry_and_notifies():
    entries = []
    errors = []
    handle_error("timeout", entries=entries, log_callbacks=[], error_callbacks=[lambda m: errors.append(m)])
    assert len(entries) == 1
    assert entries[0].direction == "error"
    assert errors == ["timeout"]


def test_receive_state_frozen():
    import pytest
    s = ReceiveState()
    with pytest.raises((AttributeError, TypeError)):
        s.measurement_ring = "x"  # type: ignore[assignment]


def test_handle_received_bytes_multiple_calls_accumulate_entries():
    state = ReceiveState()
    entries = []
    dispatcher = _FakeDispatcher([ProtocolEvent(type="byte", protocol_name="ascii", payload={"data": "41"})])
    for _ in range(3):
        handle_received_bytes(
            state=state, data=b"A", dispatcher=dispatcher,
            entries=entries, log_callbacks=[], measurement_callbacks=[],
        )
    assert len(entries) == 3


def test_handle_received_bytes_log_callback_called():
    calls = []
    event = ProtocolEvent(type="byte", protocol_name="ascii", payload={"data": "41"})
    handle_received_bytes(
        state=ReceiveState(), data=b"A", dispatcher=_FakeDispatcher([event]),
        entries=[], log_callbacks=[calls.append], measurement_callbacks=[],
    )
    assert len(calls) == 1


def test_handle_received_bytes_empty_callbacks_no_crash():
    handle_received_bytes(
        state=ReceiveState(), data=b"x", dispatcher=_FakeDispatcher([]),
        entries=[], log_callbacks=[], measurement_callbacks=[],
    )


def test_handle_received_bytes_raw_event_skips_measurement_callback():
    event = ProtocolEvent(type="byte", protocol_name="raw_data", payload={"data": "hello"})
    calls = []
    state = handle_received_bytes(
        state=ReceiveState(), data=b"hello", dispatcher=_FakeDispatcher([event]),
        entries=[], log_callbacks=[], measurement_callbacks=[calls.append],
    )
    assert calls == []
    assert state.measurement_ring is None

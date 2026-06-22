"""controller_connection_state 单元测试 — transport 生命周期辅助。

覆盖：active_local_port、bind_transport、replace_transport、disconnect_transport。
用 FakeSerialTransport 测。
"""

from __future__ import annotations

from embeddebug.serial_station.controllers.controller_connection_state import (
    active_local_port,
    bind_transport,
    disconnect_transport,
    replace_transport,
)
from embeddebug.serial_station.drivers import FakeSerialTransport, SerialPortConfig


def _open_fake() -> FakeSerialTransport:
    t = FakeSerialTransport()
    t.open(SerialPortConfig(port_name="fake", baud_rate=0))
    return t


def test_active_local_port_none_when_zero():
    t = FakeSerialTransport()
    assert active_local_port(t) is None


def test_bind_transport_registers_callbacks():
    t = FakeSerialTransport()
    bytes_cb = lambda b: None
    error_cb = lambda m: None
    bind_transport(t, bytes_cb, error_cb)
    # 不抛异常即成功。


def test_replace_transport_closes_current():
    current = _open_fake()
    replacement = FakeSerialTransport()
    result = replace_transport(current, replacement, lambda b: None, lambda m: None)
    assert result is replacement
    assert current.is_open is False


def test_replace_transport_when_current_closed():
    current = FakeSerialTransport()  # 未 open
    replacement = FakeSerialTransport()
    result = replace_transport(current, replacement, lambda b: None, lambda m: None)
    assert result is replacement


def test_disconnect_transport_when_open_logs():
    t = _open_fake()
    entries = []
    disconnect_transport(t, "serial", entries, [])
    assert t.is_open is False
    assert len(entries) == 1
    assert "disconnected" in entries[0].text


def test_disconnect_transport_when_closed_no_log():
    t = FakeSerialTransport()  # 未 open
    entries = []
    disconnect_transport(t, "serial", entries, [])
    assert len(entries) == 0

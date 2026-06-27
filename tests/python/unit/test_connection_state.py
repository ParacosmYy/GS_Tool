"""controller_connection_state 单元测试 — transport 生命周期辅助。

覆盖：active_local_port、bind_transport、replace_transport、disconnect_transport。
用 FakeSerialTransport 测。
"""

from __future__ import annotations

import pytest

from embeddebug.serial_station.controllers.controller_connection_state import (
    active_local_port,
    bind_transport,
    connect_endpoint_transport_result,
    connect_fake_transport_result,
    connect_serial_transport_result,
    disconnect_transport,
    replace_transport,
)
from embeddebug.serial_station.drivers import (
    FakeSerialTransport,
    SerialPortConfig,
    SerialTransport,
    TransportRegistry,
)


def _open_fake() -> FakeSerialTransport:
    t = FakeSerialTransport()
    t.open(SerialPortConfig(port_name="fake", baud_rate=0))
    return t


def _registry() -> TransportRegistry:
    return TransportRegistry.with_defaults(serial_factory=FakeSerialTransport)


class _NonFakeTransport(SerialTransport):
    @property
    def config(self) -> SerialPortConfig | None:
        return None

    @property
    def is_open(self) -> bool:
        return False

    def open(self, config: SerialPortConfig) -> bool:
        return True

    def close(self) -> None:
        return None

    def write(self, data: bytes) -> int:
        return len(data)

    def on_bytes_received(self, callback) -> None:
        return None

    def on_error(self, callback) -> None:
        return None


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


@pytest.mark.parametrize(
    ("current", "expected_replacements"),
    [(FakeSerialTransport(), 0), (_NonFakeTransport(), 1)],
)
def test_connect_fake_reuses_or_replaces_transport(current, expected_replacements):
    entries = []
    replaced = []
    result = connect_fake_transport_result(
        current, _registry(), replaced.append, entries, []
    )
    assert result.ok is True
    assert len(entries) == 1
    assert "connected: fake FAKE_LOOPBACK" in entries[0].text
    assert len(replaced) == expected_replacements
    if replaced:
        assert isinstance(replaced[0], FakeSerialTransport)


@pytest.mark.parametrize(
    ("connect", "expected_text"),
    [
        (
            lambda registry, replace, entries: connect_serial_transport_result(
                registry, replace, "FAKE_PORT", 115200, entries, []
            ),
            "connected: serial FAKE_PORT",
        ),
        (
            lambda registry, replace, entries: connect_endpoint_transport_result(
                registry, replace, "tcp", "127.0.0.1", 9999, entries, []
            ),
            "connected: tcp 127.0.0.1:9999",
        ),
    ],
)
def test_connect_transport_results_append_success_entry(connect, expected_text):
    registry = _registry()
    registry.register("tcp", FakeSerialTransport)
    entries = []
    replaced = []
    result = connect(registry, replaced.append, entries)
    assert result.ok is True
    assert len(replaced) == 1
    assert len(entries) == 1
    assert entries[0].text == expected_text


def test_connect_endpoint_transport_result_reports_open_failure():
    registry = _registry()
    registry.register("tcp", lambda: FakeSerialTransport(open_error="boom"))
    entries = []
    result = connect_endpoint_transport_result(
        registry, lambda t: None, "tcp", "127.0.0.1", 9999, entries, []
    )
    assert result.ok is False
    assert result.error_code == "transport_open_failed"
    assert entries == []

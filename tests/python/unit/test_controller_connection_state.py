from __future__ import annotations

from embeddebug.serial_station.controllers.controller_connection_state import (
    active_local_port,
    connect_endpoint_transport_result,
    connect_fake_transport_result,
    disconnect_transport,
    replace_transport,
)
from embeddebug.serial_station.controllers.log_entry import SerialWorkbenchLogEntry
from embeddebug.serial_station.drivers import FakeSerialTransport, SerialPortConfig, SerialTransport, TransportRegistry


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

    def on_bytes_received(self, callback):
        return None

    def on_error(self, callback):
        return None


def test_active_local_port_ignores_missing_or_zero_ports():
    assert active_local_port(FakeSerialTransport()) is None

    transport = FakeSerialTransport()
    transport.local_port = 19001

    assert active_local_port(transport) == 19001


def test_replace_transport_closes_open_current_and_binds_callbacks():
    current = _NonFakeTransport()
    replacement = FakeSerialTransport()
    received: list[bytes] = []
    errors: list[str] = []

    assert current.open(SerialPortConfig("OLD", 115200))

    selected = replace_transport(current, replacement, received.append, errors.append)
    replacement.inject_rx(b"pong")
    replacement._emit_error("denied")

    assert selected is replacement
    assert not current.is_open
    assert received == [b"pong"]
    assert errors == ["denied"]


def test_disconnect_transport_logs_only_when_connection_was_open():
    transport = FakeSerialTransport()
    entries: list[SerialWorkbenchLogEntry] = []
    logged: list[str] = []

    disconnect_transport(transport, "fake", entries, [lambda entry: logged.append(entry.text)])
    assert entries == []

    assert transport.open(SerialPortConfig("FAKE_LOOPBACK", 115200))
    disconnect_transport(transport, "fake", entries, [lambda entry: logged.append(entry.text)])

    assert not transport.is_open
    assert [(entry.direction, entry.text) for entry in entries] == [
        ("system", "disconnected: fake")
    ]
    assert logged == ["disconnected: fake"]


def test_connect_fake_transport_result_replaces_non_fake_transport_and_logs_success():
    current = _NonFakeTransport()
    registry = TransportRegistry()
    registry.register("fake", factory=FakeSerialTransport)
    entries: list[SerialWorkbenchLogEntry] = []
    logged: list[str] = []
    selected: list[FakeSerialTransport] = []

    result = connect_fake_transport_result(
        current,
        registry,
        lambda transport: selected.append(transport) or transport,
        entries,
        [lambda entry: logged.append(entry.text)],
    )

    assert result.ok
    assert selected[-1].is_open
    assert selected[-1].config is not None
    assert selected[-1].config.port_name == "FAKE_LOOPBACK"
    assert [(entry.direction, entry.text) for entry in entries] == [
        ("system", "connected: fake FAKE_LOOPBACK")
    ]
    assert logged == ["connected: fake FAKE_LOOPBACK"]


def test_connect_endpoint_transport_result_logs_mode_and_endpoint():
    tcp_transport = FakeSerialTransport()
    registry = TransportRegistry()
    registry.register("tcp", factory=lambda: tcp_transport)
    entries: list[SerialWorkbenchLogEntry] = []

    result = connect_endpoint_transport_result(
        registry,
        lambda transport: transport,
        "tcp",
        "127.0.0.1",
        19002,
        entries,
        [],
    )

    assert result.ok
    assert result.value is not None
    assert result.value.port_name == "127.0.0.1:19002"
    assert entries[-1].text == "connected: tcp 127.0.0.1:19002"

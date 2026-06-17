from __future__ import annotations

from embeddebug.serial_station.controllers.controller_io_state import (
    inject_received_text_result,
    send_text_result,
)
from embeddebug.serial_station.controllers.log_entry import SerialWorkbenchLogEntry
from embeddebug.serial_station.core import SerialDispatcher
from embeddebug.serial_station.drivers import FakeSerialTransport, SerialPortConfig, SerialTransport
from embeddebug.serial_station.protocols import create_default_registry


class _NonFakeTransport(SerialTransport):
    @property
    def config(self) -> SerialPortConfig | None:
        return None

    @property
    def is_open(self) -> bool:
        return True

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


def _dispatcher() -> SerialDispatcher:
    return SerialDispatcher(create_default_registry().create("raw_data"))


def test_send_text_result_reports_not_open_without_mutating_history():
    entries: list[SerialWorkbenchLogEntry] = []
    history: list[str] = []
    errors: list[str] = []

    result = send_text_result(
        FakeSerialTransport(),
        _dispatcher(),
        "before-open",
        history,
        entries,
        [],
        errors.append,
    )

    assert result.failed
    assert result.error_code == "transport_not_open"
    assert history == []
    assert entries == []
    assert errors == ["transport_not_open"]


def test_send_text_result_writes_payload_history_and_log_entry():
    transport = FakeSerialTransport()
    entries: list[SerialWorkbenchLogEntry] = []
    logged: list[str] = []
    history: list[str] = []

    assert transport.open(SerialPortConfig("FAKE_LOOPBACK", 115200))
    result = send_text_result(
        transport,
        _dispatcher(),
        "ping",
        history,
        entries,
        [lambda entry: logged.append(f"{entry.direction}:{entry.text}")],
        lambda message: None,
    )

    assert result.ok
    assert transport.written == [b"ping"]
    assert history == ["ping"]
    assert [(entry.direction, entry.text) for entry in entries] == [("tx", "ping")]
    assert logged == ["tx:ping"]


def test_inject_received_text_result_requires_fake_transport():
    entries: list[SerialWorkbenchLogEntry] = []
    errors: list[str] = []

    result = inject_received_text_result(_NonFakeTransport(), "pong", entries, [], errors.append)

    assert result.failed
    assert result.error_code == "fake_injection_requires_fake_transport"
    assert entries == []
    assert errors == ["fake_injection_requires_fake_transport"]

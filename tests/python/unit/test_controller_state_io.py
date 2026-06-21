"""Consolidated controller IO/transport state tests: transport, io, receive."""
from __future__ import annotations

from embeddebug.serial_station.controllers import controller_receive_state
from embeddebug.serial_station.controllers import controller_transport_state
from embeddebug.serial_station.controllers.controller_io_state import (
    inject_received_text_result,
    send_text_result,
)
from embeddebug.serial_station.controllers.log_entry import SerialWorkbenchLogEntry
from embeddebug.serial_station.core import SerialDispatcher
from embeddebug.serial_station.drivers import (
    FakeSerialTransport,
    SerialPortConfig,
    SerialTransport,
    TransportRegistry,
)
from embeddebug.serial_station.protocols import create_default_registry


class _ClosedTransport(SerialTransport):
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


class _AlwaysOpenNonFakeTransport(SerialTransport):
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


def test_transport_runtime_exposes_ports_modes_and_active_local_port():
    runtime = controller_transport_state.create_transport_runtime(
        transport=FakeSerialTransport(),
        port_provider=lambda: ["COM_RUNTIME"],
        bytes_callback=lambda data: None,
        error_callback=lambda message: None,
    )

    assert controller_transport_state.available_serial_ports(runtime) == ("COM_RUNTIME",)
    assert "fake" in controller_transport_state.available_transport_modes(runtime)
    assert controller_transport_state.active_local_port(runtime) is None

    runtime.transport.local_port = 19010

    assert controller_transport_state.active_local_port(runtime) == 19010


def test_connect_fake_replaces_transport_binds_callbacks_and_logs():
    registry = TransportRegistry()
    registry.register("fake", factory=FakeSerialTransport)
    entries: list[SerialWorkbenchLogEntry] = []
    logged: list[str] = []
    received: list[bytes] = []
    errors: list[str] = []
    runtime = controller_transport_state.create_transport_runtime(
        transport=_ClosedTransport(),
        transport_registry=registry,
        bytes_callback=received.append,
        error_callback=errors.append,
    )

    runtime, result = controller_transport_state.connect_fake_result(
        runtime,
        entries=entries,
        log_callbacks=[lambda entry: logged.append(entry.text)],
    )

    runtime.transport.inject_rx(b"runtime")
    runtime.transport._emit_error("runtime_error")

    assert result.ok
    assert controller_transport_state.is_connected(runtime)
    assert runtime.mode == "fake"
    assert received == [b"runtime"]
    assert errors == ["runtime_error"]
    assert entries[-1].text == "connected: fake FAKE_LOOPBACK"
    assert logged == ["connected: fake FAKE_LOOPBACK"]


def test_connect_endpoint_updates_mode_and_profile_config():
    endpoint_transport = FakeSerialTransport()
    registry = TransportRegistry()
    registry.register("tcp", factory=lambda: endpoint_transport)
    runtime = controller_transport_state.create_transport_runtime(
        transport=FakeSerialTransport(),
        transport_registry=registry,
        bytes_callback=lambda data: None,
        error_callback=lambda message: None,
    )

    runtime, result = controller_transport_state.connect_endpoint_result(
        runtime,
        "tcp",
        "127.0.0.1",
        19020,
        entries=[],
        log_callbacks=[],
    )

    assert result.ok
    assert runtime.mode == "tcp"
    assert runtime.transport.config is not None
    assert runtime.transport.config.port_name == "127.0.0.1:19020"


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

    result = inject_received_text_result(_AlwaysOpenNonFakeTransport(), "pong", entries, [], errors.append)

    assert result.failed
    assert result.error_code == "fake_injection_requires_fake_transport"
    assert entries == []
    assert errors == ["fake_injection_requires_fake_transport"]


def test_handle_received_bytes_appends_log_entries_and_measurement_batch():
    registry = create_default_registry()
    dispatcher = SerialDispatcher(registry.create("fire_water"))
    entries: list[SerialWorkbenchLogEntry] = []
    logged: list[SerialWorkbenchLogEntry] = []
    measured = []

    state = controller_receive_state.ReceiveState()
    state = controller_receive_state.handle_received_bytes(
        state=state,
        data=b"1.25,2.50\n",
        dispatcher=dispatcher,
        entries=entries,
        log_callbacks=[logged.append],
        measurement_callbacks=[measured.append],
    )

    assert entries[-1].direction == "rx"
    assert entries[-1].text == "1.25,2.50"
    assert logged == [entries[-1]]
    assert len(measured) == 1
    assert measured[-1].channel_names == ("ch1", "ch2")
    assert measured[-1].values.tolist() == [[1.25, 2.5]]
    assert state.measurement_ring is not None


def test_handle_error_records_log_entry_and_error_callback():
    entries: list[SerialWorkbenchLogEntry] = []
    logged: list[SerialWorkbenchLogEntry] = []
    errors: list[str] = []

    controller_receive_state.handle_error(
        "transport_not_open",
        entries=entries,
        log_callbacks=[logged.append],
        error_callbacks=[errors.append],
    )

    assert entries[-1].direction == "error"
    assert entries[-1].text == "transport_not_open"
    assert logged == [entries[-1]]
    assert errors == ["transport_not_open"]

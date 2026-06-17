from __future__ import annotations

from embeddebug.serial_station.controllers import controller_transport_state
from embeddebug.serial_station.controllers.log_entry import SerialWorkbenchLogEntry
from embeddebug.serial_station.drivers import FakeSerialTransport, SerialPortConfig, SerialTransport, TransportRegistry


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

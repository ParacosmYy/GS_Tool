"""Controller for the Python Serial Station MVP workbench."""

from __future__ import annotations

from collections.abc import Callable
from pathlib import Path
from typing import Any

from embeddebug.serial_station.core import (
    ChannelBatch,
    ChannelRingBuffer,
    SerialDispatcher,
    batch_from_measurement_events,
)
from embeddebug.serial_station.controllers.connection_results import open_transport_result
from embeddebug.serial_station.controllers.log_entry import SerialWorkbenchLogEntry
from embeddebug.serial_station.controllers.log_entry_codec import entry_from_event, event_from_entry
from embeddebug.serial_station.controllers.profile_snapshot import build_profile_snapshot
from embeddebug.serial_station.controllers.session_operations import (
    export_log_result as export_session_log_result,
    load_profile_result as load_session_profile_result,
    replay_entries_result as replay_session_entries_result,
    save_profile_result as save_session_profile_result,
)
from embeddebug.serial_station.controllers.text_decode import decode_injected_text
from embeddebug.serial_station.drivers import (
    FakeSerialTransport,
    SerialPortConfig,
    SerialTransport,
    TransportRegistry,
)
from embeddebug.serial_station.protocols import ProtocolEvent, create_default_registry
from embeddebug.shared import OperationResult


LogEntryCallback = Callable[[SerialWorkbenchLogEntry], None]
ErrorCallback = Callable[[str], None]
MeasurementCallback = Callable[[ChannelBatch], None]
TransportFactory = Callable[[], SerialTransport]
PortProvider = Callable[[], list[str]]

class SerialWorkbenchController:
    """Coordinate fake transport, raw protocol dispatch and log facts."""

    def __init__(
        self,
        transport: SerialTransport | None = None,
        transport_registry: TransportRegistry | None = None,
        serial_transport_factory: TransportFactory | None = None,
        port_provider: PortProvider | None = None,
    ) -> None:
        self._registry = create_default_registry()
        self._transport_registry = transport_registry or TransportRegistry.with_defaults(
            serial_factory=serial_transport_factory,
            serial_port_provider=port_provider,
        )
        self._transport = transport or FakeSerialTransport()
        self._transport_mode = "fake"
        self._dispatcher = SerialDispatcher(self._registry.create("raw_data"))
        self._log_callbacks: list[LogEntryCallback] = []
        self._error_callbacks: list[ErrorCallback] = []
        self._measurement_callbacks: list[MeasurementCallback] = []
        self._entries: list[SerialWorkbenchLogEntry] = []
        self._command_history: list[str] = []
        self._measurement_ring: ChannelRingBuffer | None = None
        self._transport.on_bytes_received(self._handle_bytes_received)
        self._transport.on_error(self._handle_error)

    @property
    def is_connected(self) -> bool:
        return self._transport.is_open

    @property
    def entries(self) -> tuple[SerialWorkbenchLogEntry, ...]:
        return tuple(self._entries)

    @property
    def command_history(self) -> tuple[str, ...]:
        return tuple(self._command_history)

    def on_log_entry(self, callback: LogEntryCallback) -> None:
        self._log_callbacks.append(callback)

    def on_error(self, callback: ErrorCallback) -> None:
        self._error_callbacks.append(callback)

    def on_measurement_batch(self, callback: MeasurementCallback) -> None:
        self._measurement_callbacks.append(callback)

    def available_protocols(self) -> tuple[str, ...]:
        return tuple(self._registry.names())

    def available_serial_ports(self) -> tuple[str, ...]:
        return self._transport_registry.available_ports("serial")

    def available_transport_modes(self) -> tuple[str, ...]:
        return self._transport_registry.modes

    def set_protocol(self, name: str) -> None:
        self._dispatcher.set_protocol(self._registry.create(name))
        self._measurement_ring = None

    def connect_fake(self) -> bool:
        return self.connect_fake_result().ok

    def connect_fake_result(self) -> OperationResult[SerialPortConfig]:
        if not isinstance(self._transport, FakeSerialTransport):
            self._replace_transport(self._transport_registry.create("fake"))
        self._transport_mode = "fake"
        config = SerialPortConfig(port_name="FAKE_LOOPBACK", baud_rate=115200)
        return open_transport_result(self._transport, config, "fake")

    def connect_serial(
        self,
        port_name: str,
        baud_rate: int,
        data_bits: int = 8,
        parity: str = "none",
        stop_bits: str = "1",
        flow_control: str = "none",
    ) -> bool:
        return self.connect_serial_result(
            port_name,
            baud_rate,
            data_bits=data_bits,
            parity=parity,
            stop_bits=stop_bits,
            flow_control=flow_control,
        ).ok

    def connect_serial_result(
        self,
        port_name: str,
        baud_rate: int,
        data_bits: int = 8,
        parity: str = "none",
        stop_bits: str = "1",
        flow_control: str = "none",
    ) -> OperationResult[SerialPortConfig]:
        transport = self._transport_registry.create("serial")
        self._replace_transport(transport)
        self._transport_mode = "serial"
        config = SerialPortConfig(
            port_name=port_name,
            baud_rate=baud_rate,
            data_bits=data_bits,
            parity=parity,
            stop_bits=stop_bits,
            flow_control=flow_control,
        )
        return open_transport_result(self._transport, config, "serial")

    def connect_tcp(self, host: str, port: int) -> bool:
        return self.connect_tcp_result(host, port).ok

    def connect_tcp_result(self, host: str, port: int) -> OperationResult[SerialPortConfig]:
        transport = self._transport_registry.create("tcp")
        self._replace_transport(transport)
        self._transport_mode = "tcp"
        config = SerialPortConfig(port_name=f"{host}:{port}", baud_rate=0)
        return open_transport_result(self._transport, config, "tcp")

    def disconnect(self) -> None:
        self._transport.close()

    def send_text(self, text: str) -> bool:
        return self.send_text_result(text).ok

    def send_text_result(self, text: str) -> OperationResult[SerialWorkbenchLogEntry]:
        if not self._transport.is_open:
            self._handle_error("transport_not_open")
            return OperationResult.failure(
                "transport_not_open",
                "Open a transport before sending",
            )
        payload = self._dispatcher.build_command(text)
        written = self._transport.write(payload)
        if written != len(payload):
            self._handle_error("transport_write_incomplete")
            return OperationResult.failure(
                "transport_write_incomplete",
                "Transport accepted fewer bytes than requested",
            )
        self._remember_command(text)
        entry = SerialWorkbenchLogEntry(direction="tx", text=text, raw=payload)
        self._append_entry(entry)
        return OperationResult.success(entry)

    def inject_received_text(self, text: str) -> OperationResult[None]:
        if not isinstance(self._transport, FakeSerialTransport):
            self._handle_error("fake_injection_requires_fake_transport")
            return OperationResult.failure(
                "fake_injection_requires_fake_transport",
                "Fake RX injection requires fake transport",
            )
        self._transport.inject_rx(decode_injected_text(text).encode("utf-8"))
        return OperationResult.success()

    def clear_log(self) -> None:
        self._entries.clear()

    def export_log(self, path: str | Path) -> None:
        self.export_log_result(path)

    def export_log_result(self, path: str | Path) -> OperationResult[Path]:
        return export_session_log_result(path, self._entries, self._protocol_event_from_entry)

    def replay_log(self, path: str | Path) -> None:
        self.replay_log_result(path)

    def replay_log_result(self, path: str | Path) -> OperationResult[list[SerialWorkbenchLogEntry]]:
        result = replay_session_entries_result(path, entry_from_event)
        if result.ok:
            self._entries.clear()
            for entry in result.value or []:
                self._append_entry(entry)
        return result

    def save_profile(self, path: str | Path, name: str) -> None:
        self.save_profile_result(path, name)

    def save_profile_result(self, path: str | Path, name: str) -> OperationResult[Path]:
        profile = build_profile_snapshot(
            name=name,
            mode=self._transport_mode,
            config=self._transport.config,
            is_connected=self.is_connected,
            protocol=self._dispatcher.protocol_name,
            command_history=self.command_history,
        )
        return save_session_profile_result(path, profile)

    def load_profile(self, path: str | Path) -> dict[str, Any]:
        result = self.load_profile_result(path)
        if result.failed:
            raise OSError(result.message)
        return result.value or {}

    def load_profile_result(self, path: str | Path) -> OperationResult[dict[str, Any]]:
        result = load_session_profile_result(path)
        if result.ok and result.value is not None:
            self._restore_command_history(result.value.get("commandHistory", []))
        return result

    def _handle_bytes_received(self, data: bytes) -> None:
        events = self._dispatcher.feed(data)
        for event in events:
            self._append_entry(entry_from_event(event))
        batch = batch_from_measurement_events(events)
        if batch is not None:
            self._append_measurements(batch)

    def _protocol_event_from_entry(self, entry: SerialWorkbenchLogEntry) -> ProtocolEvent:
        return event_from_entry(entry, self._dispatcher.protocol_name)

    def _append_entry(self, entry: SerialWorkbenchLogEntry) -> None:
        self._entries.append(entry)
        for callback in list(self._log_callbacks):
            callback(entry)

    def _remember_command(self, text: str) -> None:
        if text in self._command_history:
            self._command_history.remove(text)
        self._command_history.append(text)

    def _restore_command_history(self, values: object) -> None:
        self._command_history.clear()
        if not isinstance(values, list):
            return
        for value in values:
            if isinstance(value, str) and value:
                self._remember_command(value)

    def _replace_transport(self, transport: SerialTransport) -> None:
        if self._transport.is_open:
            self._transport.close()
        self._transport = transport
        self._transport.on_bytes_received(self._handle_bytes_received)
        self._transport.on_error(self._handle_error)

    def _append_measurements(self, batch: ChannelBatch) -> None:
        if (
            self._measurement_ring is None
            or self._measurement_ring.latest().values.shape[1] != batch.values.shape[1]
        ):
            self._measurement_ring = ChannelRingBuffer(
                capacity=4096,
                channel_count=batch.values.shape[1],
                channel_names=batch.channel_names,
                dt_ns=batch.dt_ns,
            )
        self._measurement_ring.append(batch)
        latest = self._measurement_ring.latest()
        for callback in list(self._measurement_callbacks):
            callback(latest)

    def _handle_error(self, message: str) -> None:
        for callback in list(self._error_callbacks):
            callback(message)

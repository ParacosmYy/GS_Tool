"""Controller for the Python Serial Station MVP workbench."""

from __future__ import annotations

from collections.abc import Callable
from pathlib import Path
from typing import Any

from embeddebug.serial_station.core import ChannelBatch
from embeddebug.serial_station.controllers import controller_io_state as io_state
from embeddebug.serial_station.controllers import controller_log_state as log_state
from embeddebug.serial_station.controllers import controller_profile_state as profile_state
from embeddebug.serial_station.controllers import controller_protocol_state as protocol_state
from embeddebug.serial_station.controllers import controller_receive_state as receive_state
from embeddebug.serial_station.controllers import controller_session_state as session_state
from embeddebug.serial_station.controllers import controller_transport_state as transport_state
from embeddebug.serial_station.controllers.log_entry import SerialWorkbenchLogEntry
from embeddebug.serial_station.controllers.log_entry_codec import entry_from_event
from embeddebug.serial_station.drivers import SerialPortConfig, SerialTransport, TransportRegistry
from embeddebug.serial_station.protocols import ProtocolEvent
from embeddebug.shared import OperationResult


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
        self._protocol_runtime = protocol_state.create_protocol_runtime()
        self._transport_runtime = transport_state.create_transport_runtime(
            transport=transport, transport_registry=transport_registry,
            serial_transport_factory=serial_transport_factory, port_provider=port_provider,
            bytes_callback=self._handle_bytes_received,
            error_callback=self._handle_error,
        )
        self._log_callbacks: list[log_state.LogEntryCallback] = []
        self._error_callbacks: list[log_state.ErrorCallback] = []
        self._measurement_callbacks: list[MeasurementCallback] = []
        self._entries: list[SerialWorkbenchLogEntry] = []
        self._command_history: list[str] = []

    @property
    def is_connected(self) -> bool:
        return transport_state.is_connected(self._transport_runtime)

    @property
    def entries(self) -> tuple[SerialWorkbenchLogEntry, ...]:
        return tuple(self._entries)

    @property
    def command_history(self) -> tuple[str, ...]:
        return tuple(self._command_history)

    @property
    def active_local_port(self) -> int | None:
        return transport_state.active_local_port(self._transport_runtime)

    def on_log_entry(self, callback: log_state.LogEntryCallback) -> None: self._log_callbacks.append(callback)

    def on_error(self, callback: log_state.ErrorCallback) -> None: self._error_callbacks.append(callback)

    def on_measurement_batch(self, callback: MeasurementCallback) -> None: self._measurement_callbacks.append(callback)

    def available_protocols(self) -> tuple[str, ...]:
        return protocol_state.available_protocols(self._protocol_runtime)

    def available_serial_ports(self) -> tuple[str, ...]:
        return transport_state.available_serial_ports(self._transport_runtime)

    def available_transport_modes(self) -> tuple[str, ...]:
        return transport_state.available_transport_modes(self._transport_runtime)

    def set_protocol(self, name: str) -> None:
        self._protocol_runtime = protocol_state.set_protocol(
            self._protocol_runtime,
            name,
            entries=self._entries,
            log_callbacks=self._log_callbacks,
        )

    def connect_fake(self) -> bool: return self.connect_fake_result().ok

    def connect_fake_result(self) -> OperationResult[SerialPortConfig]:
        self._transport_runtime, result = transport_state.connect_fake_result(
            self._transport_runtime,
            entries=self._entries,
            log_callbacks=self._log_callbacks,
        )
        return result

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
        self._transport_runtime, result = transport_state.connect_serial_result(
            self._transport_runtime,
            port_name,
            baud_rate,
            entries=self._entries,
            log_callbacks=self._log_callbacks,
            data_bits=data_bits,
            parity=parity,
            stop_bits=stop_bits,
            flow_control=flow_control,
        )
        return result

    def connect_tcp(self, host: str, port: int) -> bool: return self.connect_tcp_result(host, port).ok

    def connect_tcp_result(self, host: str, port: int) -> OperationResult[SerialPortConfig]:
        return self._connect_endpoint_result("tcp", host, port)

    def connect_udp(self, host: str, port: int) -> bool: return self.connect_udp_result(host, port).ok

    def connect_udp_result(self, host: str, port: int) -> OperationResult[SerialPortConfig]:
        return self._connect_endpoint_result("udp", host, port)

    def disconnect(self) -> None:
        transport_state.disconnect(self._transport_runtime, self._entries, self._log_callbacks)

    def send_text(self, text: str) -> bool: return self.send_text_result(text).ok

    def send_text_result(self, text: str) -> OperationResult[SerialWorkbenchLogEntry]:
        return io_state.send_text_result(
            self._transport_runtime.transport,
            self._protocol_runtime.dispatcher,
            text,
            self._command_history,
            self._entries,
            self._log_callbacks,
            self._handle_error,
        )

    def inject_received_text(self, text: str) -> OperationResult[None]:
        return io_state.inject_received_text_result(
            self._transport_runtime.transport,
            text,
            self._entries,
            self._log_callbacks,
            self._handle_error,
        )

    def clear_log(self) -> None: session_state.clear_entries(self._entries)

    def export_log(self, path: str | Path) -> None: self.export_log_result(path)

    def export_log_result(self, path: str | Path) -> OperationResult[Path]:
        return session_state.export_entries_result(path, self._entries, self._protocol_event_from_entry)

    def replay_log(self, path: str | Path) -> None: self.replay_log_result(path)

    def replay_log_result(self, path: str | Path) -> OperationResult[list[SerialWorkbenchLogEntry]]:
        return session_state.replay_entries_into_state_result(
            path,
            self._entries,
            self._log_callbacks,
            entry_from_event,
        )

    def save_profile(self, path: str | Path, name: str) -> None: self.save_profile_result(path, name)

    def save_profile_result(self, path: str | Path, name: str) -> OperationResult[Path]:
        return profile_state.save_profile_state_result(
            path,
            mode=self._transport_runtime.mode,
            name=name,
            config=self._transport_runtime.transport.config,
            is_connected=self.is_connected,
            protocol=self._protocol_runtime.dispatcher.protocol_name,
            command_history=self.command_history,
        )

    def load_profile(self, path: str | Path) -> dict[str, Any]:
        result = self.load_profile_result(path)
        if result.failed:
            raise OSError(result.message)
        return result.value or {}

    def load_profile_result(self, path: str | Path) -> OperationResult[dict[str, Any]]:
        return profile_state.load_profile_state_result(
            path,
            self._command_history,
            self._entries,
            self._log_callbacks,
        )

    def _handle_bytes_received(self, data: bytes) -> None:
        self._protocol_runtime = protocol_state.handle_received_bytes(
            self._protocol_runtime,
            data=data,
            entries=self._entries,
            log_callbacks=self._log_callbacks,
            measurement_callbacks=self._measurement_callbacks,
        )

    def _protocol_event_from_entry(self, entry: SerialWorkbenchLogEntry) -> ProtocolEvent:
        return protocol_state.protocol_event_from_entry(self._protocol_runtime, entry)

    def _connect_endpoint_result(self, mode: str, host: str, port: int) -> OperationResult[SerialPortConfig]:
        self._transport_runtime, result = transport_state.connect_endpoint_result(
            self._transport_runtime,
            mode,
            host,
            port,
            entries=self._entries,
            log_callbacks=self._log_callbacks,
        )
        return result

    def _handle_error(self, message: str) -> None:
        receive_state.handle_error(
            message,
            entries=self._entries,
            log_callbacks=self._log_callbacks,
            error_callbacks=self._error_callbacks,
        )

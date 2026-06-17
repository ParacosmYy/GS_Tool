"""Controller for the Python Serial Station MVP workbench."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass
from pathlib import Path
from typing import Any

from embeddebug.serial_station.core import (
    ChannelBatch,
    ChannelRingBuffer,
    SerialDispatcher,
    batch_from_measurement_events,
)
from embeddebug.serial_station.drivers import (
    FakeSerialTransport,
    SerialPortConfig,
    SerialTransport,
    TransportRegistry,
)
from embeddebug.serial_station.protocols import ProtocolEvent, create_default_registry
from embeddebug.serial_station.services import (
    SerialLogService,
    SerialProfileService,
    SerialReplayService,
)


@dataclass(frozen=True)
class SerialWorkbenchLogEntry:
    """A UI-ready serial workbench log fact without QWidget dependencies."""

    direction: str
    text: str
    raw: bytes


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

    def set_protocol(self, name: str) -> None:
        self._dispatcher.set_protocol(self._registry.create(name))
        self._measurement_ring = None

    def connect_fake(self) -> bool:
        if not isinstance(self._transport, FakeSerialTransport):
            self._replace_transport(self._transport_registry.create("fake"))
        self._transport_mode = "fake"
        config = SerialPortConfig(port_name="FAKE_LOOPBACK", baud_rate=115200)
        return self._transport.open(config)

    def connect_serial(
        self,
        port_name: str,
        baud_rate: int,
        data_bits: int = 8,
        parity: str = "none",
        stop_bits: str = "1",
        flow_control: str = "none",
    ) -> bool:
        transport = self._transport_registry.create("serial")
        self._replace_transport(transport)
        self._transport_mode = "serial"
        return self._transport.open(
            SerialPortConfig(
                port_name=port_name,
                baud_rate=baud_rate,
                data_bits=data_bits,
                parity=parity,
                stop_bits=stop_bits,
                flow_control=flow_control,
            )
        )

    def disconnect(self) -> None:
        self._transport.close()

    def send_text(self, text: str) -> bool:
        if not self._transport.is_open:
            self._handle_error("transport_not_open")
            return False
        payload = self._dispatcher.build_command(text)
        written = self._transport.write(payload)
        if written != len(payload):
            return False
        self._remember_command(text)
        self._append_entry(SerialWorkbenchLogEntry(direction="tx", text=text, raw=payload))
        return True

    def inject_received_text(self, text: str) -> None:
        if not isinstance(self._transport, FakeSerialTransport):
            self._handle_error("fake_injection_requires_fake_transport")
            return
        self._transport.inject_rx(_decode_injected_text(text).encode("utf-8"))

    def clear_log(self) -> None:
        self._entries.clear()

    def export_log(self, path: str | Path) -> None:
        log_service = SerialLogService(path)
        for entry in self._entries:
            log_service.append(self._event_from_entry(entry))

    def replay_log(self, path: str | Path) -> None:
        replay_service = SerialReplayService()
        self._entries.clear()
        for event in replay_service.load_events(path):
            self._append_entry(self._entry_from_event(event))

    def save_profile(self, path: str | Path, name: str) -> None:
        profile = {
            "name": name,
            "transport": {
                "mode": self._transport_mode,
                "portName": self._transport.config.port_name if self._transport.config else "",
                "baudRate": self._transport.config.baud_rate if self._transport.config else 0,
                "dataBits": self._transport.config.data_bits if self._transport.config else 8,
                "parity": self._transport.config.parity if self._transport.config else "none",
                "stopBits": self._transport.config.stop_bits if self._transport.config else "1",
                "flowControl": self._transport.config.flow_control if self._transport.config else "none",
                "connected": self.is_connected,
            },
            "protocol": self._dispatcher.protocol_name,
            "commandHistory": list(self._command_history),
        }
        SerialProfileService().save(path, profile)

    def load_profile(self, path: str | Path) -> dict[str, Any]:
        profile = SerialProfileService().load(path)
        self._restore_command_history(profile.get("commandHistory", []))
        return profile

    def _handle_bytes_received(self, data: bytes) -> None:
        events = self._dispatcher.feed(data)
        for event in events:
            self._append_entry(self._entry_from_event(event))
        batch = batch_from_measurement_events(events)
        if batch is not None:
            self._append_measurements(batch)

    def _entry_from_event(self, event: ProtocolEvent) -> SerialWorkbenchLogEntry:
        text = str(event.payload.get("text", event.raw.decode("utf-8", errors="replace")))
        direction = "tx" if event.type == "tx" else "rx"
        return SerialWorkbenchLogEntry(direction=direction, text=text, raw=event.raw)

    def _event_from_entry(self, entry: SerialWorkbenchLogEntry) -> ProtocolEvent:
        event_type = "tx" if entry.direction == "tx" else "frame"
        return ProtocolEvent(
            type=event_type,
            protocol_name=self._dispatcher.protocol_name,
            payload={"text": entry.text, "direction": entry.direction},
            raw=entry.raw,
        )

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


def _decode_injected_text(text: str) -> str:
    return text.replace("\\r", "\r").replace("\\n", "\n").replace("\\t", "\t")

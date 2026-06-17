"""Send and injection helpers for the Serial Station controller."""

from __future__ import annotations

from collections.abc import Callable

from embeddebug.serial_station.controllers import controller_log_state as log_state
from embeddebug.serial_station.controllers.command_history_state import remember_command
from embeddebug.serial_station.controllers.log_entry import SerialWorkbenchLogEntry
from embeddebug.serial_station.controllers.text_decode import decode_injected_text
from embeddebug.serial_station.core import SerialDispatcher
from embeddebug.serial_station.drivers import FakeSerialTransport, SerialTransport
from embeddebug.shared import OperationResult


ErrorHandler = Callable[[str], None]


def send_text_result(
    transport: SerialTransport,
    dispatcher: SerialDispatcher,
    text: str,
    command_history: list[str],
    entries: list[SerialWorkbenchLogEntry],
    callbacks: list[log_state.LogEntryCallback],
    handle_error: ErrorHandler,
) -> OperationResult[SerialWorkbenchLogEntry]:
    if not transport.is_open:
        handle_error("transport_not_open")
        return OperationResult.failure("transport_not_open", "Open a transport before sending")
    payload = dispatcher.build_command(text)
    written = transport.write(payload)
    if written != len(payload):
        handle_error("transport_write_incomplete")
        return OperationResult.failure(
            "transport_write_incomplete",
            "Transport accepted fewer bytes than requested",
        )
    remember_command(command_history, text)
    entry = SerialWorkbenchLogEntry(direction="tx", text=text, raw=payload)
    log_state.append_log_entry(entries, callbacks, entry)
    return OperationResult.success(entry)


def inject_received_text_result(
    transport: SerialTransport,
    text: str,
    entries: list[SerialWorkbenchLogEntry],
    log_callbacks: list[log_state.LogEntryCallback],
    handle_error: ErrorHandler,
) -> OperationResult[None]:
    if not isinstance(transport, FakeSerialTransport):
        handle_error("fake_injection_requires_fake_transport")
        return OperationResult.failure(
            "fake_injection_requires_fake_transport",
            "Fake RX injection requires fake transport",
        )
    transport.inject_rx(decode_injected_text(text).encode("utf-8"))
    return OperationResult.success()

"""Log state helpers for the Serial Station controller."""

from __future__ import annotations

from collections.abc import Callable

from embeddebug.serial_station.controllers.log_entry import SerialWorkbenchLogEntry


LogEntryCallback = Callable[[SerialWorkbenchLogEntry], None]
ErrorCallback = Callable[[str], None]


def append_log_entry(
    entries: list[SerialWorkbenchLogEntry],
    callbacks: list[LogEntryCallback],
    entry: SerialWorkbenchLogEntry,
) -> None:
    entries.append(entry)
    for callback in list(callbacks):
        callback(entry)


def append_system_entry(
    entries: list[SerialWorkbenchLogEntry],
    callbacks: list[LogEntryCallback],
    text: str,
) -> None:
    append_log_entry(
        entries,
        callbacks,
        SerialWorkbenchLogEntry("system", text, text.encode("utf-8")),
    )


def append_error_entry(
    entries: list[SerialWorkbenchLogEntry],
    log_callbacks: list[LogEntryCallback],
    error_callbacks: list[ErrorCallback],
    message: str,
) -> None:
    append_log_entry(
        entries,
        log_callbacks,
        SerialWorkbenchLogEntry("error", message, message.encode("utf-8")),
    )
    for callback in list(error_callbacks):
        callback(message)

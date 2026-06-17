"""Session state helpers for the Serial Station controller."""

from __future__ import annotations

from pathlib import Path

from embeddebug.serial_station.controllers import controller_log_state as log_state
from embeddebug.serial_station.controllers.log_entry import SerialWorkbenchLogEntry
from embeddebug.serial_station.controllers.session_operations import (
    EntryFactory,
    EventFactory,
    export_log_result as export_session_log_result,
    replay_entries_result as replay_session_entries_result,
)
from embeddebug.shared import OperationResult


def clear_entries(entries: list[SerialWorkbenchLogEntry]) -> None:
    entries.clear()


def export_entries_result(
    path: str | Path,
    entries: list[SerialWorkbenchLogEntry],
    event_factory: EventFactory,
) -> OperationResult[Path]:
    return export_session_log_result(path, entries, event_factory)


def replay_entries_into_state_result(
    path: str | Path,
    entries: list[SerialWorkbenchLogEntry],
    callbacks: list[log_state.LogEntryCallback],
    entry_factory: EntryFactory,
) -> OperationResult[list[SerialWorkbenchLogEntry]]:
    result = replay_session_entries_result(path, entry_factory)
    if result.ok:
        entries.clear()
        for entry in result.value or []:
            log_state.append_log_entry(entries, callbacks, entry)
    return result

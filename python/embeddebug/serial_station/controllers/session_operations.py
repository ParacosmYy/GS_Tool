"""Service-backed session operations for the Serial Station controller."""

from __future__ import annotations

from collections.abc import Callable, Iterable
from pathlib import Path
from typing import Any

from embeddebug.serial_station.controllers.log_entry import SerialWorkbenchLogEntry
from embeddebug.serial_station.protocols import ProtocolEvent
from embeddebug.serial_station.services import (
    SerialLogService,
    SerialProfileService,
    SerialReplayService,
)
from embeddebug.shared import OperationResult


EventFactory = Callable[[SerialWorkbenchLogEntry], ProtocolEvent]
EntryFactory = Callable[[ProtocolEvent], SerialWorkbenchLogEntry]


def export_log_result(
    path: str | Path,
    entries: Iterable[SerialWorkbenchLogEntry],
    event_factory: EventFactory,
) -> OperationResult[Path]:
    events = (event_factory(entry) for entry in entries)
    return SerialLogService(path).append_many_result(events)


def replay_entries_result(
    path: str | Path,
    entry_factory: EntryFactory,
) -> OperationResult[list[SerialWorkbenchLogEntry]]:
    events = SerialReplayService().load_events_result(path)
    if events.failed:
        return OperationResult.failure(events.error_code, events.message)
    entries = [entry_factory(event) for event in events.value or []]
    return OperationResult.success(entries)


def save_profile_result(path: str | Path, profile: dict[str, Any]) -> OperationResult[Path]:
    return SerialProfileService().save_result(path, profile)


def load_profile_result(path: str | Path) -> OperationResult[dict[str, Any]]:
    return SerialProfileService().load_result(path)

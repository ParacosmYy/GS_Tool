"""Shared mutable state for the Serial Station controller facade."""

from __future__ import annotations

from dataclasses import dataclass, field

from embeddebug.serial_station.controllers.log_entry import SerialWorkbenchLogEntry


@dataclass
class WorkbenchState:
    """Own log entries and command history for controller orchestration."""

    entries: list[SerialWorkbenchLogEntry] = field(default_factory=list)
    command_history: list[str] = field(default_factory=list)


def create_workbench_state() -> WorkbenchState:
    return WorkbenchState()


def entries_snapshot(state: WorkbenchState) -> tuple[SerialWorkbenchLogEntry, ...]:
    return tuple(state.entries)


def command_history_snapshot(state: WorkbenchState) -> tuple[str, ...]:
    return tuple(state.command_history)


def clear_entries(state: WorkbenchState) -> None:
    state.entries.clear()

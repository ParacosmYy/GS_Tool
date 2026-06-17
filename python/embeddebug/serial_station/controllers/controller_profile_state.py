"""Profile state helpers for the Serial Station controller."""

from __future__ import annotations

from pathlib import Path

from embeddebug.serial_station.controllers import controller_log_state as log_state
from embeddebug.serial_station.controllers.command_history_state import restore_command_history
from embeddebug.serial_station.controllers.log_entry import SerialWorkbenchLogEntry
from embeddebug.serial_station.controllers.profile_snapshot import build_profile_snapshot
from embeddebug.serial_station.controllers.session_operations import (
    load_profile_result as load_session_profile_result,
    save_profile_result as save_session_profile_result,
)
from embeddebug.serial_station.drivers import SerialPortConfig
from embeddebug.shared import OperationResult


def save_profile_state_result(
    path: str | Path,
    name: str,
    *,
    mode: str,
    config: SerialPortConfig | None,
    is_connected: bool,
    protocol: str,
    command_history: tuple[str, ...],
) -> OperationResult[Path]:
    profile = build_profile_snapshot(
        name=name,
        mode=mode,
        config=config,
        is_connected=is_connected,
        protocol=protocol,
        command_history=command_history,
    )
    return save_session_profile_result(path, profile)


def load_profile_state_result(
    path: str | Path,
    command_history: list[str],
    entries: list[SerialWorkbenchLogEntry],
    callbacks: list[log_state.LogEntryCallback],
) -> OperationResult[dict[str, object]]:
    result = load_session_profile_result(path)
    if result.ok and result.value is not None:
        restore_command_history(command_history, result.value.get("commandHistory", []))
        name = str(result.value.get("name", "unnamed"))
        log_state.append_system_entry(entries, callbacks, f"profile loaded: {name}")
    return result

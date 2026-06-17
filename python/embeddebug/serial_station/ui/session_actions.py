"""Session file actions for the Serial Station main window."""

from __future__ import annotations

from typing import Protocol

from embeddebug.serial_station.ui.log_view_content import clear_log_view
from embeddebug.serial_station.ui.profile_combo_options import select_profile_combo_value
from embeddebug.serial_station.ui.profile_name_text import apply_profile_name_text
from embeddebug.serial_station.ui.serial_profile_controls import apply_serial_profile_controls
from embeddebug.serial_station.ui.status_messages import (
    set_profile_label,
    set_result_status,
    set_status_text,
)
from embeddebug.serial_station.ui.tcp_controls import apply_tcp_profile_controls
from embeddebug.serial_station.ui.udp_controls import apply_udp_profile_controls


class SessionActionHost(Protocol):
    """Minimal main-window surface needed by file action handlers."""

    def tr(self, text: str) -> str: ...

    def _render_log_entries(self) -> None: ...

    def _update_log_stats(self) -> None: ...

    def _refresh_command_history(self) -> None: ...

    def _set_connected_controls(self, connected: bool) -> None: ...


def clear_log(host: SessionActionHost) -> None:
    host._controller.clear_log()
    clear_log_view(host._log_view)
    host._update_log_stats()
    set_status_text(host, "Log cleared")


def export_log(host: SessionActionHost) -> None:
    path = host._log_path_edit.text()
    if not path:
        set_status_text(host, "Log path is empty")
        return
    result = host._controller.export_log_result(path)
    if result.failed:
        set_result_status(host, result, success_text="", failure_prefix="Export failed")
        return
    set_result_status(host, result, success_text="Saved log", failure_prefix="Export failed")


def replay_log(host: SessionActionHost) -> None:
    path = host._log_path_edit.text()
    if not path:
        set_status_text(host, "Log path is empty")
        return
    result = host._controller.replay_log_result(path)
    if result.failed:
        set_result_status(host, result, success_text="", failure_prefix="Replay failed")
        return
    clear_log_view(host._log_view)
    host._render_log_entries()
    set_result_status(host, result, success_text="Replayed log", failure_prefix="Replay failed")


def save_profile(host: SessionActionHost) -> None:
    path = host._profile_path_edit.text()
    name = host._profile_name_edit.text()
    if not path:
        set_status_text(host, "Profile path is empty")
        return
    if not name:
        set_status_text(host, "Profile name is empty")
        return
    result = host._controller.save_profile_result(path, name)
    if result.failed:
        set_result_status(host, result, success_text="", failure_prefix="Save profile failed")
        return
    set_profile_label(host, name)
    set_result_status(host, result, success_text="Saved profile", failure_prefix="Save profile failed")


def load_profile(host: SessionActionHost) -> None:
    path = host._profile_path_edit.text()
    if not path:
        set_status_text(host, "Profile path is empty")
        return
    result = host._controller.load_profile_result(path)
    if result.failed or result.value is None:
        set_result_status(host, result, success_text="", failure_prefix="Load profile failed")
        return
    profile = result.value
    name = str(profile.get("name", "unnamed"))
    apply_profile_name_text(host._profile_name_edit, name)
    apply_profile_controls(host, profile)
    host._refresh_command_history()
    set_profile_label(host, name)
    set_result_status(host, result, success_text="Loaded profile", failure_prefix="Load profile failed")


def apply_profile_controls(host: SessionActionHost, profile: dict[str, object]) -> None:
    protocol = str(profile.get("protocol", ""))
    if protocol:
        select_profile_combo_value(host._protocol_combo, protocol)

    transport = profile.get("transport", {})
    if not isinstance(transport, dict):
        return
    port_name = str(transport.get("portName", ""))
    if port_name:
        select_profile_combo_value(host._port_combo, port_name)
    apply_tcp_profile_controls(host, transport, port_name)
    apply_udp_profile_controls(host, transport, port_name)
    apply_serial_profile_controls(host, transport)
    host._set_connected_controls(host._controller.is_connected)

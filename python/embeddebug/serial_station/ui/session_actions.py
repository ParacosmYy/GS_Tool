"""Session file actions for the Serial Station main window."""

from __future__ import annotations

from typing import Protocol

from PyQt6.QtWidgets import QComboBox

from embeddebug.serial_station.ui.tcp_controls import apply_tcp_profile_controls


class SessionActionHost(Protocol):
    """Minimal main-window surface needed by file action handlers."""

    def tr(self, text: str) -> str: ...

    def _render_log_entries(self) -> None: ...

    def _update_log_stats(self) -> None: ...

    def _refresh_command_history(self) -> None: ...

    def _set_connected_controls(self, connected: bool) -> None: ...


def clear_log(host: SessionActionHost) -> None:
    host._controller.clear_log()
    host._log_view.clear()
    host._update_log_stats()
    host._status_label.setText(host.tr("Log cleared"))


def export_log(host: SessionActionHost) -> None:
    path = host._log_path_edit.text()
    if not path:
        host._status_label.setText(host.tr("Log path is empty"))
        return
    result = host._controller.export_log_result(path)
    if result.failed:
        host._status_label.setText(host.tr("Export failed: {message}").format(message=result.message))
        return
    host._status_label.setText(host.tr("Saved log"))


def replay_log(host: SessionActionHost) -> None:
    path = host._log_path_edit.text()
    if not path:
        host._status_label.setText(host.tr("Log path is empty"))
        return
    result = host._controller.replay_log_result(path)
    if result.failed:
        host._status_label.setText(host.tr("Replay failed: {message}").format(message=result.message))
        return
    host._log_view.clear()
    host._render_log_entries()
    host._status_label.setText(host.tr("Replayed log"))


def save_profile(host: SessionActionHost) -> None:
    path = host._profile_path_edit.text()
    name = host._profile_name_edit.text()
    if not path:
        host._status_label.setText(host.tr("Profile path is empty"))
        return
    if not name:
        host._status_label.setText(host.tr("Profile name is empty"))
        return
    result = host._controller.save_profile_result(path, name)
    if result.failed:
        host._status_label.setText(host.tr("Save profile failed: {message}").format(message=result.message))
        return
    host._profile_label.setText(host.tr("Profile: {name}").format(name=name))
    host._status_label.setText(host.tr("Saved profile"))


def load_profile(host: SessionActionHost) -> None:
    path = host._profile_path_edit.text()
    if not path:
        host._status_label.setText(host.tr("Profile path is empty"))
        return
    result = host._controller.load_profile_result(path)
    if result.failed or result.value is None:
        host._status_label.setText(host.tr("Load profile failed: {message}").format(message=result.message))
        return
    profile = result.value
    name = str(profile.get("name", "unnamed"))
    host._profile_name_edit.setText(name)
    apply_profile_controls(host, profile)
    host._refresh_command_history()
    host._profile_label.setText(host.tr("Profile: {name}").format(name=name))
    host._status_label.setText(host.tr("Loaded profile"))


def apply_profile_controls(host: SessionActionHost, profile: dict[str, object]) -> None:
    protocol = str(profile.get("protocol", ""))
    if protocol:
        _select_combo_value(host._protocol_combo, protocol)

    transport = profile.get("transport", {})
    if not isinstance(transport, dict):
        return
    port_name = str(transport.get("portName", ""))
    if port_name:
        _select_combo_value(host._port_combo, port_name)
    apply_tcp_profile_controls(host, transport, port_name)
    baud_rate = transport.get("baudRate")
    if baud_rate is not None:
        _select_combo_value(host._baud_combo, str(baud_rate))
    data_bits = transport.get("dataBits")
    if data_bits is not None:
        _select_combo_value(host._data_bits_combo, str(data_bits))
    parity = str(transport.get("parity", ""))
    if parity:
        _select_combo_value(host._parity_combo, parity.capitalize())
    stop_bits = transport.get("stopBits")
    if stop_bits is not None:
        _select_combo_value(host._stop_bits_combo, str(stop_bits))
    flow_control = str(transport.get("flowControl", ""))
    if flow_control:
        _select_combo_value(host._flow_control_combo, flow_control.capitalize())
    host._set_connected_controls(host._controller.is_connected)


def _select_combo_value(combo: QComboBox, value: str) -> None:
    if combo.findText(value) < 0:
        combo.addItem(value)
    combo.setCurrentText(value)

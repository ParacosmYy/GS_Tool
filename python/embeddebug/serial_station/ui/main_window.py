"""Top-level Serial Station window for the Python/PyQt migration lane."""

from __future__ import annotations

from PyQt6.QtWidgets import QMainWindow

from embeddebug.serial_station.controllers import (
    SerialWorkbenchController,
    SerialWorkbenchLogEntry,
)
from embeddebug.serial_station.core import ChannelBatch
from embeddebug.serial_station.ui import (
    connection_actions,
    injection_actions,
    lifecycle_actions,
    log_actions,
    measurement_actions,
    protocol_actions,
    session_actions,
    shortcuts,
    status_actions,
)
from embeddebug.serial_station.ui.sections import build_main_layout


class SerialStationMainWindow(QMainWindow):
    """PyQt Serial Station MVP window."""

    def __init__(self) -> None:
        super().__init__()
        self._controller = SerialWorkbenchController()
        self._controller.on_log_entry(self._append_log_entry)
        self._controller.on_error(self._show_error)
        self._controller.on_measurement_batch(self._append_measurement_batch)

        self.setObjectName("embeddebugPySerialStationWindow")
        self.setWindowTitle(self.tr("EmbedDebug PyQt"))
        self.resize(960, 640)

        self.setCentralWidget(build_main_layout(self, self._controller))

    def keyPressEvent(self, event: object) -> None:
        if shortcuts.handle_key_press(self, event):
            return
        super().keyPressEvent(event)

    def _set_protocol(self, name: str) -> None:
        protocol_actions.select_protocol(self, name)

    def _refresh_serial_ports(self) -> None:
        connection_actions.refresh_serial_ports(self)

    def _connect_fake(self) -> None:
        connection_actions.connect_fake(self)

    def _connect_serial(self) -> None:
        connection_actions.connect_serial(self)

    def _connect_tcp(self) -> None:
        connection_actions.connect_tcp(self)

    def _disconnect(self) -> None:
        connection_actions.disconnect(self)

    def _set_connected_controls(self, connected: bool) -> None:
        connection_actions.set_connected_controls(self, connected)

    def _has_serial_ports(self) -> bool:
        return connection_actions.has_serial_ports(self)

    def _send_text(self) -> None:
        connection_actions.send_text(self)

    def _refresh_command_history(self) -> None:
        connection_actions.refresh_command_history(self)

    def _select_command_history(self, text: str) -> None:
        connection_actions.select_command_history(self, text)

    def _inject_received(self) -> None:
        injection_actions.inject_received(self)

    def _append_log_entry(self, entry: SerialWorkbenchLogEntry) -> None:
        log_actions.append_log_entry(self, entry)

    def _append_log_line(self, entry: SerialWorkbenchLogEntry) -> None:
        log_actions.append_log_line(self, entry)

    def _render_log_entries(self) -> None:
        log_actions.render_log_entries(self)

    def _log_entry_visible(self, entry: SerialWorkbenchLogEntry) -> bool:
        return log_actions.log_entry_visible(self, entry)

    def _update_log_stats(self) -> None:
        log_actions.update_log_stats(self)

    def _append_measurement_batch(self, batch: ChannelBatch) -> None:
        measurement_actions.append_measurement_batch(self, batch)

    def _clear_log(self) -> None:
        session_actions.clear_log(self)

    def _export_log(self) -> None:
        session_actions.export_log(self)

    def _replay_log(self) -> None:
        session_actions.replay_log(self)

    def _save_profile(self) -> None:
        session_actions.save_profile(self)

    def _load_profile(self) -> None:
        session_actions.load_profile(self)

    def _apply_profile_controls(self, profile: dict[str, object]) -> None:
        session_actions.apply_profile_controls(self, profile)

    def _show_error(self, message: str) -> None:
        status_actions.show_error(self, message)

    def closeEvent(self, event: object) -> None:
        lifecycle_actions.close_window(self)
        super().closeEvent(event)

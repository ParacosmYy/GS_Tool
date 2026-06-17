"""Top-level Serial Station window for the Python/PyQt migration lane."""

from __future__ import annotations

from PyQt6.QtCore import Qt
from PyQt6.QtWidgets import QComboBox, QMainWindow

from embeddebug.serial_station.controllers import (
    SerialWorkbenchController,
    SerialWorkbenchLogEntry,
)
from embeddebug.serial_station.core import ChannelBatch
from embeddebug.serial_station.ui import connection_actions, injection_actions, session_actions
from embeddebug.serial_station.ui.sections import build_main_layout
from embeddebug.serial_station.ui.tcp_controls import (
    apply_tcp_profile_controls,
)


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
        modifiers = event.modifiers()
        key = event.key()
        if modifiers & Qt.KeyboardModifier.ControlModifier and key in (
            Qt.Key.Key_Return,
            Qt.Key.Key_Enter,
        ):
            self._send_text()
            event.accept()
            return
        if modifiers & Qt.KeyboardModifier.ControlModifier and key == Qt.Key.Key_L:
            self._clear_log()
            event.accept()
            return
        if modifiers & Qt.KeyboardModifier.ControlModifier and key == Qt.Key.Key_R:
            self._refresh_serial_ports()
            event.accept()
            return
        super().keyPressEvent(event)

    def _set_protocol(self, name: str) -> None:
        self._controller.set_protocol(name)
        self._status_label.setText(self.tr("Protocol: {name}").format(name=name))

    def _refresh_serial_ports(self) -> None:
        self._refresh_port_combo()
        self._set_connected_controls(self._controller.is_connected)
        self._status_label.setText(self.tr("Serial ports refreshed"))

    def _refresh_port_combo(self) -> None:
        current = self._port_combo.currentText()
        self._port_combo.clear()
        ports = self._controller.available_serial_ports()
        if ports:
            self._port_combo.addItems(ports)
            if current in ports:
                self._port_combo.setCurrentText(current)
            return
        self._port_combo.addItem(self.tr("No serial ports"))

    def _connect_fake(self) -> None:
        connection_actions.connect_fake(self)

    def _connect_serial(self) -> None:
        connection_actions.connect_serial(self)

    def _connect_tcp(self) -> None:
        connection_actions.connect_tcp(self)

    def _disconnect(self) -> None:
        self._controller.disconnect()
        self._status_label.setText(self.tr("Disconnected"))
        self._set_connected_controls(False)

    def _set_connected_controls(self, connected: bool) -> None:
        self._connect_button.setEnabled(not connected)
        self._connect_serial_button.setEnabled(not connected and self._has_serial_ports())
        self._connect_tcp_button.setEnabled(not connected)
        self._disconnect_button.setEnabled(connected)

    def _has_serial_ports(self) -> bool:
        return self._port_combo.count() > 0 and self._port_combo.currentText() != self.tr("No serial ports")

    def _send_text(self) -> None:
        connection_actions.send_text(self)

    def _refresh_command_history(self) -> None:
        history = self._controller.command_history
        self._command_history_combo.blockSignals(True)
        self._command_history_combo.clear()
        self._command_history_combo.addItems(history)
        if history:
            self._command_history_combo.setCurrentText(history[-1])
        self._command_history_combo.setEnabled(bool(history))
        self._command_history_combo.blockSignals(False)

    def _select_command_history(self, text: str) -> None:
        if text:
            self._send_edit.setText(text)

    def _inject_received(self) -> None:
        injection_actions.inject_received(self)

    def _append_log_entry(self, entry: SerialWorkbenchLogEntry) -> None:
        if not self._log_entry_visible(entry):
            self._update_log_stats()
            return
        self._append_log_line(entry)
        self._update_log_stats()

    def _append_log_line(self, entry: SerialWorkbenchLogEntry) -> None:
        if entry.direction == "tx":
            line = self.tr("TX {text}").format(text=entry.text)
        else:
            line = self.tr("RX {text}").format(text=entry.text)
        self._log_view.appendPlainText(line)

    def _render_log_entries(self) -> None:
        self._log_view.clear()
        for entry in self._controller.entries:
            if self._log_entry_visible(entry):
                self._append_log_line(entry)
        self._update_log_stats()

    def _log_entry_visible(self, entry: SerialWorkbenchLogEntry) -> bool:
        selected = self._log_filter_combo.currentText()
        if selected == self.tr("TX"):
            direction_matches = entry.direction == "tx"
        elif selected == self.tr("RX"):
            direction_matches = entry.direction == "rx"
        else:
            direction_matches = True
        if not direction_matches:
            return False
        search_text = self._log_search_edit.text().strip().lower()
        if not search_text:
            return True
        prefix = "tx" if entry.direction == "tx" else "rx"
        return search_text in f"{prefix} {entry.text}".lower()

    def _update_log_stats(self) -> None:
        entries = self._controller.entries
        total = len(entries)
        tx_count = sum(1 for entry in entries if entry.direction == "tx")
        rx_count = sum(1 for entry in entries if entry.direction == "rx")
        visible = sum(1 for entry in entries if self._log_entry_visible(entry))
        self._log_stats_label.setText(
            self.tr("Visible {visible} / Total {total} | TX {tx} | RX {rx}").format(
                visible=visible,
                total=total,
                tx=tx_count,
                rx=rx_count,
            )
        )

    def _append_measurement_batch(self, batch: ChannelBatch) -> None:
        self._waveform_preview.update_batch(batch)

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
        protocol = str(profile.get("protocol", ""))
        if protocol:
            self._select_combo_value(self._protocol_combo, protocol)

        transport = profile.get("transport", {})
        if not isinstance(transport, dict):
            return
        port_name = str(transport.get("portName", ""))
        if port_name:
            self._select_combo_value(self._port_combo, port_name)
        apply_tcp_profile_controls(self, transport, port_name)
        baud_rate = transport.get("baudRate")
        if baud_rate is not None:
            self._select_combo_value(self._baud_combo, str(baud_rate))
        data_bits = transport.get("dataBits")
        if data_bits is not None:
            self._select_combo_value(self._data_bits_combo, str(data_bits))
        parity = str(transport.get("parity", ""))
        if parity:
            self._select_combo_value(self._parity_combo, parity.capitalize())
        stop_bits = transport.get("stopBits")
        if stop_bits is not None:
            self._select_combo_value(self._stop_bits_combo, str(stop_bits))
        flow_control = str(transport.get("flowControl", ""))
        if flow_control:
            self._select_combo_value(self._flow_control_combo, flow_control.capitalize())
        self._set_connected_controls(self._controller.is_connected)

    @staticmethod
    def _select_combo_value(combo: QComboBox, value: str) -> None:
        if combo.findText(value) < 0:
            combo.addItem(value)
        combo.setCurrentText(value)

    def _show_error(self, message: str) -> None:
        self._status_label.setText(self.tr("Error: {message}").format(message=message))

    def closeEvent(self, event: object) -> None:
        self._waveform_preview.shutdown()
        super().closeEvent(event)

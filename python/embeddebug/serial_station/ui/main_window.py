"""Top-level Serial Station window for the Python/PyQt migration lane."""

from __future__ import annotations

from PyQt6.QtCore import Qt
from PyQt6.QtGui import QKeySequence, QShortcut
from PyQt6.QtWidgets import (
    QComboBox,
    QHBoxLayout,
    QLabel,
    QLineEdit,
    QMainWindow,
    QPlainTextEdit,
    QPushButton,
    QVBoxLayout,
    QWidget,
)

from embeddebug.serial_station.controllers import (
    SerialWorkbenchController,
    SerialWorkbenchLogEntry,
)
from embeddebug.serial_station.core import ChannelBatch
from embeddebug.serial_station.ui.waveform_preview import SerialWaveformPreview


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

        root = QWidget(self)
        root.setObjectName("serialStationPyRoot")

        layout = QVBoxLayout(root)
        layout.setContentsMargins(16, 16, 16, 16)
        layout.setSpacing(12)

        title = QLabel(self.tr("Serial Station PyQt MVP"), root)
        title.setObjectName("serialStationPyTitle")
        title.setAlignment(Qt.AlignmentFlag.AlignLeft)

        self._status_label = QLabel(self.tr("Disconnected"), root)
        self._status_label.setObjectName("serialStationStatusLabel")
        self._status_label.setAlignment(Qt.AlignmentFlag.AlignLeft)
        self._profile_label = QLabel(self.tr("Profile: unsaved"), root)
        self._profile_label.setObjectName("serialStationProfileLabel")
        self._profile_label.setAlignment(Qt.AlignmentFlag.AlignLeft)

        toolbar = QHBoxLayout()
        toolbar.setSpacing(8)

        self._protocol_combo = QComboBox(root)
        self._protocol_combo.setObjectName("serialStationProtocolCombo")
        self._protocol_combo.setToolTip(self.tr("Select the active serial protocol"))
        self._protocol_combo.addItems(self._controller.available_protocols())
        self._protocol_combo.setCurrentText("raw_data")
        self._protocol_combo.currentTextChanged.connect(self._set_protocol)

        self._port_combo = QComboBox(root)
        self._port_combo.setObjectName("serialStationPortCombo")
        self._port_combo.setToolTip(self.tr("Select a serial port"))
        self._refresh_port_combo()

        self._refresh_ports_button = QPushButton(self.tr("Refresh Ports"), root)
        self._refresh_ports_button.setObjectName("serialStationRefreshPortsButton")
        self._refresh_ports_button.setToolTip(self.tr("Refresh available serial ports (Ctrl+R)"))
        self._refresh_ports_button.clicked.connect(self._refresh_serial_ports)

        self._baud_combo = QComboBox(root)
        self._baud_combo.setObjectName("serialStationBaudCombo")
        self._baud_combo.setToolTip(self.tr("Select baud rate"))
        self._baud_combo.addItems(["9600", "19200", "38400", "57600", "115200", "921600"])
        self._baud_combo.setCurrentText("115200")

        self._data_bits_combo = QComboBox(root)
        self._data_bits_combo.setObjectName("serialStationDataBitsCombo")
        self._data_bits_combo.setToolTip(self.tr("Select data bits"))
        self._data_bits_combo.addItems(["5", "6", "7", "8"])
        self._data_bits_combo.setCurrentText("8")

        self._parity_combo = QComboBox(root)
        self._parity_combo.setObjectName("serialStationParityCombo")
        self._parity_combo.setToolTip(self.tr("Select parity"))
        self._parity_combo.addItems(["None", "Even", "Odd", "Space", "Mark"])
        self._parity_combo.setCurrentText("None")

        self._stop_bits_combo = QComboBox(root)
        self._stop_bits_combo.setObjectName("serialStationStopBitsCombo")
        self._stop_bits_combo.setToolTip(self.tr("Select stop bits"))
        self._stop_bits_combo.addItems(["1", "1.5", "2"])
        self._stop_bits_combo.setCurrentText("1")

        self._flow_control_combo = QComboBox(root)
        self._flow_control_combo.setObjectName("serialStationFlowControlCombo")
        self._flow_control_combo.setToolTip(self.tr("Select flow control"))
        self._flow_control_combo.addItems(["None", "Hardware", "Software"])
        self._flow_control_combo.setCurrentText("None")

        self._connect_button = QPushButton(self.tr("Connect Fake"), root)
        self._connect_button.setObjectName("serialStationConnectButton")
        self._connect_button.setToolTip(self.tr("Open the fake loopback transport"))
        self._connect_button.clicked.connect(self._connect_fake)

        self._connect_serial_button = QPushButton(self.tr("Connect Serial"), root)
        self._connect_serial_button.setObjectName("serialStationConnectSerialButton")
        self._connect_serial_button.setToolTip(self.tr("Open the selected serial port"))
        self._connect_serial_button.setEnabled(self._has_serial_ports())
        self._connect_serial_button.clicked.connect(self._connect_serial)

        self._disconnect_button = QPushButton(self.tr("Disconnect"), root)
        self._disconnect_button.setObjectName("serialStationDisconnectButton")
        self._disconnect_button.setToolTip(self.tr("Close the active transport"))
        self._disconnect_button.setEnabled(False)
        self._disconnect_button.clicked.connect(self._disconnect)

        toolbar.addWidget(self._protocol_combo)
        toolbar.addWidget(self._port_combo)
        toolbar.addWidget(self._refresh_ports_button)
        toolbar.addWidget(self._baud_combo)
        toolbar.addWidget(self._data_bits_combo)
        toolbar.addWidget(self._parity_combo)
        toolbar.addWidget(self._stop_bits_combo)
        toolbar.addWidget(self._flow_control_combo)
        toolbar.addWidget(self._connect_button)
        toolbar.addWidget(self._connect_serial_button)
        toolbar.addWidget(self._disconnect_button)
        toolbar.addStretch(1)
        toolbar.addWidget(self._profile_label)
        toolbar.addWidget(self._status_label)

        send_row = QHBoxLayout()
        send_row.setSpacing(8)
        self._send_edit = QLineEdit(root)
        self._send_edit.setObjectName("serialStationSendEdit")
        self._send_edit.setPlaceholderText(self.tr("Command text"))
        self._send_edit.setToolTip(self.tr("Command text (Ctrl+Enter to send)"))
        self._send_edit.returnPressed.connect(self._send_text)
        self._command_history_combo = QComboBox(root)
        self._command_history_combo.setObjectName("serialStationCommandHistoryCombo")
        self._command_history_combo.setToolTip(self.tr("Select a previously sent command"))
        self._command_history_combo.setEnabled(False)
        self._command_history_combo.currentTextChanged.connect(self._select_command_history)
        self._send_button = QPushButton(self.tr("Send"), root)
        self._send_button.setObjectName("serialStationSendButton")
        self._send_button.setToolTip(self.tr("Send command text (Ctrl+Enter)"))
        self._send_button.clicked.connect(self._send_text)
        send_row.addWidget(self._send_edit, 1)
        send_row.addWidget(self._command_history_combo)
        send_row.addWidget(self._send_button)

        inject_row = QHBoxLayout()
        inject_row.setSpacing(8)
        self._inject_edit = QLineEdit(root)
        self._inject_edit.setObjectName("serialStationInjectEdit")
        self._inject_edit.setPlaceholderText(self.tr("Fake received text"))
        self._inject_edit.returnPressed.connect(self._inject_received)
        self._inject_button = QPushButton(self.tr("Inject RX"), root)
        self._inject_button.setObjectName("serialStationInjectButton")
        self._inject_button.setToolTip(self.tr("Inject fake received bytes"))
        self._inject_button.clicked.connect(self._inject_received)
        inject_row.addWidget(self._inject_edit, 1)
        inject_row.addWidget(self._inject_button)

        self._log_view = QPlainTextEdit(root)
        self._log_view.setObjectName("serialStationLogView")
        self._log_view.setPlaceholderText(self.tr("No serial log entries"))
        self._log_view.setReadOnly(True)

        self._log_stats_label = QLabel(root)
        self._log_stats_label.setObjectName("serialStationLogStatsLabel")
        self._log_stats_label.setAlignment(Qt.AlignmentFlag.AlignLeft)
        self._update_log_stats()

        self._waveform_preview = SerialWaveformPreview(root)

        log_row = QHBoxLayout()
        log_row.setSpacing(8)
        self._log_filter_combo = QComboBox(root)
        self._log_filter_combo.setObjectName("serialStationLogFilterCombo")
        self._log_filter_combo.setToolTip(self.tr("Filter visible log entries"))
        self._log_filter_combo.addItems([self.tr("All"), self.tr("TX"), self.tr("RX")])
        self._log_filter_combo.setCurrentText(self.tr("All"))
        self._log_filter_combo.currentTextChanged.connect(self._render_log_entries)
        self._log_search_edit = QLineEdit(root)
        self._log_search_edit.setObjectName("serialStationLogSearchEdit")
        self._log_search_edit.setPlaceholderText(self.tr("Search log text"))
        self._log_search_edit.setToolTip(self.tr("Filter visible log entries by text"))
        self._log_search_edit.textChanged.connect(self._render_log_entries)
        self._log_path_edit = QLineEdit(root)
        self._log_path_edit.setObjectName("serialStationLogPathEdit")
        self._log_path_edit.setPlaceholderText(self.tr("Session log JSONL path"))
        self._export_log_button = QPushButton(self.tr("Save Log"), root)
        self._export_log_button.setObjectName("serialStationExportLogButton")
        self._export_log_button.setToolTip(self.tr("Save current session log"))
        self._export_log_button.clicked.connect(self._export_log)
        self._replay_log_button = QPushButton(self.tr("Replay Log"), root)
        self._replay_log_button.setObjectName("serialStationReplayLogButton")
        self._replay_log_button.setToolTip(self.tr("Replay saved session log"))
        self._replay_log_button.clicked.connect(self._replay_log)
        log_row.addWidget(self._log_filter_combo)
        log_row.addWidget(self._log_search_edit)
        log_row.addWidget(self._log_stats_label)
        log_row.addWidget(self._log_path_edit, 1)
        log_row.addWidget(self._export_log_button)
        log_row.addWidget(self._replay_log_button)

        profile_row = QHBoxLayout()
        profile_row.setSpacing(8)
        self._profile_path_edit = QLineEdit(root)
        self._profile_path_edit.setObjectName("serialStationProfilePathEdit")
        self._profile_path_edit.setPlaceholderText(self.tr("Profile JSON path"))
        self._profile_name_edit = QLineEdit(root)
        self._profile_name_edit.setObjectName("serialStationProfileNameEdit")
        self._profile_name_edit.setPlaceholderText(self.tr("Profile name"))
        self._save_profile_button = QPushButton(self.tr("Save Profile"), root)
        self._save_profile_button.setObjectName("serialStationSaveProfileButton")
        self._save_profile_button.setToolTip(self.tr("Save current profile"))
        self._save_profile_button.clicked.connect(self._save_profile)
        self._load_profile_button = QPushButton(self.tr("Load Profile"), root)
        self._load_profile_button.setObjectName("serialStationLoadProfileButton")
        self._load_profile_button.setToolTip(self.tr("Load saved profile"))
        self._load_profile_button.clicked.connect(self._load_profile)
        profile_row.addWidget(self._profile_path_edit, 1)
        profile_row.addWidget(self._profile_name_edit)
        profile_row.addWidget(self._save_profile_button)
        profile_row.addWidget(self._load_profile_button)

        footer = QHBoxLayout()
        footer.addStretch(1)
        self._clear_button = QPushButton(self.tr("Clear"), root)
        self._clear_button.setObjectName("serialStationClearButton")
        self._clear_button.setToolTip(self.tr("Clear the serial log (Ctrl+L)"))
        self._clear_button.clicked.connect(self._clear_log)
        footer.addWidget(self._clear_button)

        layout.addWidget(title)
        layout.addLayout(toolbar)
        layout.addLayout(send_row)
        layout.addLayout(inject_row)
        layout.addWidget(self._waveform_preview, 1)
        layout.addWidget(self._log_view, 1)
        layout.addLayout(log_row)
        layout.addLayout(profile_row)
        layout.addLayout(footer)

        self.setCentralWidget(root)
        self._install_shortcuts()

    def _install_shortcuts(self) -> None:
        send_shortcut = QShortcut(QKeySequence("Ctrl+Return"), self)
        send_shortcut.setObjectName("serialStationSendShortcut")
        send_shortcut.setContext(Qt.ShortcutContext.ApplicationShortcut)
        send_shortcut.activated.connect(self._send_text)

        clear_shortcut = QShortcut(QKeySequence("Ctrl+L"), self)
        clear_shortcut.setObjectName("serialStationClearShortcut")
        clear_shortcut.setContext(Qt.ShortcutContext.ApplicationShortcut)
        clear_shortcut.activated.connect(self._clear_log)

        refresh_shortcut = QShortcut(QKeySequence("Ctrl+R"), self)
        refresh_shortcut.setObjectName("serialStationRefreshPortsShortcut")
        refresh_shortcut.setContext(Qt.ShortcutContext.ApplicationShortcut)
        refresh_shortcut.activated.connect(self._refresh_serial_ports)
        self._shortcuts = [send_shortcut, clear_shortcut, refresh_shortcut]

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
        if self._controller.connect_fake():
            self._status_label.setText(self.tr("Connected to fake loopback"))
            self._set_connected_controls(True)
            return
        self._status_label.setText(self.tr("Connection failed"))

    def _connect_serial(self) -> None:
        port_name = self._port_combo.currentText()
        if not port_name or not self._has_serial_ports():
            self._status_label.setText(self.tr("Serial port is empty"))
            return
        baud_rate = int(self._baud_combo.currentText())
        if self._controller.connect_serial(
            port_name,
            baud_rate,
            data_bits=int(self._data_bits_combo.currentText()),
            parity=self._parity_combo.currentText().lower(),
            stop_bits=self._stop_bits_combo.currentText(),
            flow_control=self._flow_control_combo.currentText().lower(),
        ):
            self._status_label.setText(self.tr("Connected to {port}").format(port=port_name))
            self._set_connected_controls(True)
            return
        self._status_label.setText(self.tr("Connection failed"))

    def _disconnect(self) -> None:
        self._controller.disconnect()
        self._status_label.setText(self.tr("Disconnected"))
        self._set_connected_controls(False)

    def _set_connected_controls(self, connected: bool) -> None:
        self._connect_button.setEnabled(not connected)
        self._connect_serial_button.setEnabled(not connected and self._has_serial_ports())
        self._disconnect_button.setEnabled(connected)

    def _has_serial_ports(self) -> bool:
        return self._port_combo.count() > 0 and self._port_combo.currentText() != self.tr("No serial ports")

    def _send_text(self) -> None:
        text = self._send_edit.text()
        if not text:
            self._status_label.setText(self.tr("Command is empty"))
            return
        if self._controller.send_text(text):
            self._refresh_command_history()
            self._status_label.setText(self.tr("Command sent"))
            return
        self._status_label.setText(self.tr("Send failed"))

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
        text = self._inject_edit.text()
        if not text:
            self._status_label.setText(self.tr("RX text is empty"))
            return
        self._controller.inject_received_text(text)
        self._status_label.setText(self.tr("Received fake bytes"))

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
        self._controller.clear_log()
        self._log_view.clear()
        self._update_log_stats()
        self._status_label.setText(self.tr("Log cleared"))

    def _export_log(self) -> None:
        path = self._log_path_edit.text()
        if not path:
            self._status_label.setText(self.tr("Log path is empty"))
            return
        self._controller.export_log(path)
        self._status_label.setText(self.tr("Saved log"))

    def _replay_log(self) -> None:
        path = self._log_path_edit.text()
        if not path:
            self._status_label.setText(self.tr("Log path is empty"))
            return
        self._log_view.clear()
        self._controller.replay_log(path)
        self._render_log_entries()
        self._status_label.setText(self.tr("Replayed log"))

    def _save_profile(self) -> None:
        path = self._profile_path_edit.text()
        name = self._profile_name_edit.text()
        if not path:
            self._status_label.setText(self.tr("Profile path is empty"))
            return
        if not name:
            self._status_label.setText(self.tr("Profile name is empty"))
            return
        self._controller.save_profile(path, name)
        self._profile_label.setText(self.tr("Profile: {name}").format(name=name))
        self._status_label.setText(self.tr("Saved profile"))

    def _load_profile(self) -> None:
        path = self._profile_path_edit.text()
        if not path:
            self._status_label.setText(self.tr("Profile path is empty"))
            return
        profile = self._controller.load_profile(path)
        name = str(profile.get("name", "unnamed"))
        self._profile_name_edit.setText(name)
        self._apply_profile_controls(profile)
        self._refresh_command_history()
        self._profile_label.setText(self.tr("Profile: {name}").format(name=name))
        self._status_label.setText(self.tr("Loaded profile"))

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

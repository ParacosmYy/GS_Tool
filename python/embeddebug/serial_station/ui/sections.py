"""UI section builders for the Serial Station main window."""

from __future__ import annotations

from typing import Protocol

from PyQt6.QtCore import Qt
from PyQt6.QtWidgets import QComboBox, QHBoxLayout, QLabel, QLineEdit, QPlainTextEdit, QPushButton, QVBoxLayout, QWidget

from embeddebug.serial_station.controllers import SerialWorkbenchController
from embeddebug.serial_station.ui.connection_toolbar import build_connection_toolbar
from embeddebug.serial_station.ui.log_filter_options import (
    default_log_filter_text,
    log_filter_options,
)
from embeddebug.serial_station.ui.shortcuts import install_shortcuts
from embeddebug.serial_station.ui.waveform_preview import SerialWaveformPreview


class SerialStationSectionsHost(Protocol):
    def tr(self, source_text: str) -> str: ...
    def _set_protocol(self, name: str) -> None: ...
    def _refresh_serial_ports(self) -> None: ...
    def _has_serial_ports(self) -> bool: ...
    def _connect_fake(self) -> None: ...
    def _connect_serial(self) -> None: ...
    def _connect_tcp(self) -> None: ...
    def _connect_udp(self) -> None: ...
    def _disconnect(self) -> None: ...
    def _send_text(self) -> None: ...
    def _select_command_history(self, text: str) -> None: ...
    def _inject_received(self) -> None: ...
    def _render_log_entries(self) -> None: ...
    def _update_log_stats(self) -> None: ...
    def _export_log(self) -> None: ...
    def _replay_log(self) -> None: ...
    def _save_profile(self) -> None: ...
    def _load_profile(self) -> None: ...
    def _clear_log(self) -> None: ...


def build_main_layout(owner: SerialStationSectionsHost, controller: SerialWorkbenchController) -> QWidget:
    root = QWidget(owner)
    root.setObjectName("serialStationPyRoot")

    layout = QVBoxLayout(root)
    layout.setContentsMargins(16, 16, 16, 16)
    layout.setSpacing(12)

    title = QLabel(owner.tr("Serial Station PyQt MVP"), root)
    title.setObjectName("serialStationPyTitle")
    title.setAlignment(Qt.AlignmentFlag.AlignLeft)

    owner._status_label = QLabel(owner.tr("Disconnected"), root)
    owner._status_label.setObjectName("serialStationStatusLabel")
    owner._status_label.setAlignment(Qt.AlignmentFlag.AlignLeft)
    owner._profile_label = QLabel(owner.tr("Profile: unsaved"), root)
    owner._profile_label.setObjectName("serialStationProfileLabel")
    owner._profile_label.setAlignment(Qt.AlignmentFlag.AlignLeft)

    toolbar = build_connection_toolbar(owner, controller, root)

    send_row = build_send_row(owner, root)
    inject_row = build_inject_row(owner, root)

    owner._log_view = QPlainTextEdit(root)
    owner._log_view.setObjectName("serialStationLogView")
    owner._log_view.setPlaceholderText(owner.tr("No serial log entries"))
    owner._log_view.setReadOnly(True)

    owner._log_stats_label = QLabel(root)
    owner._log_stats_label.setObjectName("serialStationLogStatsLabel")
    owner._log_stats_label.setAlignment(Qt.AlignmentFlag.AlignLeft)
    owner._update_log_stats()

    owner._waveform_preview = SerialWaveformPreview(root)
    log_row = build_log_row(owner, root)
    profile_row = build_profile_row(owner, root)
    footer = build_footer(owner, root)

    layout.addWidget(title)
    layout.addLayout(toolbar)
    layout.addLayout(send_row)
    layout.addLayout(inject_row)
    layout.addWidget(owner._waveform_preview, 1)
    layout.addWidget(owner._log_view, 1)
    layout.addLayout(log_row)
    layout.addLayout(profile_row)
    layout.addLayout(footer)
    install_shortcuts(owner)
    return root


def build_send_row(owner: SerialStationSectionsHost, root: QWidget) -> QHBoxLayout:
    row = QHBoxLayout()
    row.setSpacing(8)
    owner._send_edit = QLineEdit(root)
    owner._send_edit.setObjectName("serialStationSendEdit")
    owner._send_edit.setPlaceholderText(owner.tr("Command text"))
    owner._send_edit.setToolTip(owner.tr("Command text (Ctrl+Enter to send)"))
    owner._send_edit.returnPressed.connect(owner._send_text)
    owner._command_history_combo = QComboBox(root)
    owner._command_history_combo.setObjectName("serialStationCommandHistoryCombo")
    owner._command_history_combo.setToolTip(owner.tr("Select a previously sent command"))
    owner._command_history_combo.setEnabled(False)
    owner._command_history_combo.currentTextChanged.connect(owner._select_command_history)
    owner._send_button = QPushButton(owner.tr("Send"), root)
    owner._send_button.setObjectName("serialStationSendButton")
    owner._send_button.setToolTip(owner.tr("Send command text (Ctrl+Enter)"))
    owner._send_button.clicked.connect(owner._send_text)
    row.addWidget(owner._send_edit, 1)
    row.addWidget(owner._command_history_combo)
    row.addWidget(owner._send_button)
    return row


def build_inject_row(owner: SerialStationSectionsHost, root: QWidget) -> QHBoxLayout:
    row = QHBoxLayout()
    row.setSpacing(8)
    owner._inject_edit = QLineEdit(root)
    owner._inject_edit.setObjectName("serialStationInjectEdit")
    owner._inject_edit.setPlaceholderText(owner.tr("Fake received text"))
    owner._inject_edit.returnPressed.connect(owner._inject_received)
    owner._inject_button = QPushButton(owner.tr("Inject RX"), root)
    owner._inject_button.setObjectName("serialStationInjectButton")
    owner._inject_button.setToolTip(owner.tr("Inject fake received bytes"))
    owner._inject_button.clicked.connect(owner._inject_received)
    row.addWidget(owner._inject_edit, 1)
    row.addWidget(owner._inject_button)
    return row


def build_log_row(owner: SerialStationSectionsHost, root: QWidget) -> QHBoxLayout:
    row = QHBoxLayout()
    row.setSpacing(8)
    owner._log_filter_combo = QComboBox(root)
    owner._log_filter_combo.setObjectName("serialStationLogFilterCombo")
    owner._log_filter_combo.setToolTip(owner.tr("Filter visible log entries"))
    owner._log_filter_combo.addItems([owner.tr(text) for text in log_filter_options()])
    owner._log_filter_combo.setCurrentText(owner.tr(default_log_filter_text()))
    owner._log_filter_combo.currentTextChanged.connect(owner._render_log_entries)
    owner._log_search_edit = QLineEdit(root)
    owner._log_search_edit.setObjectName("serialStationLogSearchEdit")
    owner._log_search_edit.setPlaceholderText(owner.tr("Search log text"))
    owner._log_search_edit.setToolTip(owner.tr("Filter visible log entries by text"))
    owner._log_search_edit.textChanged.connect(owner._render_log_entries)
    owner._log_path_edit = QLineEdit(root)
    owner._log_path_edit.setObjectName("serialStationLogPathEdit")
    owner._log_path_edit.setPlaceholderText(owner.tr("Session log JSONL path"))
    owner._export_log_button = QPushButton(owner.tr("Save Log"), root)
    owner._export_log_button.setObjectName("serialStationExportLogButton")
    owner._export_log_button.setToolTip(owner.tr("Save current session log"))
    owner._export_log_button.clicked.connect(owner._export_log)
    owner._replay_log_button = QPushButton(owner.tr("Replay Log"), root)
    owner._replay_log_button.setObjectName("serialStationReplayLogButton")
    owner._replay_log_button.setToolTip(owner.tr("Replay saved session log"))
    owner._replay_log_button.clicked.connect(owner._replay_log)
    row.addWidget(owner._log_filter_combo)
    row.addWidget(owner._log_search_edit)
    row.addWidget(owner._log_stats_label)
    row.addWidget(owner._log_path_edit, 1)
    row.addWidget(owner._export_log_button)
    row.addWidget(owner._replay_log_button)
    return row


def build_profile_row(owner: SerialStationSectionsHost, root: QWidget) -> QHBoxLayout:
    row = QHBoxLayout()
    row.setSpacing(8)
    owner._profile_path_edit = QLineEdit(root)
    owner._profile_path_edit.setObjectName("serialStationProfilePathEdit")
    owner._profile_path_edit.setPlaceholderText(owner.tr("Profile JSON path"))
    owner._profile_name_edit = QLineEdit(root)
    owner._profile_name_edit.setObjectName("serialStationProfileNameEdit")
    owner._profile_name_edit.setPlaceholderText(owner.tr("Profile name"))
    owner._save_profile_button = QPushButton(owner.tr("Save Profile"), root)
    owner._save_profile_button.setObjectName("serialStationSaveProfileButton")
    owner._save_profile_button.setToolTip(owner.tr("Save current profile"))
    owner._save_profile_button.clicked.connect(owner._save_profile)
    owner._load_profile_button = QPushButton(owner.tr("Load Profile"), root)
    owner._load_profile_button.setObjectName("serialStationLoadProfileButton")
    owner._load_profile_button.setToolTip(owner.tr("Load saved profile"))
    owner._load_profile_button.clicked.connect(owner._load_profile)
    row.addWidget(owner._profile_path_edit, 1)
    row.addWidget(owner._profile_name_edit)
    row.addWidget(owner._save_profile_button)
    row.addWidget(owner._load_profile_button)
    return row


def build_footer(owner: SerialStationSectionsHost, root: QWidget) -> QHBoxLayout:
    row = QHBoxLayout()
    row.addStretch(1)
    owner._clear_button = QPushButton(owner.tr("Clear"), root)
    owner._clear_button.setObjectName("serialStationClearButton")
    owner._clear_button.setToolTip(owner.tr("Clear the serial log (Ctrl+L)"))
    owner._clear_button.clicked.connect(owner._clear_log)
    row.addWidget(owner._clear_button)
    return row

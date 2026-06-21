"""Command input section builder for the Serial Station window."""

from __future__ import annotations

from typing import Protocol

from PyQt6.QtWidgets import QComboBox, QHBoxLayout, QLineEdit, QPushButton, QWidget


class CommandSectionHost(Protocol):
    def tr(self, source_text: str) -> str: ...
    def _send_text(self) -> None: ...
    def _select_command_history(self, text: str) -> None: ...


def build_send_row(owner: CommandSectionHost, root: QWidget) -> QHBoxLayout:
    row = QHBoxLayout()
    row.setSpacing(T.SPACING_INT_MD)
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
    # Batch 15: Send 按钮接入 scale 弹性反馈（按下陷下、松手回弹）。
    try:
        from embeddebug.serial_station.ui.micro_interactions import install_scale_press
from embeddebug.serial_station.ui.theme import tokens as T

        install_scale_press(owner._send_button)
    except Exception:
        pass
    row.addWidget(owner._send_edit, 1)
    row.addWidget(owner._command_history_combo)
    row.addWidget(owner._send_button)
    return row

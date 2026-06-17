"""Keyboard shortcuts for the Serial Station UI."""

from __future__ import annotations

from typing import Protocol

from PyQt6.QtCore import Qt
from PyQt6.QtGui import QKeySequence, QShortcut


class SerialStationShortcutHost(Protocol):
    def _send_text(self) -> None: ...
    def _clear_log(self) -> None: ...
    def _refresh_serial_ports(self) -> None: ...


def install_shortcuts(owner: SerialStationShortcutHost) -> None:
    send_shortcut = QShortcut(QKeySequence("Ctrl+Return"), owner)
    send_shortcut.setObjectName("serialStationSendShortcut")
    send_shortcut.setContext(Qt.ShortcutContext.ApplicationShortcut)
    send_shortcut.activated.connect(owner._send_text)

    clear_shortcut = QShortcut(QKeySequence("Ctrl+L"), owner)
    clear_shortcut.setObjectName("serialStationClearShortcut")
    clear_shortcut.setContext(Qt.ShortcutContext.ApplicationShortcut)
    clear_shortcut.activated.connect(owner._clear_log)

    refresh_shortcut = QShortcut(QKeySequence("Ctrl+R"), owner)
    refresh_shortcut.setObjectName("serialStationRefreshPortsShortcut")
    refresh_shortcut.setContext(Qt.ShortcutContext.ApplicationShortcut)
    refresh_shortcut.activated.connect(owner._refresh_serial_ports)
    owner._shortcuts = [send_shortcut, clear_shortcut, refresh_shortcut]

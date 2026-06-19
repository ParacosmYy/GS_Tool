"""Keyboard shortcuts for the Serial Station UI."""

from __future__ import annotations

from typing import Protocol

from PyQt6.QtCore import Qt
from PyQt6.QtGui import QKeySequence, QShortcut


class SerialStationShortcutHost(Protocol):
    def _send_text(self) -> None: ...
    def _clear_log(self) -> None: ...
    def _refresh_serial_ports(self) -> None: ...
    def _open_command_palette(self) -> None: ...
    def _toggle_theme(self) -> None: ...


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

    palette_shortcut = QShortcut(QKeySequence("Ctrl+P"), owner)
    palette_shortcut.setObjectName("serialStationCommandPaletteShortcut")
    palette_shortcut.setContext(Qt.ShortcutContext.ApplicationShortcut)
    palette_shortcut.activated.connect(owner._open_command_palette)
    owner._shortcuts = [send_shortcut, clear_shortcut, refresh_shortcut, palette_shortcut]


def handle_key_press(owner: SerialStationShortcutHost, event: object) -> bool:
    modifiers = event.modifiers()
    key = event.key()
    if not modifiers & Qt.KeyboardModifier.ControlModifier:
        return False
    if key in (Qt.Key.Key_Return, Qt.Key.Key_Enter):
        owner._send_text()
        event.accept()
        return True
    if key == Qt.Key.Key_L:
        owner._clear_log()
        event.accept()
        return True
    if key == Qt.Key.Key_R:
        owner._refresh_serial_ports()
        event.accept()
        return True
    if key == Qt.Key.Key_P:
        owner._open_command_palette()
        event.accept()
        return True
    # Ctrl+Shift+T: 切换深/浅主题（Batch 13，激活 definitions.toggle_theme）。
    # 需 Ctrl + Shift 同按；单独 Ctrl+T 不触发（避免与浏览器风格的「 reopen tab」误触）。
    if key == Qt.Key.Key_T and modifiers & Qt.KeyboardModifier.ShiftModifier:
        owner._toggle_theme()
        event.accept()
        return True
    return False

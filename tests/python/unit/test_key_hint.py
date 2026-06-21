"""KeyboardShortcut 徽章测试。"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from embeddebug.serial_station.ui.controls.key_hint import KeyboardShortcut


def test_key_hint_objectname(qtbot):
    hint = KeyboardShortcut("Ctrl+K")
    qtbot.addWidget(hint)
    assert hint.objectName() == "serialStationKeyHint"


def test_key_hint_text(qtbot):
    hint = KeyboardShortcut("Ctrl+Shift+P")
    qtbot.addWidget(hint)
    assert hint.text() == "Ctrl+Shift+P"


def test_key_hint_has_stylesheet(qtbot):
    hint = KeyboardShortcut("Esc")
    qtbot.addWidget(hint)
    assert hint.styleSheet() != ""
    assert "border-radius" in hint.styleSheet()


def test_key_hint_mouse_event_absorbed(qtbot):
    from PyQt6.QtCore import QEvent, QPointF, Qt
    from PyQt6.QtGui import QMouseEvent

    hint = KeyboardShortcut("Ctrl+S")
    qtbot.addWidget(hint)
    event = QMouseEvent(
        QEvent.Type.MouseButtonPress,
        QPointF(2, 2), QPointF(2, 2),
        Qt.MouseButton.LeftButton,
        Qt.MouseButton.LeftButton,
        Qt.KeyboardModifier.NoModifier,
    )
    hint.mousePressEvent(event)
    assert event.isAccepted()

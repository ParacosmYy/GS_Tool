"""B7 全屏与交互增强测试。"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtCore import QEvent, QPointF, Qt
from PyQt6.QtGui import QMouseEvent
from PyQt6.QtWidgets import QLabel, QWidget

from embeddebug.serial_station.ui.dashboard import (
    WidgetFullscreenHandler,
    attach_double_click_fullscreen,
)


def test_fullscreen_handler_starts_not_fullscreen(qtbot):
    handler = WidgetFullscreenHandler()
    assert handler.is_fullscreen is False


def test_fullscreen_enter_and_restore(qtbot):
    host = QWidget()
    host.resize(400, 300)
    qtbot.addWidget(host)
    child = QLabel("hi", host)
    child.setGeometry(10, 10, 80, 30)
    original_parent = child.parent()
    original_geometry = child.geometry()

    handler = WidgetFullscreenHandler()
    handler.enter(child, host)
    assert handler.is_fullscreen is True
    # 全屏后应覆盖 host。
    assert child.geometry().size() == host.rect().size()

    handler.restore()
    assert handler.is_fullscreen is False
    assert child.parent() is original_parent
    assert child.geometry() == original_geometry


def test_fullscreen_toggle(qtbot):
    host = QWidget()
    host.resize(400, 300)
    qtbot.addWidget(host)
    child = QLabel("x", host)
    child.setGeometry(5, 5, 50, 20)

    handler = WidgetFullscreenHandler()
    assert handler.toggle(child, host) is True
    assert handler.is_fullscreen is True
    assert handler.toggle(child, host) is False
    assert handler.is_fullscreen is False


def test_fullscreen_restore_without_enter_is_noop(qtbot):
    handler = WidgetFullscreenHandler()
    handler.restore()  # 不应崩溃
    assert handler.is_fullscreen is False


def test_attach_double_click_fullscreen(qtbot):
    host = QWidget()
    host.resize(400, 300)
    qtbot.addWidget(host)
    child = QLabel("fs", host)
    child.setGeometry(10, 10, 60, 30)
    handler = attach_double_click_fullscreen(child, host)
    assert isinstance(handler, WidgetFullscreenHandler)
    # 模拟双击事件触发全屏。
    event = QMouseEvent(
        QEvent.Type.MouseButtonDblClick,
        QPointF(child.rect().center()),
        Qt.MouseButton.LeftButton,
        Qt.MouseButton.LeftButton,
        Qt.KeyboardModifier.NoModifier,
    )
    child.mouseDoubleClickEvent(event)
    assert handler.is_fullscreen is True

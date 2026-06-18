"""IconManager 单元测试 — SVG 加载、着色、缓存与按钮装饰。"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtGui import QIcon
from PyQt6.QtWidgets import QPushButton, QWidget

from embeddebug.serial_station.ui import button_icons
from embeddebug.serial_station.ui.icons import IconManager, button_icon
from embeddebug.serial_station.ui.theme import palette as P


def test_icon_manager_is_singleton():
    a = IconManager()
    b = IconManager()
    assert a is b


def test_icon_manager_renders_existing_lucide_icon():
    manager = IconManager()
    manager.reset()
    icon = manager.icon("send")
    assert isinstance(icon, QIcon)
    assert not icon.isNull()


def test_icon_manager_caches_by_name_and_color():
    manager = IconManager()
    manager.reset()
    first = manager.icon("send", color=P.ACCENT)
    second = manager.icon("send", color=P.ACCENT)
    assert first is second
    different = manager.icon("send", color=P.ERROR)
    assert different is not first


def test_icon_manager_returns_empty_icon_for_missing_name():
    manager = IconManager()
    manager.reset()
    icon = manager.icon("does-not-exist-xyz")
    assert isinstance(icon, QIcon)
    assert icon.isNull()


def test_button_icon_convenience_returns_qicon():
    icon = button_icon("plug")
    assert isinstance(icon, QIcon)


def test_tint_replaces_current_color():
    svg = '<svg stroke="currentColor"><path/></svg>'
    tinted = IconManager._tint(svg, "#abcdef")
    assert "currentColor" not in tinted
    assert "#abcdef" in tinted


def test_apply_button_icons_decorates_known_buttons(qtbot):
    button = QPushButton("Connect")
    button.setObjectName("serialStationConnectButton")
    parent = QWidget()
    parent.setObjectName("embeddebugPySerialStationWindow")
    button.setParent(parent)
    qtbot.addWidget(parent)

    count = button_icons.apply_button_icons(parent)
    assert count >= 1
    assert not button.icon().isNull()


def test_apply_button_icons_skips_missing_buttons(qtbot):
    parent = QWidget()
    parent.setObjectName("embeddebugPySerialStationWindow")
    qtbot.addWidget(parent)

    count = button_icons.apply_button_icons(parent)
    assert count == 0

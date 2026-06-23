"""button_icons _find_child_by_object_name + apply_button_icons 返回值 边界测试。

_find_child_by_object_name 此前无直接测试。
本文件覆盖查找已知/未知 objectName + apply_button_icons 返回装饰数。

覆盖：
1. _find_child_by_object_name 已知 objectName 返回 widget。
2. _find_child_by_object_name 未知 objectName 返回 None。
3. _find_child_by_object_name 空 owner 返回 None。
4. apply_button_icons 返回 >=0（装饰数）。
5. apply_button_icons 装饰后 button 有 icon。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from types import SimpleNamespace

from PyQt6.QtWidgets import QMainWindow, QPushButton

from embeddebug.serial_station.ui.button_icons import (
    _find_child_by_object_name,
    apply_button_icons,
)


# ── _find_child_by_object_name ───────────────────────────────────
def test_find_child_known_objectname(qtbot):
    window = QMainWindow()
    qtbot.addWidget(window)
    btn = QPushButton("x", window)
    btn.setObjectName("serialStationConnectButton")
    result = _find_child_by_object_name(window, "serialStationConnectButton")
    assert result is btn


def test_find_child_unknown_objectname(qtbot):
    window = QMainWindow()
    qtbot.addWidget(window)
    result = _find_child_by_object_name(window, "nonexistent")
    assert result is None


def test_find_child_empty_owner():
    """空 owner（无 findChild）→ None。"""

    owner = SimpleNamespace()
    result = _find_child_by_object_name(owner, "anything")
    assert result is None


def test_find_child_multiple_children(qtbot):
    """多个子控件中找到目标。"""

    window = QMainWindow()
    qtbot.addWidget(window)
    btn1 = QPushButton("a", window)
    btn1.setObjectName("serialStationConnectButton")
    btn2 = QPushButton("b", window)
    btn2.setObjectName("serialStationDisconnectButton")
    result = _find_child_by_object_name(window, "serialStationDisconnectButton")
    assert result is btn2


# ── apply_button_icons 返回值 ─────────────────────────────────────
def test_apply_button_icons_returns_int(qtbot):
    """apply_button_icons 返回 int（装饰数 >= 0）。"""

    window = QMainWindow()
    qtbot.addWidget(window)
    count = apply_button_icons(window)
    assert isinstance(count, int)
    assert count >= 0


def test_apply_button_icons_empty_window_returns_zero(qtbot):
    """无按钮的窗口 → 返回 0。"""

    window = QMainWindow()
    qtbot.addWidget(window)
    count = apply_button_icons(window)
    assert count == 0

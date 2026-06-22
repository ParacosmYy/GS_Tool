"""sections build_inject_row owner 属性 + objectName 契约测试。

test_sections_boundary 仅 _no_crash；本文件覆盖 owner 属性 + objectName 契约。

覆盖：
1. build_inject_row 返回 QHBoxLayout。
2. owner._inject_edit 创建 + QLineEdit 类型 + objectName。
3. owner._inject_edit placeholder 非空。
4. owner._inject_button 创建 + QPushButton 类型 + objectName。
5. owner._inject_button tooltip 非空。
"""

from __future__ import annotations

import os
from types import SimpleNamespace
from unittest.mock import MagicMock

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtWidgets import QHBoxLayout, QLineEdit, QPushButton, QWidget

from embeddebug.serial_station.ui.sections import build_inject_row


def _make_owner():
    owner = SimpleNamespace()
    owner.tr = lambda s: s
    owner._inject_received = MagicMock()
    return owner


# ── build_inject_row ─────────────────────────────────────────────
def test_build_inject_row_returns_hboxlayout(qtbot):
    owner = _make_owner()
    root = QWidget()
    qtbot.addWidget(root)
    row = build_inject_row(owner, root)
    assert isinstance(row, QHBoxLayout)


def test_build_inject_row_creates_inject_edit(qtbot):
    owner = _make_owner()
    root = QWidget()
    qtbot.addWidget(root)
    build_inject_row(owner, root)
    assert isinstance(owner._inject_edit, QLineEdit)
    assert owner._inject_edit.objectName() == "serialStationInjectEdit"


def test_build_inject_row_edit_has_placeholder(qtbot):
    owner = _make_owner()
    root = QWidget()
    qtbot.addWidget(root)
    build_inject_row(owner, root)
    assert owner._inject_edit.placeholderText() != ""


def test_build_inject_row_creates_inject_button(qtbot):
    owner = _make_owner()
    root = QWidget()
    qtbot.addWidget(root)
    build_inject_row(owner, root)
    assert isinstance(owner._inject_button, QPushButton)
    assert owner._inject_button.objectName() == "serialStationInjectButton"


def test_build_inject_row_button_has_tooltip(qtbot):
    owner = _make_owner()
    root = QWidget()
    qtbot.addWidget(root)
    build_inject_row(owner, root)
    assert owner._inject_button.toolTip() != ""

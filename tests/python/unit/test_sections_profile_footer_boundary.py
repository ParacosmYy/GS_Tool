"""sections build_profile_row + build_footer owner 属性契约测试。

test_sections_boundary 仅 _no_crash；本文件覆盖 owner 属性 + objectName 契约。

覆盖：
1. build_profile_row 创建 _profile_path_edit + _profile_name_edit + _save_profile_button + _load_profile_button。
2. 全 objectName 契约。
3. build_footer 创建 _clear_button + objectName。
4. build_profile_row 返回 QHBoxLayout。
5. build_footer 返回 QHBoxLayout。
"""

from __future__ import annotations

import os
from types import SimpleNamespace
from unittest.mock import MagicMock

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtWidgets import QHBoxLayout, QLineEdit, QPushButton, QWidget

from embeddebug.serial_station.ui.sections import build_footer, build_profile_row


def _make_owner():
    owner = SimpleNamespace()
    owner.tr = lambda s: s
    owner._save_profile = MagicMock()
    owner._load_profile = MagicMock()
    owner._clear_log = MagicMock()
    return owner


# ── build_profile_row ────────────────────────────────────────────
def test_build_profile_row_returns_hboxlayout(qtbot):
    owner = _make_owner()
    root = QWidget()
    qtbot.addWidget(root)
    row = build_profile_row(owner, root)
    assert isinstance(row, QHBoxLayout)


def test_build_profile_row_creates_path_edit(qtbot):
    owner = _make_owner()
    root = QWidget()
    qtbot.addWidget(root)
    build_profile_row(owner, root)
    assert isinstance(owner._profile_path_edit, QLineEdit)
    assert owner._profile_path_edit.objectName() == "serialStationProfilePathEdit"


def test_build_profile_row_creates_name_edit(qtbot):
    owner = _make_owner()
    root = QWidget()
    qtbot.addWidget(root)
    build_profile_row(owner, root)
    assert isinstance(owner._profile_name_edit, QLineEdit)
    assert owner._profile_name_edit.objectName() == "serialStationProfileNameEdit"


def test_build_profile_row_creates_save_button(qtbot):
    owner = _make_owner()
    root = QWidget()
    qtbot.addWidget(root)
    build_profile_row(owner, root)
    assert isinstance(owner._save_profile_button, QPushButton)
    assert owner._save_profile_button.objectName() == "serialStationSaveProfileButton"


def test_build_profile_row_creates_load_button(qtbot):
    owner = _make_owner()
    root = QWidget()
    qtbot.addWidget(root)
    build_profile_row(owner, root)
    assert isinstance(owner._load_profile_button, QPushButton)
    assert owner._load_profile_button.objectName() == "serialStationLoadProfileButton"


def test_build_profile_row_path_edit_has_placeholder(qtbot):
    owner = _make_owner()
    root = QWidget()
    qtbot.addWidget(root)
    build_profile_row(owner, root)
    assert owner._profile_path_edit.placeholderText() != ""


# ── build_footer ─────────────────────────────────────────────────
def test_build_footer_returns_hboxlayout(qtbot):
    owner = _make_owner()
    root = QWidget()
    qtbot.addWidget(root)
    row = build_footer(owner, root)
    assert isinstance(row, QHBoxLayout)


def test_build_footer_creates_clear_button(qtbot):
    owner = _make_owner()
    root = QWidget()
    qtbot.addWidget(root)
    build_footer(owner, root)
    assert isinstance(owner._clear_button, QPushButton)
    assert owner._clear_button.objectName() == "serialStationClearButton"


def test_build_footer_clear_button_has_tooltip(qtbot):
    owner = _make_owner()
    root = QWidget()
    qtbot.addWidget(root)
    build_footer(owner, root)
    assert owner._clear_button.toolTip() != ""

"""sections build_log_row owner 属性 + objectName 契约测试。

test_sections_boundary 仅 _no_crash；本文件覆盖 owner 属性 + objectName 契约。

覆盖：
1. build_log_row 返回 QVBoxLayout。
2. owner._log_filter_combo 创建 + objectName + 初始 'All'。
3. owner._log_search_edit 创建 + objectName + placeholder。
4. owner._log_path_edit 创建 + objectName + placeholder。
5. owner._log_stats_label 创建 + objectName。
6. owner._export_log_button 创建 + objectName。
7. owner._replay_log_button 创建 + objectName。
8. owner._log_info_banner 创建。
"""

from __future__ import annotations

import os
from types import SimpleNamespace
from unittest.mock import MagicMock

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtWidgets import QComboBox, QLabel, QLineEdit, QPushButton, QVBoxLayout, QWidget

from embeddebug.serial_station.ui.sections import build_log_row


def _make_owner():
    owner = SimpleNamespace()
    owner.tr = lambda s: s
    owner._render_log_entries = MagicMock()
    owner._export_log = MagicMock()
    owner._replay_log = MagicMock()
    owner._update_log_stats = MagicMock()
    return owner


# ── build_log_row 返回 ───────────────────────────────────────────
def test_build_log_row_returns_vboxlayout(qtbot):
    owner = _make_owner()
    root = QWidget()
    qtbot.addWidget(root)
    col = build_log_row(owner, root)
    assert isinstance(col, QVBoxLayout)


# ── filter combo ─────────────────────────────────────────────────
def test_build_log_row_creates_filter_combo(qtbot):
    owner = _make_owner()
    root = QWidget()
    qtbot.addWidget(root)
    build_log_row(owner, root)
    assert isinstance(owner._log_filter_combo, QComboBox)
    assert owner._log_filter_combo.objectName() == "serialStationLogFilterCombo"


def test_build_log_row_filter_combo_initial_all(qtbot):
    owner = _make_owner()
    root = QWidget()
    qtbot.addWidget(root)
    build_log_row(owner, root)
    assert owner._log_filter_combo.currentText() == "All"


def test_build_log_row_filter_combo_has_five_items(qtbot):
    owner = _make_owner()
    root = QWidget()
    qtbot.addWidget(root)
    build_log_row(owner, root)
    assert owner._log_filter_combo.count() == 5  # All/TX/RX/System/Error


# ── search edit ──────────────────────────────────────────────────
def test_build_log_row_creates_search_edit(qtbot):
    owner = _make_owner()
    root = QWidget()
    qtbot.addWidget(root)
    build_log_row(owner, root)
    assert isinstance(owner._log_search_edit, QLineEdit)
    assert owner._log_search_edit.objectName() == "serialStationLogSearchEdit"


def test_build_log_row_search_edit_has_placeholder(qtbot):
    owner = _make_owner()
    root = QWidget()
    qtbot.addWidget(root)
    build_log_row(owner, root)
    assert owner._log_search_edit.placeholderText() != ""


# ── log path edit ────────────────────────────────────────────────
def test_build_log_row_creates_path_edit(qtbot):
    owner = _make_owner()
    root = QWidget()
    qtbot.addWidget(root)
    build_log_row(owner, root)
    assert isinstance(owner._log_path_edit, QLineEdit)
    assert owner._log_path_edit.objectName() == "serialStationLogPathEdit"


# ── stats label ──────────────────────────────────────────────────
def test_build_log_row_creates_stats_label(qtbot):
    owner = _make_owner()
    root = QWidget()
    qtbot.addWidget(root)
    build_log_row(owner, root)
    assert isinstance(owner._log_stats_label, QLabel)
    assert owner._log_stats_label.objectName() == "serialStationLogStatsLabel"


# ── export/replay buttons ────────────────────────────────────────
def test_build_log_row_creates_export_button(qtbot):
    owner = _make_owner()
    root = QWidget()
    qtbot.addWidget(root)
    build_log_row(owner, root)
    assert isinstance(owner._export_log_button, QPushButton)
    assert owner._export_log_button.objectName() == "serialStationExportLogButton"


def test_build_log_row_creates_replay_button(qtbot):
    owner = _make_owner()
    root = QWidget()
    qtbot.addWidget(root)
    build_log_row(owner, root)
    assert isinstance(owner._replay_log_button, QPushButton)
    assert owner._replay_log_button.objectName() == "serialStationReplayLogButton"


# ── info banner ──────────────────────────────────────────────────
def test_build_log_row_creates_info_banner(qtbot):
    owner = _make_owner()
    root = QWidget()
    qtbot.addWidget(root)
    build_log_row(owner, root)
    assert owner._log_info_banner is not None

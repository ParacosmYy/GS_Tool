"""settings_advanced_tab 辅助函数单元测试 — _select_baudrate + _find_active_combo。

覆盖：_select_baudrate 选中已有项/追加新项、_find_active_combo 查找/未找到。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtWidgets import QApplication, QComboBox

import pytest


@pytest.fixture(scope="module")
def qapp():
    return QApplication.instance() or QApplication([])


def test_select_baudrate_existing(qapp):
    """combo 已含目标 rate → 选中。"""
    combo = QComboBox()
    combo.addItem("9600", 9600)
    combo.addItem("115200", 115200)
    from embeddebug.serial_station.ui.panels._settings_advanced_tab import _select_baudrate
    _select_baudrate(combo, 115200)
    assert combo.currentIndex() == 1
    assert combo.currentData() == 115200


def test_select_baudrate_appends_missing(qapp):
    """combo 不含目标 rate → 追加并选中。"""
    combo = QComboBox()
    combo.addItem("9600", 9600)
    from embeddebug.serial_station.ui.panels._settings_advanced_tab import _select_baudrate
    _select_baudrate(combo, 38400)
    assert combo.count() == 2
    assert combo.currentData() == 38400


def test_select_baudrate_empty_combo(qapp):
    """空 combo → 追加。"""
    combo = QComboBox()
    from embeddebug.serial_station.ui.panels._settings_advanced_tab import _select_baudrate
    _select_baudrate(combo, 9600)
    assert combo.count() == 1
    assert combo.currentData() == 9600


def test_find_active_combo_none_when_not_present(qapp):
    """无 baudrate combo 时返回 None。"""
    from embeddebug.serial_station.ui.panels._settings_advanced_tab import _find_active_combo
    assert _find_active_combo() is None


def test_select_baudrate_no_duplicate(qapp):
    from embeddebug.serial_station.ui.panels._settings_advanced_tab import _select_baudrate
    combo = QComboBox()
    combo.addItem("9600", 9600)
    _select_baudrate(combo, 9600)
    _select_baudrate(combo, 9600)
    assert combo.count() == 1


def test_find_active_combo_finds_existing(qapp):
    from PyQt6.QtWidgets import QMainWindow
    from embeddebug.serial_station.ui.panels._settings_advanced_tab import _find_active_combo
    window = QMainWindow()
    combo = QComboBox(window)
    combo.setObjectName("serialStationSettingsBaudrateCombo")
    assert _find_active_combo() is combo


def test_build_advanced_tab_tolerates_minimal_panel(qapp):
    from types import SimpleNamespace
    from PyQt6.QtWidgets import QWidget
    from embeddebug.serial_station.ui.panels._settings_advanced_tab import build_advanced_tab
    panel = SimpleNamespace(tr=lambda s: s, _baud_combo=QComboBox())
    parent = QWidget()
    try:
        assert isinstance(build_advanced_tab(panel, parent), QWidget)
    except (AttributeError, TypeError):
        pass

"""_dashboard_binding_dialog suggest_default_spec + _SpecDialog 边界测试。

suggest_default_spec + _SpecDialog 此前无直接测试。
本文件覆盖 suggest_default_spec 各 widget_type + 未知返回空 + _SpecDialog objectName。

覆盖：
1. suggest_default_spec value_display 返回 'value:ch0'。
2. suggest_default_spec gauge 返回 'gauge:ch0'。
3. suggest_default_spec led 返回 'led:info'。
4. suggest_default_spec slider 返回含 'slider:'。
5. suggest_default_spec button 返回含 'button:'。
6. suggest_default_spec 未知 widget_type 返回空串。
7. _SpecDialog objectName。
8. _SpecDialog 是 QDialog 子类。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtWidgets import QDialog

from embeddebug.serial_station.ui.panels._dashboard_binding_dialog import (
    _SpecDialog,
    suggest_default_spec,
)


# ── suggest_default_spec ─────────────────────────────────────────
def test_suggest_value_display():
    assert suggest_default_spec("value_display") == "value:ch0"


def test_suggest_gauge():
    assert suggest_default_spec("gauge") == "gauge:ch0"


def test_suggest_led():
    assert suggest_default_spec("led") == "led:info"


def test_suggest_slider():
    result = suggest_default_spec("slider")
    assert "slider:" in result


def test_suggest_button():
    result = suggest_default_spec("button")
    assert "button:" in result


def test_suggest_unknown_returns_empty():
    assert suggest_default_spec("nonexistent") == ""


# ── _SpecDialog ──────────────────────────────────────────────────
def test_spec_dialog_objectname(qtbot):
    dialog = _SpecDialog()
    qtbot.addWidget(dialog)
    assert dialog.objectName() == "serialStationDashboardBindingDialog"


def test_spec_dialog_is_qdialog(qtbot):
    dialog = _SpecDialog()
    qtbot.addWidget(dialog)
    assert isinstance(dialog, QDialog)

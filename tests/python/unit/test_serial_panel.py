"""SerialPanel 单元测试 — 包装 + window 属性 + on_enter/leave。"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtWidgets import QApplication

import pytest


@pytest.fixture(scope="module")
def qapp():
    return QApplication.instance() or QApplication([])


def test_serial_panel_build(qapp):
    from embeddebug.app.app_controller import AppController
    from embeddebug.serial_station.ui.panels.serial_panel import SerialPanel
    panel = SerialPanel()
    widget = panel.build(AppController())
    assert widget.objectName() == "serialStationSerialPanel"


def test_serial_panel_window_property(qapp):
    from embeddebug.app.app_controller import AppController
    from embeddebug.serial_station.ui.panels.serial_panel import SerialPanel
    panel = SerialPanel()
    assert panel.window is None
    panel.build(AppController())
    assert panel.window is not None


def test_serial_panel_on_enter_no_crash(qapp):
    from embeddebug.app.app_controller import AppController
    from embeddebug.serial_station.ui.panels.serial_panel import SerialPanel
    panel = SerialPanel()
    panel.build(AppController())
    panel.on_enter()  # 不抛异常


def test_serial_panel_on_leave_no_crash(qapp):
    from embeddebug.app.app_controller import AppController
    from embeddebug.serial_station.ui.panels.serial_panel import SerialPanel
    panel = SerialPanel()
    panel.build(AppController())
    panel.on_leave()  # 不抛异常

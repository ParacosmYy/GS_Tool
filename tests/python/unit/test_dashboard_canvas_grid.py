"""Batch 36 测试：DashboardCanvas 网格绘制 + show_grid 切换。

覆盖：
1. DashboardCanvas 默认 show_grid=True。
2. set_show_grid(False) → show_grid False + 不画网格（paintEvent 早返回）。
3. set_show_grid(True) 恢复。
4. paintEvent 不崩（offscreen 下 super().paintEvent + drawPoint）。
5. DashboardPanel 顶栏有网格按钮 + make_grid_toggle 委托。
6. make_grid_toggle 切换当前画布网格。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")


def _make_canvas(qtbot):
    from embeddebug.serial_station.ui.dashboard import DashboardCanvas

    canvas = DashboardCanvas()
    qtbot.addWidget(canvas)
    return canvas


# ── show_grid 属性 ─────────────────────────────────────────────────
def test_canvas_default_show_grid(qtbot):
    """DashboardCanvas 默认 show_grid=True。"""

    canvas = _make_canvas(qtbot)
    assert canvas.show_grid is True


def test_set_show_grid_false(qtbot):
    """set_show_grid(False) 应关闭网格。"""

    canvas = _make_canvas(qtbot)
    canvas.set_show_grid(False)
    assert canvas.show_grid is False


def test_set_show_grid_toggle(qtbot):
    """show_grid 应可来回切换。"""

    canvas = _make_canvas(qtbot)
    canvas.set_show_grid(False)
    assert canvas.show_grid is False
    canvas.set_show_grid(True)
    assert canvas.show_grid is True


# ── paintEvent 不崩 ────────────────────────────────────────────────
def test_paint_event_grid_on_no_crash(qtbot):
    """show_grid=True 时 paintEvent 应画网格点（不崩）。"""

    from PyQt6.QtGui import QPaintEvent
    from PyQt6.QtCore import QRect

    canvas = _make_canvas(qtbot)
    canvas.resize(200, 200)
    canvas.set_show_grid(True)
    # 触发 paintEvent（offscreen 下 super + drawPoint 不应崩）。
    canvas.paintEvent(QPaintEvent(QRect(0, 0, 200, 200)))


def test_paint_event_grid_off_no_crash(qtbot):
    """show_grid=False 时 paintEvent 应早返回（不画网格，不崩）。"""

    from PyQt6.QtGui import QPaintEvent
    from PyQt6.QtCore import QRect

    canvas = _make_canvas(qtbot)
    canvas.resize(200, 200)
    canvas.set_show_grid(False)
    canvas.paintEvent(QPaintEvent(QRect(0, 0, 200, 200)))


# ── DashboardPanel 网格按钮 ────────────────────────────────────────
def test_panel_has_grid_button(qtbot):
    """DashboardPanel 顶栏应有网格切换按钮（objectName）。"""

    from embeddebug.app.app_controller import AppController
    from embeddebug.serial_station.ui.panels.dashboard_panel import DashboardPanel

    panel = DashboardPanel()
    widget = panel.build(AppController())
    qtbot.addWidget(widget)
    from PyQt6.QtWidgets import QPushButton

    btn = widget.findChild(QPushButton, "serialStationDashboardGridButton")
    assert btn is not None
    assert btn.isCheckable()
    assert btn.isChecked()  # 默认勾选（显示网格）


def test_make_grid_toggle_toggles_canvas(qtbot):
    """make_grid_toggle 返回的 slot 应切换当前画布网格。"""

    from embeddebug.app.app_controller import AppController
    from embeddebug.serial_station.ui.panels.dashboard_panel import DashboardPanel
    from embeddebug.serial_station.ui.panels._dashboard_widget_menu import make_grid_toggle

    panel = DashboardPanel()
    widget = panel.build(AppController())
    qtbot.addWidget(widget)
    toggle = make_grid_toggle(panel)
    before = panel._tabs.current_canvas().show_grid
    toggle(not before)
    assert panel._tabs.current_canvas().show_grid is (not before)


# ── QSS 覆盖 ───────────────────────────────────────────────────────
def test_qss_covers_grid_button():
    """build_qss 应含 serialStationDashboardGridButton 选择器。"""

    from embeddebug.serial_station.ui.theme.qss_builder import build_qss

    assert "#serialStationDashboardGridButton" in build_qss()

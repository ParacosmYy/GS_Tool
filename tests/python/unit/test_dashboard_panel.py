"""DashboardPanel 装配 + 仪表盘模式注册测试（激活 dashboard 死代码）。

覆盖：
1. DashboardPanel 构建返回含 palette + tabs 的控件。
2. dashboard 模式注册到 registry（mode_id/icon/label）。
3. on_enter/on_leave 接入入场动画 helper。
4. add_tab/clear/save/load 操作（save/load 用 monkeypatch 拦截 QFileDialog）。
5. dashboard 子系统现已有外部消费者（Batch 16 审计测试的反转）。
"""

from __future__ import annotations

import inspect
import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from unittest.mock import MagicMock

import pytest

from embeddebug.app.mode_panel import registered_panels
from embeddebug.serial_station.ui.panels import register_default_panels


# ── DashboardPanel 构建 ────────────────────────────────────────────
def test_dashboard_panel_builds(qtbot):
    from embeddebug.app.app_controller import AppController
    from embeddebug.serial_station.ui.dashboard import DashboardTabs, WidgetPalette
    from embeddebug.serial_station.ui.panels.dashboard_panel import DashboardPanel

    panel = DashboardPanel()
    widget = panel.build(AppController())
    qtbot.addWidget(widget)
    assert widget.objectName() == "serialStationDashboardPanel"
    # 应含控件库 + 标签页。
    assert isinstance(panel._palette, WidgetPalette)
    assert isinstance(panel._tabs, DashboardTabs)
    # 初始应有一个默认标签页 + 一个 canvas。
    assert panel._tabs.count() >= 1
    assert panel._tabs.current_canvas() is not None


def test_dashboard_panel_has_status_label(qtbot):
    from embeddebug.app.app_controller import AppController
    from embeddebug.serial_station.ui.panels.dashboard_panel import DashboardPanel

    panel = DashboardPanel()
    widget = panel.build(AppController())
    qtbot.addWidget(widget)
    assert panel._status.objectName() == "serialStationDashboardStatusLabel"


# ── 模式注册 ───────────────────────────────────────────────────────
def test_dashboard_mode_registered():
    """register_default_panels 应注册 dashboard 模式。"""

    register_default_panels()
    regs = registered_panels()
    ids = [r.mode_id for r in regs]
    assert "dashboard" in ids
    reg = next(r for r in regs if r.mode_id == "dashboard")
    assert reg.icon == "layout-dashboard"
    assert reg.label == "仪表盘"


def test_dashboard_mode_factory_returns_panel():
    """dashboard 注册项的 factory 应返回 DashboardPanel 实例。"""

    register_default_panels()
    reg = next(r for r in registered_panels() if r.mode_id == "dashboard")
    from embeddebug.app.app_controller import AppController
    from embeddebug.serial_station.ui.panels.dashboard_panel import DashboardPanel

    panel = reg.factory(AppController())
    assert isinstance(panel, DashboardPanel)


# ── 入场动画 ───────────────────────────────────────────────────────
def test_dashboard_panel_on_enter_wired():
    """DashboardPanel.on_enter 应调 play_panel_enter（源码级断言）。"""

    from embeddebug.serial_station.ui.panels.dashboard_panel import DashboardPanel

    src = inspect.getsource(DashboardPanel.on_enter)
    assert "play_panel_enter" in src


def test_dashboard_panel_on_leave_wired():
    """DashboardPanel.on_leave 应调 stop_panel_enter。"""

    from embeddebug.serial_station.ui.panels.dashboard_panel import DashboardPanel

    src = inspect.getsource(DashboardPanel.on_leave)
    assert "stop_panel_enter" in src


# ── 操作 ───────────────────────────────────────────────────────────
def test_add_tab_increments_count(qtbot):
    from embeddebug.app.app_controller import AppController
    from embeddebug.serial_station.ui.panels.dashboard_panel import DashboardPanel

    panel = DashboardPanel()
    widget = panel.build(AppController())
    qtbot.addWidget(widget)
    before = panel._tabs.count()
    panel._add_tab()
    assert panel._tabs.count() == before + 1


def test_clear_canvas_empties_items(qtbot):
    from embeddebug.app.app_controller import AppController
    from embeddebug.serial_station.ui.panels.dashboard_panel import DashboardPanel
    from PyQt6.QtCore import QPoint

    panel = DashboardPanel()
    widget = panel.build(AppController())
    qtbot.addWidget(widget)
    canvas = panel._tabs.current_canvas()
    canvas.add_widget_at("led", QPoint(20, 20))
    assert len(canvas.items) == 1
    panel._clear_canvas()
    assert len(canvas.items) == 0


def test_save_layout_writes_file(qtbot, monkeypatch, tmp_path):
    """保存布局应写入 JSON 文件（monkeypatch QFileDialog 返回 tmp 路径）。"""

    from embeddebug.app.app_controller import AppController
    from embeddebug.serial_station.ui.panels import dashboard_panel as dp_module
    from embeddebug.serial_station.ui.panels.dashboard_panel import DashboardPanel
    from PyQt6.QtCore import QPoint

    panel = DashboardPanel()
    widget = panel.build(AppController())
    qtbot.addWidget(widget)
    canvas = panel._tabs.current_canvas()
    canvas.add_widget_at("led", QPoint(20, 20))
    out = tmp_path / "layout.json"
    monkeypatch.setattr(
        dp_module.QFileDialog, "getSaveFileName",
        staticmethod(lambda *a, **k: (str(out), "")),
    )
    panel._save_layout()
    assert out.exists()
    import json

    data = json.loads(out.read_text(encoding="utf-8"))
    assert len(data["items"]) == 1


def test_load_layout_restores_items(qtbot, monkeypatch, tmp_path):
    """加载布局应恢复控件数。"""

    from embeddebug.app.app_controller import AppController
    from embeddebug.serial_station.ui.panels import dashboard_panel as dp_module
    from embeddebug.serial_station.ui.panels.dashboard_panel import DashboardPanel
    from PyQt6.QtCore import QPoint

    panel = DashboardPanel()
    widget = panel.build(AppController())
    qtbot.addWidget(widget)
    canvas = panel._tabs.current_canvas()
    canvas.add_widget_at("led", QPoint(20, 20))
    canvas.add_widget_at("gauge", QPoint(60, 60))
    out = tmp_path / "layout.json"
    canvas.save_layout(out)
    # 清空后加载应恢复 2 个。
    canvas.clear()
    assert len(canvas.items) == 0
    monkeypatch.setattr(
        dp_module.QFileDialog, "getOpenFileName",
        staticmethod(lambda *a, **k: (str(out), "")),
    )
    panel._load_layout()
    assert len(canvas.items) == 2

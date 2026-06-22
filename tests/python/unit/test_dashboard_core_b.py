"""test_dashboard_core part 2 — auto-split from merged file（Batch 47 测试精简）。"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import inspect
import json

from PyQt6.QtCore import QPoint

from embeddebug.app.mode_panel import registered_panels
from embeddebug.serial_station.ui.dashboard import (
    SUPPORTED_WIDGET_TYPES,
    DashboardCanvas,
    DashboardTabs,
    WidgetPalette,
)
from embeddebug.serial_station.ui.panels import register_default_panels

def _make_panel(qtbot):
    from embeddebug.app.app_controller import AppController
    from embeddebug.serial_station.ui.panels.dashboard_panel import DashboardPanel

    panel = DashboardPanel()
    widget = panel.build(AppController())
    qtbot.addWidget(widget)
    return panel

def test_widget_palette_lists_all_widget_types(qtbot):
    palette = WidgetPalette()
    qtbot.addWidget(palette)
    types = {btn.widget_type for btn in palette.palette_buttons()}
    assert types == set(SUPPORTED_WIDGET_TYPES)

def test_dashboard_tabs_has_objectname(qtbot):
    tabs = DashboardTabs()
    qtbot.addWidget(tabs)
    assert tabs.objectName() == "serialStationDashboardTabs"

def test_dashboard_tabs_starts_with_one_tab(qtbot):
    tabs = DashboardTabs()
    qtbot.addWidget(tabs)
    assert tabs.count() == 1

def test_dashboard_tabs_add_tab(qtbot):
    tabs = DashboardTabs()
    qtbot.addWidget(tabs)
    canvas = tabs.add_tab("My Board")
    assert tabs.count() == 2
    assert isinstance(canvas, DashboardCanvas)
    assert "My Board" in tabs.tab_names()

def test_dashboard_tabs_rename(qtbot):
    tabs = DashboardTabs()
    qtbot.addWidget(tabs)
    tabs.rename_tab(0, "Renamed")
    assert tabs.tab_names()[0] == "Renamed"

def test_dashboard_tabs_close_keeps_at_least_one(qtbot):
    tabs = DashboardTabs()
    qtbot.addWidget(tabs)
    tabs.add_tab()
    assert tabs.count() == 2
    tabs._close_tab(1)
    assert tabs.count() == 1
    tabs._close_tab(0)
    assert tabs.count() == 1

def test_dashboard_tabs_current_canvas(qtbot):
    tabs = DashboardTabs()
    qtbot.addWidget(tabs)
    canvas = tabs.current_canvas()
    assert isinstance(canvas, DashboardCanvas)

def test_dashboard_panel_builds(qtbot):
    panel = _make_panel(qtbot)
    assert panel._tabs.count() >= 1
    assert panel._tabs.current_canvas() is not None
    assert isinstance(panel._palette, WidgetPalette)
    assert isinstance(panel._tabs, DashboardTabs)

def test_dashboard_panel_has_status_label(qtbot):
    panel = _make_panel(qtbot)
    assert panel._status.objectName() == "serialStationDashboardStatusLabel"

def test_dashboard_mode_registered():
    register_default_panels()
    regs = registered_panels()
    ids = [r.mode_id for r in regs]
    assert "dashboard" in ids
    reg = next(r for r in regs if r.mode_id == "dashboard")
    assert reg.icon == "layout-dashboard"
    assert reg.label == "仪表盘"

def test_dashboard_mode_factory_returns_panel():
    from embeddebug.app.app_controller import AppController
    from embeddebug.serial_station.ui.panels.dashboard_panel import DashboardPanel

    register_default_panels()
    reg = next(r for r in registered_panels() if r.mode_id == "dashboard")
    assert isinstance(reg.factory(AppController()), DashboardPanel)

def test_dashboard_panel_on_enter_wired():
    from embeddebug.serial_station.ui.panels.dashboard_panel import DashboardPanel

    assert "play_panel_enter" in inspect.getsource(DashboardPanel.on_enter)

def test_dashboard_panel_on_leave_wired():
    from embeddebug.serial_station.ui.panels.dashboard_panel import DashboardPanel

    assert "stop_panel_enter" in inspect.getsource(DashboardPanel.on_leave)

def test_add_tab_increments_count(qtbot):
    panel = _make_panel(qtbot)
    before = panel._tabs.count()
    panel._add_tab()
    assert panel._tabs.count() == before + 1

def test_clear_canvas_empties_items(qtbot):
    panel = _make_panel(qtbot)
    canvas = panel._tabs.current_canvas()
    canvas.add_widget_at("led", QPoint(20, 20))
    assert len(canvas.items) == 1
    panel._clear_canvas()
    assert len(canvas.items) == 0

def test_save_layout_writes_file(qtbot, monkeypatch, tmp_path):
    from embeddebug.serial_station.ui.panels import dashboard_panel as dp_module

    panel = _make_panel(qtbot)
    canvas = panel._tabs.current_canvas()
    canvas.add_widget_at("led", QPoint(20, 20))
    out = tmp_path / "layout.json"
    monkeypatch.setattr(dp_module.QFileDialog, "getSaveFileName",
                        staticmethod(lambda *a, **k: (str(out), "")))
    panel._save_layout()
    assert out.exists()
    data = json.loads(out.read_text(encoding="utf-8"))
    assert len(data["items"]) == 1

def test_load_layout_restores_items(qtbot, monkeypatch, tmp_path):
    from embeddebug.serial_station.ui.panels import dashboard_panel as dp_module

    panel = _make_panel(qtbot)
    canvas = panel._tabs.current_canvas()
    canvas.add_widget_at("led", QPoint(20, 20))
    canvas.add_widget_at("gauge", QPoint(60, 60))
    out = tmp_path / "layout.json"
    canvas.save_layout(out)
    canvas.clear()
    assert len(canvas.items) == 0
    monkeypatch.setattr(dp_module.QFileDialog, "getOpenFileName",
                        staticmethod(lambda *a, **k: (str(out), "")))
    panel._load_layout()
    assert len(canvas.items) == 2

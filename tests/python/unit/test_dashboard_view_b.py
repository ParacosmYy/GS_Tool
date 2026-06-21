"""test_dashboard_view part 2 — auto-split from merged file（Batch 47 测试精简）。"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import inspect

from PyQt6.QtCore import QPoint
from PyQt6.QtWidgets import QLabel, QWidget

from embeddebug.serial_station.ui.dashboard import (
    WidgetFullscreenHandler,
    attach_double_click_fullscreen,
)
from embeddebug.serial_station.ui.panels import _dashboard_layout_store as store

def _make_panel(qtbot):
    from embeddebug.app.app_controller import AppController
    from embeddebug.serial_station.ui.panels.dashboard_panel import DashboardPanel

    panel = DashboardPanel()
    widget = panel.build(AppController())
    qtbot.addWidget(widget)
    return panel

def _make_tabs(qtbot):
    from embeddebug.serial_station.ui.dashboard import DashboardTabs

    tabs = DashboardTabs()
    qtbot.addWidget(tabs)
    return tabs

def test_palette_title_uses_tr(qtbot):
    from embeddebug.serial_station.ui.dashboard import WidgetPalette

    src = inspect.getsource(WidgetPalette.__init__)
    assert 'self.tr("Widgets")' in src
    assert 'QLabel("Widgets"' not in src

def test_palette_title_rendered(qtbot):
    from PyQt6.QtWidgets import QLabel as QL
    from embeddebug.serial_station.ui.dashboard import WidgetPalette

    palette = WidgetPalette()
    qtbot.addWidget(palette)
    title = palette.findChild(QL, "serialStationPaletteTitle")
    assert title is not None
    assert title.text() == "Widgets"

def test_tabs_default_name_uses_tr(qtbot):
    from embeddebug.serial_station.ui.dashboard import DashboardTabs

    src = inspect.getsource(DashboardTabs._default_tab_name)
    assert "self.tr(" in src
    assert 'f"Dashboard' not in src

def test_tabs_default_name_rendered(qtbot):
    from embeddebug.serial_station.ui.dashboard import DashboardTabs

    tabs = DashboardTabs()
    qtbot.addWidget(tabs)
    assert "Dashboard" in tabs.tabText(0)

def test_no_hardcoded_drag_tooltip():
    from pathlib import Path

    src = Path("python/embeddebug/serial_station/ui/dashboard/palette.py").read_text(encoding="utf-8")
    assert 'f"Drag to canvas' not in src
    assert "f'Drag to canvas" not in src

def test_persist_all_tabs_stores_per_tab(qtbot, tmp_path, monkeypatch):
    monkeypatch.setattr(store, "layout_path", lambda: tmp_path / "layout.json")
    tabs = _make_tabs(qtbot)
    tabs.current_canvas().add_widget_at("led", QPoint(20, 20))
    tabs.add_tab("Page2")
    tabs.current_canvas().add_widget_at("gauge", QPoint(40, 40))
    assert store.persist_all_tabs(tabs) is True
    saved = store.load_tabs_layout()
    assert "Page2" in saved
    assert len(saved) >= 2

def test_restore_all_tabs_recovers_per_tab(qtbot, tmp_path, monkeypatch):
    monkeypatch.setattr(store, "layout_path", lambda: tmp_path / "layout.json")
    tabs = _make_tabs(qtbot)
    tabs.current_canvas().add_widget_at("led", QPoint(20, 20))
    tabs.add_tab("Page2")
    tabs.current_canvas().add_widget_at("gauge", QPoint(40, 40))
    store.persist_all_tabs(tabs)
    tabs2 = _make_tabs(qtbot)
    tabs2.add_tab("Page2")
    assert store.restore_all_tabs(tabs2) >= 1

def test_restore_empty_returns_zero(qtbot, tmp_path, monkeypatch):
    monkeypatch.setattr(store, "layout_path", lambda: tmp_path / "nope.json")
    assert store.restore_all_tabs(_make_tabs(qtbot)) == 0

def test_load_tabs_layout_legacy_single_canvas(tmp_path, monkeypatch):
    p = tmp_path / "layout.json"
    monkeypatch.setattr(store, "layout_path", lambda: p)
    store.save_layout_dict({"items": [
        {"id": "led_1", "type": "led", "x": 20, "y": 20, "width": 160, "height": 80, "config": {}},
    ]})
    tabs_layout = store.load_tabs_layout()
    assert "Dashboard 1" in tabs_layout
    assert len(tabs_layout["Dashboard 1"]["items"]) == 1

def test_dashboard_panel_restores_multiple_tabs(qtbot, tmp_path, monkeypatch):
    from embeddebug.app.app_controller import AppController
    from embeddebug.serial_station.ui.panels.dashboard_panel import DashboardPanel

    monkeypatch.setenv("EMBEDDEBUG_DASHBOARD_AUTOSAVE", "1")
    monkeypatch.setattr(store, "layout_path", lambda: tmp_path / "layout.json")
    store.save_tabs_layout({"Dashboard 1": {"items": [
        {"id": "led_1", "type": "led", "x": 20, "y": 20, "width": 160, "height": 80, "config": {}},
    ]}})
    panel = DashboardPanel()
    widget = panel.build(AppController())
    qtbot.addWidget(widget)
    assert len(panel._tabs.current_canvas().items) == 1

def test_dashboard_panel_autosaves_all_tabs(qtbot, tmp_path, monkeypatch):
    from embeddebug.app.app_controller import AppController
    from embeddebug.serial_station.ui.panels.dashboard_panel import DashboardPanel

    monkeypatch.setenv("EMBEDDEBUG_DASHBOARD_AUTOSAVE", "1")
    p = tmp_path / "layout.json"
    monkeypatch.setattr(store, "layout_path", lambda: p)
    monkeypatch.setattr(store, "restore_all_tabs", lambda tabs: 0)
    panel = DashboardPanel()
    widget = panel.build(AppController())
    qtbot.addWidget(widget)
    panel._tabs.current_canvas().add_widget_at("led", QPoint(20, 20))
    panel._tabs.add_tab("Page2")
    panel._tabs.current_canvas().add_widget_at("gauge", QPoint(40, 40))
    saved = store.load_tabs_layout()
    assert "Page2" in saved
    assert any(name != "Page2" for name in saved)

def test_tab_names_used_by_persist():
    from embeddebug.serial_station.ui.panels import _dashboard_layout_store

    src = inspect.getsource(_dashboard_layout_store.persist_all_tabs)
    assert "tab_names" in src

def test_store_exposes_multitab_apis():
    assert callable(store.persist_all_tabs)
    assert callable(store.restore_all_tabs)
    assert callable(store.load_tabs_layout)
    assert callable(store.save_tabs_layout)

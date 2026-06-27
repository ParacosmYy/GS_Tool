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
def test_fullscreen_handler_starts_not_fullscreen(qtbot):
    assert WidgetFullscreenHandler().is_fullscreen is False
def test_fullscreen_enter_and_restore(qtbot):
    host = QWidget()
    host.resize(400, 300)
    qtbot.addWidget(host)
    child = QLabel("hi", host)
    child.setGeometry(10, 10, 80, 30)
    original_parent = child.parent()
    original_geometry = child.geometry()
    handler = WidgetFullscreenHandler()
    handler.enter(child, host)
    assert handler.is_fullscreen is True
    assert child.geometry().size() == host.rect().size()
    handler.restore()
    assert handler.is_fullscreen is False
    assert child.parent() is original_parent
    assert child.geometry() == original_geometry
def test_fullscreen_toggle(qtbot):
    host = QWidget()
    host.resize(400, 300)
    qtbot.addWidget(host)
    child = QLabel("x", host)
    child.setGeometry(5, 5, 50, 20)
    handler = WidgetFullscreenHandler()
    assert handler.toggle(child, host) is True
    assert handler.is_fullscreen is True
    assert handler.toggle(child, host) is False
    assert handler.is_fullscreen is False
def test_fullscreen_restore_without_enter_is_noop(qtbot):
    handler = WidgetFullscreenHandler()
    handler.restore()
    assert handler.is_fullscreen is False
def test_attach_double_click_fullscreen(qtbot):
    from PyQt6.QtCore import QEvent, QPointF, Qt
    from PyQt6.QtGui import QMouseEvent
    host = QWidget()
    host.resize(400, 300)
    qtbot.addWidget(host)
    child = QLabel("fs", host)
    child.setGeometry(10, 10, 60, 30)
    handler = attach_double_click_fullscreen(child, host)
    assert isinstance(handler, WidgetFullscreenHandler)
    event = QMouseEvent(
        QEvent.Type.MouseButtonDblClick,
        QPointF(child.rect().center()),
        Qt.MouseButton.LeftButton,
        Qt.MouseButton.LeftButton,
        Qt.KeyboardModifier.NoModifier,
    )
    child.mouseDoubleClickEvent(event)
    assert handler.is_fullscreen is True
def test_panel_builds_with_fullscreen_handlers_list(qtbot):
    panel = _make_panel(qtbot)
    assert hasattr(panel, "_fullscreen_handlers")
    assert panel._fullscreen_handlers == []
def test_initial_canvas_wired_to_item_added(qtbot):
    panel = _make_panel(qtbot)
    canvas = panel._tabs.current_canvas()
    assert id(canvas) in panel._wired_canvases
def test_adding_widget_installs_fullscreen_handler(qtbot):
    panel = _make_panel(qtbot)
    canvas = panel._tabs.current_canvas()
    before = len(panel._fullscreen_handlers)
    canvas.add_widget_at("led", QPoint(20, 20))
    assert len(panel._fullscreen_handlers) == before + 1
def test_added_widget_has_double_click_override(qtbot):
    panel = _make_panel(qtbot)
    canvas = panel._tabs.current_canvas()
    canvas.add_widget_at("gauge", QPoint(40, 40))
    item = next(iter(canvas.items.values()))
    assert "mouseDoubleClickEvent" in item.widget.__dict__
def test_dashboard_panel_calls_attach_double_click_fullscreen():
    from embeddebug.serial_station.ui.panels import dashboard_panel
    src = inspect.getsource(dashboard_panel)
    assert "attach_double_click_fullscreen" in src
    assert "item_added" in src
    assert "_on_item_added_fullscreen" in src
def test_fullscreen_module_now_has_external_consumer():
    from pathlib import Path
    ui = Path("python/embeddebug/serial_station/ui")
    fullscreen_consumers: list[str] = []
    for py in ui.rglob("*.py"):
        if py.name == "fullscreen.py" or ("dashboard" in py.parts and py.name != "dashboard_panel.py"):
            if py.name != "dashboard_panel.py":
                continue
        try:
            text = py.read_text(encoding="utf-8")
        except OSError:
            continue
        if "attach_double_click_fullscreen" in text or "WidgetFullscreenHandler" in text:
            fullscreen_consumers.append(str(py))
    assert any("dashboard_panel.py" in p for p in fullscreen_consumers)
def test_palette_button_tooltip_uses_tr(qtbot):
    from embeddebug.serial_station.ui.dashboard.palette import WidgetPaletteButton
    src = inspect.getsource(WidgetPaletteButton.__init__)
    assert "self.tr(" in src
    assert "Drag to canvas to add {name}" in src
    assert 'f"Drag to canvas' not in src
    assert "f'Drag to canvas" not in src
def test_palette_button_tooltip_rendered(qtbot):
    from embeddebug.serial_station.ui.dashboard.palette import WidgetPaletteButton
    btn = WidgetPaletteButton("led", "LED", "circle")
    qtbot.addWidget(btn)
    assert "LED" in btn.toolTip()
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

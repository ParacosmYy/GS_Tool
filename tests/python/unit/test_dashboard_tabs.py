
from __future__ import annotations

import inspect
import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtCore import QPoint

from embeddebug.serial_station.ui.panels import _dashboard_tab_menu as menu_mod

def _make_panel(qtbot):
    from embeddebug.app.app_controller import AppController
    from embeddebug.serial_station.ui.panels.dashboard_panel import DashboardPanel

    panel = DashboardPanel()
    widget = panel.build(AppController())
    qtbot.addWidget(widget)
    return panel

# ── 重命名接线 ─────────────────────────────────────────────────────
def test_panel_wires_tab_bar_double_click():
    from embeddebug.serial_station.ui.panels.dashboard_panel import DashboardPanel

    src = inspect.getsource(DashboardPanel.build)
    assert "tabBarDoubleClicked" in src
    assert "_rename_tab_on_double_click" in src

def test_panel_has_rename_handler():
    from embeddebug.serial_station.ui.panels.dashboard_panel import DashboardPanel

    assert hasattr(DashboardPanel, "_rename_tab_on_double_click")

# ── 重命名行为 ─────────────────────────────────────────────────────
def test_rename_with_new_name(qtbot, monkeypatch):
    panel = _make_panel(qtbot)
    calls: list = []
    monkeypatch.setattr(panel._tabs, "rename_tab", lambda idx, name: calls.append((idx, name)))
    import PyQt6.QtWidgets as QtWidgets

    monkeypatch.setattr(QtWidgets.QInputDialog, "getText",
                        staticmethod(lambda *a, **k: ("MyDashboard", True)))
    panel._rename_tab_on_double_click(0)
    assert calls == [(0, "MyDashboard")]
    assert "MyDashboard" in panel._status.text()

def test_rename_cancel_keeps_original(qtbot, monkeypatch):
    panel = _make_panel(qtbot)
    calls: list = []
    monkeypatch.setattr(panel._tabs, "rename_tab", lambda idx, name: calls.append((idx, name)))
    import PyQt6.QtWidgets as QtWidgets

    monkeypatch.setattr(QtWidgets.QInputDialog, "getText",
                        staticmethod(lambda *a, **k: ("ignored", False)))
    panel._rename_tab_on_double_click(0)
    assert calls == []

def test_rename_empty_keeps_original(qtbot, monkeypatch):
    panel = _make_panel(qtbot)
    calls: list = []
    monkeypatch.setattr(panel._tabs, "rename_tab", lambda idx, name: calls.append((idx, name)))
    import PyQt6.QtWidgets as QtWidgets

    monkeypatch.setattr(QtWidgets.QInputDialog, "getText",
                        staticmethod(lambda *a, **k: ("   ", True)))
    panel._rename_tab_on_double_click(0)
    assert calls == []

def test_rename_same_name_skips(qtbot, monkeypatch):
    panel = _make_panel(qtbot)
    calls: list = []
    monkeypatch.setattr(panel._tabs, "rename_tab", lambda idx, name: calls.append((idx, name)))
    current = panel._tabs.tabText(0)
    import PyQt6.QtWidgets as QtWidgets

    monkeypatch.setattr(QtWidgets.QInputDialog, "getText",
                        staticmethod(lambda *a, **k: (current, True)))
    panel._rename_tab_on_double_click(0)
    assert calls == []

def test_rename_out_of_range_index_no_crash(qtbot, monkeypatch):
    panel = _make_panel(qtbot)
    import PyQt6.QtWidgets as QtWidgets

    monkeypatch.setattr(QtWidgets.QInputDialog, "getText",
                        staticmethod(lambda *a, **k: ("x", True)))
    panel._rename_tab_on_double_click(999)

def test_rename_tab_now_has_production_consumer():
    from pathlib import Path

    src = Path("python/embeddebug/serial_station/ui/panels/dashboard_panel.py").read_text(encoding="utf-8")
    assert "rename_tab" in src

def test_rename_tab_in_dashboard_tabs_still_present():
    from embeddebug.serial_station.ui.dashboard import DashboardTabs

    assert callable(DashboardTabs.rename_tab)

# ── 右键菜单接线 ───────────────────────────────────────────────────
def test_panel_wires_context_menu():
    from embeddebug.serial_station.ui.panels.dashboard_panel import DashboardPanel

    src = inspect.getsource(DashboardPanel.build)
    assert "customContextMenuRequested" in src
    assert "_show_tab_context_menu" in src

def test_panel_has_show_tab_context_menu():
    from embeddebug.serial_station.ui.panels.dashboard_panel import DashboardPanel

    assert hasattr(DashboardPanel, "_show_tab_context_menu")

# ── duplicate_tab ──────────────────────────────────────────────────
def test_duplicate_tab_clones_widgets(qtbot):
    panel = _make_panel(qtbot)
    src = panel._tabs.current_canvas()
    src.add_widget_at("led", QPoint(20, 20))
    src.add_widget_at("gauge", QPoint(40, 40))
    before = panel._tabs.count()
    menu_mod.duplicate_tab(panel, 0)
    assert panel._tabs.count() == before + 1
    new_canvas = panel._tabs.widget(panel._tabs.count() - 1)
    assert len(new_canvas.items) == 2

def test_duplicate_tab_out_of_range_no_crash(qtbot):
    panel = _make_panel(qtbot)
    menu_mod.duplicate_tab(panel, 999)
    assert panel._tabs.count() == 1

# ── close_tab_by_index ─────────────────────────────────────────────
def test_close_tab_by_index_removes(qtbot):
    panel = _make_panel(qtbot)
    panel._tabs.add_tab("Page2")
    assert panel._tabs.count() == 2
    menu_mod.close_tab_by_index(panel, 0)
    assert panel._tabs.count() == 1

def test_close_tab_keeps_at_least_one(qtbot):
    panel = _make_panel(qtbot)
    assert panel._tabs.count() == 1
    menu_mod.close_tab_by_index(panel, 0)
    assert panel._tabs.count() == 1

# ── show_tab_context_menu ─────────────────────────────────────────
def test_show_tab_context_menu_miss_no_crash(qtbot, monkeypatch):
    panel = _make_panel(qtbot)
    bar = panel._tabs.tabBar()
    monkeypatch.setattr(bar, "tabAt", lambda _pos: -1)
    import PyQt6.QtWidgets as QtWidgets

    exec_calls: list = []
    monkeypatch.setattr(QtWidgets.QMenu, "exec", lambda *a, **k: exec_calls.append(1))
    menu_mod.show_tab_context_menu(panel, QPoint(0, 0))
    assert exec_calls == []

def test_show_tab_context_menu_hit_builds_menu(qtbot, monkeypatch):
    panel = _make_panel(qtbot)
    bar = panel._tabs.tabBar()
    monkeypatch.setattr(bar, "tabAt", lambda _pos: 0)
    import PyQt6.QtWidgets as QtWidgets

    built: list = []
    original_init = QtWidgets.QMenu.__init__

    def _init(self, *a, **k):
        original_init(self, *a, **k)
        built.append(self)

    monkeypatch.setattr(QtWidgets.QMenu, "__init__", _init)
    monkeypatch.setattr(QtWidgets.QMenu, "exec", lambda *a, **k: None)
    menu_mod.show_tab_context_menu(panel, QPoint(10, 10))
    assert len(built) == 1
    assert built[0].actions().__len__() == 3

def test_menu_module_exposes_helpers():
    assert callable(menu_mod.show_tab_context_menu)
    assert callable(menu_mod.duplicate_tab)
    assert callable(menu_mod.close_tab_by_index)

# ── 拖拽重排序 ─────────────────────────────────────────────────────
def test_dashboard_tabs_is_movable(qtbot):
    from embeddebug.serial_station.ui.dashboard import DashboardTabs

    tabs = DashboardTabs()
    qtbot.addWidget(tabs)
    assert tabs.isMovable() is True

def test_dashboard_tabs_has_tab_moved_signal(qtbot):
    from embeddebug.serial_station.ui.dashboard import DashboardTabs

    tabs = DashboardTabs()
    qtbot.addWidget(tabs)
    received: list = []
    tabs.tab_moved.connect(lambda frm, to: received.append((frm, to)))
    tabs.tabBar().tabMoved.emit(0, 1)
    assert received == [(0, 1)]

def test_dashboard_panel_wires_tab_moved_to_autosave():
    from embeddebug.serial_station.ui.panels.dashboard_panel import DashboardPanel

    src = inspect.getsource(DashboardPanel.build)
    assert "tab_moved" in src
    assert "_autosave_layout" in src

def test_dashboard_panel_autosaves_on_tab_move(qtbot, monkeypatch):
    from embeddebug.app.app_controller import AppController
    from embeddebug.serial_station.ui.panels.dashboard_panel import DashboardPanel

    panel = DashboardPanel()
    widget = panel.build(AppController())
    qtbot.addWidget(widget)
    calls: list = []
    monkeypatch.setattr(panel, "_autosave_layout", lambda: calls.append(1))
    panel._tabs.add_tab("Page2")
    panel._tabs.tabBar().tabMoved.emit(0, 1)
    assert len(calls) >= 1

def test_setMovable_in_source():
    from embeddebug.serial_station.ui.dashboard import DashboardTabs

    assert "setMovable(True)" in inspect.getsource(DashboardTabs.__init__)

def test_tab_moved_signal_in_source():
    from embeddebug.serial_station.ui.dashboard import DashboardTabs

    assert "tab_moved = pyqtSignal" in inspect.getsource(DashboardTabs)

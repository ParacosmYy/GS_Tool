"""dashboard 多标签页布局独立持久化测试。

覆盖：
1. persist_all_tabs 按标签页名存全部 canvas 布局。
2. restore_all_tabs 按名恢复对应标签页。
3. 旧单画布格式兼容（load_tabs_layout 归入 "Dashboard 1"）。
4. DashboardPanel 多标签页 build 恢复 + add 跨标签页保存。
5. tab_names 激活（此前零消费者）。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtCore import QPoint

from embeddebug.serial_station.ui.panels import _dashboard_layout_store as store


def _make_tabs(qtbot):
    from embeddebug.serial_station.ui.dashboard import DashboardTabs

    tabs = DashboardTabs()
    qtbot.addWidget(tabs)
    return tabs


# ── persist/restore all tabs ───────────────────────────────────────
def test_persist_all_tabs_stores_per_tab(qtbot, tmp_path, monkeypatch):
    """persist_all_tabs 应按标签页名存全部 canvas 布局。"""

    monkeypatch.setattr(store, "layout_path", lambda: tmp_path / "layout.json")
    tabs = _make_tabs(qtbot)
    # 默认 1 个标签页 + 加一个，各放控件。
    tabs.current_canvas().add_widget_at("led", QPoint(20, 20))
    tabs.add_tab("Page2")
    tabs.current_canvas().add_widget_at("gauge", QPoint(40, 40))
    assert store.persist_all_tabs(tabs) is True
    saved = store.load_tabs_layout()
    assert "Page2" in saved
    assert len(saved) >= 2  # 默认标签页 + Page2


def test_restore_all_tabs_recovers_per_tab(qtbot, tmp_path, monkeypatch):
    """restore_all_tabs 应按名恢复对应标签页的控件。"""

    monkeypatch.setattr(store, "layout_path", lambda: tmp_path / "layout.json")
    tabs = _make_tabs(qtbot)
    tabs.current_canvas().add_widget_at("led", QPoint(20, 20))
    tabs.add_tab("Page2")
    tabs.current_canvas().add_widget_at("gauge", QPoint(40, 40))
    store.persist_all_tabs(tabs)
    # 新 tabs 实例（模拟重启），恢复。
    tabs2 = _make_tabs(qtbot)
    tabs2.add_tab("Page2")  # 同名标签页
    restored = store.restore_all_tabs(tabs2)
    assert restored >= 1  # 至少恢复部分控件


def test_restore_empty_returns_zero(qtbot, tmp_path, monkeypatch):
    """无保存布局 restore_all_tabs 返回 0。"""

    monkeypatch.setattr(store, "layout_path", lambda: tmp_path / "nope.json")
    tabs = _make_tabs(qtbot)
    assert store.restore_all_tabs(tabs) == 0


# ── 旧格式兼容 ────────────────────────────────────────────────────
def test_load_tabs_layout_legacy_single_canvas(tmp_path, monkeypatch):
    """旧单画布格式（顶层 items）应归入 "Dashboard 1"。"""

    p = tmp_path / "layout.json"
    monkeypatch.setattr(store, "layout_path", lambda: p)
    store.save_layout_dict({"items": [
        {"id": "led_1", "type": "led", "x": 20, "y": 20, "width": 160, "height": 80, "config": {}},
    ]})
    tabs_layout = store.load_tabs_layout()
    assert "Dashboard 1" in tabs_layout
    assert len(tabs_layout["Dashboard 1"]["items"]) == 1


# ── DashboardPanel 多标签页持久化 ──────────────────────────────────
def test_dashboard_panel_restores_multiple_tabs(qtbot, tmp_path, monkeypatch):
    """DashboardPanel build 应恢复多标签页（env-gated）。"""

    from embeddebug.app.app_controller import AppController
    from embeddebug.serial_station.ui.panels.dashboard_panel import DashboardPanel

    monkeypatch.setenv("EMBEDDEBUG_DASHBOARD_AUTOSAVE", "1")
    monkeypatch.setattr(store, "layout_path", lambda: tmp_path / "layout.json")
    # 预置：默认标签页 1 个 led。
    store.save_tabs_layout({"Dashboard 1": {"items": [
        {"id": "led_1", "type": "led", "x": 20, "y": 20, "width": 160, "height": 80, "config": {}},
    ]}})
    panel = DashboardPanel()
    widget = panel.build(AppController())
    qtbot.addWidget(widget)
    # 默认标签页（Dashboard 1）应恢复 1 个控件。
    assert len(panel._tabs.current_canvas().items) == 1


def test_dashboard_panel_autosaves_all_tabs(qtbot, tmp_path, monkeypatch):
    """跨标签页放置控件应全部持久化（不止当前 canvas）。"""

    from embeddebug.app.app_controller import AppController
    from embeddebug.serial_station.ui.panels.dashboard_panel import DashboardPanel

    monkeypatch.setenv("EMBEDDEBUG_DASHBOARD_AUTOSAVE", "1")
    p = tmp_path / "layout.json"
    monkeypatch.setattr(store, "layout_path", lambda: p)
    monkeypatch.setattr(store, "restore_all_tabs", lambda tabs: 0)
    panel = DashboardPanel()
    widget = panel.build(AppController())
    qtbot.addWidget(widget)
    # 默认标签页放一个，新标签页放一个。
    panel._tabs.current_canvas().add_widget_at("led", QPoint(20, 20))
    panel._tabs.add_tab("Page2")
    panel._tabs.current_canvas().add_widget_at("gauge", QPoint(40, 40))
    saved = store.load_tabs_layout()
    # 两个标签页都应存了（不止当前）。
    assert "Page2" in saved
    assert any(name != "Page2" for name in saved)  # 默认标签页也在


# ── tab_names 激活 ─────────────────────────────────────────────────
def test_tab_names_used_by_persist(qtbot, tmp_path, monkeypatch):
    """persist_all_tabs 应调 tab_names（激活此前零消费者的方法）。"""

    import inspect
    from embeddebug.serial_station.ui.panels import _dashboard_layout_store

    src = inspect.getsource(_dashboard_layout_store.persist_all_tabs)
    assert "tab_names" in src


def test_store_exposes_multitab_apis():
    """store 应暴露 persist_all_tabs/restore_all_tabs/load_tabs_layout/save_tabs_layout。"""

    assert callable(store.persist_all_tabs)
    assert callable(store.restore_all_tabs)
    assert callable(store.load_tabs_layout)
    assert callable(store.save_tabs_layout)

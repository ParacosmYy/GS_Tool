"""仪表盘布局持久化 + 网格绘制合并测试。

源文件：
- test_dashboard_layout_persist.py（store 往返 / restore / persist / 自动保存）
- test_dashboard_canvas_grid.py（show_grid 切换 / paintEvent / 网格按钮）
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import pytest
from PyQt6.QtCore import QPoint

from embeddebug.serial_station.ui.panels import _dashboard_layout_store as store


# ── store 往返 ─────────────────────────────────────────────────────
def test_save_load_dict_roundtrip(tmp_path, monkeypatch):
    monkeypatch.setattr(store, "layout_path", lambda: tmp_path / "layout.json")
    layout = {"items": [{"id": "led_1", "type": "led", "x": 20, "y": 20,
                         "width": 160, "height": 80, "config": {}}]}
    assert store.save_layout_dict(layout) is True
    assert store.load_layout_dict() == layout


def test_load_missing_returns_empty(tmp_path, monkeypatch):
    monkeypatch.setattr(store, "layout_path", lambda: tmp_path / "nope.json")
    assert store.load_layout_dict() == {}


def test_load_corrupt_returns_empty(tmp_path, monkeypatch):
    p = tmp_path / "bad.json"
    p.write_text("{not valid json", encoding="utf-8")
    monkeypatch.setattr(store, "layout_path", lambda: p)
    assert store.load_layout_dict() == {}


# ── restore_to_canvas / persist_from_canvas ────────────────────────
def test_restore_to_canvas_restores_items(qtbot, tmp_path, monkeypatch):
    from embeddebug.serial_station.ui.dashboard import DashboardCanvas

    monkeypatch.setattr(store, "layout_path", lambda: tmp_path / "layout.json")
    store.save_layout_dict({"items": [
        {"id": "led_1", "type": "led", "x": 20, "y": 20, "width": 160, "height": 80, "config": {}},
        {"id": "gauge_1", "type": "gauge", "x": 60, "y": 60, "width": 160, "height": 80, "config": {}},
    ]})
    canvas = DashboardCanvas()
    qtbot.addWidget(canvas)
    assert store.restore_to_canvas(canvas) == 2
    assert len(canvas.items) == 2


def test_restore_empty_layout_returns_zero(qtbot, tmp_path, monkeypatch):
    from embeddebug.serial_station.ui.dashboard import DashboardCanvas

    monkeypatch.setattr(store, "layout_path", lambda: tmp_path / "layout.json")
    store.save_layout_dict({"items": []})
    canvas = DashboardCanvas()
    qtbot.addWidget(canvas)
    assert store.restore_to_canvas(canvas) == 0


def test_persist_from_canvas_writes_layout(qtbot, tmp_path, monkeypatch):
    from embeddebug.serial_station.ui.dashboard import DashboardCanvas

    p = tmp_path / "layout.json"
    monkeypatch.setattr(store, "layout_path", lambda: p)
    canvas = DashboardCanvas()
    qtbot.addWidget(canvas)
    canvas.add_widget_at("led", QPoint(20, 20))
    assert store.persist_from_canvas(canvas) is True
    loaded = store.load_layout_dict()
    assert len(loaded["items"]) == 1
    assert loaded["items"][0]["type"] == "led"


# ── DashboardPanel 自动持久化（env-gated） ─────────────────────────
def test_dashboard_panel_restores_on_build(qtbot, tmp_path, monkeypatch):
    from embeddebug.app.app_controller import AppController
    from embeddebug.serial_station.ui.panels.dashboard_panel import DashboardPanel

    monkeypatch.setenv("EMBEDDEBUG_DASHBOARD_AUTOSAVE", "1")
    monkeypatch.setattr(store, "layout_path", lambda: tmp_path / "layout.json")
    store.save_layout_dict({"items": [
        {"id": "led_1", "type": "led", "x": 20, "y": 20, "width": 160, "height": 80, "config": {}},
    ]})
    panel = DashboardPanel()
    widget = panel.build(AppController())
    qtbot.addWidget(widget)
    assert len(panel._tabs.current_canvas().items) == 1


def test_dashboard_panel_autosaves_on_add(qtbot, tmp_path, monkeypatch):
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
    loaded = store.load_layout_dict()
    all_items = sum(len(tab.get("items", [])) for tab in loaded.values() if isinstance(tab, dict))
    assert all_items >= 1


def test_autosave_disabled_by_default(qtbot, tmp_path, monkeypatch):
    from embeddebug.app.app_controller import AppController
    from embeddebug.serial_station.ui.panels.dashboard_panel import DashboardPanel

    monkeypatch.delenv("EMBEDDEBUG_DASHBOARD_AUTOSAVE", raising=False)
    p = tmp_path / "layout.json"
    monkeypatch.setattr(store, "layout_path", lambda: p)
    store.save_layout_dict({"items": [
        {"id": "led_1", "type": "led", "x": 20, "y": 20, "width": 160, "height": 80, "config": {}},
    ]})
    panel = DashboardPanel()
    widget = panel.build(AppController())
    qtbot.addWidget(widget)
    assert len(panel._tabs.current_canvas().items) == 0
    panel._tabs.current_canvas().add_widget_at("led", QPoint(20, 20))
    assert len(store.load_layout_dict()["items"]) == 1


@pytest.mark.skip(reason="settings 模块未落地（PRD-135/136），SettingKey 待恢复。")
def test_dashboard_layout_setting_key_exists():
    from embeddebug.serial_station.settings.keys import DEFAULTS, SettingKey

    assert hasattr(SettingKey, "DASHBOARD_LAYOUT")
    assert SettingKey.DASHBOARD_LAYOUT.value in DEFAULTS
    assert DEFAULTS[SettingKey.DASHBOARD_LAYOUT.value] == {}


# ── DashboardCanvas 网格绘制 ──────────────────────────────────────
def _make_canvas(qtbot):
    from embeddebug.serial_station.ui.dashboard import DashboardCanvas

    canvas = DashboardCanvas()
    qtbot.addWidget(canvas)
    return canvas


def test_canvas_default_show_grid(qtbot):
    assert _make_canvas(qtbot).show_grid is True


def test_set_show_grid_false(qtbot):
    canvas = _make_canvas(qtbot)
    canvas.set_show_grid(False)
    assert canvas.show_grid is False


def test_set_show_grid_toggle(qtbot):
    canvas = _make_canvas(qtbot)
    canvas.set_show_grid(False)
    assert canvas.show_grid is False
    canvas.set_show_grid(True)
    assert canvas.show_grid is True


def test_paint_event_grid_on_no_crash(qtbot):
    from PyQt6.QtGui import QPaintEvent
    from PyQt6.QtCore import QRect

    canvas = _make_canvas(qtbot)
    canvas.resize(200, 200)
    canvas.set_show_grid(True)
    canvas.paintEvent(QPaintEvent(QRect(0, 0, 200, 200)))


def test_paint_event_grid_off_no_crash(qtbot):
    from PyQt6.QtGui import QPaintEvent
    from PyQt6.QtCore import QRect

    canvas = _make_canvas(qtbot)
    canvas.resize(200, 200)
    canvas.set_show_grid(False)
    canvas.paintEvent(QPaintEvent(QRect(0, 0, 200, 200)))


def test_panel_has_grid_button(qtbot):
    from PyQt6.QtWidgets import QPushButton
    from embeddebug.app.app_controller import AppController
    from embeddebug.serial_station.ui.panels.dashboard_panel import DashboardPanel

    panel = DashboardPanel()
    widget = panel.build(AppController())
    qtbot.addWidget(widget)
    btn = widget.findChild(QPushButton, "serialStationDashboardGridButton")
    assert btn is not None
    assert btn.isCheckable()
    assert btn.isChecked()


def test_make_grid_toggle_toggles_canvas(qtbot):
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


def test_qss_covers_grid_button():
    from embeddebug.serial_station.ui.theme.qss_builder import build_qss

    assert "#serialStationDashboardGridButton" in build_qss()

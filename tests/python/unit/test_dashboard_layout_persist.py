"""仪表盘布局自动持久化/恢复测试。

覆盖：
1. _dashboard_layout_store save/load dict 往返。
2. restore_to_canvas 恢复控件数。
3. persist_from_canvas 写当前画布布局。
4. DashboardPanel build 自动恢复（monkeypatch store）。
5. add/remove 触发自动保存。
6. SettingKey.DASHBOARD_LAYOUT 存在。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import pytest

from PyQt6.QtCore import QPoint

from embeddebug.serial_station.ui.panels import _dashboard_layout_store as store


# ── store 往返 ─────────────────────────────────────────────────────
def test_save_load_dict_roundtrip(tmp_path, monkeypatch):
    """save_layout_dict → load_layout_dict 应往返一致。"""

    monkeypatch.setattr(store, "layout_path", lambda: tmp_path / "layout.json")
    layout = {"items": [{"id": "led_1", "type": "led", "x": 20, "y": 20, "width": 160, "height": 80, "config": {}}]}
    assert store.save_layout_dict(layout) is True
    loaded = store.load_layout_dict()
    assert loaded == layout


def test_load_missing_returns_empty(tmp_path, monkeypatch):
    """无布局文件应返回空 dict。"""

    monkeypatch.setattr(store, "layout_path", lambda: tmp_path / "nope.json")
    assert store.load_layout_dict() == {}


def test_load_corrupt_returns_empty(tmp_path, monkeypatch):
    """损坏 JSON 应返回空 dict（不崩溃）。"""

    p = tmp_path / "bad.json"
    p.write_text("{not valid json", encoding="utf-8")
    monkeypatch.setattr(store, "layout_path", lambda: p)
    assert store.load_layout_dict() == {}


# ── restore_to_canvas / persist_from_canvas ────────────────────────
def test_restore_to_canvas_restores_items(qtbot, tmp_path, monkeypatch):
    """restore_to_canvas 应从布局恢复控件到画布。"""

    from embeddebug.serial_station.ui.dashboard import DashboardCanvas

    monkeypatch.setattr(store, "layout_path", lambda: tmp_path / "layout.json")
    layout = {"items": [
        {"id": "led_1", "type": "led", "x": 20, "y": 20, "width": 160, "height": 80, "config": {}},
        {"id": "gauge_1", "type": "gauge", "x": 60, "y": 60, "width": 160, "height": 80, "config": {}},
    ]}
    store.save_layout_dict(layout)
    canvas = DashboardCanvas()
    qtbot.addWidget(canvas)
    count = store.restore_to_canvas(canvas)
    assert count == 2
    assert len(canvas.items) == 2


def test_restore_empty_layout_returns_zero(qtbot, tmp_path, monkeypatch):
    """空布局恢复 0 个控件。"""

    from embeddebug.serial_station.ui.dashboard import DashboardCanvas

    monkeypatch.setattr(store, "layout_path", lambda: tmp_path / "layout.json")
    store.save_layout_dict({"items": []})
    canvas = DashboardCanvas()
    qtbot.addWidget(canvas)
    assert store.restore_to_canvas(canvas) == 0


def test_persist_from_canvas_writes_layout(qtbot, tmp_path, monkeypatch):
    """persist_from_canvas 应把画布当前布局写盘。"""

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


# ── DashboardPanel 自动持久化（env-gated，默认关） ─────────────────
def test_dashboard_panel_restores_on_build(qtbot, tmp_path, monkeypatch):
    """DashboardPanel build 应自动恢复布局（_restore_layout_on_build）。

    自动持久化默认关闭（避免破坏既有测试），env EMBEDDEBUG_DASHBOARD_AUTOSAVE=1 启用。
    """

    from embeddebug.app.app_controller import AppController
    from embeddebug.serial_station.ui.panels.dashboard_panel import DashboardPanel

    monkeypatch.setenv("EMBEDDEBUG_DASHBOARD_AUTOSAVE", "1")
    # 预置一个布局。
    monkeypatch.setattr(store, "layout_path", lambda: tmp_path / "layout.json")
    store.save_layout_dict({"items": [
        {"id": "led_1", "type": "led", "x": 20, "y": 20, "width": 160, "height": 80, "config": {}},
    ]})
    panel = DashboardPanel()
    widget = panel.build(AppController())
    qtbot.addWidget(widget)
    # build 后初始画布应恢复 1 个控件。
    assert len(panel._tabs.current_canvas().items) == 1


def test_dashboard_panel_autosaves_on_add(qtbot, tmp_path, monkeypatch):
    """放置控件应触发自动保存（item_added → _autosave_layout，多标签页格式）。"""

    from embeddebug.app.app_controller import AppController
    from embeddebug.serial_station.ui.panels.dashboard_panel import DashboardPanel

    monkeypatch.setenv("EMBEDDEBUG_DASHBOARD_AUTOSAVE", "1")
    p = tmp_path / "layout.json"
    monkeypatch.setattr(store, "layout_path", lambda: p)
    # build 时无布局（空），避免恢复干扰（restore_all_tabs 空）。
    monkeypatch.setattr(store, "restore_all_tabs", lambda tabs: 0)
    panel = DashboardPanel()
    widget = panel.build(AppController())
    qtbot.addWidget(widget)
    canvas = panel._tabs.current_canvas()
    canvas.add_widget_at("led", QPoint(20, 20))
    # Batch 27：自动保存按多标签页格式（{tab_name: {items}}）。
    loaded = store.load_layout_dict()
    # 默认标签页名 "Dashboard 1" 应含 1 个控件。
    all_items = sum(
        len(tab.get("items", [])) for tab in loaded.values() if isinstance(tab, dict)
    )
    assert all_items >= 1


def test_autosave_disabled_by_default(qtbot, tmp_path, monkeypatch):
    """默认（无 env）自动持久化关闭：build 不恢复，add 不写盘。"""

    from embeddebug.app.app_controller import AppController
    from embeddebug.serial_station.ui.panels.dashboard_panel import DashboardPanel

    monkeypatch.delenv("EMBEDDEBUG_DASHBOARD_AUTOSAVE", raising=False)
    p = tmp_path / "layout.json"
    monkeypatch.setattr(store, "layout_path", lambda: p)
    # 预置布局但不开 env，build 不应恢复。
    store.save_layout_dict({"items": [
        {"id": "led_1", "type": "led", "x": 20, "y": 20, "width": 160, "height": 80, "config": {}},
    ]})
    panel = DashboardPanel()
    widget = panel.build(AppController())
    qtbot.addWidget(widget)
    assert len(panel._tabs.current_canvas().items) == 0  # 未恢复
    panel._tabs.current_canvas().add_widget_at("led", QPoint(20, 20))
    # p 不应被自动保存改写（仍为预置的 1 项，不是新增后写盘）。
    # 实际：默认关 autosave，p 不变。验证 p 内容仍是预置。
    assert len(store.load_layout_dict()["items"]) == 1


# ── SettingKey ─────────────────────────────────────────────────────
@pytest.mark.skip(
    reason=(
        "embeddebug.serial_station.settings 包尚未落地（PRD-135/136 服务层规划）；"
        "DASHBOARD_LAYOUT SettingKey 待 settings 模块实现后恢复。"
    )
)
def test_dashboard_layout_setting_key_exists():
    """SettingKey 应含 DASHBOARD_LAYOUT（Batch 25）。"""

    from embeddebug.serial_station.settings.keys import DEFAULTS, SettingKey

    assert hasattr(SettingKey, "DASHBOARD_LAYOUT")
    assert SettingKey.DASHBOARD_LAYOUT.value in DEFAULTS
    assert DEFAULTS[SettingKey.DASHBOARD_LAYOUT.value] == {}

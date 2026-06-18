"""Batch 28 测试：dashboard 标签页双击重命名（激活 rename_tab 死代码）。

覆盖：
1. DashboardPanel 接 tabBarDoubleClicked → _rename_tab_on_double_click。
2. 确认重命名（monkeypatch QInputDialog 返回新名）→ rename_tab 调用 + 状态更新。
3. 取消/空名保持原名。
4. rename_tab 现有生产消费者（DashboardPanel，激活死代码）。
5. 源码接入断言。
"""

from __future__ import annotations

import inspect
import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from unittest.mock import MagicMock


def _make_panel(qtbot):
    from embeddebug.app.app_controller import AppController
    from embeddebug.serial_station.ui.panels.dashboard_panel import DashboardPanel

    panel = DashboardPanel()
    widget = panel.build(AppController())
    qtbot.addWidget(widget)
    return panel


# ── 接线 ───────────────────────────────────────────────────────────
def test_panel_wires_tab_bar_double_click():
    """DashboardPanel build 应接 tabBarDoubleClicked → _rename_tab_on_double_click。"""

    from embeddebug.serial_station.ui.panels.dashboard_panel import DashboardPanel

    src = inspect.getsource(DashboardPanel.build)
    assert "tabBarDoubleClicked" in src
    assert "_rename_tab_on_double_click" in src


def test_panel_has_rename_handler():
    """DashboardPanel 应有 _rename_tab_on_double_click 方法。"""

    from embeddebug.serial_station.ui.panels.dashboard_panel import DashboardPanel

    assert hasattr(DashboardPanel, "_rename_tab_on_double_click")


# ── 重命名行为 ─────────────────────────────────────────────────────
def test_rename_with_new_name(qtbot, monkeypatch):
    """确认新名应调 rename_tab 并更新状态。"""

    panel = _make_panel(qtbot)
    rename_calls: list = []
    monkeypatch.setattr(panel._tabs, "rename_tab", lambda idx, name: rename_calls.append((idx, name)))
    import PyQt6.QtWidgets as QtWidgets

    monkeypatch.setattr(
        QtWidgets.QInputDialog, "getText",
        staticmethod(lambda *a, **k: ("MyDashboard", True)),
    )
    panel._rename_tab_on_double_click(0)
    assert len(rename_calls) == 1
    assert rename_calls[0] == (0, "MyDashboard")
    assert "MyDashboard" in panel._status.text()


def test_rename_cancel_keeps_original(qtbot, monkeypatch):
    """取消（ok=False）不应调 rename_tab。"""

    panel = _make_panel(qtbot)
    rename_calls: list = []
    monkeypatch.setattr(panel._tabs, "rename_tab", lambda idx, name: rename_calls.append((idx, name)))
    import PyQt6.QtWidgets as QtWidgets

    monkeypatch.setattr(
        QtWidgets.QInputDialog, "getText",
        staticmethod(lambda *a, **k: ("ignored", False)),
    )
    panel._rename_tab_on_double_click(0)
    assert rename_calls == []


def test_rename_empty_keeps_original(qtbot, monkeypatch):
    """空名（ok=True 但空）不应调 rename_tab。"""

    panel = _make_panel(qtbot)
    rename_calls: list = []
    monkeypatch.setattr(panel._tabs, "rename_tab", lambda idx, name: rename_calls.append((idx, name)))
    import PyQt6.QtWidgets as QtWidgets

    monkeypatch.setattr(
        QtWidgets.QInputDialog, "getText",
        staticmethod(lambda *a, **k: ("   ", True)),
    )
    panel._rename_tab_on_double_click(0)
    assert rename_calls == []


def test_rename_same_name_skips(qtbot, monkeypatch):
    """与原名相同不应调 rename_tab（避免无谓重存）。"""

    panel = _make_panel(qtbot)
    rename_calls: list = []
    monkeypatch.setattr(panel._tabs, "rename_tab", lambda idx, name: rename_calls.append((idx, name)))
    current = panel._tabs.tabText(0)
    import PyQt6.QtWidgets as QtWidgets

    monkeypatch.setattr(
        QtWidgets.QInputDialog, "getText",
        staticmethod(lambda *a, **k: (current, True)),
    )
    panel._rename_tab_on_double_click(0)
    assert rename_calls == []


def test_rename_out_of_range_index_no_crash(qtbot, monkeypatch):
    """非法 index 不应崩溃。"""

    panel = _make_panel(qtbot)
    import PyQt6.QtWidgets as QtWidgets

    monkeypatch.setattr(
        QtWidgets.QInputDialog, "getText",
        staticmethod(lambda *a, **k: ("x", True)),
    )
    panel._rename_tab_on_double_click(999)  # 越界，不应抛异常


# ── rename_tab 激活 ────────────────────────────────────────────────
def test_rename_tab_now_has_production_consumer():
    """rename_tab 现应有生产消费者（DashboardPanel），不再是死代码。"""

    from pathlib import Path

    src = Path("python/embeddebug/serial_station/ui/panels/dashboard_panel.py").read_text(encoding="utf-8")
    assert "rename_tab" in src


def test_rename_tab_in_dashboard_tabs_still_present():
    """DashboardTabs.rename_tab 应保留（生产消费者激活，非删除）。"""

    from embeddebug.serial_station.ui.dashboard import DashboardTabs

    assert hasattr(DashboardTabs, "rename_tab")
    assert callable(DashboardTabs.rename_tab)

"""Batch 30 测试：dashboard 标签页拖拽重排序（setMovable + tab_moved）。

覆盖：
1. DashboardTabs setMovable(True)（标签页可拖拽）。
2. tab_moved 信号在拖拽时发出（from, to）。
3. DashboardPanel 接 tab_moved → _autosave_layout（自动重存）。
4. 源码接入断言。
"""

from __future__ import annotations

import inspect
import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")


def test_dashboard_tabs_is_movable(qtbot):
    """DashboardTabs 应 setMovable(True)（Batch 30 标签页可拖拽重排序）。"""

    from embeddebug.serial_station.ui.dashboard import DashboardTabs

    tabs = DashboardTabs()
    qtbot.addWidget(tabs)
    assert tabs.isMovable() is True


def test_dashboard_tabs_has_tab_moved_signal(qtbot):
    """DashboardTabs 应有 tab_moved 信号（Batch 30）。"""

    from embeddebug.serial_station.ui.dashboard import DashboardTabs

    tabs = DashboardTabs()
    qtbot.addWidget(tabs)
    received: list = []
    tabs.tab_moved.connect(lambda frm, to: received.append((frm, to)))
    # 直接触发底层 tabBar 的 tabMoved（模拟拖拽完成）。
    tabs.tabBar().tabMoved.emit(0, 1)
    assert received == [(0, 1)]


def test_dashboard_panel_wires_tab_moved_to_autosave():
    """DashboardPanel build 应接 tab_moved → _autosave_layout（源码断言）。"""

    from embeddebug.serial_station.ui.panels.dashboard_panel import DashboardPanel

    src = inspect.getsource(DashboardPanel.build)
    assert "tab_moved" in src
    assert "_autosave_layout" in src


def test_dashboard_panel_autosaves_on_tab_move(qtbot, monkeypatch):
    """标签页拖拽重排序应触发 _autosave_layout（tab 顺序是持久化 key 顺序）。"""

    from embeddebug.app.app_controller import AppController
    from embeddebug.serial_station.ui.panels.dashboard_panel import DashboardPanel

    panel = DashboardPanel()
    widget = panel.build(AppController())
    qtbot.addWidget(widget)
    autosave_calls: list = []
    monkeypatch.setattr(panel, "_autosave_layout", lambda: autosave_calls.append(1))
    # 加第二个标签页后触发 move（0→1）。
    panel._tabs.add_tab("Page2")
    panel._tabs.tabBar().tabMoved.emit(0, 1)
    assert len(autosave_calls) >= 1


def test_setMovable_in_source():
    """DashboardTabs 源码应含 setMovable(True)（Batch 30）。"""

    from embeddebug.serial_station.ui.dashboard import DashboardTabs

    src = inspect.getsource(DashboardTabs.__init__)
    assert "setMovable(True)" in src


def test_tab_moved_signal_in_source():
    """DashboardTabs 源码应含 tab_moved 信号定义。"""

    from embeddebug.serial_station.ui.dashboard import DashboardTabs

    src = inspect.getsource(DashboardTabs)
    assert "tab_moved = pyqtSignal" in src

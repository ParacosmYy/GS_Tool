"""dashboard 标签页右键菜单测试（重命名/复制/关闭）。

覆盖：
1. dashboard_panel 接 customContextMenuRequested → _show_tab_context_menu。
2. duplicate_tab 克隆源画布全部控件到新标签页。
3. close_tab_by_index 关闭标签页（至少保留一个）。
4. close_tab_by_index 单标签页时禁用（保留一个）。
5. _show_tab_context_menu 未点中标签页不弹菜单。
6. 源码接入断言。
"""

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


# ── 接线 ───────────────────────────────────────────────────────────
def test_panel_wires_context_menu():
    """DashboardPanel build 应接 customContextMenuRequested → _show_tab_context_menu。"""

    from embeddebug.serial_station.ui.panels.dashboard_panel import DashboardPanel

    src = inspect.getsource(DashboardPanel.build)
    assert "customContextMenuRequested" in src
    assert "_show_tab_context_menu" in src


def test_panel_has_show_tab_context_menu():
    """DashboardPanel 应有 _show_tab_context_menu 委托方法。"""

    from embeddebug.serial_station.ui.panels.dashboard_panel import DashboardPanel

    assert hasattr(DashboardPanel, "_show_tab_context_menu")


# ── duplicate_tab ──────────────────────────────────────────────────
def test_duplicate_tab_clones_widgets(qtbot):
    """duplicate_tab 应克隆源画布全部控件到新标签页。"""

    panel = _make_panel(qtbot)
    src = panel._tabs.current_canvas()
    src.add_widget_at("led", QPoint(20, 20))
    src.add_widget_at("gauge", QPoint(40, 40))
    before_tabs = panel._tabs.count()
    menu_mod.duplicate_tab(panel, 0)
    assert panel._tabs.count() == before_tabs + 1
    # 新标签页（最后）应含 2 个克隆控件。
    new_canvas = panel._tabs.widget(panel._tabs.count() - 1)
    assert len(new_canvas.items) == 2


def test_duplicate_tab_out_of_range_no_crash(qtbot):
    """越界 index 不应崩溃。"""

    panel = _make_panel(qtbot)
    menu_mod.duplicate_tab(panel, 999)  # 不应抛异常
    # 标签页数不变。
    assert panel._tabs.count() == 1


# ── close_tab_by_index ─────────────────────────────────────────────
def test_close_tab_by_index_removes(qtbot):
    """多标签页时 close_tab_by_index 应关闭指定页。"""

    panel = _make_panel(qtbot)
    panel._tabs.add_tab("Page2")
    assert panel._tabs.count() == 2
    menu_mod.close_tab_by_index(panel, 0)
    assert panel._tabs.count() == 1


def test_close_tab_keeps_at_least_one(qtbot):
    """单标签页时 close_tab_by_index 应保留（至少一个）。"""

    panel = _make_panel(qtbot)
    assert panel._tabs.count() == 1
    menu_mod.close_tab_by_index(panel, 0)
    assert panel._tabs.count() == 1  # 未关闭


# ── show_tab_context_menu（不实际 exec，验证不崩溃） ────────────────
def test_show_tab_context_menu_miss_no_crash(qtbot, monkeypatch):
    """右键未点中标签页（tabAt 返回 -1）不应弹菜单/崩溃。"""

    panel = _make_panel(qtbot)
    # monkeypatch tabAt 返回 -1（未点中）。
    bar = panel._tabs.tabBar()
    monkeypatch.setattr(bar, "tabAt", lambda _pos: -1)
    # monkeypatch QMenu.exec 防止实际弹窗。
    import PyQt6.QtWidgets as QtWidgets

    exec_calls: list = []
    monkeypatch.setattr(QtWidgets.QMenu, "exec", lambda *a, **k: exec_calls.append(1))
    menu_mod.show_tab_context_menu(panel, QPoint(0, 0))
    # 未点中 → 不应 exec 菜单。
    assert exec_calls == []


def test_show_tab_context_menu_hit_builds_menu(qtbot, monkeypatch):
    """右键点中标签页应构建并 exec 菜单（3 个 action）。"""

    panel = _make_panel(qtbot)
    bar = panel._tabs.tabBar()
    monkeypatch.setattr(bar, "tabAt", lambda _pos: 0)
    import PyQt6.QtWidgets as QtWidgets

    built_menus: list = []
    original_init = QtWidgets.QMenu.__init__

    def _init(self, *a, **k):
        original_init(self, *a, **k)
        built_menus.append(self)

    monkeypatch.setattr(QtWidgets.QMenu, "__init__", _init)
    monkeypatch.setattr(QtWidgets.QMenu, "exec", lambda *a, **k: None)
    menu_mod.show_tab_context_menu(panel, QPoint(10, 10))
    assert len(built_menus) == 1
    # 菜单应含 3 个 action（重命名/复制/关闭）。
    assert built_menus[0].actions().__len__() == 3


# ── 模块存在性 ─────────────────────────────────────────────────────
def test_menu_module_exposes_helpers():
    """_dashboard_tab_menu 应暴露 show_tab_context_menu/duplicate_tab/close_tab_by_index。"""

    assert callable(menu_mod.show_tab_context_menu)
    assert callable(menu_mod.duplicate_tab)
    assert callable(menu_mod.close_tab_by_index)

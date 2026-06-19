"""dashboard 放置控件右键删除菜单测试。

覆盖：
1. attach_widget_delete_menu 给控件装 customContextMenu（删除项可触发 remove_item）。
2. 删除动作实际调 canvas.remove_item（控件从 items 移除）。
3. _safe_remove 失败静默（不崩溃）。
4. DashboardPanel 放置控件后接了删除菜单（源码断言）。
5. 模块存在性。
"""

from __future__ import annotations

import inspect
import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtCore import QPoint

from embeddebug.serial_station.ui.panels import _dashboard_widget_menu as wm


def _make_canvas_with_widget(qtbot, widget_type="led"):
    from embeddebug.serial_station.ui.dashboard import DashboardCanvas

    canvas = DashboardCanvas()
    qtbot.addWidget(canvas)
    item_id = canvas.add_widget_at(widget_type, QPoint(20, 20))
    item = canvas.items[item_id]
    return canvas, item.widget, item_id


# ── attach_widget_delete_menu ──────────────────────────────────────
def test_attach_sets_custom_context_policy(qtbot):
    """装菜单后控件 ContextMenuPolicy 应为 CustomContextMenu。"""

    canvas, widget, item_id = _make_canvas_with_widget(qtbot)
    wm.attach_widget_delete_menu(widget, canvas, item_id)
    from PyQt6.QtCore import Qt

    assert widget.contextMenuPolicy() == Qt.ContextMenuPolicy.CustomContextMenu


def test_delete_action_removes_item(qtbot, monkeypatch):
    """删除菜单动作应调 canvas.remove_item（控件从 items 移除）。"""

    canvas, widget, item_id = _make_canvas_with_widget(qtbot)
    wm.attach_widget_delete_menu(widget, canvas, item_id)
    assert item_id in canvas.items
    # 直接触发底层删除（_safe_remove）。
    wm._safe_remove(canvas, item_id)
    assert item_id not in canvas.items


def test_safe_remove_missing_id_no_crash(qtbot):
    """删除不存在的 item_id 不崩溃（静默）。"""

    from embeddebug.serial_station.ui.dashboard import DashboardCanvas

    canvas = DashboardCanvas()
    qtbot.addWidget(canvas)
    wm._safe_remove(canvas, "nonexistent_id")  # 不应抛异常


def test_safe_remove_invalid_canvas_no_crash(qtbot):
    """canvas 为 None 不崩溃。"""

    wm._safe_remove(None, "x")  # 不应抛异常


# ── attach_widget_delete_menu None 安全 ────────────────────────────
def test_attach_none_widget_no_crash(qtbot):
    """widget/canvas 为 None 不崩溃。"""

    assert wm.attach_widget_delete_menu(None, None, "x") is None


# ── DashboardPanel 接入断言 ────────────────────────────────────────
def test_dashboard_panel_wires_widget_delete_menu():
    """DashboardPanel._on_item_added_fullscreen 应调 attach_widget_delete_menu。"""

    from embeddebug.serial_station.ui.panels.dashboard_panel import DashboardPanel

    src = inspect.getsource(DashboardPanel._on_item_added_fullscreen)
    assert "attach_widget_delete_menu" in src


def test_widget_menu_module_exposes_helper():
    """_dashboard_widget_menu 应暴露 attach_widget_delete_menu。"""

    assert callable(wm.attach_widget_delete_menu)

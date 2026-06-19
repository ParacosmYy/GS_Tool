"""dashboard 放置控件右键「复制控件」测试。

覆盖：
1. _safe_duplicate 在偏移位置克隆同类型控件（items 数 +1）。
2. 复制的控件类型与源一致。
3. _safe_duplicate 源已删（item_id 不存在）静默不崩。
4. _safe_duplicate None canvas 不崩。
5. 右键菜单现含 2 个 action（复制控件/删除控件）。
6. 源码接入断言。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtCore import QPoint

from embeddebug.serial_station.ui.panels import _dashboard_widget_menu as wm


def _make_canvas_with_widget(qtbot, widget_type="gauge"):
    from embeddebug.serial_station.ui.dashboard import DashboardCanvas

    canvas = DashboardCanvas()
    qtbot.addWidget(canvas)
    item_id = canvas.add_widget_at(widget_type, QPoint(40, 60))
    return canvas, item_id


# ── _safe_duplicate ────────────────────────────────────────────────
def test_duplicate_adds_clone(qtbot):
    """_safe_duplicate 应在偏移位置克隆同类型控件（items 数 +1）。"""

    canvas, item_id = _make_canvas_with_widget(qtbot, "gauge")
    before = len(canvas.items)
    wm._safe_duplicate(canvas, item_id)
    assert len(canvas.items) == before + 1


def test_duplicate_preserves_type(qtbot):
    """复制出的控件类型应与源一致。"""

    canvas, item_id = _make_canvas_with_widget(qtbot, "led")
    wm._safe_duplicate(canvas, item_id)
    types = [item.widget_type for item in canvas.items.values()]
    assert types.count("led") == 2  # 源 + 克隆


def test_duplicate_missing_id_no_crash(qtbot):
    """源 item_id 不存在应静默不崩。"""

    from embeddebug.serial_station.ui.dashboard import DashboardCanvas

    canvas = DashboardCanvas()
    qtbot.addWidget(canvas)
    wm._safe_duplicate(canvas, "nonexistent")  # 不应抛异常
    assert len(canvas.items) == 0


def test_duplicate_none_canvas_no_crash(qtbot):
    """canvas 为 None 不崩。"""

    wm._safe_duplicate(None, "x")  # 不应抛异常


def test_duplicate_offset_applied(qtbot):
    """复制控件应在源位置偏移 _DUPLICATE_OFFSET 处（避免完全重叠）。"""

    canvas, item_id = _make_canvas_with_widget(qtbot, "led")
    src = canvas.items[item_id]
    wm._safe_duplicate(canvas, item_id)
    # 找克隆（同类型，位置 = 源 + offset）。
    clones = [
        item for item in canvas.items.values()
        if item.widget_type == src.widget_type and item.geometry != src.geometry
    ]
    assert len(clones) == 1
    clone = clones[0]
    assert clone.geometry.x() == src.geometry.x() + wm._DUPLICATE_OFFSET
    assert clone.geometry.y() == src.geometry.y() + wm._DUPLICATE_OFFSET


# ── 菜单 action 数（Batch 33 加「属性...」后共 3 个） ──────────────
def test_duplicate_menu_has_three_actions(qtbot, monkeypatch):
    """右键菜单应含 3 个 action（复制控件/属性.../删除控件）。

    Batch 32 时为 2 个（复制/删除），Batch 33 加属性编辑后为 3 个。
    """

    canvas, item_id = _make_canvas_with_widget(qtbot, "led")
    widget = canvas.items[item_id].widget
    wm.attach_widget_delete_menu(widget, canvas, item_id)
    import PyQt6.QtWidgets as QtWidgets

    built: list = []
    original_init = QtWidgets.QMenu.__init__

    def _init(self, *a, **k):
        original_init(self, *a, **k)
        built.append(self)

    monkeypatch.setattr(QtWidgets.QMenu, "__init__", _init)
    monkeypatch.setattr(QtWidgets.QMenu, "exec", lambda *a, **k: None)
    widget.customContextMenuRequested.emit(QPoint(5, 5))
    assert len(built) == 1
    # Batch 38 后菜单 6 action（复制/属性/置顶/置底/锁定/删除）。
    assert built[0].actions().__len__() == 6


# ── 源码接入断言 ───────────────────────────────────────────────────
def test_widget_menu_has_duplicate_helper():
    """_dashboard_widget_menu 应含 _safe_duplicate。"""

    import inspect
    from embeddebug.serial_station.ui.panels import _dashboard_widget_menu

    src = inspect.getsource(_dashboard_widget_menu)
    assert "_safe_duplicate" in src
    assert "复制控件" in src

"""dashboard 控件右键「属性编辑」测试（宽/高调整）。

覆盖：
1. _edit_properties 调整宽/高 → widget.setGeometry + item.geometry/config 同步。
2. 取消（ok=False）不改变几何。
3. item 不存在（None）静默不崩。
4. 右键菜单现含 3 个 action（复制控件/属性.../删除控件）。
5. 源码接入断言。
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
    return canvas, item_id


# ── _edit_properties ───────────────────────────────────────────────
def test_edit_properties_resizes(qtbot, monkeypatch):
    """属性编辑应同步 widget.setGeometry + item.geometry/config（宽 200/高 120）。"""

    canvas, item_id = _make_canvas_with_widget(qtbot, "gauge")
    widget = canvas.items[item_id].widget
    import PyQt6.QtWidgets as QtWidgets

    # getInt 连续两次返回（200, True）/（120, True）。
    values = iter([200, 120])
    monkeypatch.setattr(
        QtWidgets.QInputDialog, "getInt",
        staticmethod(lambda *a, **k: (next(values), True)),
    )
    wm._edit_properties(widget, canvas, item_id)
    item = canvas.items[item_id]
    assert item.geometry.width() == 200
    assert item.geometry.height() == 120
    assert widget.geometry().width() == 200
    assert widget.geometry().height() == 120
    assert item.config["width"] == 200
    assert item.config["height"] == 120


def test_edit_properties_cancel_keeps_geometry(qtbot, monkeypatch):
    """取消（ok=False）应保持原几何不变。"""

    canvas, item_id = _make_canvas_with_widget(qtbot, "led")
    widget = canvas.items[item_id].widget
    before = canvas.items[item_id].geometry
    import PyQt6.QtWidgets as QtWidgets

    monkeypatch.setattr(
        QtWidgets.QInputDialog, "getInt",
        staticmethod(lambda *a, **k: (999, False)),
    )
    wm._edit_properties(widget, canvas, item_id)
    assert canvas.items[item_id].geometry == before  # 未变


def test_edit_properties_missing_item_no_crash(qtbot, monkeypatch):
    """item_id 不存在应静默不崩（不弹对话框的副作用）。"""

    canvas, item_id = _make_canvas_with_widget(qtbot, "led")
    widget = canvas.items[item_id].widget
    import PyQt6.QtWidgets as QtWidgets

    called: list = []
    monkeypatch.setattr(
        QtWidgets.QInputDialog, "getInt",
        staticmethod(lambda *a, **k: (called.append(1) or (1, True))),
    )
    wm._edit_properties(widget, canvas, "nonexistent")
    # item 不存在应早返回，getInt 不被调用。
    assert called == []


# ── 菜单含 3 action ────────────────────────────────────────────────
def test_properties_menu_has_three_actions(qtbot, monkeypatch):
    """右键菜单应含 3 个 action（复制控件/属性.../删除控件）。"""

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
    assert built[0].actions().__len__() == 3


# ── 源码接入断言 ───────────────────────────────────────────────────
def test_widget_menu_has_edit_properties_helper():
    """_dashboard_widget_menu 应含 _edit_properties + 属性... action。"""

    from embeddebug.serial_station.ui.panels import _dashboard_widget_menu

    src = inspect.getsource(_dashboard_widget_menu)
    assert "_edit_properties" in src
    assert "属性..." in src

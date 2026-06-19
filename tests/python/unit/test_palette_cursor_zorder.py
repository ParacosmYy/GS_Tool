"""Batch 37 测试：WidgetPaletteButton 手型光标 + 控件 z-order 置顶/置底。

覆盖：
1. WidgetPaletteButton 默认 PointingHandCursor（拖拽可发现性）。
2. _safe_raise 调 widget.raise_（z-order 置顶）。
3. _safe_lower 调 widget.lower（z-order 置底）。
4. _safe_raise/_safe_lower None 安全。
5. 右键菜单含「置顶」「置底」action（5 action 总）。
6. 源码接入断言。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtCore import QPoint, Qt

from embeddebug.serial_station.ui.panels import _dashboard_widget_menu as wm


def _make_button(qtbot):
    from embeddebug.serial_station.ui.dashboard.palette import WidgetPaletteButton

    btn = WidgetPaletteButton("led", "LED", "circle")
    qtbot.addWidget(btn)
    return btn


def _make_canvas_with_widget(qtbot, widget_type="led"):
    from embeddebug.serial_station.ui.dashboard import DashboardCanvas

    canvas = DashboardCanvas()
    qtbot.addWidget(canvas)
    item_id = canvas.add_widget_at(widget_type, QPoint(20, 20))
    return canvas, item_id


# ── WidgetPaletteButton 手型光标 ───────────────────────────────────
def test_palette_button_has_pointing_hand_cursor(qtbot):
    """WidgetPaletteButton 应默认 PointingHandCursor（拖拽可发现性）。"""

    btn = _make_button(qtbot)
    assert btn.cursor().shape() == Qt.CursorShape.PointingHandCursor


# ── _safe_raise / _safe_lower ──────────────────────────────────────
def test_safe_raise_calls_widget_raise(qtbot, monkeypatch):
    """_safe_raise 应调 widget.raise_（置顶）。"""

    canvas, item_id = _make_canvas_with_widget(qtbot, "gauge")
    widget = canvas.items[item_id].widget
    raise_calls: list = []
    monkeypatch.setattr(widget, "raise_", lambda: raise_calls.append(1))
    wm._safe_raise(widget)
    assert len(raise_calls) == 1


def test_safe_lower_calls_widget_lower(qtbot, monkeypatch):
    """_safe_lower 应调 widget.lower（置底）。"""

    canvas, item_id = _make_canvas_with_widget(qtbot, "gauge")
    widget = canvas.items[item_id].widget
    lower_calls: list = []
    monkeypatch.setattr(widget, "lower", lambda: lower_calls.append(1))
    wm._safe_lower(widget, canvas)
    assert len(lower_calls) == 1


def test_safe_raise_none_no_crash(qtbot):
    """_safe_raise None widget 不崩。"""

    wm._safe_raise(None)  # 不应抛异常


def test_safe_lower_none_no_crash(qtbot):
    """_safe_lower None widget 不崩。"""

    wm._safe_lower(None, None)  # 不应抛异常


# ── 菜单含置顶/置底 ────────────────────────────────────────────────
def test_menu_has_raise_lower_actions(qtbot, monkeypatch):
    """右键菜单应含「置顶」「置底」action（共 5 action）。"""

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
    texts = [a.text() for a in built[0].actions()]
    assert "置顶" in texts
    assert "置底" in texts
    assert len(texts) == 5  # 复制/属性/置顶/置底/删除


# ── 源码接入断言 ───────────────────────────────────────────────────
def test_widget_menu_has_raise_lower_helpers():
    """_dashboard_widget_menu 应含 _safe_raise / _safe_lower。"""

    import inspect
    from embeddebug.serial_station.ui.panels import _dashboard_widget_menu

    src = inspect.getsource(_dashboard_widget_menu)
    assert "_safe_raise" in src
    assert "_safe_lower" in src
    assert "置顶" in src
    assert "置底" in src

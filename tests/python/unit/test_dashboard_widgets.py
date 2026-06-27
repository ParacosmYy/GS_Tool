"""test_dashboard_widgets part 2 — auto-split from merged file（Batch 47 测试精简）。"""
from __future__ import annotations
import os
os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")
import inspect
from PyQt6.QtCore import QPoint, Qt
from embeddebug.serial_station.ui.panels import _dashboard_widget_menu as wm
def _make_canvas_with_widget(qtbot, widget_type="led", pos=None):
    from embeddebug.serial_station.ui.dashboard import DashboardCanvas
    canvas = DashboardCanvas()
    qtbot.addWidget(canvas)
    item_id = canvas.add_widget_at(widget_type, pos or QPoint(20, 20))
    return canvas, item_id
def test_edit_properties_resizes_and_moves(qtbot, monkeypatch):
    canvas, item_id = _make_canvas_with_widget(qtbot, "gauge")
    widget = canvas.items[item_id].widget
    import PyQt6.QtWidgets as QtWidgets
    values = iter([80, 60, 200, 120])
    monkeypatch.setattr(QtWidgets.QInputDialog, "getInt",
                        staticmethod(lambda *a, **k: (next(values), True)))
    wm._edit_properties(widget, canvas, item_id)
    item = canvas.items[item_id]
    assert item.geometry.x() == 80
    assert item.geometry.y() == 60
    assert item.geometry.width() == 200
    assert item.geometry.height() == 120
    assert widget.geometry().x() == 80
    assert widget.geometry().width() == 200
    assert widget.geometry().height() == 120
    assert item.config["width"] == 200
    assert item.config["height"] == 120
def test_edit_properties_cancel_keeps_geometry(qtbot, monkeypatch):
    canvas, item_id = _make_canvas_with_widget(qtbot, "led")
    widget = canvas.items[item_id].widget
    before = canvas.items[item_id].geometry
    import PyQt6.QtWidgets as QtWidgets
    monkeypatch.setattr(QtWidgets.QInputDialog, "getInt",
                        staticmethod(lambda *a, **k: (999, False)))
    wm._edit_properties(widget, canvas, item_id)
    assert canvas.items[item_id].geometry == before
def test_edit_properties_snaps_position_to_grid(qtbot, monkeypatch):
    canvas, item_id = _make_canvas_with_widget(qtbot, "led")
    widget = canvas.items[item_id].widget
    import PyQt6.QtWidgets as QtWidgets
    values = iter([83, 57, 160, 80])
    monkeypatch.setattr(QtWidgets.QInputDialog, "getInt",
                        staticmethod(lambda *a, **k: (next(values), True)))
    wm._edit_properties(widget, canvas, item_id)
    item = canvas.items[item_id]
    assert item.geometry.x() == 80
    assert item.geometry.y() == 60
def test_edit_properties_missing_item_no_crash(qtbot, monkeypatch):
    canvas, item_id = _make_canvas_with_widget(qtbot, "led")
    widget = canvas.items[item_id].widget
    import PyQt6.QtWidgets as QtWidgets
    called: list = []
    monkeypatch.setattr(QtWidgets.QInputDialog, "getInt",
                        staticmethod(lambda *a, **k: (called.append(1) or (1, True))))
    wm._edit_properties(widget, canvas, "nonexistent")
    assert called == []
def test_properties_menu_has_six_actions(qtbot, monkeypatch):
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
    assert built[0].actions().__len__() == 6
def test_widget_menu_has_edit_properties_helper():
    from embeddebug.serial_station.ui.panels import _dashboard_widget_menu
    src = inspect.getsource(_dashboard_widget_menu)
    assert "_edit_properties" in src
    assert "属性..." in src
def test_duplicate_adds_clone(qtbot):
    canvas, item_id = _make_canvas_with_widget(qtbot, "gauge", QPoint(40, 60))
    before = len(canvas.items)
    wm._safe_duplicate(canvas, item_id)
    assert len(canvas.items) == before + 1
def test_duplicate_preserves_type(qtbot):
    canvas, item_id = _make_canvas_with_widget(qtbot, "led", QPoint(40, 60))
    wm._safe_duplicate(canvas, item_id)
    types = [item.widget_type for item in canvas.items.values()]
    assert types.count("led") == 2
def test_duplicate_missing_id_no_crash(qtbot):
    from embeddebug.serial_station.ui.dashboard import DashboardCanvas
    canvas = DashboardCanvas()
    qtbot.addWidget(canvas)
    wm._safe_duplicate(canvas, "nonexistent")
    assert len(canvas.items) == 0
def test_duplicate_none_canvas_no_crash(qtbot):
    wm._safe_duplicate(None, "x")
def test_duplicate_offset_applied(qtbot):
    canvas, item_id = _make_canvas_with_widget(qtbot, "led", QPoint(40, 60))
    src = canvas.items[item_id]
    wm._safe_duplicate(canvas, item_id)
    clones = [item for item in canvas.items.values()
              if item.widget_type == src.widget_type and item.geometry != src.geometry]
    assert len(clones) == 1
    assert clones[0].geometry.x() == src.geometry.x() + wm._DUPLICATE_OFFSET
    assert clones[0].geometry.y() == src.geometry.y() + wm._DUPLICATE_OFFSET
def test_widget_menu_has_duplicate_helper():
    from embeddebug.serial_station.ui.panels import _dashboard_widget_menu
    src = inspect.getsource(_dashboard_widget_menu)
    assert "_safe_duplicate" in src
    assert "复制控件" in src
def test_attach_sets_custom_context_policy(qtbot):
    canvas, item_id = _make_canvas_with_widget(qtbot)
    widget = canvas.items[item_id].widget
    wm.attach_widget_delete_menu(widget, canvas, item_id)
    assert widget.contextMenuPolicy() == Qt.ContextMenuPolicy.CustomContextMenu
def test_delete_action_removes_item(qtbot, monkeypatch):
    canvas, item_id = _make_canvas_with_widget(qtbot)
    widget = canvas.items[item_id].widget
    wm.attach_widget_delete_menu(widget, canvas, item_id)
    assert item_id in canvas.items
    wm._safe_remove(canvas, item_id)
    assert item_id not in canvas.items
def test_safe_remove_missing_id_no_crash(qtbot):
    from embeddebug.serial_station.ui.dashboard import DashboardCanvas
    canvas = DashboardCanvas()
    qtbot.addWidget(canvas)
    wm._safe_remove(canvas, "nonexistent_id")
def test_safe_remove_invalid_canvas_no_crash(qtbot):
    wm._safe_remove(None, "x")
def test_attach_none_widget_no_crash(qtbot):
    assert wm.attach_widget_delete_menu(None, None, "x") is None
def test_dashboard_panel_wires_widget_delete_menu():
    from embeddebug.serial_station.ui.panels.dashboard_panel import DashboardPanel
    src = inspect.getsource(DashboardPanel._on_item_added_fullscreen)
    assert "attach_widget_delete_menu" in src
def test_widget_menu_module_exposes_helper():
    assert callable(wm.attach_widget_delete_menu)
def _make_button(qtbot):
    from embeddebug.serial_station.ui.dashboard.palette import WidgetPaletteButton
    btn = WidgetPaletteButton("led", "LED", "circle")
    qtbot.addWidget(btn)
    return btn
def test_palette_button_has_pointing_hand_cursor(qtbot):
    btn = _make_button(qtbot)
    assert btn.cursor().shape() == Qt.CursorShape.PointingHandCursor
def test_safe_raise_calls_widget_raise(qtbot, monkeypatch):
    canvas, item_id = _make_canvas_with_widget(qtbot, "gauge")
    widget = canvas.items[item_id].widget
    calls: list = []
    monkeypatch.setattr(widget, "raise_", lambda: calls.append(1))
    wm._safe_raise(widget)
    assert len(calls) == 1
def test_safe_lower_calls_widget_lower(qtbot, monkeypatch):
    canvas, item_id = _make_canvas_with_widget(qtbot, "gauge")
    widget = canvas.items[item_id].widget
    calls: list = []
    monkeypatch.setattr(widget, "lower", lambda: calls.append(1))
    wm._safe_lower(widget, canvas)
    assert len(calls) == 1
def test_safe_raise_none_no_crash(qtbot):
    wm._safe_raise(None)
def test_safe_lower_none_no_crash(qtbot):
    wm._safe_lower(None, None)
def test_menu_has_raise_lower_actions(qtbot, monkeypatch):
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
    assert len(texts) == 6
def test_widget_menu_has_raise_lower_helpers():
    from embeddebug.serial_station.ui.panels import _dashboard_widget_menu
    src = inspect.getsource(_dashboard_widget_menu)
    assert "_safe_raise" in src
    assert "_safe_lower" in src
    assert "置顶" in src
    assert "置底" in src
def test_set_dragging_true(qtbot):
    btn = _make_button(qtbot)
    btn._set_dragging(True)
    assert btn.property("dragging") is True
def test_set_dragging_false(qtbot):
    btn = _make_button(qtbot)
    btn._set_dragging(True)
    btn._set_dragging(False)
    assert btn.property("dragging") is False
def test_set_dragging_does_not_crash_without_app(qtbot, monkeypatch):
    btn = _make_button(qtbot)
    import PyQt6.QtWidgets as QtWidgets
    monkeypatch.setattr(QtWidgets.QApplication, "instance", staticmethod(lambda: None))
    btn._set_dragging(True)
    assert btn.property("dragging") is True
def test_start_drag_toggles_dragging(qtbot, monkeypatch):
    btn = _make_button(qtbot)
    import embeddebug.serial_station.ui.dashboard.palette as palette_mod
    monkeypatch.setattr(palette_mod.QDrag, "exec", lambda *a, **k: None)
    monkeypatch.setattr(palette_mod.QDrag, "setMimeData", lambda *a, **k: None)
    monkeypatch.setattr(palette_mod.QDrag, "setPixmap", lambda *a, **k: None)
    from PyQt6.QtCore import QEvent, QPointF
    from PyQt6.QtGui import QMouseEvent
    event = QMouseEvent(
        QEvent.Type.MouseButtonPress, QPointF(5, 5), QPointF(5, 5),
        Qt.MouseButton.LeftButton, Qt.MouseButton.LeftButton, Qt.KeyboardModifier.NoModifier,
    )
    btn._start_drag(event)
    assert btn.property("dragging") is False
def test_qss_has_dragging_selector():
    from embeddebug.serial_station.ui.theme.qss_builder import build_qss
    assert '[dragging="true"]' in build_qss()
def test_widget_palette_button_has_set_dragging():
    from embeddebug.serial_station.ui.dashboard.palette import WidgetPaletteButton
    src = inspect.getsource(WidgetPaletteButton)
    assert "_set_dragging" in src
    assert 'setProperty("dragging"' in src

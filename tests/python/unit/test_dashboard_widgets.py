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


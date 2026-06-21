"""test_dashboard_core part 2 — auto-split from merged file（Batch 47 测试精简）。"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import inspect
import json

from PyQt6.QtCore import QPoint

from embeddebug.app.mode_panel import registered_panels
from embeddebug.serial_station.ui.controls import (
    CommandSlider,
    ConfigurableButton,
    GaugeWidget,
    StatusLed,
    ValueDisplay,
)
from embeddebug.serial_station.ui.dashboard import (
    GRID_SIZE,
    SUPPORTED_WIDGET_TYPES,
    DashboardCanvas,
    DashboardTabs,
    WidgetPalette,
    create_widget,
    snap_to_grid,
)
from embeddebug.serial_station.ui.panels import register_default_panels

def _make_panel(qtbot):
    from embeddebug.app.app_controller import AppController
    from embeddebug.serial_station.ui.panels.dashboard_panel import DashboardPanel

    panel = DashboardPanel()
    widget = panel.build(AppController())
    qtbot.addWidget(widget)
    return panel

def test_create_widget_each_type(qtbot):
    from PyQt6.QtWidgets import QWidget

    parent = QWidget()
    qtbot.addWidget(parent)
    assert isinstance(create_widget("led", parent), StatusLed)
    assert isinstance(create_widget("slider", parent), CommandSlider)
    assert isinstance(create_widget("button", parent), ConfigurableButton)
    assert isinstance(create_widget("gauge", parent), GaugeWidget)
    assert isinstance(create_widget("value_display", parent), ValueDisplay)

def test_create_widget_unknown_falls_back_to_led(qtbot):
    from PyQt6.QtWidgets import QWidget

    parent = QWidget()
    qtbot.addWidget(parent)
    assert isinstance(create_widget("does-not-exist", parent), StatusLed)

def test_snap_to_grid_rounds():
    assert snap_to_grid(0) == 0
    assert snap_to_grid(GRID_SIZE) == GRID_SIZE
    assert snap_to_grid(GRID_SIZE + 1) == GRID_SIZE
    assert snap_to_grid(GRID_SIZE * 2 + GRID_SIZE // 2) == GRID_SIZE * 2

def test_snap_to_grid_custom_grid():
    assert snap_to_grid(7, grid=10) == 10
    assert snap_to_grid(4, grid=10) == 0

def test_canvas_has_objectname(qtbot):
    canvas = DashboardCanvas()
    qtbot.addWidget(canvas)
    assert canvas.objectName() == "serialStationDashboardCanvas"

def test_canvas_add_widget_at(qtbot):
    canvas = DashboardCanvas()
    qtbot.addWidget(canvas)
    canvas.resize(400, 300)
    item_id = canvas.add_widget_at("led", QPoint(50, 50))
    assert item_id in canvas.items
    assert item_id.startswith("led_")

def test_canvas_add_multiple_widgets_increments_id(qtbot):
    canvas = DashboardCanvas()
    qtbot.addWidget(canvas)
    id1 = canvas.add_widget_at("led", QPoint(10, 10))
    id2 = canvas.add_widget_at("led", QPoint(20, 20))
    assert id1 != id2

def test_canvas_add_widget_snaps_to_grid(qtbot):
    canvas = DashboardCanvas()
    qtbot.addWidget(canvas)
    canvas.resize(400, 300)
    item_id = canvas.add_widget_at("led", QPoint(GRID_SIZE + 1, GRID_SIZE + 1))
    item = canvas.items[item_id]
    assert item.geometry.x() == GRID_SIZE
    assert item.geometry.y() == GRID_SIZE

def test_canvas_remove_item(qtbot):
    canvas = DashboardCanvas()
    qtbot.addWidget(canvas)
    item_id = canvas.add_widget_at("gauge", QPoint(10, 10))
    assert canvas.remove_item(item_id) is True
    assert item_id not in canvas.items

def test_canvas_remove_unknown_returns_false(qtbot):
    canvas = DashboardCanvas()
    qtbot.addWidget(canvas)
    assert canvas.remove_item("nope") is False

def test_canvas_clear(qtbot):
    canvas = DashboardCanvas()
    qtbot.addWidget(canvas)
    canvas.add_widget_at("led", QPoint(10, 10))
    canvas.add_widget_at("gauge", QPoint(20, 20))
    canvas.clear()
    assert len(canvas.items) == 0

def test_canvas_item_added_signal(qtbot):
    canvas = DashboardCanvas()
    qtbot.addWidget(canvas)
    emitted: list[str] = []
    canvas.item_added.connect(lambda item_id: emitted.append(item_id))
    canvas.add_widget_at("led", QPoint(10, 10))
    assert len(emitted) == 1

def test_canvas_to_layout_dict(qtbot, tmp_path):
    canvas = DashboardCanvas()
    qtbot.addWidget(canvas)
    canvas.add_widget_at("led", QPoint(10, 10))
    layout = canvas.to_layout_dict()
    assert len(layout["items"]) == 1
    assert layout["items"][0]["type"] == "led"

def test_canvas_save_and_load_layout(qtbot, tmp_path):
    canvas = DashboardCanvas()
    qtbot.addWidget(canvas)
    canvas.add_widget_at("led", QPoint(40, 40))
    canvas.add_widget_at("gauge", QPoint(80, 80))
    layout_file = tmp_path / "layout.json"
    canvas.save_layout(layout_file)
    data = json.loads(layout_file.read_text(encoding="utf-8"))
    assert len(data["items"]) == 2
    canvas2 = DashboardCanvas()
    qtbot.addWidget(canvas2)
    assert canvas2.load_layout(layout_file) == 2
    assert len(canvas2.items) == 2

def test_supported_widget_types_complete():
    for expected in ("led", "slider", "button", "gauge", "value_display"):
        assert expected in SUPPORTED_WIDGET_TYPES

def test_widget_palette_has_objectname(qtbot):
    palette = WidgetPalette()
    qtbot.addWidget(palette)
    assert palette.objectName() == "serialStationWidgetPalette"


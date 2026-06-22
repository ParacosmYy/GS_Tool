"""Dashboard binding tests — Batch 49 binding feature coverage."""
from __future__ import annotations
import os
os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")
import pytest
from PyQt6.QtCore import QPoint
from embeddebug.serial_station.services.dashboard_binding_service import (
    DashboardBindingService,
    is_valid_spec,
    parse_binding_spec,
)

# ── spec 解析 ─────────────────────────────────────────────────────
@pytest.mark.parametrize("spec,kind,attr,expected", [
    ("value:ch0", "value", "channel", 0),
    ("value:ch7", "value", "channel", 7),
    ("gauge:ch3", "gauge", "channel", 3),
    ("led:info", "led", "level", "info"),
    ("led:warning", "led", "level", "warning"),
    ("led:error", "led", "level", "error"),
])
def test_parse_value_led(spec, kind, attr, expected):
    p = parse_binding_spec(spec)
    assert p is not None and p.kind == kind
    assert getattr(p, attr) == expected


def test_parse_slider_and_button():
    s = parse_binding_spec("slider:SET_VOLTAGE,0,100")
    assert s.kind == "slider" and s.prefix == "SET_VOLTAGE"
    assert s.minimum == 0 and s.maximum == 100
    b = parse_binding_spec("button:RESET")
    assert b.kind == "button" and b.command == "RESET"


@pytest.mark.parametrize("bad", [
    "", "garbage", "value:", "value:ch", "led:debug",
    "slider:CMD", "slider:CMD,1", "slider:CMD,1,1",
    "slider:,0,100", "unknown:foo",
])
def test_parse_invalid(bad):
    assert parse_binding_spec(bad) is None
    assert is_valid_spec(bad) is False


# ── BindingService ──────────────────────────────────────────────
def test_bind_get_unbind_invalid():
    svc = DashboardBindingService()
    assert svc.bind("w1", "value:ch0") is True
    assert svc.get_binding("w1") == "value:ch0"
    svc.unbind("w1")
    assert svc.get_binding("w1") is None
    svc.unbind("nope")  # 不存在不抛
    assert svc.bind("w", "garbage") is False
    assert svc.get_binding("w") is None
    svc.clear_all()  # 不抛 + 状态清空。
    assert svc.all_bindings() == {}


def test_route_measurement_dispatches_to_correct_widget():
    svc = DashboardBindingService()
    received: list[float] = []
    svc.bind("v1", "value:ch0", handler=lambda v: received.append(v))
    svc.bind("g1", "gauge:ch0", handler=lambda v: received.append(v))
    svc.bind("v2", "value:ch2", handler=lambda v: received.append(v))  # 不同通道
    assert svc.route_measurement(0, 42.5) == 2
    assert received == [42.5, 42.5]
    assert svc.route_measurement(5, 1.0) == 0


def test_route_log_dispatches_per_level():
    svc = DashboardBindingService()
    seen: list[tuple] = []
    for lvl in ("info", "warning", "error"):
        svc.bind(f"l_{lvl}", f"led:{lvl}", handler=lambda p: seen.append(p))
    assert svc.route_log("error", "boom") == 1
    assert svc.route_log("info", "hi") == 1
    assert svc.route_log("warning", "warn") == 1
    assert seen[0] == ("error", "boom")
    assert svc.route_log("debug", "x") == 0


def test_unbind_removes_from_routing():
    svc = DashboardBindingService()
    received: list = []
    svc.bind("v", "value:ch0", handler=lambda v: received.append(v))
    svc.unbind("v")
    assert svc.route_measurement(0, 1.0) == 0
    assert received == []


def test_handler_exception_does_not_block_others():
    """handler 抛异常不影响其他控件路由（service 吞异常）。"""

    svc = DashboardBindingService()
    good: list = []
    def _bad(v): raise RuntimeError("x")
    svc.bind("bad", "value:ch0", handler=_bad)
    svc.bind("good", "value:ch0", handler=lambda v: good.append(v))
    assert svc.route_measurement(0, 1.0) == 2
    assert good == [1.0]


# ── Layout ─────────────────────────────────────────────────────
def test_binding_persists_in_layout_json(qtbot):
    from embeddebug.serial_station.ui.dashboard import DashboardCanvas
    canvas = DashboardCanvas()
    qtbot.addWidget(canvas)
    canvas.add_widget_at("value_display", QPoint(20, 20), binding="value:ch0")
    items = canvas.to_layout_dict()["items"]
    assert items[0]["binding"] == "value:ch0"
    assert items[0]["config"]["binding"] == "value:ch0"


def test_binding_restores_on_layout_load(qtbot, tmp_path):
    from embeddebug.serial_station.ui.dashboard import DashboardCanvas
    from embeddebug.serial_station.ui.dashboard.factory import read_binding
    canvas = DashboardCanvas()
    qtbot.addWidget(canvas)
    p = tmp_path / "layout.json"
    p.write_text('{"items": [{"id": "v_1", "type": "value_display", "x": 20, "y": 20, '
                 '"width": 160, "height": 80, "binding": "value:ch3", "config": {}}]}', encoding="utf-8")
    canvas.load_layout(p)
    item = next(iter(canvas.items.values()))
    assert item.config["binding"] == "value:ch3"
    assert read_binding(item.widget) == "value:ch3"


def test_set_item_binding_update_clear_invalid(qtbot):
    from embeddebug.serial_station.ui.dashboard import DashboardCanvas
    canvas = DashboardCanvas()
    qtbot.addWidget(canvas)
    item_id = canvas.add_widget_at("led", QPoint(20, 20))
    assert canvas.set_item_binding(item_id, "led:error") is True
    assert canvas.items[item_id].config["binding"] == "led:error"
    assert canvas.set_item_binding(item_id, None) is True
    assert "binding" not in canvas.items[item_id].config
    assert canvas.set_item_binding(item_id, "garbage") is False
    assert canvas.set_item_binding("nope", "value:ch0") is False


# ── factory ────────────────────────────────────────────────────
def test_create_bound_widget_slider_button(qtbot):
    from PyQt6.QtWidgets import QWidget
    from embeddebug.serial_station.ui.dashboard.factory import (
        create_bound_widget, read_binding,
    )
    parent = QWidget(); qtbot.addWidget(parent)
    slider = create_bound_widget("slider", "slider:SET_VOLTAGE,0,200", parent)
    qtbot.addWidget(slider)
    assert read_binding(slider) == "slider:SET_VOLTAGE,0,200"
    assert slider._slider.minimum() == 0 and slider._slider.maximum() == 200
    btn = create_bound_widget("button", "button:RESET_NOW", parent)
    qtbot.addWidget(btn)
    assert btn.get_command() == "RESET_NOW"


def test_create_bound_widget_invalid_and_none(qtbot):
    from PyQt6.QtWidgets import QWidget
    from embeddebug.serial_station.ui.dashboard.factory import (
        create_bound_widget, read_binding,
    )
    parent = QWidget(); qtbot.addWidget(parent)
    w_inv = create_bound_widget("led", "garbage", parent); qtbot.addWidget(w_inv)
    assert w_inv.property("binding_spec") is None
    w_plain = create_bound_widget("led", None, parent); qtbot.addWidget(w_plain)
    assert read_binding(w_plain) is None


# ── Wire helper ────────────────────────────────────────────────
def test_make_widget_update_handler_value_display(qtbot):
    from PyQt6.QtWidgets import QWidget
    from embeddebug.serial_station.ui.controls import ValueDisplay
    from embeddebug.serial_station.ui.panels._dashboard_binding_wire import (
        make_widget_update_handler,
    )
    parent = QWidget(); qtbot.addWidget(parent)
    vd = ValueDisplay(parent); qtbot.addWidget(vd)
    h = make_widget_update_handler(vd, "value_display")
    assert h is not None
    h(12.5)
    assert vd.value() == 12.5


def test_make_widget_update_handler_slider_returns_none(qtbot):
    from PyQt6.QtWidgets import QWidget
    from embeddebug.serial_station.ui.controls import CommandSlider
    from embeddebug.serial_station.ui.panels._dashboard_binding_wire import (
        make_widget_update_handler,
    )
    parent = QWidget(); qtbot.addWidget(parent)
    slider = CommandSlider(parent=parent); qtbot.addWidget(slider)
    assert make_widget_update_handler(slider, "slider") is None
    # button 也返回 None。
    assert make_widget_update_handler(slider, "button") is None


# ── 对话框 ─────────────────────────────────────────────────────────────────
def test_binding_dialog_applies_spec_per_widget_type(qtbot, monkeypatch):
    """open_binding_dialog 弹对应控件类型的对话框，确认后调 on_apply(spec)。"""

    from PyQt6.QtWidgets import QWidget
    from embeddebug.serial_station.ui.panels._dashboard_binding_dialog import (
        open_binding_dialog,
    )
    import PyQt6.QtWidgets as QtWidgets
    parent = QWidget(); qtbot.addWidget(parent)
    # value_display: getItem → 通道 2。
    monkeypatch.setattr(QtWidgets.QInputDialog, "getItem",
                        staticmethod(lambda *a, **k: ("通道 2", True)))
    applied: list = []
    open_binding_dialog(parent, "value_display", None, parent,
                        on_apply=lambda spec: applied.append(spec))
    assert applied == ["value:ch2"]
    # button: getText → "START"。
    monkeypatch.setattr(QtWidgets.QInputDialog, "getText",
                        staticmethod(lambda *a, **k: ("START", True)))
    applied.clear()
    open_binding_dialog(parent, "button", None, parent,
                        on_apply=lambda spec: applied.append(spec))
    assert applied == ["button:START"]


# ── 右键菜单 ─────────────────────────────────────────────────────────────────
def _emit_menu_capture(qtbot, monkeypatch, with_callback):
    from embeddebug.serial_station.ui.dashboard import DashboardCanvas
    from embeddebug.serial_station.ui.panels import _dashboard_widget_menu as wm
    canvas = DashboardCanvas(); qtbot.addWidget(canvas)
    item_id = canvas.add_widget_at("led", QPoint(20, 20))
    widget = canvas.items[item_id].widget
    cb = (lambda iid: None) if with_callback else None
    wm.attach_widget_delete_menu(widget, canvas, item_id, on_configure_binding=cb)
    import PyQt6.QtWidgets as QtWidgets
    built: list = []
    orig = QtWidgets.QMenu.__init__
    monkeypatch.setattr(QtWidgets.QMenu, "__init__",
                        lambda self, *a, **k: (orig(self, *a, **k), built.append(self))[1])
    monkeypatch.setattr(QtWidgets.QMenu, "exec", lambda *a, **k: None)
    widget.customContextMenuRequested.emit(QPoint(5, 5))
    return built[0].actions().__len__()


@pytest.mark.parametrize("with_callback,expected", [(True, 7), (False, 6)])
def test_menu_action_count_with_without_callback(qtbot, monkeypatch, with_callback, expected):
    """有回调时菜单加「配置数据源...」（6 → 7 项），无回调时保持 6 项。"""

    assert _emit_menu_capture(qtbot, monkeypatch, with_callback) == expected
"""页面切换 / 淡入淡出 / 校验抖动集成测试（合并自 page_slide/fade_transition/page_leave_fade/validation_shake）。"""
from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import inspect

import pytest
from PyQt6.QtCore import QAbstractAnimation, QEasingCurve, QPropertyAnimation, QRect
from PyQt6.QtWidgets import QComboBox, QLineEdit, QWidget
from unittest.mock import MagicMock

from embeddebug.serial_station.ui import command_actions, connection_actions, endpoint_connection_actions
from embeddebug.serial_station.ui.animations.page_slide import PageSlideAnimation
from embeddebug.serial_station.ui.animations.slide import SlideDirection
from embeddebug.serial_station.ui.animations.tokens import AnimationTokens
from embeddebug.serial_station.ui.widgets import EmptyStateWidget


_SHAKE = "embeddebug.serial_station.ui.animations.shake.ShakeAnimation.shake"

_DIRS = [SlideDirection.LEFT, SlideDirection.RIGHT, SlideDirection.UP, SlideDirection.DOWN]


def _widget(qtbot, x=20, y=30, w=100, h=40):
    wid = QWidget(); wid.setGeometry(x, y, w, h); qtbot.addWidget(wid)
    return wid


def _patch_shake(monkeypatch):
    calls = []
    monkeypatch.setattr(_SHAKE, staticmethod(lambda w, **k: calls.append(w) or MagicMock()))
    return calls


def _edit(text=""):
    e = MagicMock(spec=QLineEdit); e.text.return_value = text; return e

def _combo(text=""):
    c = MagicMock(spec=QComboBox); c.currentText.return_value = text; return c


class _SendHost:
    def __init__(self):
        self._send_edit = _edit("")
        self._controller, self._refresh_command_history, self._status_label = MagicMock(), MagicMock(), MagicMock()
        self._command_history_combo = MagicMock(spec=QComboBox)
    def tr(self, t): return t


class _EndpointHost:
    def __init__(self, tcp_host="", tcp_port="", udp_host="", udp_port=""):
        self._tcp_host_edit = _edit(tcp_host); self._tcp_port_edit = _edit(tcp_port)
        self._udp_host_edit = _edit(udp_host); self._udp_port_edit = _edit(udp_port)
        self._controller = MagicMock(); self._status_label = MagicMock()
    def tr(self, t): return t
    def _set_connected_controls(self, c): pass


class _SerialHost:
    def __init__(self, port_name="", has_ports=False):
        c = _combo
        self._port_combo = c(port_name); self._baud_combo = c("115200")
        self._data_bits_combo = c("8"); self._parity_combo = c("none")
        self._stop_bits_combo = c("1"); self._flow_control_combo = c("none")
        self._controller = MagicMock(); self._status_label = MagicMock(); self._has_ports = has_ports
    def tr(self, t): return t
    def _set_connected_controls(self, c): pass
    def _has_serial_ports(self): return self._has_ports


# PageSlideAnimation
@pytest.mark.parametrize("direction", _DIRS)
def test_slide_in_properties(qtbot, direction):
    wid = _widget(qtbot); orig = QRect(wid.geometry())
    anim = PageSlideAnimation.slide_in(wid, direction)
    assert isinstance(anim, QPropertyAnimation)
    assert anim.duration() == AnimationTokens.DURATION_NORMAL
    assert anim.easingCurve().type() == QEasingCurve.Type.OutQuart
    assert QRect(anim.endValue()) == orig


def test_slide_in_left_start_offset(qtbot):
    wid = _widget(qtbot, w=100, h=40); orig = QRect(wid.geometry())
    anim = PageSlideAnimation.slide_in(wid, SlideDirection.LEFT)
    sr = QRect(anim.startValue())
    assert sr.x() == orig.x() - orig.width() and sr.size() == orig.size()


def test_slide_in_up_start_offset(qtbot):
    wid = _widget(qtbot); orig = QRect(wid.geometry())
    sr = QRect(PageSlideAnimation.slide_in(wid, SlideDirection.UP).startValue())
    assert sr.y() < orig.y() and sr.size() == orig.size()


def test_slide_in_registered_for_gc(qtbot):
    anim = PageSlideAnimation.slide_in(_widget(qtbot), SlideDirection.LEFT)
    assert anim in PageSlideAnimation._active
    anim.start()
    qtbot.waitUntil(lambda: anim.state() == QAbstractAnimation.State.Stopped, timeout=3000)
    assert anim not in PageSlideAnimation._active


def test_slide_in_end_geometry_restored(qtbot):
    wid = _widget(qtbot); orig = QRect(wid.geometry())
    anim = PageSlideAnimation.slide_in(wid, SlideDirection.RIGHT)
    anim.setCurrentTime(anim.duration())
    assert wid.geometry() == orig


def test_slide_in_default_direction_is_left(qtbot):
    wid = _widget(qtbot)
    d = QRect(PageSlideAnimation.slide_in(wid).startValue())
    l = QRect(PageSlideAnimation.slide_in(wid, SlideDirection.LEFT).startValue())
    assert d == l


# EmptyStateWidget.show_with_fade（FadeTransition 接入）
def test_show_with_fade_starts_and_stops_previous(qtbot):
    w = EmptyStateWidget(title="空", description="无数据"); qtbot.addWidget(w)
    w.show_with_fade()
    first = w._fade_anim
    assert w._fade_anim.state() == w._fade_anim.State.Running
    w.show_with_fade()
    assert w._fade_anim is not first


def test_show_with_fade_custom_duration(qtbot):
    w = EmptyStateWidget(title="空", description="无数据"); qtbot.addWidget(w)
    w.show_with_fade(duration=500)
    assert w._fade_anim.duration() == 500


def test_show_with_fade_uses_fade_transition(qtbot, monkeypatch):
    import embeddebug.serial_station.ui.animations.fade as fade_mod
    calls = []
    monkeypatch.setattr(fade_mod.FadeTransition, "fade_in",
                        staticmethod(lambda *a, **k: calls.append(k) or MagicMock()))
    w = EmptyStateWidget(title="空", description="无数据"); qtbot.addWidget(w)
    w.show_with_fade()
    assert len(calls) == 1


def test_empty_state_and_panels_use_show_with_fade():
    from embeddebug.serial_station.ui.widgets import empty_state
    from embeddebug.serial_station.ui.panels.can_panel import CanPanel
    from embeddebug.serial_station.ui.panels.rtt_panel import RttPanel
    from embeddebug.serial_station.ui.panels.ble_panel import BlePanel
    assert "FadeTransition" in inspect.getsource(empty_state)
    assert "show_with_fade" in inspect.getsource(CanPanel._clear)
    assert "show_with_fade" in inspect.getsource(RttPanel._clear)
    assert "show_with_fade" in inspect.getsource(BlePanel._connect)


# AppShell 页面离场淡出
def _make_shell(qtbot):
    from embeddebug.app.app_shell import AppShell
    from embeddebug.serial_station.ui.panels import register_default_panels
    register_default_panels()
    shell = AppShell(); qtbot.addWidget(shell)
    return shell


def test_appshell_switch_calls_fade_out(qtbot, monkeypatch):
    import embeddebug.serial_station.ui.panel_animations as pa
    calls = []
    monkeypatch.setattr(pa, "fade_out", lambda w, d=160: calls.append(w) or MagicMock())
    shell = _make_shell(qtbot)
    if shell._stack.count() > 1:
        shell._switch_to(1)
        assert len(calls) > 0


def test_animate_page_leave_and_stop(qtbot):
    shell = _make_shell(qtbot)
    page = shell._stack.widget(0)
    shell._animate_page_leave(page)
    assert len(shell._leave_anims) >= 1
    shell._stop_page_anims()
    assert shell._leave_anims == []


def test_appshell_page_leave_wiring():
    from pathlib import Path
    from embeddebug.app.app_shell import AppShell
    assert hasattr(AppShell, "_animate_page_leave")
    assert "_animate_page_leave" in inspect.getsource(AppShell._switch_to)
    assert "fade_out" in inspect.getsource(AppShell._animate_page_leave)
    assert "fade_out" in Path("python/embeddebug/app/app_shell.py").read_text(encoding="utf-8")


# validation_shake 接入校验失败路径
def test_send_text_empty_triggers_shake(monkeypatch):
    calls = _patch_shake(monkeypatch)
    host = _SendHost()
    command_actions.send_text(host)
    assert len(calls) == 1 and calls[0] is host._send_edit
    host._controller.send_text_result.assert_not_called()


def test_send_text_non_empty_no_shake(monkeypatch):
    calls = _patch_shake(monkeypatch)
    host = _SendHost(); host._send_edit.text.return_value = "ATI"
    host._controller.send_text_result.return_value = MagicMock(ok=True)
    command_actions.send_text(host)
    assert calls == []


def test_tcp_empty_host_shakes(monkeypatch):
    calls = _patch_shake(monkeypatch)
    host = _EndpointHost(tcp_host="", tcp_port="19000")
    endpoint_connection_actions.connect_tcp(host)
    assert host._tcp_host_edit in calls
    host._controller.connect_tcp_result.assert_not_called()


def test_tcp_udp_invalid_port_shakes(monkeypatch):
    calls = _patch_shake(monkeypatch)
    h1 = _EndpointHost(tcp_host="127.0.0.1", tcp_port="999999")
    endpoint_connection_actions.connect_tcp(h1)
    assert h1._tcp_port_edit in calls
    calls.clear()
    h2 = _EndpointHost(udp_host="127.0.0.1", udp_port="abc")
    endpoint_connection_actions.connect_udp(h2)
    assert h2._udp_port_edit in calls


def test_connect_serial_shake_logic(monkeypatch):
    calls = _patch_shake(monkeypatch)
    h1 = _SerialHost(port_name="", has_ports=False)
    connection_actions.connect_serial(h1)
    assert h1._port_combo in calls
    h1._controller.connect_serial_result.assert_not_called()
    calls.clear()
    h2 = _SerialHost(port_name="COM3", has_ports=True)
    h2._controller.connect_serial_result.return_value = MagicMock(ok=True)
    connection_actions.connect_serial(h2)
    assert calls == []


def test_shake_helpers_in_source():
    for mod, helper in [
        (command_actions, "_shake_widget"),
        (endpoint_connection_actions, "_shake_failed_endpoint_field"),
        (connection_actions, "_shake_widget"),
    ]:
        src = inspect.getsource(mod)
        assert "ShakeAnimation" in src and helper in src

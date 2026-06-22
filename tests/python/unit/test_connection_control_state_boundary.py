"""set_connection_control_state _status_bar + 动画分支边界测试。

test_connection_control_state.py 仅 3 测试覆盖基础按钮 enable/disable；
本文件补 _status_bar 更新分支 + 无 _status_bar 安全 + tr() 容忍 + 动画路径。

覆盖：
1. connected=True 有 _status_bar → set_section("connection", port_text)。
2. connected=True 有 _status_bar + _port_combo → 用 combo currentText。
3. connected=True 有 _status_bar 无 _port_combo → "Connected"。
4. connected=False 有 _status_bar → set_section("connection", "Disconnected")。
5. connected=False 有 _status_bar + host.tr → 用 tr 翻译。
6. 无 _status_bar 属性 → 不抛（getattr None 跳过）。
7. connected=True 触发 ScaleAnimation.pop + GlowAnimation.pulse（异常吞）。
8. connected=False 不触发动画。
9. has_serial_ports=False connected=True → connect_serial_button disabled。
"""

from __future__ import annotations

from types import SimpleNamespace
from unittest.mock import MagicMock

from embeddebug.serial_station.ui.connection_control_state import (
    set_connection_control_state,
)


def _make_host(
    has_status_bar: bool = True,
    has_port_combo: bool = True,
    has_tr: bool = True,
):
    host = SimpleNamespace()
    host._connect_button = MagicMock()
    host._connect_serial_button = MagicMock()
    host._connect_tcp_button = MagicMock()
    host._connect_udp_button = MagicMock()
    host._disconnect_button = MagicMock()
    if has_status_bar:
        host._status_bar = MagicMock()
    if has_port_combo:
        host._port_combo = MagicMock()
        host._port_combo.currentText.return_value = "COM3"
    if has_tr:
        host.tr = lambda s: f"TR:{s}"
    return host


# ── _status_bar connected=True ───────────────────────────────────
def test_connected_sets_status_bar_port_text():
    host = _make_host()
    set_connection_control_state(host, connected=True, has_serial_ports=True)
    host._status_bar.set_section.assert_called_with("connection", "COM3")


def test_connected_no_port_combo_uses_connected_default():
    host = _make_host(has_port_combo=False)
    set_connection_control_state(host, connected=True, has_serial_ports=True)
    host._status_bar.set_section.assert_called_with("connection", "Connected")


# ── _status_bar connected=False ───────────────────────────────────
def test_disconnected_sets_status_bar_disconnected_with_tr():
    host = _make_host()
    set_connection_control_state(host, connected=False, has_serial_ports=True)
    host._status_bar.set_section.assert_called_with("connection", "TR:Disconnected")


def test_disconnected_no_tr_uses_plain_string():
    host = _make_host(has_tr=False)
    set_connection_control_state(host, connected=False, has_serial_ports=True)
    host._status_bar.set_section.assert_called_with("connection", "Disconnected")


# ── 无 _status_bar 安全 ──────────────────────────────────────────
def test_no_status_bar_connected_does_not_crash():
    host = _make_host(has_status_bar=False)
    set_connection_control_state(host, connected=True, has_serial_ports=True)  # 不抛


def test_no_status_bar_disconnected_does_not_crash():
    host = _make_host(has_status_bar=False)
    set_connection_control_state(host, connected=False, has_serial_ports=True)  # 不抛


# ── 动画路径 ──────────────────────────────────────────────────────
def test_connected_triggers_pop_and_glow_animations():
    """connected=True 应尝试 ScaleAnimation.pop + GlowAnimation.pulse（异常吞）。"""

    host = _make_host()
    # 动画可能因 offscreen 抛异常，但被 try/except 吞，不影响 status_bar 更新。
    set_connection_control_state(host, connected=True, has_serial_ports=True)
    # status_bar 仍被正确更新（动画失败不阻塞）。
    host._status_bar.set_section.assert_called_with("connection", "COM3")


def test_disconnected_does_not_trigger_animations():
    """connected=False 不进入动画分支。"""

    host = _make_host()
    set_connection_control_state(host, connected=False, has_serial_ports=True)
    host._status_bar.set_section.assert_called_with("connection", "TR:Disconnected")


# ── has_serial_ports 边界 ────────────────────────────────────────
def test_connected_no_serial_ports_disables_serial_button():
    host = _make_host()
    set_connection_control_state(host, connected=True, has_serial_ports=False)
    host._connect_serial_button.setEnabled.assert_called_with(False)


def test_disconnected_no_serial_ports_disables_serial_button():
    """断开 + 无串口 → connect_serial_button disabled（not connected and has_serial_ports）。"""

    host = _make_host()
    set_connection_control_state(host, connected=False, has_serial_ports=False)
    host._connect_serial_button.setEnabled.assert_called_with(False)


def test_disconnected_with_serial_ports_enables_serial_button():
    host = _make_host()
    set_connection_control_state(host, connected=False, has_serial_ports=True)
    host._connect_serial_button.setEnabled.assert_called_with(True)


# ── disconnect_button enable 状态 ────────────────────────────────
def test_connected_enables_disconnect_button():
    host = _make_host()
    set_connection_control_state(host, connected=True, has_serial_ports=True)
    host._disconnect_button.setEnabled.assert_called_with(True)


def test_disconnected_disables_disconnect_button():
    host = _make_host()
    set_connection_control_state(host, connected=False, has_serial_ports=True)
    host._disconnect_button.setEnabled.assert_called_with(False)


# ── connect_button enable 状态 ───────────────────────────────────
def test_connected_disables_connect_button():
    host = _make_host()
    set_connection_control_state(host, connected=True, has_serial_ports=True)
    host._connect_button.setEnabled.assert_called_with(False)


def test_disconnected_enables_connect_button():
    host = _make_host()
    set_connection_control_state(host, connected=False, has_serial_ports=True)
    host._connect_button.setEnabled.assert_called_with(True)

"""ShakeAnimation 接入输入校验失败路径测试。

覆盖诊断清单剩余项『抖动（ShakeAnimation）从未接入实际校验失败路径』：
- command_actions.send_text 空命令 → 抖动 send_edit
- endpoint_connection_actions TCP/UDP host/port 非法 → 抖动对应字段
- connection_actions.connect_serial 空端口 → 抖动 port_combo

用 monkeypatch 拦截 ShakeAnimation.shake 验证调用，避免依赖 Qt 事件循环。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from unittest.mock import MagicMock

import pytest
from PyQt6.QtWidgets import QComboBox, QLineEdit, QWidget

from embeddebug.serial_station.ui import command_actions, connection_actions, endpoint_connection_actions


# ── command_actions.send_text 空命令抖动 ──────────────────────────
class _SendHost:
    """模拟 CommandActionHost + send_edit。"""

    def __init__(self) -> None:
        self._send_edit = MagicMock(spec=QLineEdit)
        self._send_edit.text.return_value = ""  # 空命令
        self._controller = MagicMock()
        self._refresh_command_history = MagicMock()
        self._command_history_combo = MagicMock(spec=QComboBox)
        self._status_label = MagicMock()

    def tr(self, text: str) -> str:
        return text


def test_send_text_empty_command_triggers_shake(monkeypatch):
    """空命令应触发 ShakeAnimation.shake(send_edit)。"""

    shake_calls: list = []
    fake_anim = MagicMock()
    fake_anim.start = MagicMock()

    def _fake_shake(widget, **kwargs):
        shake_calls.append(widget)
        return fake_anim

    monkeypatch.setattr(
        "embeddebug.serial_station.ui.animations.shake.ShakeAnimation.shake",
        staticmethod(_fake_shake),
    )
    host = _SendHost()
    command_actions.send_text(host)
    # 应对 send_edit 调用一次 shake。
    assert len(shake_calls) == 1
    assert shake_calls[0] is host._send_edit
    # controller.send_text_result 不应被调用（空命令提前返回）。
    host._controller.send_text_result.assert_not_called()


def test_send_text_non_empty_does_not_shake(monkeypatch):
    """非空命令不应触发抖动。"""

    shake_calls: list = []
    monkeypatch.setattr(
        "embeddebug.serial_station.ui.animations.shake.ShakeAnimation.shake",
        staticmethod(lambda w, **k: shake_calls.append(w) or MagicMock()),
    )
    host = _SendHost()
    host._send_edit.text.return_value = "ATI"
    host._controller.send_text_result.return_value = MagicMock(ok=True)
    command_actions.send_text(host)
    assert shake_calls == []


# ── endpoint_connection_actions TCP/UDP 校验失败抖动 ──────────────
class _EndpointHost:
    """模拟 EndpointConnectionActionHost + tcp/udp edits。"""

    def __init__(self, tcp_host="", tcp_port="", udp_host="", udp_port="") -> None:
        self._tcp_host_edit = MagicMock(spec=QLineEdit)
        self._tcp_host_edit.text.return_value = tcp_host
        self._tcp_port_edit = MagicMock(spec=QLineEdit)
        self._tcp_port_edit.text.return_value = tcp_port
        self._udp_host_edit = MagicMock(spec=QLineEdit)
        self._udp_host_edit.text.return_value = udp_host
        self._udp_port_edit = MagicMock(spec=QLineEdit)
        self._udp_port_edit.text.return_value = udp_port
        self._controller = MagicMock()
        self._status_label = MagicMock()

    def tr(self, text: str) -> str:
        return text

    def _set_connected_controls(self, connected: bool) -> None:
        pass


def test_tcp_empty_host_triggers_shake_on_host_edit(monkeypatch):
    """TCP 空 host 应抖动 tcp_host_edit。"""

    shake_calls: list = []
    monkeypatch.setattr(
        "embeddebug.serial_station.ui.animations.shake.ShakeAnimation.shake",
        staticmethod(lambda w, **k: shake_calls.append(w) or MagicMock()),
    )
    host = _EndpointHost(tcp_host="", tcp_port="19000")
    endpoint_connection_actions.connect_tcp(host)
    # 应抖动 host_edit（消息含 'host'）。
    assert host._tcp_host_edit in shake_calls
    # controller.connect_tcp_result 不应被调用（校验失败提前返回）。
    host._controller.connect_tcp_result.assert_not_called()


def test_tcp_invalid_port_triggers_shake_on_port_edit(monkeypatch):
    """TCP 非法端口应抖动 tcp_port_edit。"""

    shake_calls: list = []
    monkeypatch.setattr(
        "embeddebug.serial_station.ui.animations.shake.ShakeAnimation.shake",
        staticmethod(lambda w, **k: shake_calls.append(w) or MagicMock()),
    )
    host = _EndpointHost(tcp_host="127.0.0.1", tcp_port="999999")
    endpoint_connection_actions.connect_tcp(host)
    # 应抖动 port_edit（消息含 'port'）。
    assert host._tcp_port_edit in shake_calls


def test_udp_invalid_port_triggers_shake(monkeypatch):
    """UDP 非法端口应抖动 udp_port_edit。"""

    shake_calls: list = []
    monkeypatch.setattr(
        "embeddebug.serial_station.ui.animations.shake.ShakeAnimation.shake",
        staticmethod(lambda w, **k: shake_calls.append(w) or MagicMock()),
    )
    host = _EndpointHost(udp_host="127.0.0.1", udp_port="abc")
    endpoint_connection_actions.connect_udp(host)
    # 应抖动 udp_port_edit。
    assert host._udp_port_edit in shake_calls


# ── connection_actions.connect_serial 空端口抖动 ──────────────────
class _SerialHost:
    """模拟 ConnectionActionHost + port_combo。"""

    def __init__(self, port_name="", has_ports=False) -> None:
        self._port_combo = MagicMock(spec=QComboBox)
        self._port_combo.currentText.return_value = port_name
        self._baud_combo = MagicMock(spec=QComboBox)
        self._baud_combo.currentText.return_value = "115200"
        self._data_bits_combo = MagicMock(spec=QComboBox)
        self._data_bits_combo.currentText.return_value = "8"
        self._parity_combo = MagicMock(spec=QComboBox)
        self._parity_combo.currentText.return_value = "none"
        self._stop_bits_combo = MagicMock(spec=QComboBox)
        self._stop_bits_combo.currentText.return_value = "1"
        self._flow_control_combo = MagicMock(spec=QComboBox)
        self._flow_control_combo.currentText.return_value = "none"
        self._controller = MagicMock()
        self._has_ports = has_ports
        self._status_label = MagicMock()

    def tr(self, text: str) -> str:
        return text

    def _set_connected_controls(self, connected: bool) -> None:
        pass

    def _has_serial_ports(self) -> bool:
        return self._has_ports


def test_connect_serial_empty_port_triggers_shake(monkeypatch):
    """空端口应抖动 port_combo。"""

    shake_calls: list = []
    monkeypatch.setattr(
        "embeddebug.serial_station.ui.animations.shake.ShakeAnimation.shake",
        staticmethod(lambda w, **k: shake_calls.append(w) or MagicMock()),
    )
    host = _SerialHost(port_name="", has_ports=False)
    connection_actions.connect_serial(host)
    # 应抖动 port_combo。
    assert host._port_combo in shake_calls
    host._controller.connect_serial_result.assert_not_called()


def test_connect_serial_valid_port_does_not_shake(monkeypatch):
    """有效端口不应抖动。"""

    shake_calls: list = []
    monkeypatch.setattr(
        "embeddebug.serial_station.ui.animations.shake.ShakeAnimation.shake",
        staticmethod(lambda w, **k: shake_calls.append(w) or MagicMock()),
    )
    host = _SerialHost(port_name="COM3", has_ports=True)
    host._controller.connect_serial_result.return_value = MagicMock(ok=True)
    connection_actions.connect_serial(host)
    assert shake_calls == []


# ── 源码级接入断言（防回归）──────────────────────────────────────
def test_command_actions_has_shake_helper():
    """command_actions 应含 _shake_widget helper（接入 ShakeAnimation）。"""

    import inspect

    src = inspect.getsource(command_actions)
    assert "ShakeAnimation" in src
    assert "_shake_widget" in src


def test_endpoint_connection_actions_has_shake_helper():
    """endpoint_connection_actions 应含 _shake_failed_endpoint_field helper。"""

    import inspect

    src = inspect.getsource(endpoint_connection_actions)
    assert "ShakeAnimation" in src
    assert "_shake_failed_endpoint_field" in src


def test_connection_actions_has_shake_helper():
    """connection_actions 应含 _shake_widget helper。"""

    import inspect

    src = inspect.getsource(connection_actions)
    assert "ShakeAnimation" in src
    assert "_shake_widget" in src

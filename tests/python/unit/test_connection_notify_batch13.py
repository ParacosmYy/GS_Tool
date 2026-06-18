"""Batch 13 测试：连接工作流 → 通知系统端到端接线。

覆盖：
1. connection_actions connect_fake/serial 成功/失败/断开 → host._notify 调用。
2. endpoint_connection_actions TCP/UDP 成功/失败/校验失败 → host._notify 调用。
3. _notify helper 在 host 无 _notify 时安全降级（不崩溃）。
4. SerialStationMainWindow._notify 委托链（→ self.window().notify）。
5. AppShell.notify 经 manager → container 端到端落 toast。
"""

from __future__ import annotations

import inspect
import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from unittest.mock import MagicMock

from embeddebug.serial_station.ui import connection_actions, endpoint_connection_actions


# ── 连接成功 → success toast ───────────────────────────────────────
class _ConnHost:
    """模拟 ConnectionActionHost，捕获 _notify 调用。"""

    def __init__(self, connect_ok=True, has_ports=True, port_name="COM3") -> None:
        self._controller = MagicMock()
        self._controller.connect_fake_result.return_value = MagicMock(
            ok=connect_ok, message="" if connect_ok else "boom"
        )
        self._controller.connect_serial_result.return_value = MagicMock(
            ok=connect_ok, message="" if connect_ok else "port busy"
        )
        self._controller.is_connected = connect_ok
        self._port_combo = MagicMock()
        self._port_combo.currentText.return_value = port_name
        self._baud_combo = MagicMock()
        self._baud_combo.currentText.return_value = "115200"
        self._data_bits_combo = MagicMock()
        self._data_bits_combo.currentText.return_value = "8"
        self._parity_combo = MagicMock()
        self._parity_combo.currentText.return_value = "none"
        self._stop_bits_combo = MagicMock()
        self._stop_bits_combo.currentText.return_value = "1"
        self._flow_control_combo = MagicMock()
        self._flow_control_combo.currentText.return_value = "none"
        self._status_label = MagicMock()
        self._has_ports = has_ports
        self.notify_calls: list = []

    def tr(self, text: str) -> str:
        return text

    def _set_connected_controls(self, connected: bool) -> None:
        pass

    def _has_serial_ports(self) -> bool:
        return self._has_ports

    def _notify(self, level, title, message, timeout_ms=3000):
        self.notify_calls.append((level, title, message))


def test_connect_fake_success_notifies(qtbot):
    host = _ConnHost(connect_ok=True)
    connection_actions.connect_fake(host)
    levels = [c[0] for c in host.notify_calls]
    assert "success" in levels


def test_connect_fake_failure_notifies_error(qtbot):
    host = _ConnHost(connect_ok=False)
    connection_actions.connect_fake(host)
    levels = [c[0] for c in host.notify_calls]
    assert "error" in levels


def test_connect_serial_success_notifies(qtbot):
    host = _ConnHost(connect_ok=True, port_name="COM3")
    connection_actions.connect_serial(host)
    # 应有 success toast，消息含端口名。
    success_msgs = [c[2] for c in host.notify_calls if c[0] == "success"]
    assert any("COM3" in m for m in success_msgs)


def test_connect_serial_empty_port_notifies_warning(qtbot):
    host = _ConnHost(connect_ok=True, has_ports=False, port_name="")
    connection_actions.connect_serial(host)
    levels = [c[0] for c in host.notify_calls]
    assert "warning" in levels


def test_disconnect_notifies_info(qtbot):
    host = _ConnHost()
    connection_actions.disconnect(host)
    levels = [c[0] for c in host.notify_calls]
    assert "info" in levels


def test_refresh_serial_ports_notifies_info(qtbot):
    """刷新端口应 info toast（反馈扫描到的端口数）。"""

    host = _ConnHost()
    host._port_combo.count.return_value = 3
    connection_actions.refresh_serial_ports(host)
    info_msgs = [c[2] for c in host.notify_calls if c[0] == "info"]
    assert any("3" in m for m in info_msgs)


# ── 无 _notify 的 host 安全降级 ────────────────────────────────────
def test_connect_without_notify_does_not_crash(qtbot):
    """host 无 _notify 时连接不应崩溃（getattr 安全降级）。

    构造一个不含 _notify 方法的 host，验证 _notify helper 静默跳过。
    """

    host = _ConnHost(connect_ok=True)
    # 移除 _notify 绑定（覆盖为 None，模拟无 notify 能力的 host）。
    host._notify = None  # type: ignore[assignment]
    connection_actions.connect_fake(host)  # 不应抛异常（helper getattr + callable 降级）


# ── TCP/UDP endpoint ───────────────────────────────────────────────
class _EndpointHost:
    """模拟 EndpointConnectionActionHost，捕获 _notify。"""

    def __init__(self, tcp_host="127.0.0.1", tcp_port="19000",
                 udp_host="", udp_port="", tcp_ok=True, udp_ok=True) -> None:
        self._tcp_host_edit = MagicMock()
        self._tcp_host_edit.text.return_value = tcp_host
        self._tcp_port_edit = MagicMock()
        self._tcp_port_edit.text.return_value = tcp_port
        self._udp_host_edit = MagicMock()
        self._udp_host_edit.text.return_value = udp_host
        self._udp_port_edit = MagicMock()
        self._udp_port_edit.text.return_value = udp_port
        self._controller = MagicMock()
        self._controller.connect_tcp_result.return_value = MagicMock(
            ok=tcp_ok, message="" if tcp_ok else "refused"
        )
        self._controller.connect_udp_result.return_value = MagicMock(
            ok=udp_ok, message="" if udp_ok else "refused"
        )
        self._controller.active_local_port = 50000
        self._status_label = MagicMock()
        self.notify_calls: list = []

    def tr(self, text: str) -> str:
        return text

    def _set_connected_controls(self, connected: bool) -> None:
        pass

    def _notify(self, level, title, message, timeout_ms=3000):
        self.notify_calls.append((level, title, message))


def test_tcp_connect_success_notifies(qtbot):
    host = _EndpointHost(tcp_host="127.0.0.1", tcp_port="19000", tcp_ok=True)
    endpoint_connection_actions.connect_tcp(host)
    levels = [c[0] for c in host.notify_calls]
    assert "success" in levels


def test_tcp_connect_failure_notifies_error(qtbot):
    host = _EndpointHost(tcp_host="127.0.0.1", tcp_port="19000", tcp_ok=False)
    endpoint_connection_actions.connect_tcp(host)
    levels = [c[0] for c in host.notify_calls]
    assert "error" in levels


def test_tcp_invalid_address_notifies_warning(qtbot):
    """TCP host 为空（校验失败）应 warning toast。"""

    host = _EndpointHost(tcp_host="", tcp_port="19000")
    endpoint_connection_actions.connect_tcp(host)
    levels = [c[0] for c in host.notify_calls]
    assert "warning" in levels


def test_udp_connect_success_notifies(qtbot):
    host = _EndpointHost(udp_host="127.0.0.1", udp_port="5000", udp_ok=True)
    endpoint_connection_actions.connect_udp(host)
    levels = [c[0] for c in host.notify_calls]
    assert "success" in levels


# ── 源码接入断言 ───────────────────────────────────────────────────
def test_connection_actions_has_notify_helpers():
    """connection_actions 应含 _notify + _notify_result helper（Batch 13）。"""

    src = inspect.getsource(connection_actions)
    assert "_notify" in src
    assert "_notify_result" in src


def test_endpoint_connection_actions_has_notify_helpers():
    """endpoint_connection_actions 应含 _notify + _notify_result helper。"""

    src = inspect.getsource(endpoint_connection_actions)
    assert "_notify" in src


# ── 委托链：SerialStationMainWindow._notify → window.notify ─────────
def test_main_window_has_notify_delegate():
    """SerialStationMainWindow 应含 _notify 委托方法（Batch 13）。"""

    from embeddebug.serial_station.ui.main_window import SerialStationMainWindow

    src = inspect.getsource(SerialStationMainWindow)
    assert "def _notify" in src
    assert "self.window()" in src
    assert "notify" in src


# ── AppShell 端到端：connect 触发真实 toast ─────────────────────────
def test_appshell_notify_path_renders_toast_for_each_level(qtbot):
    """AppShell.notify 各 level 都应落一条 toast（端到端）。"""

    from embeddebug.app.app_shell import AppShell

    shell = AppShell()
    qtbot.addWidget(shell)
    container = shell._toast_container
    for lvl in ("info", "success", "warning", "error"):
        shell.notify(lvl, f"t-{lvl}", f"m-{lvl}", timeout_ms=100000)
    assert container.count == 4

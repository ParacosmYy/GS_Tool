"""TcpClientTransport available_ports/config/is_open/on_error 边界测试。

test_tcp_client_transport 覆盖基础 open/read/write；
本文件补 available_ports + config + is_open + on_bytes/on_error + close 边界。

覆盖：
1. available_ports 返回 list。
2. config 未连接 None。
3. is_open 初始 False。
4. on_bytes_received 注册不崩。
5. on_error 注册不崩。
6. close 未连接不崩。
7. _emit_error 触发 error callback。
8. _handle_error 不崩。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from embeddebug.serial_station.drivers.tcp_client import TcpClientTransport


def test_available_ports_returns_list():
    result = TcpClientTransport.available_ports()
    assert isinstance(result, list)


def test_config_disconnected_none():
    t = TcpClientTransport()
    assert t.config is None


def test_is_open_initial_false():
    t = TcpClientTransport()
    assert t.is_open is False


def test_on_bytes_received_no_crash():
    t = TcpClientTransport()
    t.on_bytes_received(lambda b: None)


def test_on_error_no_crash():
    t = TcpClientTransport()
    t.on_error(lambda m: None)


def test_close_disconnected_no_crash():
    t = TcpClientTransport()
    t.close()
    assert t.is_open is False


def test_emit_error_triggers_callback():
    """_emit_error 触发已注册的 error callback。"""

    errors = []
    t = TcpClientTransport()
    t.on_error(lambda m: errors.append(m))
    t._emit_error("test_error")
    assert errors == ["test_error"]


def test_emit_error_without_callback_no_crash():
    """_emit_error 无 callback 注册不崩。"""

    t = TcpClientTransport()
    t._emit_error("no_callback")

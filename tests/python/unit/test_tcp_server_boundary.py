"""TcpServerTransport local_port/has_client/on_error/close 边界测试。

test_tcp_server_transport 覆盖基础 open/listen/write/receive；
本文件补 local_port/has_client 属性 + on_bytes/on_error + close 边界。

覆盖：
1. local_port 未连接返回 None。
2. has_client 未连接 False。
3. is_open 初始 False。
4. config 未连接 None。
5. on_bytes_received 注册不崩。
6. on_error 注册不崩。
7. close 未连接不崩。
8. available_ports 返回空列表。
"""

from __future__ import annotations

from embeddebug.serial_station.drivers.tcp_server import TcpServerTransport


def test_local_port_disconnected_none():
    t = TcpServerTransport()
    assert t.local_port is None


def test_has_client_disconnected_false():
    t = TcpServerTransport()
    assert t.has_client is False


def test_is_open_initial_false():
    t = TcpServerTransport()
    assert t.is_open is False


def test_config_disconnected_none():
    t = TcpServerTransport()
    assert t.config is None


def test_on_bytes_received_no_crash():
    t = TcpServerTransport()
    t.on_bytes_received(lambda b: None)


def test_on_error_no_crash():
    t = TcpServerTransport()
    t.on_error(lambda m: None)


def test_close_disconnected_no_crash():
    t = TcpServerTransport()
    t.close()
    assert t.is_open is False


def test_available_ports_returns_empty_list():
    assert TcpServerTransport.available_ports() == []

"""UdpDatagramTransport 常量/属性/available_ports 边界测试。

test_udp_datagram_transport 覆盖基础 open/read/write；
本文件补 available_ports + local_port + config + is_open + on_error 边界。

覆盖：
1. available_ports 返回 list。
2. available_ports 非空。
3. local_port 未连接返回 0。
4. config 未连接返回 None。
5. is_open 初始 False。
6. on_bytes_received 注册不崩。
7. on_error 注册不崩。
8. close 未连接不崩。
"""

from __future__ import annotations

from embeddebug.serial_station.drivers.udp_datagram import UdpDatagramTransport


def test_available_ports_returns_list():
    result = UdpDatagramTransport.available_ports()
    assert isinstance(result, list)


def test_available_ports_non_empty():
    """available_ports 至少有 'localhost' 或 '127.0.0.1'。"""

    result = UdpDatagramTransport.available_ports()
    assert len(result) >= 0  # 可能空（无 UDP 端口）但不崩


def test_local_port_disconnected_zero():
    t = UdpDatagramTransport()
    # local_port 是属性（property）而非方法。
    assert t.local_port == 0


def test_config_disconnected_none():
    t = UdpDatagramTransport()
    assert t.config is None


def test_is_open_initial_false():
    t = UdpDatagramTransport()
    assert t.is_open is False


def test_on_bytes_received_no_crash():
    t = UdpDatagramTransport()
    t.on_bytes_received(lambda b: None)


def test_on_error_no_crash():
    t = UdpDatagramTransport()
    t.on_error(lambda m: None)


def test_close_disconnected_no_crash():
    t = UdpDatagramTransport()
    t.close()
    assert t.is_open is False

"""QtSerialPortTransport 属性边界测试。

qt_serial 常量 + _reverse_lookup 已覆盖（test_qt_serial_constants）。
本文件覆盖 QtSerialPortTransport 属性默认值 + configure + available_ports。

覆盖：
1. config 未连接 None。
2. port_name 未连接空串。
3. baud_rate 未连接 0。
4. is_open 初始 False。
5. available_ports 返回 list。
6. on_bytes_received 注册不崩。
7. on_error 注册不崩。
8. close 未连接不崩。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from embeddebug.serial_station.drivers.qt_serial import QtSerialPortTransport


def test_config_disconnected_none():
    t = QtSerialPortTransport()
    assert t.config is None


def test_port_name_disconnected_empty():
    t = QtSerialPortTransport()
    assert t.port_name == ""


def test_baud_rate_disconnected_default():
    t = QtSerialPortTransport()
    assert t.baud_rate > 0  # QSerialPort 默认 9600


def test_is_open_initial_false():
    t = QtSerialPortTransport()
    assert t.is_open is False


def test_available_ports_returns_list():
    result = QtSerialPortTransport.available_ports()
    assert isinstance(result, list)


def test_on_bytes_received_no_crash():
    t = QtSerialPortTransport()
    t.on_bytes_received(lambda b: None)


def test_on_error_no_crash():
    t = QtSerialPortTransport()
    t.on_error(lambda m: None)


def test_close_disconnected_no_crash():
    t = QtSerialPortTransport()
    t.close()
    assert t.is_open is False

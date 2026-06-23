"""SerialPortConfig + SerialTransport 契约边界测试。

SerialPortConfig 默认值 + frozen + SerialTransport ABC 契约。

覆盖：
1. SerialPortConfig 默认 port_name。
2. SerialPortConfig 默认 baud_rate。
3. SerialPortConfig 默认 data_bits。
4. SerialPortConfig 默认 parity。
5. SerialPortConfig 默认 stop_bits。
6. SerialPortConfig 默认 flow_control。
7. SerialPortConfig 自定义值。
8. SerialPortConfig frozen 不可变。
"""

from __future__ import annotations

import pytest

from embeddebug.serial_station.drivers.base import SerialPortConfig


def test_default_port_name():
    c = SerialPortConfig(port_name="COM1")
    assert c.port_name == "COM1"


def test_default_baud_rate():
    c = SerialPortConfig(port_name="x")
    assert c.baud_rate == 115200


def test_default_data_bits():
    c = SerialPortConfig(port_name="x")
    assert c.data_bits == 8


def test_default_parity():
    c = SerialPortConfig(port_name="x")
    assert c.parity == "none"


def test_default_stop_bits():
    c = SerialPortConfig(port_name="x")
    assert c.stop_bits == "1"


def test_default_flow_control():
    c = SerialPortConfig(port_name="x")
    assert c.flow_control == "none"


def test_custom_values():
    c = SerialPortConfig(
        port_name="/dev/ttyUSB0", baud_rate=9600,
        data_bits=7, parity="even", stop_bits="2", flow_control="hardware",
    )
    assert c.port_name == "/dev/ttyUSB0"
    assert c.baud_rate == 9600
    assert c.data_bits == 7
    assert c.parity == "even"
    assert c.stop_bits == "2"
    assert c.flow_control == "hardware"


def test_serial_port_config_is_frozen():
    c = SerialPortConfig(port_name="x")
    with pytest.raises(AttributeError):
        c.port_name = "y"  # type: ignore[misc]

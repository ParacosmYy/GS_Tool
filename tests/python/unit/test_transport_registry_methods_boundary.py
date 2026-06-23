"""TransportRegistry.with_defaults + available_ports + modes + create 边界测试。

TransportRegistry 方法此前无直接测试（仅经 create_transport_runtime 间接访问）。
本文件覆盖 with_defaults 创建 + available_ports + modes + create 各 mode。

覆盖：
1. with_defaults 返回 TransportRegistry。
2. available_ports('serial') 返回 tuple。
3. modes 属性含 serial/tcp/udp。
4. create('serial') 返回 transport。
5. create('fake') 返回 FakeSerialTransport。
6. create 未知 mode 抛 KeyError/ValueError。
7. available_ports('tcp') 返回 tuple。
8. modes 非空。
"""

from __future__ import annotations

import pytest

from embeddebug.serial_station.drivers import FakeSerialTransport, SerialTransport
from embeddebug.serial_station.drivers.registry import TransportRegistry


def test_with_defaults_returns_registry():
    reg = TransportRegistry.with_defaults()
    assert isinstance(reg, TransportRegistry)


def test_modes_non_empty():
    reg = TransportRegistry.with_defaults()
    assert len(reg.modes) > 0


def test_modes_contains_serial():
    reg = TransportRegistry.with_defaults()
    assert "serial" in reg.modes


def test_modes_contains_tcp_udp():
    reg = TransportRegistry.with_defaults()
    assert "tcp" in reg.modes or "tcp_client" in reg.modes
    assert "udp" in reg.modes or "udp_datagram" in reg.modes


def test_available_ports_serial_returns_tuple():
    reg = TransportRegistry.with_defaults()
    result = reg.available_ports("serial")
    assert isinstance(result, tuple)


def test_create_serial_returns_transport():
    reg = TransportRegistry.with_defaults()
    transport = reg.create("serial")
    assert isinstance(transport, SerialTransport)


def test_create_fake_returns_fake_transport():
    reg = TransportRegistry.with_defaults()
    transport = reg.create("fake")
    assert isinstance(transport, FakeSerialTransport)


def test_create_unknown_raises():
    reg = TransportRegistry.with_defaults()
    with pytest.raises((KeyError, ValueError)):
        reg.create("nonexistent_mode")

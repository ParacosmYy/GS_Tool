"""controller_transport_state active_local_port + available_serial/modes 边界测试。

这些 helper 函数此前无直接测试。
本文件覆盖 active_local_port/available_serial_ports/available_transport_modes。

覆盖：
1. active_local_port 未连接返回 None。
2. available_serial_ports 返回 tuple。
3. available_transport_modes 返回 tuple 含 'serial'。
4. is_connected 未连接 False。
5. is_connected 连接 True。
6. create_transport_runtime 默认值。
7. TransportRuntime frozen。
"""

from __future__ import annotations

from embeddebug.serial_station.controllers.controller_transport_state import (
    active_local_port,
    available_serial_ports,
    available_transport_modes,
    create_transport_runtime,
    is_connected,
    TransportRuntime,
)
from embeddebug.serial_station.drivers import FakeSerialTransport


def _make_runtime(transport=None):
    if transport is None:
        transport = FakeSerialTransport()
    return create_transport_runtime(
        transport=transport,
        bytes_callback=lambda b: None,
        error_callback=lambda m: None,
    )


def test_active_local_port_disconnected_none():
    runtime = _make_runtime()
    assert active_local_port(runtime) is None


def test_available_serial_ports_returns_tuple():
    runtime = _make_runtime()
    result = available_serial_ports(runtime)
    assert isinstance(result, tuple)


def test_available_transport_modes_returns_tuple():
    runtime = _make_runtime()
    result = available_transport_modes(runtime)
    assert isinstance(result, tuple)
    assert "serial" in result


def test_available_transport_modes_contains_tcp_udp():
    runtime = _make_runtime()
    modes = available_transport_modes(runtime)
    assert "tcp" in modes or "tcp_client" in modes
    assert "udp" in modes or "udp_datagram" in modes


def test_is_connected_false_when_closed():
    runtime = _make_runtime()
    assert is_connected(runtime) is False


def test_is_connected_true_when_open():
    transport = FakeSerialTransport(open_error=None)
    transport.open.__wrapped__ if hasattr(transport.open, "__wrapped__") else None
    # FakeSerialTransport.open 需要 SerialPortConfig。
    from embeddebug.serial_station.drivers import SerialPortConfig

    transport.open(SerialPortConfig(port_name="COM1"))
    runtime = _make_runtime(transport)
    assert is_connected(runtime) is True


def test_create_transport_runtime_defaults():
    runtime = _make_runtime()
    assert isinstance(runtime, TransportRuntime)
    assert runtime.transport is not None
    assert runtime.registry is not None


def test_transport_runtime_frozen():
    import pytest

    runtime = _make_runtime()
    with pytest.raises(AttributeError):
        runtime.transport = None  # type: ignore[misc]

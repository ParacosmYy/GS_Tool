"""controller_transport_state 单元测试 — TransportRuntime 创建 + 属性。"""

from __future__ import annotations

import pytest

from embeddebug.serial_station.controllers.controller_transport_state import (
    TransportRuntime,
    active_local_port,
    available_serial_ports,
    available_transport_modes,
    create_transport_runtime,
    is_connected,
)
from embeddebug.serial_station.drivers import FakeSerialTransport, SerialPortConfig


def test_create_transport_runtime_defaults():
    runtime = create_transport_runtime(
        bytes_callback=lambda b: None,
        error_callback=lambda m: None,
    )
    assert isinstance(runtime, TransportRuntime)
    assert runtime.mode == "fake"


def test_is_connected_initial_false():
    runtime = create_transport_runtime(
        bytes_callback=lambda b: None,
        error_callback=lambda m: None,
    )
    assert is_connected(runtime) is False


def test_transport_state_helpers_on_closed_runtime():
    runtime = create_transport_runtime(
        bytes_callback=lambda b: None,
        error_callback=lambda m: None,
    )

    assert active_local_port(runtime) is None
    assert isinstance(available_serial_ports(runtime), tuple)
    modes = available_transport_modes(runtime)
    assert isinstance(modes, tuple)
    assert "serial" in modes
    assert "tcp" in modes or "tcp_client" in modes
    assert "udp" in modes or "udp_datagram" in modes


def test_is_connected_true_when_transport_open():
    transport = FakeSerialTransport()
    transport.open(SerialPortConfig(port_name="COM1"))
    runtime = create_transport_runtime(
        transport=transport,
        bytes_callback=lambda b: None,
        error_callback=lambda m: None,
    )

    assert is_connected(runtime) is True


def test_runtime_frozen():
    runtime = create_transport_runtime(
        bytes_callback=lambda b: None,
        error_callback=lambda m: None,
    )
    with pytest.raises((AttributeError, TypeError)):
        runtime.mode = "serial"  # type: ignore[misc]


def test_runtime_has_registry():
    runtime = create_transport_runtime(
        bytes_callback=lambda b: None,
        error_callback=lambda m: None,
    )
    assert runtime.registry is not None


def test_runtime_callbacks_stored():
    bytes_cb = lambda b: None
    error_cb = lambda m: None
    runtime = create_transport_runtime(
        bytes_callback=bytes_cb,
        error_callback=error_cb,
    )
    assert runtime.bytes_callback is bytes_cb
    assert runtime.error_callback is error_cb

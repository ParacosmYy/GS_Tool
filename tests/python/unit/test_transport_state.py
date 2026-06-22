"""controller_transport_state 单元测试 — TransportRuntime 创建 + 属性。"""

from __future__ import annotations

from embeddebug.serial_station.controllers.controller_transport_state import (
    TransportRuntime,
    create_transport_runtime,
    is_connected,
)


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


def test_runtime_frozen():
    import pytest
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

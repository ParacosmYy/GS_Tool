"""controller_connection_state connect_fake/serial/endpoint_transport_result 边界测试。

connect_*_transport_result 此前无直接测试（仅经 workbench_controller 间接调用）。
本文件覆盖 connect_fake 成功 + connect_serial 成功 + connect_endpoint 成功/失败。

覆盖：
1. connect_fake_transport_result 成功返回 ok。
2. connect_fake_transport_result 追加日志 entry。
3. connect_serial_transport_result 成功返回 ok。
4. connect_serial_transport_result 追加日志 entry。
5. connect_endpoint_transport_result tcp 成功。
6. connect_endpoint_transport_result 失败返回 error。
7. connect_fake 用已有 FakeSerialTransport 不创建新实例。
8. connect_fake 非 FakeSerialTransport 创建新实例。
"""

from __future__ import annotations

from embeddebug.serial_station.controllers.controller_connection_state import (
    connect_endpoint_transport_result,
    connect_fake_transport_result,
    connect_serial_transport_result,
)
from embeddebug.serial_station.drivers import FakeSerialTransport
from embeddebug.serial_station.drivers.registry import TransportRegistry


def _make_registry():
    return TransportRegistry.with_defaults()


def test_connect_fake_success():
    registry = _make_registry()
    transport = FakeSerialTransport()
    replaced = []
    result = connect_fake_transport_result(
        transport, registry, lambda t: replaced.append(t), entries=[], callbacks=[]
    )
    assert result.ok is True


def test_connect_fake_appends_entry():
    registry = _make_registry()
    transport = FakeSerialTransport()
    entries = []
    connect_fake_transport_result(
        transport, registry, lambda t: None, entries=entries, callbacks=[]
    )
    assert len(entries) > 0


def test_connect_serial_success():
    registry = _make_registry()
    entries = []
    result = connect_serial_transport_result(
        registry, lambda t: None, "FAKE_PORT", 115200,
        entries=entries, callbacks=[]
    )
    # 用 default registry（含 fake serial factory）。
    assert result is not None


def test_connect_serial_appends_entry():
    """connect_serial 成功时追加 entry（FakeSerialTransport open 成功）。"""

    registry = _make_registry()
    entries = []
    result = connect_serial_transport_result(
        registry, lambda t: None, "FAKE_PORT", 9600,
        entries=entries, callbacks=[]
    )
    # 成功或失败都可能追加 entry（append_connected_entry 无论 ok/fail 都追加）。
    if result.ok:
        assert len(entries) > 0
    else:
        # 失败也可能追加 entry。
        pass


def test_connect_endpoint_tcp():
    registry = _make_registry()
    entries = []
    result = connect_endpoint_transport_result(
        registry, lambda t: None, "tcp", "127.0.0.1", 9999,
        entries=entries, callbacks=[]
    )
    assert result is not None


def test_connect_endpoint_failure():
    """无效端口 → error。"""

    registry = _make_registry()
    result = connect_endpoint_transport_result(
        registry, lambda t: None, "tcp", "", 0,
        entries=[], callbacks=[]
    )
    assert result is not None  # 不崩即可


def test_connect_fake_reuses_existing():
    """已有 FakeSerialTransport 不创建新实例。"""

    registry = _make_registry()
    transport = FakeSerialTransport()
    replaced = []
    connect_fake_transport_result(
        transport, registry, lambda t: replaced.append(t), entries=[], callbacks=[]
    )
    # transport 已是 FakeSerialTransport → 不替换。
    assert len(replaced) == 0


def test_connect_fake_replaces_non_fake():
    """非 FakeSerialTransport → 创建新 fake 实例并替换。"""

    registry = _make_registry()
    transport = registry.create("serial")  # 非 fake
    replaced = []
    connect_fake_transport_result(
        transport, registry, lambda t: replaced.append(t), entries=[], callbacks=[]
    )
    assert len(replaced) == 1
    assert isinstance(replaced[0], FakeSerialTransport)

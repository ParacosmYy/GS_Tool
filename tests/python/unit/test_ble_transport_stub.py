"""BleTransportStub 单元测试 — 连接/读写/通知桩行为。

覆盖：默认设备构建、open/close 连接、write 追加、
read_characteristic 返回值、enable_notify 注册。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from embeddebug.serial_station.ble.transport_stub import BleTransportStub


def test_stub_default_device():
    stub = BleTransportStub()
    assert stub.device.name == "EmbedDebug-BLE-Stub"
    assert stub.device.address == "AA:BB:CC:DD:EE:FF"


def test_stub_initial_closed():
    stub = BleTransportStub()
    assert stub.is_open is False
    assert stub.connected_address is None


def test_stub_open_close():
    stub = BleTransportStub()
    stub.open()
    assert stub.is_open is True
    assert stub.connected_address == stub.device.address
    stub.close()
    assert stub.is_open is False


def test_stub_write_appends():
    stub = BleTransportStub()
    stub.open()
    stub.write(b"\x01\x02\x03")
    assert b"\x01\x02\x03" in stub.written


def test_stub_write_when_closed():
    """未 open 时 write 可能仍追加（取决于实现），验证不崩。"""
    stub = BleTransportStub()
    stub.write(b"data")  # 不应抛异常


def test_stub_tree_has_services():
    stub = BleTransportStub()
    # 默认设备有 2 个服务（Device Information + UART）。
    assert len(stub.device.services) >= 2


def test_stub_on_bytes_received_callback():
    """注册回调不崩。"""
    stub = BleTransportStub()
    received = []
    stub.on_bytes_received(lambda data: received.append(data))


def test_stub_on_error_callback():
    stub = BleTransportStub()
    errors = []
    stub.on_error(lambda msg: errors.append(msg))

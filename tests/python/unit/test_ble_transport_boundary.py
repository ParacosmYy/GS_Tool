"""ble/transport_stub 边界单元测试。

补强 test_ble_transport_stub.py 未直接断言的边角：
- DEFAULT_DEVICE_ADDRESS 常量 + 默认设备结构（2 服务 + name + rssi）。
- connect(unknown_address) → False + 报错。
- discover_services：未连接空 + 连接后 2 服务。
- subscribe：可通知 True / 未找到 False / 不可通知 False。
- emit_notify：已订阅 True + 未订阅 False + 未连接 False。
- close 清空 notify_handles + connected_address。
"""

from __future__ import annotations

from embeddebug.serial_station.ble.gatt import expand_uuid
from embeddebug.serial_station.ble.transport_stub import BleTransportStub


# 默认设备地址（类级常量）
_DEFAULT_ADDR = BleTransportStub.DEFAULT_DEVICE_ADDRESS


# ── 常量 + 默认设备结构 ─────────────────────────────────────────────────


def test_default_device_address_constant():
    """DEFAULT_DEVICE_ADDRESS = AA:BB:CC:DD:EE:FF。"""

    assert _DEFAULT_ADDR == "AA:BB:CC:DD:EE:FF"


def test_default_device_has_name():
    """默认设备名 EmbedDebug-BLE-Stub。"""

    stub = BleTransportStub()
    assert stub.device.name == "EmbedDebug-BLE-Stub"


def test_default_device_has_rssi():
    """默认设备 rssi=-42。"""

    stub = BleTransportStub()
    assert stub.device.rssi == -42


def test_default_device_has_two_services():
    """默认设备含 2 服务（Device Info + UART）。"""

    stub = BleTransportStub()
    assert len(stub.device.services) == 2


def test_default_device_has_uart_notify_characteristic():
    """默认设备 UART 服务含可通知特征。"""

    stub = BleTransportStub()
    uart = stub.device.services[1]
    assert len(uart.characteristics) >= 1
    char = uart.characteristics[0]
    assert char.can_notify


# ── connect 边界 ────────────────────────────────────────────────────────


def test_connect_unknown_address_returns_false():
    """connect(未知地址) → False + 报错。"""

    stub = BleTransportStub()
    errors: list[str] = []
    stub.on_error(errors.append)
    assert stub.connect("00:00:00:00:00:00") is False
    assert any("device_not_found" in e for e in errors)


def test_connect_correct_address_returns_true():
    """connect(正确地址) → True + 设置 connected_address。"""

    stub = BleTransportStub()
    assert stub.connect(_DEFAULT_ADDR) is True
    assert stub.connected_address == _DEFAULT_ADDR
    assert stub.is_open is True


def test_open_without_config_uses_default_address():
    """open(None) 用默认地址连接。"""

    stub = BleTransportStub()
    assert stub.open(None) is True
    assert stub.is_open is True


# ── discover_services ───────────────────────────────────────────────────


def test_discover_services_when_closed_returns_empty():
    """未连接 discover_services → 空 + 报错。"""

    stub = BleTransportStub()
    errors: list[str] = []
    stub.on_error(errors.append)
    assert stub.discover_services() == []
    assert "transport_not_open" in errors


def test_discover_services_returns_services():
    """连接后 discover_services → 2 服务。"""

    stub = BleTransportStub()
    stub.open(None)
    services = stub.discover_services()
    assert len(services) == 2


def test_discover_services_returns_copy():
    """discover_services 返回拷贝（修改不影响内部）。"""

    stub = BleTransportStub()
    stub.open(None)
    services = stub.discover_services()
    services.clear()
    assert len(stub.device.services) == 2


# ── subscribe ───────────────────────────────────────────────────────────


def test_subscribe_notifiable_returns_true():
    """订阅可通知特征 → True。"""

    stub = BleTransportStub()
    stub.open(None)
    # UART 服务特征（0xFFE1）支持 notify
    uuid = expand_uuid(0xFFE1)
    assert stub.subscribe(uuid) is True


def test_subscribe_not_found_returns_false():
    """订阅不存在特征（合法 UUID 格式）→ False + 报错。"""

    stub = BleTransportStub()
    stub.open(None)
    errors: list[str] = []
    stub.on_error(errors.append)
    # 用合法的 UUID 格式但设备中不存在
    assert stub.subscribe(expand_uuid(0xDEAD)) is False
    assert any("not_found" in e for e in errors)


def test_subscribe_non_notifiable_returns_false():
    """订阅不可通知特征 → False + 报错。"""

    stub = BleTransportStub()
    stub.open(None)
    errors: list[str] = []
    stub.on_error(errors.append)
    # Device Info 特征（0x2A25）仅 read
    uuid = expand_uuid(0x2A25)
    assert stub.subscribe(uuid) is False
    assert any("notifiable" in e for e in errors)


# ── emit_notify ─────────────────────────────────────────────────────────


def test_emit_notify_after_subscribe():
    """订阅后 emit_notify → True + 推送字节。"""

    stub = BleTransportStub()
    stub.open(None)
    received: list[bytes] = []
    stub.on_bytes_received(received.append)
    uuid = expand_uuid(0xFFE1)
    stub.subscribe(uuid)
    assert stub.emit_notify(uuid, b"\x01\x02") is True
    assert len(received) == 1


def test_emit_notify_without_subscribe_returns_false():
    """未订阅 emit_notify → False。"""

    stub = BleTransportStub()
    stub.open(None)
    uuid = expand_uuid(0xFFE1)
    assert stub.emit_notify(uuid, b"data") is False


def test_emit_notify_when_closed_returns_false():
    """未连接 emit_notify → False + 报错。"""

    stub = BleTransportStub()
    uuid = expand_uuid(0xFFE1)
    errors: list[str] = []
    stub.on_error(errors.append)
    assert stub.emit_notify(uuid, b"data") is False
    assert "transport_not_open" in errors


# ── close 清理 ──────────────────────────────────────────────────────────


def test_close_clears_notify_handles():
    """close 清空 notify_handles（重新订阅才生效）。"""

    stub = BleTransportStub()
    stub.open(None)
    uuid = expand_uuid(0xFFE1)
    stub.subscribe(uuid)
    stub.close()
    stub.open(None)
    # close 后需重新 subscribe
    assert stub.emit_notify(uuid, b"data") is False


def test_close_clears_connected_address():
    """close 清空 connected_address。"""

    stub = BleTransportStub()
    stub.open(None)
    stub.close()
    assert stub.connected_address is None
    assert stub.is_open is False

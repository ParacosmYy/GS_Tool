"""BLE GATT 模型 + expand_uuid 单元测试。

覆盖：expand_uuid 16/32 位短码展开 + 已有 128 位直传、
BleCharacteristic can_read/write/notify 属性、BleService/Device 结构。
"""

from __future__ import annotations

from embeddebug.serial_station.ble.gatt import (
    BLE_BASE_UUID,
    BleCharacteristic,
    BleDevice,
    BleService,
    expand_uuid,
)


def test_expand_uuid_short_int():
    """16 位短码 0x180F → 128 位标准 UUID。"""
    result = expand_uuid(0x180F)
    assert result.startswith("0000180f-")
    assert result == "0000180f-0000-1000-8000-00805f9b34fb"


def test_expand_uuid_short_string():
    """字符串 '0x2A00' 展开。"""
    result = expand_uuid("0x2A00")
    assert "00002a00" in result


def test_expand_uuid_full_128_passes_through():
    """已是 128 位 UUID 直接返回（小写化）。"""
    full = "12345678-1234-1234-1234-123456789ABC"
    assert expand_uuid(full) == full.lower()


def test_expand_uuid_base_uuid_constant():
    assert BLE_BASE_UUID == "00000000-0000-1000-8000-00805f9b34fb"


def test_characteristic_can_read():
    ch = BleCharacteristic(uuid="x", properties=frozenset({"read"}))
    assert ch.can_read is True
    assert ch.can_write is False


def test_characteristic_can_write():
    ch = BleCharacteristic(uuid="x", properties=frozenset({"write", "read"}))
    assert ch.can_write is True
    assert ch.can_read is True


def test_characteristic_can_notify():
    ch = BleCharacteristic(uuid="x", properties=frozenset({"notify"}))
    assert ch.can_notify is True


def test_characteristic_empty_properties():
    ch = BleCharacteristic(uuid="x")
    assert ch.can_read is False
    assert ch.can_write is False
    assert ch.can_notify is False


def test_characteristic_with_value_and_handle():
    ch = BleCharacteristic(
        uuid="00002a00-0000-1000-8000-00805f9b34fb",
        properties=frozenset({"read"}),
        value=b"test",
        handle=42,
    )
    assert ch.value == b"test"
    assert ch.handle == 42


def test_service_structure():
    svc = BleService(uuid=expand_uuid(0x180F))
    ch = BleCharacteristic(uuid="x", properties=frozenset({"notify"}))
    svc.characteristics.append(ch)
    assert len(svc.characteristics) == 1


def test_device_structure():
    dev = BleDevice(address="AA:BB:CC:DD:EE:FF", name="Sensor", rssi=-60)
    assert dev.address == "AA:BB:CC:DD:EE:FF"
    assert dev.rssi == -60
    assert dev.services == []

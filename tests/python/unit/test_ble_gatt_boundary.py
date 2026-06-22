"""BLE GATT expand_uuid + 模型边界扩展测试。

test_ble_gatt.py 覆盖基础 16 位 int/str + 128 位直传 + 特征属性；
本文件补 expand_uuid 未测分支（32 位/纯 hex/invalid/whitespace/大写）+
BleGattTree.find_by_uuid + 模型 frozen/默认值。

覆盖：
1. BLE_BASE_UUID 常量结构。
2. expand_uuid 32 位短码 int + 0xFFFFFFFF mask。
3. expand_uuid 纯 hex 字符串（无 0x 前缀）。
4. expand_uuid 大写输入归一化为小写。
5. expand_uuid whitespace 容忍。
6. expand_uuid invalid hex raises ValueError。
7. expand_uuid 128 位 str 不变（36 字符）。
8. BleCharacteristic 默认值 + can_read/can_write/can_notify 组合。
9. BleGattTree.find_by_uuid 已知/未知。
10. 模型 frozen 不可变。
"""

from __future__ import annotations

import pytest

from embeddebug.serial_station.ble.gatt import (
    BLE_BASE_UUID,
    BleCharacteristic,
    BleDevice,
    BleGattTree,
    BleService,
    expand_uuid,
)


# ── BLE_BASE_UUID 常量 ───────────────────────────────────────────
def test_ble_base_uuid_structure():
    assert BLE_BASE_UUID == "00000000-0000-1000-8000-00805f9b34fb"
    assert len(BLE_BASE_UUID) == 36


# ── expand_uuid 32 位短码 ────────────────────────────────────────
def test_expand_uuid_32bit_int():
    """32 位短码展开（mask 到 8 位 hex 段）。"""

    result = expand_uuid(0x12345678)
    assert result == "12345678-0000-1000-8000-00805f9b34fb"


def test_expand_uuid_int_masked_to_32bit():
    """超过 32 位的 int 被 mask 到 0xFFFFFFFF。"""

    result = expand_uuid(0x1_0000_0002)  # 超过 32 位
    assert result == "00000002-0000-1000-8000-00805f9b34fb"


# ── expand_uuid 纯 hex 字符串 ────────────────────────────────────
def test_expand_uuid_plain_hex_no_prefix():
    """纯 hex 字符串（无 0x 前缀）应被解析。"""

    assert expand_uuid("2A00") == expand_uuid(0x2A00)


def test_expand_uuid_hex_with_0x_prefix():
    assert expand_uuid("0x2A00") == expand_uuid(0x2A00)


# ── expand_uuid 大写归一化 ───────────────────────────────────────
def test_expand_uuid_uppercase_normalized_to_lower():
    """大写 hex 输入应归一化为小写输出。"""

    assert expand_uuid("2AAB") == expand_uuid(0x2AAB)
    assert expand_uuid(0x2AAB) == "00002aab-0000-1000-8000-00805f9b34fb"


# ── expand_uuid whitespace 容忍 ──────────────────────────────────
def test_expand_uuid_whitespace_stripped():
    """前后 whitespace 应被 strip。"""

    result = expand_uuid("  2A00  ")
    assert result == expand_uuid(0x2A00)


# ── expand_uuid invalid hex ──────────────────────────────────────
def test_expand_uuid_invalid_hex_raises():
    with pytest.raises(ValueError):
        expand_uuid("not_hex")


def test_expand_uuid_empty_string_raises():
    with pytest.raises(ValueError):
        expand_uuid("")


# ── expand_uuid 128 位 str 不变 ──────────────────────────────────
def test_expand_uuid_128bit_string_passthrough():
    full = "12345678-1234-1234-1234-123456789abc"
    assert expand_uuid(full) == full  # 36 字符直传


def test_expand_uuid_128bit_uppercase_normalized():
    """128 位大写输入归一化为小写。"""

    full_upper = "12345678-1234-1234-1234-123456789ABC"
    assert expand_uuid(full_upper) == full_upper.lower()


# ── BleCharacteristic 默认值 + 属性 ──────────────────────────────
def test_characteristic_defaults():
    c = BleCharacteristic(uuid="test")
    assert c.properties == frozenset()
    assert c.value == b""
    assert c.handle == 0
    assert c.can_read is False
    assert c.can_write is False
    assert c.can_notify is False


def test_characteristic_read_only():
    c = BleCharacteristic(uuid="r", properties=frozenset({"read"}))
    assert c.can_read is True
    assert c.can_write is False


def test_characteristic_all_properties():
    c = BleCharacteristic(
        uuid="rw",
        properties=frozenset({"read", "write", "notify"}),
        value=b"\x01",
        handle=5,
    )
    assert c.can_read and c.can_write and c.can_notify
    assert c.value == b"\x01"
    assert c.handle == 5


# ── BleGattTree.find_by_uuid ─────────────────────────────────────
def test_gatt_tree_find_by_uuid_known():
    svc = BleService(uuid="svc", characteristics=[
        BleCharacteristic(uuid="0000abcd-0000-1000-8000-00805f9b34fb", handle=1),
    ])
    tree = BleGattTree(device=BleDevice(name="d", address="A", services=[svc]))
    found = tree.find_by_uuid("0000abcd-0000-1000-8000-00805f9b34fb")
    assert found is not None
    assert found.handle == 1


def test_gatt_tree_find_by_uuid_unknown_returns_none():
    svc = BleService(uuid="svc", characteristics=[])
    tree = BleGattTree(device=BleDevice(name="d", address="A", services=[svc]))
    assert tree.find_by_uuid("0000ffff-0000-1000-8000-00805f9b34fb") is None


def test_gatt_tree_find_by_uuid_multiple_services():
    """跨多服务查找应遍历所有服务的特征。"""

    svc1 = BleService(uuid="s1", characteristics=[
        BleCharacteristic(uuid="0000aaa1-0000-1000-8000-00805f9b34fb", handle=1),
    ])
    svc2 = BleService(uuid="s2", characteristics=[
        BleCharacteristic(uuid="0000bbb2-0000-1000-8000-00805f9b34fb", handle=2),
    ])
    tree = BleGattTree(device=BleDevice(name="d", address="A", services=[svc1, svc2]))
    found = tree.find_by_uuid("0000bbb2-0000-1000-8000-00805f9b34fb")
    assert found is not None and found.handle == 2


# ── 模型 frozen ───────────────────────────────────────────────────
def test_characteristic_is_frozen():
    c = BleCharacteristic(uuid="x")
    with pytest.raises(AttributeError):
        c.uuid = "y"  # type: ignore[misc]


def test_service_is_mutable():
    """BleService 非 frozen（可变，便于构建期修改）。"""

    s = BleService(uuid="x")
    s.uuid = "y"  # 可赋值（非 frozen）
    assert s.uuid == "y"


def test_device_is_mutable():
    """BleDevice 非 frozen（可变）。"""

    d = BleDevice(name="d", address="A")
    d.name = "e"  # 可赋值
    assert d.name == "e"

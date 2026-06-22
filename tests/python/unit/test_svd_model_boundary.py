"""svd/model 边界单元测试。

补强 test_svd.py 未直接断言的边角：
- Access 枚举：3 成员 + value 值 + 互异。
- SvdField：mask 计算（1/2/4/8 位）+ bit_end 属性 + frozen。
- SvdRegister：absolute_address + field 未知名 None + 默认 access。
- SvdPeripheral：register 未知名 None。
- SvdDevice：peripheral 未知名 None + 空设备 register_count/field_count=0。
"""

from __future__ import annotations

import pytest

from embeddebug.serial_station.svd.model import (
    Access,
    SvdDevice,
    SvdField,
    SvdPeripheral,
    SvdRegister,
)


# ── Access 枚举 ───────────────────────────────────────────────────────


def test_access_has_three_members():
    """Access 含 3 个成员。"""

    assert len(Access) == 3


def test_access_values():
    """Access value 小写字符串。"""

    assert Access.READ_ONLY.value == "read-only"
    assert Access.WRITE_ONLY.value == "write-only"
    assert Access.READ_WRITE.value == "read-write"


def test_access_values_distinct():
    """3 个 value 互不相同。"""

    values = {a.value for a in Access}
    assert len(values) == 3


# ── SvdField mask 计算 ────────────────────────────────────────────────


def _field(offset: int = 0, width: int = 1) -> SvdField:
    """构造 SvdField。"""

    return SvdField(
        name="F", description="", bit_offset=offset, bit_width=width,
        access=Access.READ_WRITE,
    )


def test_field_mask_single_bit():
    """1 位 field → mask=1。"""

    assert _field(0, 1).mask() == 1


def test_field_mask_two_bits():
    """2 位 field → mask=3。"""

    assert _field(0, 2).mask() == 3


def test_field_mask_four_bits():
    """4 位 field → mask=15。"""

    assert _field(0, 4).mask() == 15


def test_field_mask_eight_bits():
    """8 位 field → mask=255。"""

    assert _field(0, 8).mask() == 255


def test_field_bit_end():
    """bit_end = bit_offset + bit_width - 1。"""

    assert _field(4, 3).bit_end == 6


def test_field_bit_end_offset_zero():
    """bit_offset=0, bit_width=1 → bit_end=0。"""

    assert _field(0, 1).bit_end == 0


def test_field_is_frozen():
    """SvdField 是 frozen。"""

    f = _field()
    with pytest.raises((AttributeError, Exception)):
        f.name = "Y"  # type: ignore[misc]


# ── SvdRegister 边界 ──────────────────────────────────────────────────


def _register(name: str = "R", offset: int = 0) -> SvdRegister:
    """构造 SvdRegister。"""

    return SvdRegister(name=name, description="", address_offset=offset)


def test_register_absolute_address():
    """absolute_address = peripheral_base + address_offset。"""

    reg = _register("DATA", 0x04)
    assert reg.absolute_address(0x40000000) == 0x40000004


def test_register_absolute_address_zero_base():
    """base=0 → absolute_address=offset。"""

    reg = _register("CTRL", 0x10)
    assert reg.absolute_address(0) == 0x10


def test_register_field_unknown_returns_none():
    """field(未知名) → None。"""

    reg = _register()
    assert reg.field("ghost") is None


def test_register_default_access_read_write():
    """SvdRegister 默认 access=READ_WRITE。"""

    assert _register().access == Access.READ_WRITE


# ── SvdPeripheral 边界 ────────────────────────────────────────────────


def _peripheral(name: str = "P", base: int = 0x40000000) -> SvdPeripheral:
    """构造 SvdPeripheral。"""

    return SvdPeripheral(name=name, description="", base_address=base)


def test_peripheral_register_unknown_returns_none():
    """register(未知名) → None。"""

    p = _peripheral()
    assert p.register("ghost") is None


def test_peripheral_no_registers():
    """无 registers → register() → None。"""

    p = _peripheral()
    assert p.register("anything") is None


# ── SvdDevice 边界 ────────────────────────────────────────────────────


def _device() -> SvdDevice:
    """构造空 SvdDevice。"""

    return SvdDevice(name="D", description="", peripherals=())


def test_device_peripheral_unknown_returns_none():
    """peripheral(未知名) → None。"""

    dev = _device()
    assert dev.peripheral("ghost") is None


def test_device_empty_register_count_zero():
    """空设备 register_count=0。"""

    assert _device().register_count == 0


def test_device_empty_field_count_zero():
    """空设备 field_count=0。"""

    assert _device().field_count == 0


def test_device_empty_iter_registers_empty():
    """空设备 iter_registers → 空。"""

    assert list(_device().iter_registers()) == []

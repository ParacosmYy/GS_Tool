"""SVD 数据模型单元测试 — 位域掩码 + 寄存器/外设结构。

覆盖：SvdField mask/bit_end、Access 枚举、SvdRegister 地址计算、frozen 不可变。
"""

from __future__ import annotations

import pytest

from embeddebug.serial_station.svd.model import Access, SvdField


def test_svd_field_mask_width_4():
    """4 位宽掩码 = 0b1111 = 0xF。"""
    f = SvdField(name="ENABLE", description="", bit_offset=0, bit_width=4)
    assert f.mask() == 0xF


def test_svd_field_mask_width_8():
    """8 位宽掩码 = 0xFF。"""
    f = SvdField(name="DATA", description="", bit_offset=0, bit_width=8)
    assert f.mask() == 0xFF


def test_svd_field_mask_width_0():
    """0 位宽掩码 = 0（防御）。"""
    f = SvdField(name="X", description="", bit_offset=0, bit_width=0)
    assert f.mask() == 0


def test_svd_field_bit_end():
    """bit_end = offset + width - 1。"""
    f = SvdField(name="X", description="", bit_offset=8, bit_width=4)
    assert f.bit_end == 11


def test_svd_field_default_access():
    """默认访问权限 READ_WRITE。"""
    f = SvdField(name="X", description="", bit_offset=0, bit_width=1)
    assert f.access == Access.READ_WRITE


def test_svd_field_access_variants():
    """三种访问权限。"""
    for access in (Access.READ_ONLY, Access.WRITE_ONLY, Access.READ_WRITE):
        f = SvdField(name="X", description="", bit_offset=0, bit_width=1, access=access)
        assert f.access == access


def test_svd_field_frozen():
    """frozen 不可变。"""
    f = SvdField(name="X", description="", bit_offset=0, bit_width=1)
    with pytest.raises((AttributeError, TypeError)):
        f.bit_offset = 1


def test_access_values_are_distinct():
    assert {a.value for a in Access} == {"read-only", "write-only", "read-write"}


def test_svd_field_mask_small_widths():
    assert SvdField("F", "", 0, 1).mask() == 1
    assert SvdField("F", "", 0, 2).mask() == 3


def test_svd_register_boundaries():
    from embeddebug.serial_station.svd.model import SvdRegister
    reg = SvdRegister(name="DATA", description="", address_offset=0x04)
    assert reg.absolute_address(0x40000000) == 0x40000004
    assert reg.absolute_address(0) == 0x04
    assert reg.field("ghost") is None
    assert reg.access == Access.READ_WRITE


def test_svd_peripheral_empty_register_lookup():
    from embeddebug.serial_station.svd.model import SvdPeripheral
    peripheral = SvdPeripheral(name="P", description="", base_address=0x40000000)
    assert peripheral.register("ghost") is None


def test_svd_device_empty_counts_and_lookup():
    from embeddebug.serial_station.svd.model import SvdDevice
    device = SvdDevice(name="D", description="", peripherals=())
    assert device.peripheral("ghost") is None
    assert device.register_count == 0
    assert device.field_count == 0
    assert list(device.iter_registers()) == []

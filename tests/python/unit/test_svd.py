"""CMSIS SVD 解析器单测（纯 Python，无 qtbot）。

覆盖：设备元信息、外设/寄存器/位域解析、位范围两种写法、``<dim>`` 数组展开、
access 枚举全分支、非法 XML、查询 helper。对齐 ``test_can.py`` 头约定。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import pytest

from embeddebug.serial_station.svd import (
    Access,
    SvdDevice,
    SvdField,
    SvdParser,
    SvdPeripheral,
    SvdRegister,
)

# 最小但完整的 CMSIS SVD 样例：1 设备 / 2 外设（GPIO 含位域 + UART 数组展开）。
SVD_SAMPLE = """<?xml version="1.0" encoding="UTF-8"?>
<device schemaVersion="1.3">
    <name>STM32F407</name>
    <description>Arm Cortex-M4 设备演示 SVD</description>
    <width>32</width>
    <size>32</size>
    <peripherals>
        <peripheral>
            <name>GPIOA</name>
            <description>通用 IO 端口 A</description>
            <groupName>GPIO</groupName>
            <baseAddress>0x40020000</baseAddress>
            <access>read-write</access>
            <registers>
                <register>
                    <name>MODER</name>
                    <description>端口模式寄存器</description>
                    <addressOffset>0x00</addressOffset>
                    <size>32</size>
                    <access>read-write</access>
                    <resetValue>0xA8000000</resetValue>
                    <fields>
                        <field>
                            <name>MODE0</name>
                            <description>引脚 0 模式</description>
                            <bitOffset>0</bitOffset>
                            <bitWidth>2</bitWidth>
                            <access>read-write</access>
                            <resetValue>0</resetValue>
                        </field>
                        <field>
                            <name>MODE1</name>
                            <description>引脚 1 模式</description>
                            <bitOffset>2</bitOffset>
                            <bitWidth>2</bitWidth>
                            <resetValue>2</resetValue>
                        </field>
                    </fields>
                </register>
                <register>
                    <name>IDR</name>
                    <description>端口输入数据寄存器</description>
                    <addressOffset>0x10</addressOffset>
                    <size>32</size>
                    <access>read-only</access>
                    <fields>
                        <field>
                            <name>ID15</name>
                            <description>引脚 15 输入电平</description>
                            <msb>15</msb>
                            <lsb>15</lsb>
                            <access>read-only</access>
                        </field>
                    </fields>
                </register>
            </registers>
        </peripheral>
        <peripheral>
            <name>USART%s</name>
            <dim>3</dim>
            <dimIncrement>0x400</dimIncrement>
            <description>通用同步异步收发器</description>
            <baseAddress>0x40011000</baseAddress>
            <registers>
                <register>
                    <name>SR</name>
                    <description>状态寄存器</description>
                    <addressOffset>0x00</addressOffset>
                    <access>read-write</access>
                    <fields>
                        <field>
                            <name>TXE</name>
                            <description>发送数据寄存器空</description>
                            <bitOffset>7</bitOffset>
                            <bitWidth>1</bitWidth>
                        </field>
                    </fields>
                </register>
            </registers>
        </peripheral>
    </peripherals>
</device>
"""


def test_parse_device_metadata():
    dev = SvdParser.parse(SVD_SAMPLE)
    assert isinstance(dev, SvdDevice)
    assert dev.name == "STM32F407"
    assert dev.width == 32
    assert "Cortex-M4" in dev.description


def test_parse_peripherals_count_and_query():
    dev = SvdParser.parse(SVD_SAMPLE)
    # GPIOA + USART0/1/2 数组展开 = 4 个外设。
    names = [p.name for p in dev.peripherals]
    assert "GPIOA" in names
    assert names.count("GPIOA") == 1
    assert dev.peripheral("GPIOA") is not None
    assert dev.peripheral("NOPE") is None


def test_parse_dim_array_expands_three_uarts():
    dev = SvdParser.parse(SVD_SAMPLE)
    usarts = [p for p in dev.peripherals if p.name.startswith("USART")]
    assert len(usarts) == 3
    # 地址按 dimIncrement=0x400 递增：0x40011000, 0x40011400, 0x40011800。
    assert usarts[0].base_address == 0x40011000
    assert usarts[1].base_address == 0x40011400
    assert usarts[2].base_address == 0x40011800
    assert usarts[0].name == "USART0"


def test_parse_register_address_and_size():
    dev = SvdParser.parse(SVD_SAMPLE)
    gpio = dev.peripheral("GPIOA")
    moder = gpio.register("MODER")
    assert moder is not None
    assert moder.address_offset == 0x00
    assert moder.size == 32
    assert moder.reset_value == 0xA8000000
    # 绝对地址 = 外设基址 + 偏移。
    assert moder.absolute_address(gpio.base_address) == 0x40020000
    idr = gpio.register("IDR")
    assert idr.address_offset == 0x10


def test_parse_field_bitoffset_width_style():
    dev = SvdParser.parse(SVD_SAMPLE)
    moder = dev.peripheral("GPIOA").register("MODER")
    mode0 = moder.field("MODE0")
    assert mode0.bit_offset == 0
    assert mode0.bit_width == 2
    assert mode0.bit_end == 1
    assert mode0.mask() == 0b11
    mode1 = moder.field("MODE1")
    assert mode1.reset_value == 2


def test_parse_field_msb_lsb_style():
    dev = SvdParser.parse(SVD_SAMPLE)
    idr = dev.peripheral("GPIOA").register("IDR")
    id15 = idr.field("ID15")
    assert id15.bit_offset == 15
    assert id15.bit_width == 1
    assert id15.bit_end == 15


def test_access_enum_read_only():
    dev = SvdParser.parse(SVD_SAMPLE)
    idr = dev.peripheral("GPIOA").register("IDR")
    assert idr.access == Access.READ_ONLY
    id15 = idr.field("ID15")
    assert id15.access == Access.READ_ONLY


def test_access_enum_unknown_defaults_read_write():
    # 缺省 access 的位域（TXE 无 <access>）兜底 READ_WRITE。
    dev = SvdParser.parse(SVD_SAMPLE)
    usart0 = next(p for p in dev.peripherals if p.name == "USART0")
    txe = usart0.register("SR").field("TXE")
    assert txe.access == Access.READ_WRITE


def test_register_and_field_helpers():
    dev = SvdParser.parse(SVD_SAMPLE)
    moder = dev.peripheral("GPIOA").register("MODER")
    # field() 未命中返回 None。
    assert moder.field("MISSING") is None
    # registers 缺失时 register() 也返回 None。
    empty_periph = SvdPeripheral(name="X", description="", base_address=0)
    assert empty_periph.register("Y") is None


def test_aggregate_counts():
    dev = SvdParser.parse(SVD_SAMPLE)
    # GPIOA 有 MODER(2 fields) + IDR(1 field)；3 个 USART 各有 SR(1 field)。
    assert dev.register_count == 2 + 3
    assert dev.field_count == 3 + 3


def test_parse_invalid_xml_raises_value_error():
    with pytest.raises(ValueError):
        SvdParser.parse("<not valid xml")


def test_parse_empty_device_has_no_peripherals():
    dev = SvdParser.parse(
        '<?xml version="1.0"?><device><name>Empty</name></device>'
    )
    assert dev.name == "Empty"
    assert dev.peripherals == ()
    assert dev.register_count == 0


def test_parse_file_round_trip(tmp_path):
    svd_file = tmp_path / "demo.svd"
    svd_file.write_text(SVD_SAMPLE, encoding="utf-8")
    dev = SvdParser.parse_file(svd_file)
    assert dev.name == "STM32F407"
    assert dev.peripheral("GPIOA") is not None


def test_field_mask_zero_width():
    """位宽 0 的退化位域掩码恒 0（防 ``1 << 0`` 越界）。"""

    f = SvdField(name="x", description="", bit_offset=0, bit_width=0)
    assert f.mask() == 0


def test_iter_registers_yields_peripheral_register_pairs():
    dev = SvdParser.parse(SVD_SAMPLE)
    pairs = list(dev.iter_registers())
    # 每个条目是 (SvdPeripheral, SvdRegister)。
    periph, reg = pairs[0]
    assert isinstance(periph, SvdPeripheral)
    assert isinstance(reg, SvdRegister)
    # 总数与 register_count 一致。
    assert len(pairs) == dev.register_count

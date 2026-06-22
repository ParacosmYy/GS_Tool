"""CMSIS SVD 解析器单元测试 — XML 解析 + 外设/寄存器/位域。

覆盖：SvdParser.parse 最小设备 + 外设/寄存器/位域提取 + Access 枚举 +
parse_file 文件入口 + 无效 XML 抛 ValueError。
"""

from __future__ import annotations

import pytest

from embeddebug.serial_station.svd.model import Access, SvdPeripheral, SvdRegister
from embeddebug.serial_station.svd.parser import SvdParser

_MINIMAL_SVD = """\
<?xml version="1.0" encoding="UTF-8"?>
<device>
  <name>TEST_MCU</name>
  <description>Test device</description>
  <width>32</width>
  <peripherals>
    <peripheral>
      <name>GPIOA</name>
      <baseAddress>0x40020000</baseAddress>
      <registers>
        <register>
          <name>MODER</name>
          <addressOffset>0x00</addressOffset>
          <fields>
            <field>
              <name>MODE0</name>
              <bitOffset>0</bitOffset>
              <bitWidth>2</bitWidth>
              <access>read-write</access>
            </field>
          </fields>
        </register>
      </registers>
    </peripheral>
  </peripherals>
</device>
"""


def test_parse_device_name():
    device = SvdParser.parse(_MINIMAL_SVD)
    assert device.name == "TEST_MCU"
    assert device.description == "Test device"
    assert device.width == 32


def test_parse_peripheral():
    device = SvdParser.parse(_MINIMAL_SVD)
    assert len(device.peripherals) >= 1
    p = device.peripherals[0]
    assert isinstance(p, SvdPeripheral)
    assert p.name == "GPIOA"


def test_parse_register():
    device = SvdParser.parse(_MINIMAL_SVD)
    p = device.peripherals[0]
    assert len(p.registers) >= 1
    reg = p.registers[0]
    assert isinstance(reg, SvdRegister)
    assert reg.name == "MODER"


def test_parse_field():
    device = SvdParser.parse(_MINIMAL_SVD)
    reg = device.peripherals[0].registers[0]
    assert len(reg.fields) >= 1
    field = reg.fields[0]
    assert field.name == "MODE0"
    assert field.bit_offset == 0
    assert field.bit_width == 2
    assert field.access == Access.READ_WRITE


def test_parse_field_mask():
    """位域掩码 = 0b11 = 3。"""
    device = SvdParser.parse(_MINIMAL_SVD)
    field = device.peripherals[0].registers[0].fields[0]
    assert field.mask() == 3


def test_parse_empty_peripherals():
    """无 peripherals 节点的设备。"""
    svd = '<device><name>X</name></device>'
    device = SvdParser.parse(svd)
    assert device.name == "X"
    assert device.peripherals == ()


def test_parse_invalid_xml():
    with pytest.raises(ValueError):
        SvdParser.parse("<not valid xml")


def test_parse_unknown_access_defaults_read_write():
    """未识别的 access 值回退到 READ_WRITE。"""
    svd = """\
<device>
  <name>X</name>
  <peripherals>
    <peripheral>
      <name>P</name>
      <baseAddress>0x1000</baseAddress>
      <registers>
        <register>
          <name>R</name>
          <fields>
            <field>
              <name>F</name>
              <bitOffset>0</bitOffset>
              <bitWidth>1</bitWidth>
              <access>writeOnce</access>
            </field>
          </fields>
        </register>
      </registers>
    </peripheral>
  </peripherals>
</device>
"""
    device = SvdParser.parse(svd)
    field = device.peripherals[0].registers[0].fields[0]
    assert field.access == Access.READ_WRITE  # 兜底


def test_parse_file(tmp_path):
    f = tmp_path / "test.svd"
    f.write_text(_MINIMAL_SVD, encoding="utf-8")
    device = SvdParser.parse_file(f)
    assert device.name == "TEST_MCU"

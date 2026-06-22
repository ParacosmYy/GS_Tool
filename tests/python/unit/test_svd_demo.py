"""svd.demo 单元测试 — demo_device + DEMO_SVD_XML 演示数据契约。

demo_device() 是「加载演示」按钮的入口；DEMO_SVD_XML 是内嵌的最小完整 SVD
样例（1 设备 / 2 外设 / GPIOA 含位域 + USART 数组展开成 3 个）。
本测试验证：
- demo_device 返回合法 SvdDevice（设备元信息 / 外设结构 / dim 数组展开）
- DEMO_SVD_XML 字符串契约稳定（含关键 XML 标签）
- demo_device 与 SvdParser.parse 幂等
- SvdDevice 辅助方法（peripheral/register/iter_registers/register_count/field_count）
  在演示数据上行为正确
"""

from __future__ import annotations

from xml.etree import ElementTree as ET

from embeddebug.serial_station.svd.demo import DEMO_SVD_XML, demo_device
from embeddebug.serial_station.svd.model import Access, SvdDevice
from embeddebug.serial_station.svd.parser import SvdParser


# ----------------------- DEMO_SVD_XML 契约 -----------------------


def test_demo_svd_xml_is_well_formed_xml():
    """DEMO_SVD_XML 必须是合法 XML（演示数据被解析前的基础约束）。"""
    root = ET.fromstring(DEMO_SVD_XML)
    assert root.tag == "device"


def test_demo_svd_xml_contains_expected_device_metadata():
    """DEMO_SVD_XML 含设备名/描述/位宽等关键标签。"""
    assert "<name>DemoMCU</name>" in DEMO_SVD_XML
    assert "<description>演示 SVD" in DEMO_SVD_XML
    assert "<width>32</width>" in DEMO_SVD_XML
    assert "<size>32</size>" in DEMO_SVD_XML


def test_demo_svd_xml_contains_two_peripherals_definition():
    """DEMO_SVD_XML 定义 2 个外设：GPIOA + USART%s 数组。"""
    assert "<name>GPIOA</name>" in DEMO_SVD_XML
    assert "<name>USART%s</name>" in DEMO_SVD_XML
    assert "<dim>3</dim>" in DEMO_SVD_XML
    assert "<dimIncrement>0x400</dimIncrement>" in DEMO_SVD_XML


def test_demo_svd_xml_gpioa_has_moder_with_bit_offset_fields():
    """GPIOA.MODER 含 MODE0/MODE1（bitOffset+bitWidth 写法）。"""
    assert "<name>MODER</name>" in DEMO_SVD_XML
    assert "<name>MODE0</name>" in DEMO_SVD_XML
    assert "<bitOffset>0</bitOffset>" in DEMO_SVD_XML
    assert "<bitWidth>2</bitWidth>" in DEMO_SVD_XML


def test_demo_svd_xml_idr_uses_msb_lsb_alternate_syntax():
    """GPIOA.IDR.ID15 用 msb/lsb 写法（非 bitOffset/bitWidth）。"""
    assert "<name>IDR</name>" in DEMO_SVD_XML
    assert "<msb>15</msb>" in DEMO_SVD_XML
    assert "<lsb>15</lsb>" in DEMO_SVD_XML


# ----------------------- demo_device 返回契约 -----------------------


def test_demo_device_returns_svddevice_instance():
    device = demo_device()
    assert isinstance(device, SvdDevice)


def test_demo_device_metadata():
    device = demo_device()
    assert device.name == "DemoMCU"
    assert "演示 SVD" in device.description
    assert device.width == 32


def test_demo_device_expands_usart_array_into_three_peripherals():
    """USART%s dim=3 应展开为 USART0/USART1/USART2（共 4 个外设）。"""
    device = demo_device()
    peripheral_names = [p.name for p in device.peripherals]
    assert peripheral_names == ["GPIOA", "USART0", "USART1", "USART2"]


def test_demo_device_gpioa_base_address():
    device = demo_device()
    gpioa = device.peripheral("GPIOA")
    assert gpioa is not None
    assert gpioa.base_address == 0x40020000


def test_demo_device_usart_array_addresses_increment_by_dimincrement():
    """USART0/1/2 基址按 dimIncrement=0x400 递增。"""
    device = demo_device()
    base = 0x40011000
    for i, name in enumerate(["USART0", "USART1", "USART2"]):
        p = device.peripheral(name)
        assert p is not None
        assert p.base_address == base + i * 0x400


# ----------------------- SvdDevice 辅助方法（演示数据上验证） -----------------------


def test_demo_device_register_count_aggregates_across_peripherals():
    """register_count = GPIOA(2) + USART0(1) + USART1(1) + USART2(1) = 5。"""
    device = demo_device()
    assert device.register_count == 5


def test_demo_device_field_count_aggregates_across_registers():
    """field_count = GPIOA.MODER(2) + GPIOA.IDR(1) + 3×USART.SR(2) = 9。"""
    device = demo_device()
    assert device.field_count == 9


def test_demo_device_iter_registers_yields_peripheral_register_pairs():
    device = demo_device()
    pairs = list(device.iter_registers())
    assert len(pairs) == device.register_count
    # 第一个 pair 应是 (GPIOA, MODER)
    peripheral, register = pairs[0]
    assert peripheral.name == "GPIOA"
    assert register.name == "MODER"


def test_demo_device_peripheral_lookup_unknown_returns_none():
    device = demo_device()
    assert device.peripheral("GHOST") is None


def test_demo_device_gpioa_register_lookup_by_name():
    device = demo_device()
    gpioa = device.peripheral("GPIOA")
    moder = gpioa.register("MODER")
    assert moder is not None
    assert moder.access == Access.READ_WRITE


def test_demo_device_gpioa_register_unknown_returns_none():
    device = demo_device()
    gpioa = device.peripheral("GPIOA")
    assert gpioa.register("GHOST") is None


# ----------------------- demo_device 幂等性 -----------------------


def test_demo_device_is_idempotent_with_svdparser_parse():
    """demo_device() 等价于 SvdParser.parse(DEMO_SVD_XML)。"""
    via_helper = demo_device()
    via_parser = SvdParser.parse(DEMO_SVD_XML)
    assert via_helper.name == via_parser.name
    assert [p.name for p in via_helper.peripherals] == [p.name for p in via_parser.peripherals]
    assert via_helper.register_count == via_parser.register_count


def test_demo_device_repeat_calls_return_equal_structure():
    """多次调用 demo_device() 返回结构等价的设备（不缓存但数据稳定）。"""
    first = demo_device()
    second = demo_device()
    assert first.name == second.name
    assert first.register_count == second.register_count
    assert first.field_count == second.field_count


# ----------------------- GPIOA 位域语义（演示数据可信度） -----------------------


def test_demo_device_gpioa_moder_has_two_fields_with_correct_offsets():
    """GPIOA.MODER 有 MODE0(offset 0) + MODE1(offset 2)。"""
    device = demo_device()
    moder = device.peripheral("GPIOA").register("MODER")
    field_names = [f.name for f in moder.fields]
    assert field_names == ["MODE0", "MODE1"]
    offsets = [f.bit_offset for f in moder.fields]
    assert offsets == [0, 2]


def test_demo_device_gpioa_idr_id15_uses_msb_lsb_resolved_to_offset_width():
    """IDR.ID15 msb=lsb=15 应解析为 offset=15 width=1。"""
    device = demo_device()
    idr = device.peripheral("GPIOA").register("IDR")
    id15 = next(f for f in idr.fields if f.name == "ID15")
    assert id15.bit_offset == 15
    assert id15.bit_width == 1


def test_demo_device_usart_sr_has_txe_and_rxne_fields():
    device = demo_device()
    usart0 = device.peripheral("USART0")
    sr = usart0.register("SR")
    field_names = {f.name for f in sr.fields}
    assert field_names == {"TXE", "RXNE"}

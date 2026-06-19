"""演示用 CMSIS SVD 样例（内嵌 XML 文本 + 便捷构造器）。

提供一份真实风格的小型 SVD，供 SvdPanel「加载演示」按钮直接展示，无需用户
准备 .svd 文件即可体验寄存器浏览（对齐 BLE/CAN 面板的 stub 演示模式）。
"""

from __future__ import annotations

from embeddebug.serial_station.svd.model import SvdDevice
from embeddebug.serial_station.svd.parser import SvdParser

# 最小但完整：1 设备 / 2 外设（GPIOA 含位域 + USART 数组展开成 3 个）。
# 与 test_svd.py 的样例同源，保证「演示数据」与「测试数据」语义一致。
DEMO_SVD_XML = """<?xml version="1.0" encoding="UTF-8"?>
<device schemaVersion="1.3">
    <name>DemoMCU</name>
    <description>演示 SVD（Cortex-M4，含 GPIO/USART 外设）</description>
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
                    <description>端口模式寄存器（每 2 位配一个引脚）</description>
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
                        <field>
                            <name>RXNE</name>
                            <description>读数据寄存器非空</description>
                            <bitOffset>5</bitOffset>
                            <bitWidth>1</bitWidth>
                            <access>read-only</access>
                        </field>
                    </fields>
                </register>
            </registers>
        </peripheral>
    </peripherals>
</device>
"""


def demo_device() -> SvdDevice:
    """解析演示 SVD → ``SvdDevice``（「加载演示」按钮调用）。"""

    return SvdParser.parse(DEMO_SVD_XML)


__all__ = ["DEMO_SVD_XML", "demo_device"]

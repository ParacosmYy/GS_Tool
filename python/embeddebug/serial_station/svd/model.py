"""CMSIS SVD 数据模型（纯 Python 无外部依赖）。

封装从 SVD XML 解析得到的设备/外设/寄存器/位域结构。所有值类型用
``@dataclass(frozen=True)``（不可变）；容器类型存 ``tuple`` 而非 ``list``，
避免可变性导致的隐患。解析逻辑在 ``parser.py``，本模块只描述结构。

约束：仅依赖标准库；不 import PyQt、不依赖 transport；不反向依赖 UI。
"""

from __future__ import annotations

from dataclasses import dataclass
from enum import Enum
from collections.abc import Iterator


class Access(Enum):
    """SVD ``<access>`` 访问权限枚举。

    CMSIS SVD 规范定义 6 种值，本查看器收敛到三种常用态；解析时未识别的
    值（如 writeOnce/read-writeOnce）按 ``READ_WRITE`` 兜底。
    """

    READ_ONLY = "read-only"
    WRITE_ONLY = "write-only"
    READ_WRITE = "read-write"


@dataclass(frozen=True)
class SvdField:
    """单个位域（``<field>``）。

    - ``bit_offset`` / ``bit_width``：位范围，来自 ``<bitOffset>`` + ``<bitWidth>``
      或 ``<msb>`` + ``<lsb>``（解析器统一成 offset/width）。
    - ``reset_value``：复位值（十六进制原样，``int`` 已转好）；缺省为 ``0``。
    """

    name: str
    description: str
    bit_offset: int
    bit_width: int
    access: Access = Access.READ_WRITE
    reset_value: int = 0

    @property
    def bit_end(self) -> int:
        """位域最高位序号（offset + width - 1）。"""

        return self.bit_offset + max(self.bit_width - 1, 0)

    def mask(self) -> int:
        """位域掩码（width 个连续 1，例如 width=4 → 0b1111）。"""

        return (1 << self.bit_width) - 1 if self.bit_width > 0 else 0


@dataclass(frozen=True)
class SvdRegister:
    """单个寄存器（``<register>``）。

    - ``address_offset``：相对外设基址的偏移（字节）。
    - ``size``：位宽（8/16/32/64），缺省 32。
    - ``fields``：位域元组（按 SVD 文档顺序）。
    """

    name: str
    description: str
    address_offset: int
    size: int = 32
    access: Access = Access.READ_WRITE
    reset_value: int = 0
    fields: tuple[SvdField, ...] = ()

    def absolute_address(self, peripheral_base: int) -> int:
        """寄存器绝对地址 = 外设基址 + 偏移。"""

        return peripheral_base + self.address_offset

    def field(self, name: str) -> SvdField | None:
        """按名称查位域，未命中返回 ``None``。"""

        for f in self.fields:
            if f.name == name:
                return f
        return None


@dataclass(frozen=True)
class SvdPeripheral:
    """单个外设（``<peripheral>``）。

    - ``base_address``：外设基址（绝对，已含设备级偏移）。
    - ``registers``：寄存器元组（按 SVD 文档顺序）。
    """

    name: str
    description: str
    base_address: int
    group_name: str = ""
    registers: tuple[SvdRegister, ...] = ()

    def register(self, name: str) -> SvdRegister | None:
        """按名称查寄存器，未命中返回 ``None``。"""

        for r in self.registers:
            if r.name == name:
                return r
        return None


@dataclass(frozen=True)
class SvdDevice:
    """SVD ``<device>`` 根节点：设备元信息 + 外设元组。"""

    name: str
    description: str
    width: int = 32
    peripherals: tuple[SvdPeripheral, ...] = ()

    def peripheral(self, name: str) -> SvdPeripheral | None:
        """按名称查外设，未命中返回 ``None``。"""

        for p in self.peripherals:
            if p.name == name:
                return p
        return None

    def iter_registers(self) -> Iterator[tuple[SvdPeripheral, SvdRegister]]:
        """扁平遍历 (外设, 寄存器) 对，便于全局搜索/计数。"""

        for p in self.peripherals:
            for r in p.registers:
                yield p, r

    @property
    def register_count(self) -> int:
        """全设备寄存器总数（跨外设求和）。"""

        return sum(len(p.registers) for p in self.peripherals)

    @property
    def field_count(self) -> int:
        """全设备位域总数（跨寄存器求和）。"""

        return sum(len(r.fields) for _, r in self.iter_registers())


__all__ = [
    "Access",
    "SvdDevice",
    "SvdField",
    "SvdPeripheral",
    "SvdRegister",
]

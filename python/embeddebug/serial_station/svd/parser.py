"""CMSIS SVD XML 解析器（纯 Python，仅依赖标准库 ``xml.etree.ElementTree``）。

入口 ``SvdParser.parse(text)`` 把 SVD XML 文本转成 ``SvdDevice``。覆盖 CMSIS SVD
规范的常用子集：device 元信息、``<peripherals>`` / ``<registers>`` / ``<fields>``、
``<dim>`` 数组外设展开、位域的 ``<bitOffset>+<bitWidth>`` 与 ``<msb>+<lsb>`` 两种写法。

约定（对齐 ``can/dbc.py``）：``@classmethod parse`` 为公开入口，``_parse_*`` 私有
helper，绝对 import。不 import PyQt、不依赖 transport。
"""

from __future__ import annotations

import xml.etree.ElementTree as ET
from pathlib import Path

from embeddebug.serial_station.svd.model import (
    Access,
    SvdDevice,
    SvdField,
    SvdPeripheral,
    SvdRegister,
)

# SVD <access> 文本 → Access 枚举；未识别的值兜底 READ_WRITE（见 model.Access 注释）。
_ACCESS_BY_TEXT = {
    "read-only": Access.READ_ONLY,
    "write-only": Access.WRITE_ONLY,
    "read-write": Access.READ_WRITE,
}


def _text(el: ET.Element | None, tag: str, default: str = "") -> str:
    """取子元素 ``tag`` 的文本，缺失返回 ``default``。"""

    if el is None:
        return default
    child = el.find(tag)
    if child is None or child.text is None:
        return default
    return child.text.strip()


def _int(el: ET.Element | None, tag: str, default: int = 0) -> int:
    """取子元素 ``tag`` 文本并解析为 int；支持 ``0x`` 十六进制与十进制。

    非法或缺省返回 ``default``，不抛（SVD 字段常缺失，兜底 0 更稳）。
    """

    raw = _text(el, tag)
    if not raw:
        return default
    try:
        return int(raw, 0)  # base=0 自动识别 0x 前缀。
    except ValueError:
        return default


def _access(el: ET.Element | None) -> Access:
    """解析 ``<access>`` 文本为枚举；未识别/缺省 → ``READ_WRITE``。"""

    return _ACCESS_BY_TEXT.get(_text(el, "access"), Access.READ_WRITE)


def _parse_field(el: ET.Element) -> SvdField:
    """单个 ``<field>`` → ``SvdField``。位范围兼容两种写法。"""

    name = _text(el, "name", "?")
    desc = _text(el, "description")
    # 位范围：优先 bitOffset+bitWidth，否则用 msb/lsb 推算。
    if el.find("bitOffset") is not None:
        offset = _int(el, "bitOffset")
        width = _int(el, "bitWidth", 1)
    elif el.find("msb") is not None and el.find("lsb") is not None:
        msb = _int(el, "msb")
        lsb = _int(el, "lsb")
        offset = lsb
        width = msb - lsb + 1
    else:
        # 无位范围信息，置 0/1（位宽至少 1 避免掩码恒 0）。
        offset = 0
        width = 1
    return SvdField(
        name=name,
        description=desc,
        bit_offset=offset,
        bit_width=width,
        access=_access(el),
        reset_value=_int(el, "resetValue"),
    )


def _parse_register(el: ET.Element) -> SvdRegister:
    """单个 ``<register>`` → ``SvdRegister``，含位域。"""

    fields_el = el.find("fields")
    fields = tuple(
        _parse_field(f) for f in (fields_el.findall("field") if fields_el is not None else [])
    )
    return SvdRegister(
        name=_text(el, "name", "?"),
        description=_text(el, "description"),
        address_offset=_int(el, "addressOffset"),
        size=_int(el, "size", 32),
        access=_access(el),
        reset_value=_int(el, "resetValue"),
        fields=fields,
    )


def _parse_peripheral(el: ET.Element) -> list[SvdPeripheral]:
    """单个 ``<peripheral>`` → 一个或多个 ``SvdPeripheral``（含 ``<dim>`` 数组展开）。

    ``<dim>`` 数组外设：SVD 用 ``dim``+``dimIncrement``+``name="%s[idx]"`` 表达
    N 个相同外设。本查看器展开为 N 个独立 ``SvdPeripheral``（仅做名称替换，
    地址按 dimIncrement 递增；寄存器块共享引用，足够浏览用）。返回 list 便于
    调用方 ``extend``（非数组外设返回单元素 list）。
    """

    base = _int(el, "baseAddress")
    name = _text(el, "name", "?")
    group = _text(el, "groupName")
    desc = _text(el, "description")
    registers_el = el.find("registers")
    registers = tuple(
        _parse_register(r)
        for r in (registers_el.findall("register") if registers_el is not None else [])
    )
    dim = _int(el, "dim", 1)
    dim_inc = _int(el, "dimIncrement", 0x400)  # 典型外设间距 0x400。
    has_array_marker = "%s" in name or "[" in name
    if dim <= 1 or not has_array_marker:
        return [
            SvdPeripheral(
                name=name,
                description=desc,
                base_address=base,
                group_name=group,
                registers=registers,
            )
        ]
    # 数组展开：name 含 %s 占位或 [N] 时按 dim 索引替换，地址递增。
    out: list[SvdPeripheral] = []
    for i in range(dim):
        if "%s" in name:
            idx_name = name.replace("%s", str(i))
        else:
            import re

            idx_name = re.sub(r"\[\d*\]", f"[{i}]", name)
        out.append(
            SvdPeripheral(
                name=idx_name,
                description=desc,
                base_address=base + i * dim_inc,
                group_name=group,
                registers=registers,
            )
        )
    return out


def _parse_peripherals(root: ET.Element) -> tuple[SvdPeripheral, ...]:
    """解析全部 ``<peripheral>``，展开数组外设。"""

    periph_root = root.find("peripherals")
    if periph_root is None:
        return ()
    out: list[SvdPeripheral] = []
    for el in periph_root.findall("peripheral"):
        out.extend(_parse_peripheral(el))
    return tuple(out)


class SvdParser:
    """CMSIS SVD 解析器。无实例状态，``parse`` 为类方法入口。"""

    @classmethod
    def parse(cls, text: str) -> SvdDevice:
        """解析 SVD XML 文本 → ``SvdDevice``。

        抛 ``ValueError``：XML 格式错误或无 ``<device>`` 根。
        """

        try:
            root = ET.fromstring(text)
        except ET.ParseError as exc:
            raise ValueError(f"invalid SVD XML: {exc}") from exc
        return cls._build_device(root)

    @classmethod
    def parse_file(cls, path: str | Path) -> SvdDevice:
        """从文件路径读取并解析（便捷入口，UI 的 QFileDialog 后调用）。"""

        return cls.parse(Path(path).read_text(encoding="utf-8"))

    @classmethod
    def _build_device(cls, root: ET.Element) -> SvdDevice:
        return SvdDevice(
            name=_text(root, "name", "UnknownDevice"),
            description=_text(root, "description"),
            width=_int(root, "width", 32),
            peripherals=_parse_peripherals(root),
        )


__all__ = ["SvdParser"]

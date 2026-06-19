"""CMSIS SVD 寄存器描述子模块。

提供 SVD XML 解析与外设/寄存器/位域数据模型。纯 Python、仅依赖标准库，
不 import PyQt、不依赖 transport。UI 层（``ui/panels/svd_panel.py``）只 import
本 ``__init__`` 聚合的公共 API。

公开符号：
- ``Access``：访问权限枚举。
- ``SvdDevice`` / ``SvdPeripheral`` / ``SvdRegister`` / ``SvdField``：数据结构。
- ``SvdParser``：XML 解析入口（``parse`` / ``parse_file``）。
"""

from __future__ import annotations

from embeddebug.serial_station.svd.model import (
    Access,
    SvdDevice,
    SvdField,
    SvdPeripheral,
    SvdRegister,
)
from embeddebug.serial_station.svd.parser import SvdParser

__all__ = [
    "Access",
    "SvdDevice",
    "SvdField",
    "SvdParser",
    "SvdPeripheral",
    "SvdRegister",
]

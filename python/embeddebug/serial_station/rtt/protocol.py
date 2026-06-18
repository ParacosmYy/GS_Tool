"""SEGGER RTT 协议契约与控制块布局描述（纯 Python 桩）。

本模块只定义 RTT 通道/配置的数据结构与 SEGGER RTT 控制块（Control Block）
的布局常量，不包含任何 J-Link DLL 或原生内存访问；真实读写在后续迭代由
目标端替身或真实调试探针注入。
"""

from __future__ import annotations

from dataclasses import dataclass
from typing import Literal

SEGGER_RTT_MAGIC = "RTT\0"
SEGGER_RTT_MAGIC_BYTES = SEGGER_RTT_MAGIC.encode("ascii")
SEGGER_RTT_CB_ID = "SEGGER RTT"
SEGGER_RTT_CB_ID_BYTES = SEGGER_RTT_CB_ID.encode("ascii")

CB_ACID_OFFSET = 0
CB_ACID_SIZE = 16
CB_MAX_UP_BUFFERS_OFFSET = 16
CB_MAX_DOWN_BUFFERS_OFFSET = 20
CB_BUFFERS_OFFSET = 24

BUFFER_HEADER_SIZE = 16
BUFFER_SIZE_OF_BUFFER_OFFSET = 0
BUFFER_WR_OFF_OFFSET = 4
BUFFER_RD_OFF_OFFSET = 8
BUFFER_FLAGS_OFFSET = 12

RttChannelMode = Literal["up", "down"]


@dataclass(frozen=True)
class RttChannel:
    """单个 RTT 通道配置。

    name         通道名（对应 SEGGER aUp/aDown 数组项的语义描述）。
    buffer_size  目标端环形缓冲容量（字节），同时作为主机端镜像环上限。
    mode         通道方向，``up`` 或 ``down``。
    """

    name: str
    buffer_size: int
    mode: RttChannelMode


@dataclass(frozen=True)
class RttConfig:
    """RTT 会话配置。

    channels  本会话使用的通道集合（按定义顺序，索引即 SEGGER 通道号）。
    ram_base  目标 RAM 中控制块预期基地址（桩阶段仅记录，不访问）。
    """

    channels: tuple[RttChannel, ...]
    ram_base: int = 0x2000_0000


def control_block_layout(max_up: int, max_down: int) -> list[tuple[str, int, int]]:
    """返回控制块字段 ``(name, offset, size)`` 描述表（纯描述，不打包内存）。"""

    layout: list[tuple[str, int, int]] = [
        ("acID", CB_ACID_OFFSET, CB_ACID_SIZE),
        ("MaxNumUpBuffers", CB_MAX_UP_BUFFERS_OFFSET, 4),
        ("MaxNumDownBuffers", CB_MAX_DOWN_BUFFERS_OFFSET, 4),
    ]
    offset = CB_BUFFERS_OFFSET
    for index in range(max_up):
        layout.append((f"aUp[{index}].header", offset, BUFFER_HEADER_SIZE))
        offset += BUFFER_HEADER_SIZE
    for index in range(max_down):
        layout.append((f"aDown[{index}].header", offset, BUFFER_HEADER_SIZE))
        offset += BUFFER_HEADER_SIZE
    return layout


def channel_index(channels: tuple[RttChannel, ...], name: str) -> int:
    """返回通道名对应的索引；不存在时抛出 ``KeyError``。"""
    for index, channel in enumerate(channels):
        if channel.name == name:
            return index
    raise KeyError(name)

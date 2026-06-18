"""CAN / CAN-FD 帧与标识符、过滤器的基础数据结构。"""

from __future__ import annotations

from dataclasses import dataclass
from typing import Any

STANDARD_ID_MAX = 0x7FF
EXTENDED_ID_MAX = 0x1FFFFFFF
CAN_MAX_DLC = 8
CAN_FD_MAX_DLC = 64


@dataclass(frozen=True)
class CanId:
    """CAN 标识符封装，区分标准帧(11 位)与扩展帧(29 位)。"""

    value: int
    is_extended: bool = False

    def __post_init__(self) -> None:
        limit = EXTENDED_ID_MAX if self.is_extended else STANDARD_ID_MAX
        if not 0 <= self.value <= limit:
            raise ValueError(
                f"非法 CAN ID 0x{self.value:X}，"
                f"{'扩展' if self.is_extended else '标准'}帧上限 0x{limit:X}"
            )

    def is_standard(self) -> bool:
        """是否为 11 位标准帧。"""
        return not self.is_extended

    def as_hex(self) -> str:
        """以十六进制字符串返回。"""
        width = 8 if self.is_extended else 3
        return f"{self.value:0{width}X}"


@dataclass(frozen=True)
class CanFrame:
    """一帧 CAN 或 CAN-FD 报文。"""

    can_id: CanId
    data: bytes
    is_fd: bool = False
    timestamp: float = 0.0
    frame_index: int = 0

    @property
    def dlc(self) -> int:
        """数据长度码。"""
        return len(self.data)

    def __post_init__(self) -> None:
        limit = CAN_FD_MAX_DLC if self.is_fd else CAN_MAX_DLC
        if not 0 <= len(self.data) <= limit:
            raise ValueError(
                f"非法 DLC {len(self.data)}，{'CAN-FD' if self.is_fd else 'CAN'} 上限 {limit}"
            )

    def to_payload(self) -> dict[str, Any]:
        """转为协议事件 payload 字典。"""
        return {
            "canId": self.can_id.value,
            "canIdHex": self.can_id.as_hex(),
            "isExtended": self.can_id.is_extended,
            "isFd": self.is_fd,
            "dlc": self.dlc,
            "data": bytes(self.data),
            "dataHex": self.data.hex(" "),
            "timestamp": self.timestamp,
            "frameIndex": self.frame_index,
        }


@dataclass
class CanFilter:
    """硬件式 CAN 过滤器：按 id 与 mask 匹配 CAN ID。"""

    id: int
    mask: int = 0
    is_extended: bool | None = None

    def matches(self, can_id: CanId) -> bool:
        """判断给定 CanId 是否命中过滤器。"""
        if self.is_extended is not None and can_id.is_extended != self.is_extended:
            return False
        return (can_id.value & self.mask) == (self.id & self.mask)

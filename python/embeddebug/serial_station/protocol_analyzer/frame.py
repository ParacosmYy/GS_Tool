"""协议帧数据结构。"""

from __future__ import annotations

import time
from dataclasses import dataclass, field
from typing import Any

DIRECTION_TX = "tx"
DIRECTION_RX = "rx"
HEX_DUMP_WIDTH = 16


@dataclass(frozen=True)
class ProtocolFrame:
    """单个协议帧的不可变快照。"""
    raw_bytes: bytes
    timestamp_ns: int = field(default_factory=time.time_ns)
    direction: str = DIRECTION_RX
    decoded: dict[str, Any] = field(default_factory=dict)

    def __post_init__(self) -> None:
        if isinstance(self.raw_bytes, bytearray):
            object.__setattr__(self, "raw_bytes", bytes(self.raw_bytes))

    @property
    def size(self) -> int:
        return len(self.raw_bytes)

    def hex_dump(self) -> str:
        data = self.raw_bytes
        if not data:
            return f"0000:  {' ' * (HEX_DUMP_WIDTH * 3 - 1)}  <empty>"
        lines: list[str] = []
        for offset in range(0, len(data), HEX_DUMP_WIDTH):
            chunk = data[offset : offset + HEX_DUMP_WIDTH]
            hex_part = " ".join(f"{b:02X}" for b in chunk)
            if len(chunk) < HEX_DUMP_WIDTH:
                hex_part = hex_part.ljust(HEX_DUMP_WIDTH * 3 - 1)
            ascii_part = "".join(chr(b) if 0x20 <= b < 0x7F else "." for b in chunk)
            lines.append(f"{offset:04X}:  {hex_part}  {ascii_part}")
        return "\n".join(lines)

    def to_dict(self) -> dict[str, Any]:
        return {"raw_hex": self.raw_bytes.hex(" "), "size": self.size, "timestamp_ns": self.timestamp_ns, "direction": self.direction, "decoded": dict(self.decoded)}

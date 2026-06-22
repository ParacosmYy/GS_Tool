"""十六进制格式化器。"""
from __future__ import annotations
import struct

class HexFormatter:
    """原始字节的 hex dump 与数值解码工具。"""

    @staticmethod
    def format_bytes(data: bytes, offset: int = 0, bytes_per_line: int = 16) -> list[str]:
        if not data:
            return []
        bpl = max(1, min(16, bytes_per_line))
        lines: list[str] = []
        for start in range(0, len(data), bpl):
            chunk = data[start:start + bpl]
            addr = offset + start
            hex_col = " ".join(f"{b:02X}" for b in chunk).ljust(bpl * 3 - 1)
            ascii_col = "".join(chr(b) if 0x20 <= b < 0x7F else "." for b in chunk)
            lines.append(f"{addr:08X}  {hex_col}  |{ascii_col}|")
        return lines

    @staticmethod
    def format_int(data: bytes, signed: bool = True, big_endian: bool = False) -> int:
        if not data:
            raise ValueError("empty")
        order = "big" if big_endian else "little"
        if len(data) in (1, 2, 4, 8):
            fc = {1: "b", 2: "h", 4: "i", 8: "q"}[len(data)]
            if not signed:
                fc = fc.upper()
            prefix = ">" if big_endian else "<"
            return int(struct.unpack(prefix + fc, data)[0])
        return int.from_bytes(data, byteorder=order, signed=signed)

    @staticmethod
    def format_float(data: bytes, big_endian: bool = False) -> float:
        if len(data) == 4:
            fmt = ">f" if big_endian else "<f"
        elif len(data) == 8:
            fmt = ">d" if big_endian else "<d"
        else:
            raise ValueError(f"unsupported float width: {len(data)}")
        return float(struct.unpack(fmt, data)[0])

    @staticmethod
    def format_ascii(data: bytes) -> str:
        return "".join(chr(b) if 0x20 <= b < 0x7F else "." for b in data)

"""YModem 传输：批量文件头与多文件支持。"""

from __future__ import annotations

from embeddebug.serial_station.ota.config import OtaConfig, OtaProgress
from embeddebug.serial_station.ota.xmodem import SOH, XModemTransfer, crc16_ccitt


class YModemTransfer(XModemTransfer):
    """YModem：首块为文件信息。"""

    def __init__(self, config: OtaConfig, progress: OtaProgress | None = None) -> None:
        super().__init__(config, progress)

    def build_header_block(self, filename: str, size: int) -> bytes:
        """构建 YModem 文件信息块（块号 0，128 字节）。"""
        payload = bytearray(filename.encode("utf-8"))
        payload.append(0x00)
        payload.extend(str(size).encode("ascii"))
        payload.append(0x00)
        if len(payload) > 128:
            payload = payload[:128]
        else:
            payload.extend(b"\x00" * (128 - len(payload)))
        body = bytes(payload)
        crc = crc16_ccitt(body)
        return bytes([SOH, 0x00, 0xFF]) + body + bytes([(crc >> 8) & 0xFF, crc & 0xFF])

    def parse_header_block(self, frame: bytes) -> tuple[str, int] | None:
        """从帧还原 (filename, size)。"""
        if len(frame) < 4 or frame[0] != SOH or frame[1] != 0x00:
            return None
        body = frame[3:3 + 128]
        if len(body) < 128:
            return None
        name_part = body.split(b"\x00", 1)
        filename = name_part[0].decode("utf-8", errors="replace")
        size = 0
        if len(name_part) > 1 and name_part[1]:
            size_field = name_part[1].split(b"\x00", 1)[0]
            try:
                size = int(size_field.decode("ascii"))
            except ValueError:
                size = 0
        return filename, size

    def build_end_batch_block(self) -> bytes:
        """结束批次的空文件名块。"""
        body = b"\x00" * 128
        crc = crc16_ccitt(body)
        return bytes([SOH, 0x00, 0xFF]) + body + bytes([(crc >> 8) & 0xFF, crc & 0xFF])

"""ZModem hex 帧编解码助手（16 位 CRC）。"""

from __future__ import annotations

from embeddebug.serial_station.ota.xmodem import crc16_ccitt

ZRINIT = 0x01
ZFILE = 0x04
ZDATA = 0x0A
ZEOF = 0x0B

ZPAD = ord("*")
ZDLE = 0x18
ZHEX = ord("A")
CR = 0x0D
LF = 0x0A
XON = 0x11


def _hex_byte(value: int) -> bytes:
    return ("%02X" % (value & 0xFF)).encode("ascii")


class ZModemTransfer:
    """ZModem hex 帧编解码助手。"""

    def build_hex_frame(self, frame_type: int, data4: bytes) -> bytes:
        """构建十六进制头帧。"""
        if len(data4) != 4:
            raise ValueError("hex 帧数据字段必须为 4 字节")
        body = bytes([frame_type & 0xFF]) + bytes(data4)
        crc = crc16_ccitt(body)
        out = bytearray([ZPAD, ZPAD, ZDLE, ZHEX])
        out += _hex_byte(frame_type)
        for value in data4:
            out += _hex_byte(value)
        out += _hex_byte((crc >> 8) & 0xFF)
        out += _hex_byte(crc & 0xFF)
        out += bytes([CR, LF, XON])
        return bytes(out)

    def parse_hex_frame(self, frame: bytes) -> tuple[int, bytes, int] | None:
        """解析 hex 帧，返回 (type, 4B data, crc) 或 None。"""
        marker = bytes([ZDLE, ZHEX])
        start = frame.find(marker)
        if start < 0:
            return None
        pos = start + len(marker)
        nibbles = bytearray()
        hex_set = set(b"0123456789abcdefABCDEF")
        while pos < len(frame) and frame[pos:pos + 1] in (bytes([c]) for c in hex_set) and len(nibbles) < 14:
            nibbles.append(frame[pos])
            pos += 1
        if len(nibbles) < 14:
            return None
        values = [int(bytes(nibbles[k:k + 2]), 16) for k in range(0, 14, 2)]
        frame_type = values[0]
        data4 = bytes(values[1:5])
        crc = (values[5] << 8) | values[6]
        return frame_type, data4, crc

    @staticmethod
    def is_crc_valid(frame_type: int, data4: bytes, crc: int) -> bool:
        return crc == crc16_ccitt(bytes([frame_type]) + bytes(data4))

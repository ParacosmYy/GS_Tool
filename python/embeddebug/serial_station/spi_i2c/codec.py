"""SPI/I2C 桥接帧编解码与 CRC8。"""

from __future__ import annotations

import struct
from dataclasses import dataclass

from embeddebug.serial_station.spi_i2c.config import I2cConfig, SpiConfig

SOF = 0xAA
EOF = 0x55
OP_SPI_XFER = 0x01
OP_I2C_READ = 0x02
OP_I2C_WRITE = 0x03
OP_RESPONSE = 0x81
STATUS_OK = 0x00
STATUS_ERROR = 0x01
STATUS_ERR_ADDRESS = 0x02
STATUS_ERR_LENGTH = 0x03
_REGISTER_NONE = 0xFF


@dataclass(frozen=True)
class DecodedFrame:
    """解码后的响应帧。"""

    op: int
    status: int
    payload: bytes
    raw: bytes


def crc8(data: bytes) -> int:
    """CRC-8/SMBus。"""
    crc = 0x00
    for byte in data:
        crc ^= byte
        for _ in range(8):
            crc = ((crc << 1) ^ 0x07) & 0xFF if crc & 0x80 else (crc << 1) & 0xFF
    return crc & 0xFF


class SpiI2cFrameCodec:
    """命令帧编码器 + 响应帧流式解码器。"""

    def __init__(self, use_crc: bool = True) -> None:
        self.use_crc = use_crc
        self._buffer: bytearray = bytearray()

    def encode_spi_xfer(self, config: SpiConfig, mosi: bytes) -> bytes:
        flags = 0x01 if config.cs_active_low else 0x00
        payload = bytes([config.mode & 0xFF, flags, config.word_size & 0xFF])
        payload += struct.pack("<I", config.max_speed_hz) + mosi
        return self._wrap(OP_SPI_XFER, payload)

    def encode_i2c_read(self, config: I2cConfig, register: int | None, length: int) -> bytes:
        if not 0 <= length <= 0xFF:
            raise ValueError("i2c read length out of range")
        flags = 0x01 if config.is_ten_bit else 0x00
        reg = _REGISTER_NONE if register is None else register
        payload = struct.pack("<H", config.address) + bytes([flags]) + struct.pack("<H", config.speed_khz) + bytes([reg & 0xFF, length & 0xFF])
        return self._wrap(OP_I2C_READ, payload)

    def encode_i2c_write(self, config: I2cConfig, register: int | None, data: bytes) -> bytes:
        flags = 0x01 if config.is_ten_bit else 0x00
        reg = _REGISTER_NONE if register is None else register
        payload = struct.pack("<H", config.address) + bytes([flags]) + struct.pack("<H", config.speed_khz) + bytes([reg & 0xFF]) + data
        return self._wrap(OP_I2C_WRITE, payload)

    def build_response(self, orig_op: int, status: int, body: bytes = b"") -> bytes:
        return self._wrap(OP_RESPONSE, bytes([orig_op & 0xFF, status & 0xFF]) + body)

    def _wrap(self, op: int, payload: bytes) -> bytes:
        frame = bytearray([SOF, op & 0xFF]) + struct.pack("<H", len(payload)) + payload
        if self.use_crc:
            frame.append(crc8(bytes(frame)))
        frame.append(EOF)
        return bytes(frame)

    def feed(self, data: bytes) -> list[DecodedFrame]:
        if data:
            self._buffer.extend(data)
        frames: list[DecodedFrame] = []
        while True:
            found = self._scan(self._buffer)
            if found is None:
                self._trim_to_sof()
                break
            full, _start, end = found
            del self._buffer[:end]
            decoded = self._decode_one(full)
            if decoded is not None:
                frames.append(decoded)
        return frames

    def reset(self) -> None:
        self._buffer.clear()

    def decode_frame(self, frame: bytes) -> tuple[int, bytes]:
        result = self._scan(bytearray(frame))
        if result is None:
            raise ValueError("incomplete or malformed frame")
        full, start, end = result
        if start != 0 or end != len(frame):
            raise ValueError("frame has leading or trailing junk")
        length = struct.unpack("<H", full[2:4])[0]
        return full[1], bytes(full[4 : 4 + length])

    def _scan(self, buf: bytes) -> tuple[bytes, int, int] | None:
        pos = 0
        n = len(buf)
        crc_len = 1 if self.use_crc else 0
        while pos < n:
            if buf[pos] != SOF:
                pos += 1
                continue
            if pos + 4 > n:
                return None
            length = struct.unpack("<H", bytes(buf[pos + 2 : pos + 4]))[0]
            end = pos + 4 + length + crc_len + 1
            if end > n:
                return None
            frame = bytes(buf[pos:end])
            if frame[-1] != EOF:
                pos += 1
                continue
            if self.use_crc and crc8(frame[:-2]) != frame[-2]:
                pos += 1
                continue
            return frame, pos, end
        return None

    def _trim_to_sof(self) -> None:
        first = self._buffer.find(bytes([SOF]))
        if first == -1:
            self._buffer.clear()
        elif first > 0:
            del self._buffer[:first]

    def _decode_one(self, frame: bytes) -> DecodedFrame | None:
        if frame[1] != OP_RESPONSE:
            return None
        length = struct.unpack("<H", frame[2:4])[0]
        payload = frame[4 : 4 + length]
        if len(payload) < 2:
            return None
        return DecodedFrame(op=payload[0], status=payload[1], payload=bytes(payload[2:]), raw=frame)

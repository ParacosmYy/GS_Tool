"""SPI/I2C 桥内存替身。"""

from __future__ import annotations

import struct

from embeddebug.serial_station.spi_i2c.codec import (
    OP_I2C_READ,
    OP_I2C_WRITE,
    OP_SPI_XFER,
    STATUS_ERR_ADDRESS,
    STATUS_ERR_LENGTH,
    STATUS_OK,
    SpiI2cFrameCodec,
)
from embeddebug.serial_station.spi_i2c.config import I2cConfig, SpiConfig
from embeddebug.serial_station.spi_i2c.transaction import TransactionResult

REG_WHO_AM_I = 0x75
WHO_AM_I_VALUE = 0x68

_DEFAULT_REGS: dict[int, int] = {REG_WHO_AM_I: WHO_AM_I_VALUE, 0x1A: 0x03, 0x41: 0x40}


class SpiI2cBridgeStub:
    """内存桥替身：SPI 回环 + I2C 假传感器寄存器读写。"""

    def __init__(self, codec: SpiI2cFrameCodec | None = None, registers: dict[int, int] | None = None) -> None:
        self._codec = codec or SpiI2cFrameCodec()
        self._regs: dict[int, int] = dict(registers or _DEFAULT_REGS)

    def xfer_spi(self, config: SpiConfig, mosi: bytes) -> bytes:
        """SPI 全双工回环。"""
        return bytes(mosi)

    def read_i2c(self, config: I2cConfig, register: int | None, length: int) -> TransactionResult:
        if not config.is_valid_address():
            return TransactionResult(False, 0, 0, b"", "invalid address")
        start = 0 if register is None else register & 0xFF
        out = bytearray(self._regs.get((start + offset) & 0xFF, 0x00) for offset in range(length))
        return TransactionResult(True, length, 0, bytes(out), None)

    def write_i2c(self, config: I2cConfig, register: int | None, data: bytes) -> TransactionResult:
        if not config.is_valid_address():
            return TransactionResult(False, 0, 0, b"", "invalid address")
        if register is None:
            return TransactionResult(False, 0, 0, b"", "register required for write")
        start = register & 0xFF
        for index, value in enumerate(data):
            self._regs[(start + index) & 0xFF] = value & 0xFF
        return TransactionResult(True, 0, len(data), b"", None)

    def dispatch_frame(self, frame: bytes) -> bytes:
        op, payload = self._codec.decode_frame(frame)
        if op == OP_SPI_XFER:
            return self._handle_spi(payload)
        if op == OP_I2C_READ:
            return self._handle_i2c_read(payload)
        if op == OP_I2C_WRITE:
            return self._handle_i2c_write(payload)
        return self._codec.build_response(op, STATUS_ERR_ADDRESS)

    def _handle_spi(self, payload: bytes) -> bytes:
        config = SpiConfig(mode=payload[0], cs_active_low=bool(payload[1] & 0x01), word_size=payload[2], max_speed_hz=struct.unpack("<I", payload[3:7])[0])
        miso = self.xfer_spi(config, bytes(payload[7:]))
        return self._codec.build_response(OP_SPI_XFER, STATUS_OK, miso)

    def _handle_i2c_read(self, payload: bytes) -> bytes:
        config = I2cConfig(address=struct.unpack("<H", payload[0:2])[0], is_ten_bit=bool(payload[2] & 0x01), speed_khz=struct.unpack("<H", payload[3:5])[0])
        reg_byte = payload[5]
        length = payload[6]
        register = None if reg_byte == 0xFF else reg_byte
        result = self.read_i2c(config, register, length)
        status = STATUS_OK if result.success else STATUS_ERR_LENGTH
        return self._codec.build_response(OP_I2C_READ, status, result.data)

    def _handle_i2c_write(self, payload: bytes) -> bytes:
        config = I2cConfig(address=struct.unpack("<H", payload[0:2])[0], is_ten_bit=bool(payload[2] & 0x01), speed_khz=struct.unpack("<H", payload[3:5])[0])
        reg_byte = payload[5]
        data = bytes(payload[6:])
        register = None if reg_byte == 0xFF else reg_byte
        result = self.write_i2c(config, register, data)
        status = STATUS_OK if result.success else STATUS_ERR_ADDRESS
        return self._codec.build_response(OP_I2C_WRITE, status, bytes([result.bytes_written & 0xFF]))

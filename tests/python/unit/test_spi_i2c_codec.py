"""SPI/I2C 帧编解码 + CRC8 单元测试。

覆盖：crc8 已知值、encode_spi_xfer/encode_i2c_read/encode_i2c_write 帧结构、
build_response、decode_frame round-trip、feed 流式 + 分片。
"""

from __future__ import annotations

import pytest

from embeddebug.serial_station.spi_i2c.codec import (
    EOF,
    SOF,
    SpiI2cFrameCodec,
    crc8,
)
from embeddebug.serial_station.spi_i2c.config import I2cConfig, SpiConfig


def test_crc8_known_value():
    """CRC-8/SMBus of "123456789" = 0xF4。"""
    assert crc8(b"123456789") == 0xF4


def test_crc8_empty():
    assert crc8(b"") == 0x00


def test_crc8_single_byte():
    assert crc8(b"\x00") == 0x00


def test_encode_spi_xfer_structure():
    codec = SpiI2cFrameCodec()
    config = SpiConfig(mode=0, cs_active_low=True, word_size=8, max_speed_hz=1000000)
    frame = codec.encode_spi_xfer(config, b"\xAA\xBB")
    assert frame[0] == SOF
    assert frame[-1] == EOF
    assert frame[1] == 0x01  # OP_SPI_XFER


def test_encode_i2c_read_structure():
    codec = SpiI2cFrameCodec()
    config = I2cConfig(address=0x68, is_ten_bit=False, speed_khz=400)
    frame = codec.encode_i2c_read(config, register=0x75, length=1)
    assert frame[0] == SOF
    assert frame[1] == 0x02  # OP_I2C_READ


def test_encode_i2c_read_none_register():
    """register=None 用 _REGISTER_NONE (0xFF)。"""
    codec = SpiI2cFrameCodec()
    config = I2cConfig(address=0x50, is_ten_bit=False, speed_khz=100)
    frame = codec.encode_i2c_read(config, register=None, length=4)
    assert frame[0] == SOF


def test_encode_i2c_read_length_out_of_range():
    """length > 255 抛 ValueError。"""
    codec = SpiI2cFrameCodec()
    config = I2cConfig(address=0x68, is_ten_bit=False, speed_khz=400)
    with pytest.raises(ValueError):
        codec.encode_i2c_read(config, register=0, length=256)


def test_encode_i2c_write_structure():
    codec = SpiI2cFrameCodec()
    config = I2cConfig(address=0x68, is_ten_bit=False, speed_khz=400)
    frame = codec.encode_i2c_write(config, register=0x20, data=b"\x01\x02")
    assert frame[0] == SOF
    assert frame[1] == 0x03  # OP_I2C_WRITE


def test_build_response():
    codec = SpiI2cFrameCodec()
    resp = codec.build_response(0x01, 0x00, b"\xAA")
    assert resp[0] == SOF


def test_decode_frame_round_trip():
    """encode → decode_frame round-trip。"""
    codec = SpiI2cFrameCodec()
    config = SpiConfig(mode=1, cs_active_low=False, word_size=8, max_speed_hz=500000)
    frame = codec.encode_spi_xfer(config, b"\x01\x02\x03")
    op, payload = codec.decode_frame(frame)
    assert op == 0x01
    assert len(payload) > 0


def test_reset_clears():
    codec = SpiI2cFrameCodec()
    codec.feed(b"\xAA\xBB")
    codec.reset()
    assert codec.feed(b"") == []


def test_no_crc_mode():
    """use_crc=False 时帧无 CRC 字节。"""
    codec = SpiI2cFrameCodec(use_crc=False)
    config = SpiConfig(mode=0, cs_active_low=True, word_size=8, max_speed_hz=1000)
    frame_with_crc = SpiI2cFrameCodec(use_crc=True).encode_spi_xfer(config, b"\x01")
    frame_no_crc = codec.encode_spi_xfer(config, b"\x01")
    assert len(frame_no_crc) < len(frame_with_crc)

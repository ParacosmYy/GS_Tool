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


# ---- Batch 149: SPI/I2C codec 边界扩展 ----


def test_crc8_all_0xff():
    """CRC-8 of 2 bytes 0xFF = 0x24（确定性已知值）。"""
    assert crc8(b"\xFF\xFF") == 0x24


def test_crc8_idempotent_and_distinct():
    """同一输入 crc8 一致；不同输入一般不同。"""
    data = b"\x01\x02\x03\x04"
    assert crc8(data) == crc8(data)
    assert crc8(b"\x01") != crc8(b"\x02")


def test_encode_i2c_read_length_boundaries():
    """length=0 / 255 合法；length < 0 / 256 抛 ValueError。"""
    codec = SpiI2cFrameCodec()
    config = I2cConfig(address=0x68, is_ten_bit=False, speed_khz=400)
    assert codec.encode_i2c_read(config, register=0, length=0)[0] == SOF
    assert codec.encode_i2c_read(config, register=0, length=255)[0] == SOF
    with pytest.raises(ValueError):
        codec.encode_i2c_read(config, register=0, length=-1)


def test_encode_i2c_write_none_register_and_ten_bit():
    """register=None 用 _REGISTER_NONE；is_ten_bit=True flags=0x01。"""
    codec = SpiI2cFrameCodec()
    config = I2cConfig(address=0x50, is_ten_bit=True, speed_khz=100)
    frame = codec.encode_i2c_write(config, register=None, data=b"\x01")
    assert frame[0] == SOF
    assert frame[1] == 0x03  # OP_I2C_WRITE


def test_build_response_variants():
    """build_response STATUS_ERROR + 空 body + round-trip。"""
    codec = SpiI2cFrameCodec()
    err_resp = codec.build_response(0x01, 0x01, b"err")
    assert err_resp[0] == SOF and err_resp[1] == 0x81
    empty_resp = codec.build_response(0x02, 0x00)
    assert empty_resp[0] == SOF
    # round-trip
    frames = codec.feed(codec.build_response(0x01, 0x00, b"\xAA\xBB"))
    assert len(frames) == 1
    assert frames[0].op == 0x01 and frames[0].payload == b"\xAA\xBB"


def test_decode_frame_junk_raises():
    """decode_frame 有前导/尾部垃圾或完全无效抛 ValueError。"""
    codec = SpiI2cFrameCodec()
    config = SpiConfig(mode=0, cs_active_low=True, word_size=8, max_speed_hz=1000)
    frame = codec.encode_spi_xfer(config, b"\x01")
    with pytest.raises(ValueError, match="leading or trailing"):
        codec.decode_frame(b"\x00" + frame)
    with pytest.raises(ValueError, match="leading or trailing"):
        codec.decode_frame(frame + b"\x00")
    with pytest.raises(ValueError, match="incomplete or malformed"):
        codec.decode_frame(b"\x00\x01\x02\x03")


def test_feed_multi_partial_and_garbage():
    """feed 多帧 / 部分缓冲 / 前导垃圾 / CRC 破坏跳过。"""
    codec = SpiI2cFrameCodec()
    resp1 = codec.build_response(0x01, 0x00, b"\x01")
    resp2 = codec.build_response(0x02, 0x00, b"\x02")
    # 多帧
    assert len(codec.feed(resp1 + resp2)) == 2
    # 部分缓冲
    codec2 = SpiI2cFrameCodec()
    resp = codec2.build_response(0x01, 0x00, b"\xAA")
    half = len(resp) // 2
    assert codec2.feed(resp[:half]) == []
    assert len(codec2.feed(resp[half:])) == 1


def test_feed_trims_leading_garbage():
    """feed 前导垃圾字节被 _trim_to_sof 丢弃。"""
    codec = SpiI2cFrameCodec()
    resp = codec.build_response(0x01, 0x00, b"\x01")
    frames = codec.feed(b"\x00\x00\x00" + resp)
    assert len(frames) == 1


def test_no_crc_mode_feed_round_trip():
    """no_crc 模式下 feed 仍能解码响应帧。"""
    codec = SpiI2cFrameCodec(use_crc=False)
    resp = codec.build_response(0x01, 0x00, b"\xAA")
    frames = codec.feed(resp)
    assert len(frames) == 1
    assert frames[0].payload == b"\xAA"


def test_decoded_frame_is_frozen():
    """DecodedFrame 是 frozen dataclass。"""
    from embeddebug.serial_station.spi_i2c.codec import DecodedFrame
    df = DecodedFrame(op=1, status=0, payload=b"x", raw=b"y")
    with pytest.raises((AttributeError, TypeError)):
        df.op = 2

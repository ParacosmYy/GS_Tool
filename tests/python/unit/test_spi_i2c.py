"""SPI/I2C 桥接模块单测。"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import pytest

from embeddebug.serial_station.spi_i2c import (
    I2cConfig,
    I2cTransaction,
    SpiConfig,
    SpiI2cBridgeStub,
    SpiI2cFrameCodec,
    SpiTransaction,
)
from embeddebug.serial_station.spi_i2c.codec import OP_I2C_READ, OP_SPI_XFER, STATUS_OK, crc8


def test_spi_config_accepts_all_four_modes():
    for mode in range(4):
        assert SpiConfig(mode=mode).mode == mode


def test_spi_config_rejects_bad_mode():
    with pytest.raises(ValueError):
        SpiConfig(mode=4)


def test_i2c_config_validates_address_range():
    assert I2cConfig(address=0x68).address == 0x68
    with pytest.raises(ValueError):
        I2cConfig(address=128)


def test_spi_loopback_round_trip():
    bridge = SpiI2cBridgeStub()
    assert bridge.xfer_spi(SpiConfig(mode=0), b"\x01\x02\x03") == b"\x01\x02\x03"


def test_i2c_read_who_am_i():
    bridge = SpiI2cBridgeStub()
    result = bridge.read_i2c(I2cConfig(address=0x68), register=0x75, length=1)
    assert result.success and result.data == bytes([0x68])


def test_i2c_write_then_read():
    bridge = SpiI2cBridgeStub()
    bridge.write_i2c(I2cConfig(address=0x68), register=0x1A, data=b"\xAB\xCD")
    assert bridge.read_i2c(I2cConfig(address=0x68), register=0x1A, length=2).data == b"\xAB\xCD"


def test_transaction_guard_clauses():
    with pytest.raises(ValueError):
        SpiTransaction(mosi=b"\x01", miso_expected=b"\x01\x02")
    with pytest.raises(ValueError):
        I2cTransaction(is_read=True, data=b"\x01")


def test_encode_decode_spi_frame():
    codec = SpiI2cFrameCodec()
    frame = codec.encode_spi_xfer(SpiConfig(mode=2, cs_active_low=False, word_size=16), b"\xAA\xBB")
    op, payload = codec.decode_frame(frame)
    assert op == OP_SPI_XFER and payload[0] == 2 and payload[7:] == b"\xAA\xBB"


def test_bridge_dispatch_i2c_read():
    codec = SpiI2cFrameCodec()
    bridge = SpiI2cBridgeStub(codec=codec)
    cmd = codec.encode_i2c_read(I2cConfig(address=0x68), register=0x75, length=1)
    frame = codec.feed(bridge.dispatch_frame(cmd))[0]
    assert frame.op == OP_I2C_READ and frame.payload == bytes([0x68])


def test_crc8_known_vector():
    assert crc8(b"123456789") == 0xF4


def test_feed_rejects_corrupted_crc():
    codec = SpiI2cFrameCodec()
    resp = codec.build_response(OP_SPI_XFER, STATUS_OK, b"\x01")
    bad = bytearray(resp)
    bad[4] ^= 0xFF
    assert codec.feed(bytes(bad)) == []


def test_codec_without_crc():
    codec = SpiI2cFrameCodec(use_crc=False)
    frame = codec.encode_spi_xfer(SpiConfig(), b"\x09")
    op, payload = codec.decode_frame(frame)
    assert op == OP_SPI_XFER and payload[7:] == b"\x09"

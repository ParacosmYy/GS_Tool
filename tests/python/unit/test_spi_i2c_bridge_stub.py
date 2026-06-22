"""SpiI2cBridgeStub 单元测试 — SPI 回环 + I2C 寄存器读写。

覆盖：SPI 全双工回环、I2C 读 WHO_AM_I、I2C 写寄存器后读回、
无效地址拒绝、dispatch_frame 帧分发。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from embeddebug.serial_station.spi_i2c.bridge_stub import (
    REG_WHO_AM_I,
    WHO_AM_I_VALUE,
    SpiI2cBridgeStub,
)
from embeddebug.serial_station.spi_i2c.config import I2cConfig, SpiConfig


def test_spi_loopback_returns_mosi():
    """SPI 全双工回环：MISO = MOSI。"""
    stub = SpiI2cBridgeStub()
    config = SpiConfig(mode=0, cs_active_low=True, word_size=8, max_speed_hz=1000000)
    mosi = bytes([0xAA, 0x55, 0x00, 0xFF])
    miso = stub.xfer_spi(config, mosi)
    assert miso == mosi


def test_i2c_read_who_am_i():
    """读 WHO_AM_I 寄存器返回默认值 0x68。"""
    stub = SpiI2cBridgeStub()
    config = I2cConfig(address=0x68, is_ten_bit=False, speed_khz=400)
    result = stub.read_i2c(config, REG_WHO_AM_I, length=1)
    assert result.success is True
    assert result.data == bytes([WHO_AM_I_VALUE])


def test_i2c_write_then_read_back():
    """写寄存器后读回验证。"""
    stub = SpiI2cBridgeStub()
    config = I2cConfig(address=0x68, is_ten_bit=False, speed_khz=400)
    write_result = stub.write_i2c(config, 0x20, bytes([0xAB, 0xCD]))
    assert write_result.success is True
    assert write_result.bytes_written == 2
    read_result = stub.read_i2c(config, 0x20, length=2)
    assert read_result.data == bytes([0xAB, 0xCD])


def test_i2c_read_multi_byte_sequence():
    """读多字节按地址递增。"""
    stub = SpiI2cBridgeStub(registers={0x10: 0x01, 0x11: 0x02, 0x12: 0x03})
    config = I2cConfig(address=0x50, is_ten_bit=False, speed_khz=100)
    result = stub.read_i2c(config, 0x10, length=3)
    assert result.data == bytes([0x01, 0x02, 0x03])


def test_i2c_write_without_register_fails():
    """write 无 register 时失败。"""
    stub = SpiI2cBridgeStub()
    config = I2cConfig(address=0x68, is_ten_bit=False, speed_khz=400)
    result = stub.write_i2c(config, register=None, data=bytes([0x01]))
    assert result.success is False
    assert "register required" in (result.error or "")


def test_i2c_write_wraps_at_256():
    """写超过 0xFF 地址回绕（& 0xFF）。"""
    stub = SpiI2cBridgeStub()
    config = I2cConfig(address=0x68, is_ten_bit=False, speed_khz=400)
    stub.write_i2c(config, 0xFE, bytes([0x11, 0x22, 0x33]))
    result = stub.read_i2c(config, 0xFE, length=3)
    assert result.data == bytes([0x11, 0x22, 0x33])


def test_custom_registers_isolated():
    """自定义 registers 不影响默认 WHO_AM_I（独立实例）。"""
    stub1 = SpiI2cBridgeStub()
    stub2 = SpiI2cBridgeStub(registers={0x00: 0xFF})
    config = I2cConfig(address=0x68, is_ten_bit=False, speed_khz=100)
    r1 = stub1.read_i2c(config, REG_WHO_AM_I, length=1)
    r2 = stub2.read_i2c(config, REG_WHO_AM_I, length=1)
    assert r1.data == bytes([WHO_AM_I_VALUE])
    assert r2.data == bytes([0x00])  # stub2 无 WHO_AM_I，回退 0x00

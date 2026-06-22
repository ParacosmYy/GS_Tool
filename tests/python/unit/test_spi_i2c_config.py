"""SPI/I2C 配置 + 事务模型单元测试。

覆盖：SpiConfig mode/speed/word_size 验证、I2cConfig 地址/speed 验证、
SpiTransaction miso 长度匹配、I2cTransaction read 不带 data 约束、
TransactionResult 结构。
"""

from __future__ import annotations

import pytest

from embeddebug.serial_station.spi_i2c.config import I2cConfig, SpiConfig
from embeddebug.serial_station.spi_i2c.transaction import (
    I2cTransaction,
    SpiTransaction,
    TransactionResult,
)


# ── SpiConfig ──────────────────────────────────────────────────────

def test_spi_config_defaults():
    c = SpiConfig()
    assert c.mode == 0
    assert c.max_speed_hz == 1_000_000
    assert c.cs_active_low is True


def test_spi_config_mode_out_of_range():
    with pytest.raises(ValueError):
        SpiConfig(mode=4)


def test_spi_config_mode_negative():
    with pytest.raises(ValueError):
        SpiConfig(mode=-1)


def test_spi_config_speed_zero():
    with pytest.raises(ValueError):
        SpiConfig(max_speed_hz=0)


def test_spi_config_word_size_out_of_range():
    with pytest.raises(ValueError):
        SpiConfig(word_size=33)


def test_spi_config_valid_modes():
    for mode in range(4):
        SpiConfig(mode=mode)  # 不抛异常


# ── I2cConfig ──────────────────────────────────────────────────────

def test_i2c_config_defaults():
    c = I2cConfig()
    assert c.address == 0x00
    assert c.is_ten_bit is False
    assert c.speed_khz == 100


def test_i2c_config_address_7bit_max():
    I2cConfig(address=0x7F)  # 不抛异常


def test_i2c_config_address_7bit_overflow():
    with pytest.raises(ValueError):
        I2cConfig(address=0x80)


def test_i2c_config_ten_bit_address():
    I2cConfig(address=0x3FF, is_ten_bit=True)  # 不抛异常


def test_i2c_config_ten_bit_overflow():
    with pytest.raises(ValueError):
        I2cConfig(address=0x400, is_ten_bit=True)


def test_i2c_config_invalid_speed():
    with pytest.raises(ValueError):
        I2cConfig(speed_khz=50)


def test_i2c_config_valid_speeds():
    for speed in (10, 100, 400, 1000, 3400):
        I2cConfig(speed_khz=speed)


def test_i2c_config_is_valid_address():
    """is_valid_address 对合法地址返回 True（构造时已验证）。"""
    assert I2cConfig(address=0x50).is_valid_address() is True
    assert I2cConfig(address=0x00).is_valid_address() is True


# ── SpiTransaction ─────────────────────────────────────────────────

def test_spi_transaction_defaults():
    t = SpiTransaction()
    assert t.mosi == b""
    assert t.assert_cs is True


def test_spi_transaction_miso_length_mismatch():
    with pytest.raises(ValueError):
        SpiTransaction(mosi=b"\x01\x02", miso_expected=b"\x01")


def test_spi_transaction_matching_lengths():
    t = SpiTransaction(mosi=b"\x01\x02", miso_expected=b"\x03\x04")
    assert len(t.miso_expected) == 2


# ── I2cTransaction ─────────────────────────────────────────────────

def test_i2c_transaction_read_with_data_raises():
    with pytest.raises(ValueError):
        I2cTransaction(is_read=True, data=b"\x01")


def test_i2c_transaction_register_out_of_range():
    with pytest.raises(ValueError):
        I2cTransaction(register=256)


def test_i2c_transaction_valid_write():
    t = I2cTransaction(address=0x50, is_read=False, register=0x10, data=b"\xAA")
    assert t.data == b"\xAA"


def test_i2c_transaction_valid_read():
    t = I2cTransaction(address=0x50, is_read=True, register=0x10)
    assert t.is_read is True
    assert t.data == b""


# ── TransactionResult ──────────────────────────────────────────────

def test_transaction_result_success():
    r = TransactionResult(success=True, bytes_read=4, data=b"\x01\x02\x03\x04")
    assert r.success is True
    assert r.bytes_read == 4
    assert r.error is None


def test_transaction_result_failure():
    r = TransactionResult(success=False, error="timeout")
    assert r.success is False
    assert r.error == "timeout"

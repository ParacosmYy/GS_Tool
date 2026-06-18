"""SPI/I2C 桥接调试模块。"""

from __future__ import annotations

from embeddebug.serial_station.spi_i2c.bridge_stub import SpiI2cBridgeStub
from embeddebug.serial_station.spi_i2c.codec import SpiI2cFrameCodec
from embeddebug.serial_station.spi_i2c.config import I2cConfig, SpiConfig
from embeddebug.serial_station.spi_i2c.transaction import (
    I2cTransaction,
    SpiTransaction,
    TransactionResult,
)

__all__ = [
    "I2cConfig",
    "I2cTransaction",
    "SpiConfig",
    "SpiI2cBridgeStub",
    "SpiI2cFrameCodec",
    "SpiTransaction",
    "TransactionResult",
]

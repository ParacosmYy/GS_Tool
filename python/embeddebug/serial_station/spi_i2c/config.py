"""SPI/I2C 总线配置。"""

from __future__ import annotations

from dataclasses import dataclass


@dataclass(frozen=True)
class SpiConfig:
    """SPI 总线配置。"""

    mode: int = 0
    max_speed_hz: int = 1_000_000
    cs_active_low: bool = True
    word_size: int = 8

    def __post_init__(self) -> None:
        if not 0 <= self.mode <= 3:
            raise ValueError(f"spi mode must be 0-3, got {self.mode}")
        if self.max_speed_hz <= 0:
            raise ValueError("max_speed_hz must be positive")
        if not 1 <= self.word_size <= 32:
            raise ValueError(f"word_size must be 1-32 bits, got {self.word_size}")


@dataclass(frozen=True)
class I2cConfig:
    """I2C 总线配置。"""

    address: int = 0x00
    is_ten_bit: bool = False
    speed_khz: int = 100

    def __post_init__(self) -> None:
        limit = 1023 if self.is_ten_bit else 127
        if not 0 <= self.address <= limit:
            raise ValueError(f"address {self.address:#x} out of range")
        if self.speed_khz not in (10, 100, 400, 1000, 3400):
            raise ValueError(f"unsupported i2c speed {self.speed_khz} kHz")

    def is_valid_address(self) -> bool:
        return 0 <= self.address <= (1023 if self.is_ten_bit else 127)

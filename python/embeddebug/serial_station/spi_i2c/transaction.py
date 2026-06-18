"""SPI/I2C 事务与结果。"""

from __future__ import annotations

from dataclasses import dataclass


@dataclass(frozen=True)
class SpiTransaction:
    """一次 SPI 事务。"""

    mosi: bytes = b""
    miso_expected: bytes = b""
    assert_cs: bool = True
    deassert_cs: bool = True

    def __post_init__(self) -> None:
        if self.miso_expected and len(self.miso_expected) != len(self.mosi):
            raise ValueError("miso_expected length must equal mosi length")


@dataclass(frozen=True)
class I2cTransaction:
    """一次 I2C 事务。"""

    address: int = 0x00
    is_read: bool = False
    register: int | None = None
    data: bytes = b""
    repeated_start: bool = False

    def __post_init__(self) -> None:
        if self.register is not None and not 0 <= self.register <= 0xFF:
            raise ValueError("register must be 0-255 or None")
        if self.is_read and self.data:
            raise ValueError("read transaction should not carry write data")


@dataclass(frozen=True)
class TransactionResult:
    """事务执行结果。"""

    success: bool
    bytes_read: int = 0
    bytes_written: int = 0
    data: bytes = b""
    error: str | None = None

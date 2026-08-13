"""Bounded Modbus RTU ADU validation.

This module deliberately validates one already-framed RTU application data
unit.  It does not infer frame boundaries from ordinary serial read chunks:
RTU framing requires the transport layer to observe the t1.5/t3.5 timing
rules.  Keeping that distinction here prevents a component codec from
silently claiming to be a complete RTU stream framer.
"""

from __future__ import annotations

from dataclasses import dataclass
from typing import Final

from .errors import ConfigurationError
from .protocols import FrameStatus

MAX_MODBUS_RTU_FRAME_BYTES: Final = 256
MAX_MODBUS_RTU_DATA_BYTES: Final = 252
MIN_MODBUS_RTU_FRAME_BYTES: Final = 4
MAX_MODBUS_RTU_ADDRESS: Final = 247
MODBUS_RTU_BROADCAST_ADDRESS: Final = 0


@dataclass(frozen=True, slots=True)
class ModbusRtuFrame:
    """One bounded RTU ADU and its integrity/format result."""

    payload: bytes
    status: FrameStatus
    error: str | None = None
    address: int | None = None
    function: int | None = None
    data: bytes = b""
    received_crc: int | None = None
    calculated_crc: int | None = None

    def __post_init__(self) -> None:
        if not isinstance(self.payload, (bytes, bytearray, memoryview)):
            raise ConfigurationError("Modbus RTU payload 必须是 bytes-like。")
        payload = bytes(self.payload)
        if len(payload) > 65_536:
            raise ConfigurationError("Modbus RTU payload 超过通用 frame 上限。")
        if not isinstance(self.status, FrameStatus):
            raise ConfigurationError("Modbus RTU status 必须使用 FrameStatus。")
        if self.error is not None and not self.error.strip():
            raise ConfigurationError("Modbus RTU error 不能为空字符串。")
        for name, value, upper in (
            ("address", self.address, 0xFF),
            ("function", self.function, 0xFF),
            ("received_crc", self.received_crc, 0xFFFF),
            ("calculated_crc", self.calculated_crc, 0xFFFF),
        ):
            if value is not None and (
                isinstance(value, bool) or not isinstance(value, int) or not 0 <= value <= upper
            ):
                raise ConfigurationError(f"Modbus RTU {name} 超出范围。")
        if not isinstance(self.data, (bytes, bytearray, memoryview)):
            raise ConfigurationError("Modbus RTU data 必须是 bytes-like。")
        data = bytes(self.data)
        if len(data) > MAX_MODBUS_RTU_DATA_BYTES:
            raise ConfigurationError("Modbus RTU data 超过标准上限。")
        object.__setattr__(self, "payload", payload)
        object.__setattr__(self, "data", data)

    @property
    def is_exception(self) -> bool | None:
        """Return exception-response state when a valid function byte exists."""

        if self.function is None or self.function in {0x00, 0x80}:
            return None
        return bool(self.function & 0x80)

    @property
    def base_function(self) -> int | None:
        """Return the application function code without an exception bit."""

        if self.function is None or self.function in {0x00, 0x80}:
            return None
        return self.function & 0x7F


def crc16_modbus(payload: bytes | bytearray | memoryview) -> int:
    """Calculate the Modbus CRC16 register value.

    The returned integer is the conventional register value.  On the wire,
    callers append its low byte first and its high byte second.
    """

    if not isinstance(payload, (bytes, bytearray, memoryview)):
        raise ConfigurationError("Modbus CRC payload 必须是 bytes-like。")
    crc = 0xFFFF
    for byte in bytes(payload):
        crc ^= byte
        for _ in range(8):
            if crc & 0x0001:
                crc = (crc >> 1) ^ 0xA001
            else:
                crc >>= 1
    return crc & 0xFFFF


def decode_modbus_rtu(payload: bytes | bytearray | memoryview) -> ModbusRtuFrame:
    """Validate one complete, already-framed Modbus RTU ADU.

    Address 0 is retained as a broadcast address.  Reserved addresses 248–255
    and function bytes 0/0x80 are format errors.  Function-specific PDU
    lengths and application semantics are intentionally left to a future
    profile layer.
    """

    if not isinstance(payload, (bytes, bytearray, memoryview)):
        raise ConfigurationError("Modbus RTU payload 必须是 bytes-like。")
    raw = bytes(payload)
    if len(raw) > MAX_MODBUS_RTU_FRAME_BYTES:
        return ModbusRtuFrame(
            payload=raw,
            status=FrameStatus.OVERSIZE,
            error=(
                f"Modbus RTU ADU 超过 {MAX_MODBUS_RTU_FRAME_BYTES} B 标准上限 "
                f"（收到 {len(raw)} B）。"
            ),
        )
    if len(raw) < MIN_MODBUS_RTU_FRAME_BYTES:
        return ModbusRtuFrame(
            payload=raw,
            status=FrameStatus.INCOMPLETE,
            error=(
                f"Modbus RTU ADU 至少需要 {MIN_MODBUS_RTU_FRAME_BYTES} B （收到 {len(raw)} B）。"
            ),
            address=raw[0] if raw else None,
            function=raw[1] if len(raw) > 1 else None,
        )

    address = raw[0]
    function = raw[1]
    data = raw[2:-2]
    received_crc = int.from_bytes(raw[-2:], "little")
    calculated_crc = crc16_modbus(raw[:-2])
    if address > MAX_MODBUS_RTU_ADDRESS:
        status = FrameStatus.INVALID_FORMAT
        error = f"Modbus RTU 地址 {address} 保留；允许 0 到 {MAX_MODBUS_RTU_ADDRESS}。"
    elif function in {0x00, 0x80}:
        status = FrameStatus.INVALID_FORMAT
        error = f"Modbus RTU function 0x{function:02X} 无效。"
    elif received_crc != calculated_crc:
        status = FrameStatus.INVALID_CHECKSUM
        error = (
            "Modbus RTU CRC16 不匹配："
            f"expected={calculated_crc:04X} actual={received_crc:04X} "
            "（wire=低字节在前）。"
        )
    else:
        status = FrameStatus.VALID
        error = None
    return ModbusRtuFrame(
        payload=raw,
        status=status,
        error=error,
        address=address,
        function=function,
        data=data,
        received_crc=received_crc,
        calculated_crc=calculated_crc,
    )


__all__ = [
    "MAX_MODBUS_RTU_ADDRESS",
    "MAX_MODBUS_RTU_DATA_BYTES",
    "MAX_MODBUS_RTU_FRAME_BYTES",
    "MIN_MODBUS_RTU_FRAME_BYTES",
    "MODBUS_RTU_BROADCAST_ADDRESS",
    "ModbusRtuFrame",
    "crc16_modbus",
    "decode_modbus_rtu",
]

"""Bounded MAVLink v1/v2 frame validation.

This module consumes one already-framed packet.  MAVLink packet length is
header-dependent and its stream resynchronisation belongs to a future timing/
byte-stream boundary adapter, not to the component codec.
"""

from __future__ import annotations

from dataclasses import dataclass
from enum import StrEnum
from typing import Final

from .errors import ConfigurationError
from .protocols import FrameStatus

MAVLINK_V1_MAGIC: Final = 0xFE
MAVLINK_V2_MAGIC: Final = 0xFD
MAVLINK_V1_HEADER_BYTES: Final = 6
MAVLINK_V2_HEADER_BYTES: Final = 10
MAVLINK_CRC_BYTES: Final = 2
MAVLINK_V2_SIGNATURE_BYTES: Final = 13
MAVLINK_MAX_PAYLOAD_BYTES: Final = 255
MAVLINK_MAX_V1_PACKET_BYTES: Final = 263
MAVLINK_MAX_V2_PACKET_BYTES: Final = 280
MAVLINK_MAX_PACKET_BYTES: Final = MAVLINK_MAX_V2_PACKET_BYTES
MAVLINK_V2_SIGNED_FLAG: Final = 0x01
MAVLINK_V2_KNOWN_INCOMPAT_FLAGS: Final = MAVLINK_V2_SIGNED_FLAG


class MavlinkVersion(StrEnum):
    """Wire versions identified by their start marker."""

    V1 = "v1"
    V2 = "v2"


@dataclass(frozen=True, slots=True)
class MavlinkFrame:
    """One bounded MAVLink packet and its validation result."""

    payload: bytes
    status: FrameStatus
    error: str | None = None
    version: MavlinkVersion | None = None
    payload_length: int | None = None
    incompat_flags: int | None = None
    compat_flags: int | None = None
    sequence: int | None = None
    system_id: int | None = None
    component_id: int | None = None
    message_id: int | None = None
    message_payload: bytes = b""
    signed: bool | None = None
    signature: bytes = b""
    crc_extra: int | None = None
    received_crc: int | None = None
    calculated_crc: int | None = None

    def __post_init__(self) -> None:
        if not isinstance(self.payload, (bytes, bytearray, memoryview)):
            raise ConfigurationError("MAVLink payload 必须是 bytes-like。")
        payload = bytes(self.payload)
        if len(payload) > 65_536:
            raise ConfigurationError("MAVLink payload 超过通用 frame 上限。")
        if not isinstance(self.status, FrameStatus):
            raise ConfigurationError("MAVLink status 必须使用 FrameStatus。")
        if self.version is not None and not isinstance(self.version, MavlinkVersion):
            raise ConfigurationError("MAVLink version 类型无效。")
        if self.error is not None and not self.error.strip():
            raise ConfigurationError("MAVLink error 不能为空字符串。")
        ranges = (
            ("payload_length", self.payload_length, 0xFF),
            ("incompat_flags", self.incompat_flags, 0xFF),
            ("compat_flags", self.compat_flags, 0xFF),
            ("sequence", self.sequence, 0xFF),
            ("system_id", self.system_id, 0xFF),
            ("component_id", self.component_id, 0xFF),
            ("message_id", self.message_id, 0xFF_FFFF),
            ("crc_extra", self.crc_extra, 0xFF),
            ("received_crc", self.received_crc, 0xFFFF),
            ("calculated_crc", self.calculated_crc, 0xFFFF),
        )
        for name, value, upper in ranges:
            if value is not None and (
                isinstance(value, bool) or not isinstance(value, int) or not 0 <= value <= upper
            ):
                raise ConfigurationError(f"MAVLink {name} 超出范围。")
        if self.signed is not None and not isinstance(self.signed, bool):
            raise ConfigurationError("MAVLink signed 必须是 bool 或 None。")
        if not isinstance(self.message_payload, (bytes, bytearray, memoryview)):
            raise ConfigurationError("MAVLink message_payload 必须是 bytes-like。")
        message_payload = bytes(self.message_payload)
        if len(message_payload) > MAVLINK_MAX_PAYLOAD_BYTES:
            raise ConfigurationError("MAVLink message_payload 超过 255 B。")
        if not isinstance(self.signature, (bytes, bytearray, memoryview)):
            raise ConfigurationError("MAVLink signature 必须是 bytes-like。")
        signature = bytes(self.signature)
        if len(signature) > MAVLINK_V2_SIGNATURE_BYTES:
            raise ConfigurationError("MAVLink signature 超过 13 B。")
        object.__setattr__(self, "payload", payload)
        object.__setattr__(self, "message_payload", message_payload)
        object.__setattr__(self, "signature", signature)


def crc16_mcrf4xx(payload: bytes | bytearray | memoryview) -> int:
    """Calculate the MAVLink CRC-16/MCRF4XX (X.25) register value."""

    if not isinstance(payload, (bytes, bytearray, memoryview)):
        raise ConfigurationError("MAVLink CRC payload 必须是 bytes-like。")
    crc = 0xFFFF
    for byte in bytes(payload):
        value = (byte ^ (crc & 0xFF)) & 0xFF
        value ^= (value << 4) & 0xFF
        crc = ((crc >> 8) ^ (value << 8) ^ (value << 3) ^ (value >> 4)) & 0xFFFF
    return crc


def decode_mavlink(
    payload: bytes | bytearray | memoryview,
    *,
    crc_extra: int | None = None,
) -> MavlinkFrame:
    """Validate one already-framed MAVLink v1/v2 packet.

    ``crc_extra`` must come from a pinned dialect/message-definition mapping.
    Without it, the packet structure is decoded but wire CRC validation is
    explicitly reported as :class:`FrameStatus.UNVERIFIED`.  v2 signatures
    are retained and identified but never cryptographically authenticated.
    """

    if not isinstance(payload, (bytes, bytearray, memoryview)):
        raise ConfigurationError("MAVLink payload 必须是 bytes-like。")
    if crc_extra is not None and (
        isinstance(crc_extra, bool) or not isinstance(crc_extra, int) or not 0 <= crc_extra <= 0xFF
    ):
        raise ConfigurationError("MAVLink crc_extra 必须在 0 到 255 之间。")
    raw = bytes(payload)
    if len(raw) > MAVLINK_MAX_PACKET_BYTES:
        return MavlinkFrame(
            payload=raw,
            status=FrameStatus.OVERSIZE,
            error=(
                f"MAVLink packet 超过 {MAVLINK_MAX_PACKET_BYTES} B 上限 （收到 {len(raw)} B）。"
            ),
        )
    if not raw:
        return MavlinkFrame(
            payload=raw,
            status=FrameStatus.INCOMPLETE,
            error="MAVLink packet 缺少 magic。",
        )
    magic = raw[0]
    if magic not in {MAVLINK_V1_MAGIC, MAVLINK_V2_MAGIC}:
        return MavlinkFrame(
            payload=raw,
            status=FrameStatus.INVALID_FORMAT,
            error=f"MAVLink magic 0x{magic:02X} 不是 0xFE(v1) 或 0xFD(v2)。",
        )
    version = MavlinkVersion.V1 if magic == MAVLINK_V1_MAGIC else MavlinkVersion.V2
    header_bytes = (
        MAVLINK_V1_HEADER_BYTES if version is MavlinkVersion.V1 else MAVLINK_V2_HEADER_BYTES
    )
    if len(raw) < 2:
        return MavlinkFrame(
            payload=raw,
            status=FrameStatus.INCOMPLETE,
            error="MAVLink packet 缺少 payload length。",
            version=version,
        )
    payload_length = raw[1]
    incompat_flags: int | None = None
    compat_flags: int | None = None
    sequence: int | None = None
    system_id: int | None = None
    component_id: int | None = None
    message_id: int | None = None
    signed: bool | None = False if version is MavlinkVersion.V1 else None
    if version is MavlinkVersion.V1:
        if len(raw) >= 6:
            sequence, system_id, component_id, message_id = raw[2:6]
    elif len(raw) >= 10:
        incompat_flags = raw[2]
        compat_flags = raw[3]
        sequence = raw[4]
        system_id = raw[5]
        component_id = raw[6]
        message_id = int.from_bytes(raw[7:10], "little")
        signed = bool(incompat_flags & MAVLINK_V2_SIGNED_FLAG)
    if len(raw) < header_bytes:
        return MavlinkFrame(
            payload=raw,
            status=FrameStatus.INCOMPLETE,
            error=(
                f"MAVLink {version.value} header 不完整：需要 {header_bytes} B，收到 {len(raw)} B。"
            ),
            version=version,
            payload_length=payload_length,
            incompat_flags=incompat_flags,
            compat_flags=compat_flags,
            sequence=sequence,
            system_id=system_id,
            component_id=component_id,
            message_id=message_id,
            signed=signed,
        )
    signature_bytes = MAVLINK_V2_SIGNATURE_BYTES if signed else 0
    expected_length = header_bytes + payload_length + MAVLINK_CRC_BYTES + signature_bytes
    message_payload = raw[header_bytes : header_bytes + payload_length]
    base = dict(
        payload=raw,
        version=version,
        payload_length=payload_length,
        incompat_flags=incompat_flags,
        compat_flags=compat_flags,
        sequence=sequence,
        system_id=system_id,
        component_id=component_id,
        message_id=message_id,
        message_payload=message_payload,
        signed=signed,
        crc_extra=crc_extra,
    )
    if len(raw) < expected_length:
        return MavlinkFrame(
            status=FrameStatus.INCOMPLETE,
            error=(
                f"MAVLink {version.value} packet 不完整："
                f"需要 {expected_length} B，收到 {len(raw)} B。"
            ),
            **base,
        )
    if len(raw) > expected_length:
        return MavlinkFrame(
            status=FrameStatus.INVALID_LENGTH,
            error=(
                f"MAVLink {version.value} packet 有多余字节："
                f"期望 {expected_length} B，收到 {len(raw)} B。"
            ),
            **base,
        )
    if system_id == 0 or component_id == 0:
        return MavlinkFrame(
            status=FrameStatus.INVALID_FORMAT,
            error="MAVLink sysid/compid 不能为 0。",
            **base,
        )
    if version is MavlinkVersion.V2 and incompat_flags is not None:
        unknown_flags = incompat_flags & ~MAVLINK_V2_KNOWN_INCOMPAT_FLAGS
        if unknown_flags:
            return MavlinkFrame(
                status=FrameStatus.INVALID_FORMAT,
                error=f"MAVLink v2 incompat_flags 包含未支持位 0x{unknown_flags:02X}。",
                **base,
            )
    crc_start = header_bytes + payload_length
    received_crc = int.from_bytes(raw[crc_start : crc_start + MAVLINK_CRC_BYTES], "little")
    signature = raw[crc_start + MAVLINK_CRC_BYTES :] if signed else b""
    if crc_extra is None:
        error = "MAVLink CRC_EXTRA 缺失；wire CRC 未验证。"
        if signed:
            error += " v2 signature 存在但未认证。"
        return MavlinkFrame(
            status=FrameStatus.UNVERIFIED,
            error=error,
            received_crc=received_crc,
            signature=signature,
            **base,
        )
    calculated_crc = crc16_mcrf4xx(raw[1:crc_start] + bytes((crc_extra,)))
    if received_crc != calculated_crc:
        return MavlinkFrame(
            status=FrameStatus.INVALID_CHECKSUM,
            error=(
                "MAVLink CRC 不匹配："
                f"expected={calculated_crc:04X} actual={received_crc:04X} "
                "（wire=低字节在前）。"
            ),
            received_crc=received_crc,
            calculated_crc=calculated_crc,
            signature=signature,
            **base,
        )
    if signed:
        return MavlinkFrame(
            status=FrameStatus.UNVERIFIED,
            error="MAVLink v2 signature 存在但未认证；wire CRC 已验证。",
            received_crc=received_crc,
            calculated_crc=calculated_crc,
            signature=signature,
            **base,
        )
    return MavlinkFrame(
        status=FrameStatus.VALID,
        error=None,
        received_crc=received_crc,
        calculated_crc=calculated_crc,
        **base,
    )


__all__ = [
    "MAVLINK_CRC_BYTES",
    "MAVLINK_MAX_PACKET_BYTES",
    "MAVLINK_MAX_PAYLOAD_BYTES",
    "MAVLINK_MAX_V1_PACKET_BYTES",
    "MAVLINK_MAX_V2_PACKET_BYTES",
    "MAVLINK_V1_MAGIC",
    "MAVLINK_V2_MAGIC",
    "MAVLINK_V2_SIGNATURE_BYTES",
    "MAVLINK_V2_SIGNED_FLAG",
    "MavlinkFrame",
    "MavlinkVersion",
    "crc16_mcrf4xx",
    "decode_mavlink",
]

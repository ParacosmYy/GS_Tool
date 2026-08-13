"""Bounded, transport-neutral streaming framing and checksum contracts."""

from __future__ import annotations

import binascii
import math
from dataclasses import dataclass, field
from enum import StrEnum
from typing import Protocol
from uuid import UUID

from .errors import ConfigurationError, ProtocolInputError
from .models import PeerAddress, RecordDirection, TransportKind
from .timing import GapObservation, ModbusRtuTiming

MAX_PROTOCOL_DELIMITER_BYTES = 64
MAX_PROTOCOL_FRAME_BYTES = 65_536
MAX_PROTOCOL_BUFFER_BYTES = 262_144
MAX_PROTOCOL_BATCH_FRAMES = 512


class FramingKind(StrEnum):
    """Ways a byte stream becomes bounded protocol frames."""

    RAW = "raw"
    LINE = "line"
    DELIMITER = "delimiter"
    LENGTH_PREFIXED = "length_prefixed"
    MAVLINK_STREAM = "mavlink_stream"
    MODBUS_RTU_TIMED = "modbus_rtu_timed"


class ChecksumKind(StrEnum):
    """Checksums supported by the first protocol slice."""

    NONE = "none"
    XOR8 = "xor8"
    CRC16_MODBUS = "crc16_modbus"
    CRC32 = "crc32"
    NMEA0183 = "nmea0183"


class FrameStatus(StrEnum):
    """A decoded frame remains visible even when its integrity check fails."""

    VALID = "valid"
    UNVERIFIED = "unverified"
    INVALID_CHECKSUM = "invalid_checksum"
    INVALID_FORMAT = "invalid_format"
    INVALID_LENGTH = "invalid_length"
    OVERSIZE = "oversize"
    INCOMPLETE = "incomplete"


class ProtocolBoundary(StrEnum):
    """Whether an ingress unit may be joined with the next unit."""

    STREAM = "stream"
    DATAGRAM = "datagram"
    GATT_MESSAGE = "gatt_message"


class DataOrigin(StrEnum):
    """Identify whether bytes came from a live device or historical replay."""

    LIVE = "live"
    HISTORICAL = "historical"


@dataclass(frozen=True, slots=True)
class ProtocolSource:
    """Stable parser-state identity; address text is never used as peer identity."""

    session_id: UUID
    transport: TransportKind
    peer: PeerAddress | None = None
    peer_id: UUID | None = None
    channel: str | None = None
    direction: RecordDirection = RecordDirection.RECEIVE
    origin: DataOrigin = DataOrigin.LIVE

    def __post_init__(self) -> None:
        if not isinstance(self.session_id, UUID):
            raise ConfigurationError("协议 source session_id 必须使用 UUID。")
        if not isinstance(self.transport, TransportKind):
            raise ConfigurationError("协议 source transport 必须使用 TransportKind。")
        if self.peer is not None and not isinstance(self.peer, PeerAddress):
            raise ConfigurationError("协议 source peer 必须使用 PeerAddress。")
        if self.peer_id is not None and not isinstance(self.peer_id, UUID):
            raise ConfigurationError("协议 source peer_id 必须使用 UUID。")
        if self.channel is not None and not self.channel.strip():
            raise ConfigurationError("协议 source channel 不能为空字符串。")
        if not isinstance(self.direction, RecordDirection):
            raise ConfigurationError("协议 source direction 必须使用 RecordDirection。")
        if not isinstance(self.origin, DataOrigin):
            raise ConfigurationError("协议 source origin 必须使用 DataOrigin。")

    @property
    def display(self) -> str:
        """Return a bounded component-view label for this source."""

        if self.channel:
            label = self.channel[:128]
        elif self.peer_id:
            label = f"{self.transport.value} · peer {self.peer_id}"
        elif self.peer:
            label = f"{self.transport.value} · {self.peer.display}"
        else:
            label = self.transport.value
        return f"历史 · {label}" if self.origin is DataOrigin.HISTORICAL else label


@dataclass(frozen=True, slots=True)
class ProtocolIngressUnit:
    """Raw bytes plus enough context to keep parser state per peer/channel.

    ``segment_id`` is parser-lifecycle metadata, not a transport/source identity.
    It lets a replay source declare that a new contiguous recording session has
    started without leaking replay-specific state into downstream events.
    """

    source: ProtocolSource
    boundary: ProtocolBoundary
    payload: bytes
    direction: RecordDirection = RecordDirection.RECEIVE
    occurred_at: float | None = None
    segment_id: UUID | None = None
    timing: GapObservation = field(default_factory=GapObservation)

    def __post_init__(self) -> None:
        if not isinstance(self.source, ProtocolSource):
            raise ConfigurationError("协议 ingress source 类型无效。")
        if not isinstance(self.boundary, ProtocolBoundary):
            raise ConfigurationError("协议 ingress boundary 类型无效。")
        if not isinstance(self.direction, RecordDirection):
            raise ConfigurationError("协议 ingress direction 类型无效。")
        if self.direction is not self.source.direction:
            raise ConfigurationError("协议 ingress direction 必须与 source direction 一致。")
        if self.segment_id is not None and not isinstance(self.segment_id, UUID):
            raise ConfigurationError("协议 ingress segment_id 必须使用 UUID。")
        if not isinstance(self.timing, GapObservation):
            raise ConfigurationError("协议 ingress timing 必须使用 GapObservation。")
        if self.occurred_at is not None and (
            isinstance(self.occurred_at, bool)
            or not isinstance(self.occurred_at, (int, float))
            or not math.isfinite(self.occurred_at)
            or self.occurred_at < 0
        ):
            raise ConfigurationError("协议 ingress occurred_at 必须是有限非负数字。")
        if not isinstance(self.payload, (bytes, bytearray, memoryview)):
            raise ProtocolInputError("协议 ingress payload 必须是 bytes-like。")
        payload = bytes(self.payload)
        if not payload:
            raise ProtocolInputError("协议 ingress payload 不能为空。")
        object.__setattr__(self, "payload", payload)


@dataclass(frozen=True, slots=True)
class ProtocolConfig:
    """Versioned parser settings with explicit framing and checksum semantics."""

    framing: FramingKind = FramingKind.RAW
    checksum: ChecksumKind = ChecksumKind.NONE
    delimiter: bytes = b"\n"
    length_bytes: int = 2
    byteorder: str = "little"
    checksum_byteorder: str = "little"
    max_frame_bytes: int = 4_096
    schema_version: int = 1
    modbus_timing: ModbusRtuTiming | None = None

    def __post_init__(self) -> None:
        if not isinstance(self.framing, FramingKind):
            raise ConfigurationError("协议 framing 必须使用 FramingKind。")
        if not isinstance(self.checksum, ChecksumKind):
            raise ConfigurationError("协议 checksum 必须使用 ChecksumKind。")
        if self.modbus_timing is not None and not isinstance(self.modbus_timing, ModbusRtuTiming):
            raise ConfigurationError("协议 modbus_timing 必须使用 ModbusRtuTiming。")
        if not isinstance(self.delimiter, (bytes, bytearray, memoryview)):
            raise ConfigurationError("协议 delimiter 必须是 bytes-like。")
        delimiter = bytes(self.delimiter)
        if not delimiter or len(delimiter) > MAX_PROTOCOL_DELIMITER_BYTES:
            raise ConfigurationError(
                f"协议 delimiter 长度必须在 1 到 {MAX_PROTOCOL_DELIMITER_BYTES} 字节内。"
            )
        if self.framing is FramingKind.LINE:
            delimiter = b"\n"
        if self.framing is FramingKind.RAW and self.checksum is not ChecksumKind.NONE:
            raise ConfigurationError("Raw framing 不启用 checksum；请选择 Line/Delimiter/Length。")
        if self.checksum is ChecksumKind.NMEA0183 and self.framing is not FramingKind.LINE:
            raise ConfigurationError("NMEA 0183 checksum 只支持 Line (LF/CRLF) framing。")
        if self.framing in {FramingKind.MAVLINK_STREAM, FramingKind.MODBUS_RTU_TIMED}:
            if self.checksum is not ChecksumKind.NONE:
                raise ConfigurationError(
                    "MAVLink/Modbus stream framing 不叠加通用 checksum；由协议 codec 校验。"
                )
        if self.framing is FramingKind.MODBUS_RTU_TIMED and self.modbus_timing is None:
            raise ConfigurationError("Modbus RTU timed framing 必须提供 UART timing。")
        if self.framing is not FramingKind.MODBUS_RTU_TIMED and self.modbus_timing is not None:
            raise ConfigurationError("modbus_timing 只能用于 Modbus RTU timed framing。")
        if (
            isinstance(self.length_bytes, bool)
            or not isinstance(self.length_bytes, int)
            or self.length_bytes not in {1, 2, 4}
        ):
            raise ConfigurationError("协议 length_bytes 只能是 1、2 或 4。")
        if self.byteorder not in {"little", "big"}:
            raise ConfigurationError("协议 byteorder 必须是 little 或 big。")
        if self.checksum_byteorder not in {"little", "big"}:
            raise ConfigurationError("协议 checksum_byteorder 必须是 little 或 big。")
        if (
            isinstance(self.max_frame_bytes, bool)
            or not isinstance(self.max_frame_bytes, int)
            or not 1 <= self.max_frame_bytes <= MAX_PROTOCOL_FRAME_BYTES
        ):
            raise ConfigurationError(
                f"协议 max_frame_bytes 必须在 1 到 {MAX_PROTOCOL_FRAME_BYTES} 之间。"
            )
        if self.schema_version != 1:
            raise ConfigurationError("不支持的协议配置 schema 版本。")
        object.__setattr__(self, "delimiter", delimiter)

    @property
    def checksum_bytes(self) -> int:
        """Return the number of trailing checksum bytes in each frame."""

        return {
            ChecksumKind.NONE: 0,
            ChecksumKind.XOR8: 1,
            ChecksumKind.CRC16_MODBUS: 2,
            ChecksumKind.CRC32: 4,
            ChecksumKind.NMEA0183: 3,
        }[self.checksum]


@dataclass(frozen=True, slots=True)
class DecodedFrame:
    """A bounded frame result; invalid integrity never disappears silently."""

    sequence: int
    payload: bytes
    status: FrameStatus
    error: str | None = None
    source: ProtocolSource | None = None

    def __post_init__(self) -> None:
        if isinstance(self.sequence, bool) or self.sequence < 1:
            raise ConfigurationError("协议 frame sequence 必须是正整数。")
        if not isinstance(self.status, FrameStatus):
            raise ConfigurationError("协议 frame status 必须使用 FrameStatus。")
        if self.source is not None and not isinstance(self.source, ProtocolSource):
            raise ConfigurationError("协议 frame source 类型无效。")
        if not isinstance(self.payload, (bytes, bytearray, memoryview)):
            raise ConfigurationError("协议 frame payload 必须是 bytes-like。")
        payload = bytes(self.payload)
        if len(payload) > MAX_PROTOCOL_FRAME_BYTES:
            raise ConfigurationError("协议 frame payload 超过上限。")
        if self.error is not None and not self.error.strip():
            raise ConfigurationError("协议 frame error 不能为空字符串。")
        object.__setattr__(self, "payload", payload)


@dataclass(frozen=True, slots=True)
class ProtocolStats:
    """Bounded parser counters exposed to a component view."""

    bytes_in: int = 0
    frames_valid: int = 0
    frames_invalid: int = 0
    frames_incomplete: int = 0
    dropped_bytes: int = 0
    buffered_bytes: int = 0
    gap_boundaries: int = 0
    resyncs: int = 0


class FrameDecoderPort(Protocol):
    """Pure streaming decoder interface independent of transport and Qt."""

    @property
    def config(self) -> ProtocolConfig:
        """Return the active immutable configuration."""

    @property
    def stats(self) -> ProtocolStats:
        """Return bounded parser counters."""

    def feed(self, payload: bytes) -> tuple[DecodedFrame, ...]:
        """Consume bytes and return only complete frame results."""

    def finish(self) -> tuple[DecodedFrame, ...]:
        """Flush a partial frame as an explicit incomplete result."""

    def reset(self) -> None:
        """Drop a partial frame and reset counters for a new session."""


class IngressFrameDecoderPort(FrameDecoderPort, Protocol):
    """Optional decoder contract that consumes parser-only timing facts."""

    def feed_unit(self, unit: ProtocolIngressUnit) -> tuple[DecodedFrame, ...]:
        """Consume bytes plus the gap observation preceding this ingress unit."""


class StreamingFrameDecoder:
    """Bounded line/delimiter/length decoder with visible checksum failures."""

    def __init__(self, config: ProtocolConfig | None = None) -> None:
        self._config = config or ProtocolConfig()
        self._buffer = bytearray()
        self._sequence = 0
        self._bytes_in = 0
        self._frames_valid = 0
        self._frames_invalid = 0
        self._frames_incomplete = 0
        self._dropped_bytes = 0
        self._gap_boundaries = 0
        self._resyncs = 0

    @property
    def config(self) -> ProtocolConfig:
        return self._config

    @property
    def stats(self) -> ProtocolStats:
        return ProtocolStats(
            bytes_in=self._bytes_in,
            frames_valid=self._frames_valid,
            frames_invalid=self._frames_invalid,
            frames_incomplete=self._frames_incomplete,
            dropped_bytes=self._dropped_bytes,
            buffered_bytes=len(self._buffer),
            gap_boundaries=self._gap_boundaries,
            resyncs=self._resyncs,
        )

    def configure(self, config: ProtocolConfig) -> None:
        if not isinstance(config, ProtocolConfig):
            raise ConfigurationError("协议 decoder 只接受 ProtocolConfig。")
        self._config = config
        self.reset()

    def reset(self) -> None:
        self._buffer.clear()
        self._sequence = 0
        self._bytes_in = 0
        self._frames_valid = 0
        self._frames_invalid = 0
        self._frames_incomplete = 0
        self._dropped_bytes = 0
        self._gap_boundaries = 0
        self._resyncs = 0

    def feed(self, payload: bytes) -> tuple[DecodedFrame, ...]:
        if not isinstance(payload, (bytes, bytearray, memoryview)):
            raise ProtocolInputError("协议 decoder payload 必须是 bytes-like。")
        value = bytes(payload)
        if not value:
            return ()
        self._bytes_in += len(value)
        if self._config.framing is FramingKind.RAW:
            if len(value) > self._config.max_frame_bytes:
                self._dropped_bytes += len(value)
                return (
                    self._result(b"", FrameStatus.OVERSIZE, "Raw chunk 超过 max_frame_bytes。"),
                )
            return (self._finalize(value),)
        self._buffer.extend(value)
        if len(self._buffer) > MAX_PROTOCOL_BUFFER_BYTES:
            dropped = len(self._buffer)
            self._buffer.clear()
            self._dropped_bytes += dropped
            return (
                self._result(
                    b"",
                    FrameStatus.OVERSIZE,
                    "协议 partial buffer 超过 256 KiB，已丢弃并重新同步。",
                ),
            )
        frames: list[DecodedFrame] = []
        while len(frames) < MAX_PROTOCOL_BATCH_FRAMES:
            extracted = self._extract_one()
            if extracted is None:
                break
            body, status, error = extracted
            if status is not None:
                frames.append(self._result(body, status, error))
                continue
            frames.append(self._finalize(body))
        return tuple(frames)

    def finish(self) -> tuple[DecodedFrame, ...]:
        if not self._buffer:
            return ()
        payload = bytes(self._buffer)
        self._buffer.clear()
        self._dropped_bytes += len(payload)
        return (self._result(payload, FrameStatus.INCOMPLETE, "source 在完整 frame 前结束。"),)

    def _extract_one(self) -> tuple[bytes, FrameStatus | None, str | None] | None:
        if self._config.framing in {FramingKind.LINE, FramingKind.DELIMITER}:
            delimiter = self._config.delimiter
            position = self._buffer.find(delimiter)
            if position < 0:
                if len(self._buffer) > self._config.max_frame_bytes + len(delimiter):
                    dropped = len(self._buffer)
                    self._buffer.clear()
                    self._dropped_bytes += dropped
                    return (
                        b"",
                        FrameStatus.OVERSIZE,
                        "delimiter frame 超过 max_frame_bytes，已丢弃并重新同步。",
                    )
                return None
            body = bytes(self._buffer[:position])
            del self._buffer[: position + len(delimiter)]
            if self._config.framing is FramingKind.LINE and body.endswith(b"\r"):
                body = body[:-1]
            if len(body) > self._config.max_frame_bytes:
                return b"", FrameStatus.OVERSIZE, "line/delimiter frame 超过 max_frame_bytes。"
            return body, None, None

        width = self._config.length_bytes
        if len(self._buffer) < width:
            return None
        length = int.from_bytes(self._buffer[:width], self._config.byteorder)
        checksum_bytes = self._config.checksum_bytes
        if length > self._config.max_frame_bytes:
            del self._buffer[:width]
            self._dropped_bytes += width
            return b"", FrameStatus.INVALID_LENGTH, "length prefix 超过 max_frame_bytes。"
        total = width + length + checksum_bytes
        if len(self._buffer) < total:
            return None
        body_start = width
        body_end = body_start + length + checksum_bytes
        body = bytes(self._buffer[body_start:body_end])
        del self._buffer[:body_end]
        return body, None, None

    def _finalize(self, body: bytes) -> DecodedFrame:
        if self._config.checksum is ChecksumKind.NMEA0183:
            return self._finalize_nmea0183(body)
        checksum_bytes = self._config.checksum_bytes
        if checksum_bytes == 0:
            return self._result(body, FrameStatus.VALID, None)
        if len(body) < checksum_bytes:
            return self._result(
                body,
                FrameStatus.INVALID_CHECKSUM,
                "frame 长度不足以包含 checksum。",
            )
        content = body[:-checksum_bytes]
        actual = body[-checksum_bytes:]
        expected = self._calculate_checksum(content)
        if actual != expected:
            return self._result(
                content,
                FrameStatus.INVALID_CHECKSUM,
                f"checksum 不匹配：expected={expected.hex(' ')} actual={actual.hex(' ')}。",
            )
        return self._result(content, FrameStatus.VALID, None)

    def _finalize_nmea0183(self, body: bytes) -> DecodedFrame:
        """Validate one NMEA sentence and remove only its ``*HH`` suffix."""

        if not body or body[:1] != b"$":
            return self._result(
                body,
                FrameStatus.INVALID_FORMAT,
                "NMEA sentence 必须以 $ 开始。",
            )
        if body.count(b"*") != 1:
            return self._result(
                body,
                FrameStatus.INVALID_FORMAT,
                "NMEA sentence 必须包含唯一 * checksum 分隔符。",
            )
        separator = body.find(b"*")
        content = body[:separator]
        checksum_text = body[separator + 1 :]
        if separator <= 1 or not content[1:].isascii():
            return self._result(
                body,
                FrameStatus.INVALID_FORMAT,
                "NMEA sentence 的 $ 与 * 之间必须是非空 ASCII。",
            )
        if len(checksum_text) != 2 or not checksum_text.isascii():
            return self._result(
                body,
                FrameStatus.INVALID_FORMAT,
                "NMEA checksum 必须是两个 ASCII 十六进制字符。",
            )
        try:
            actual = int(checksum_text.decode("ascii"), 16)
        except ValueError:
            return self._result(
                body,
                FrameStatus.INVALID_FORMAT,
                "NMEA checksum 不是有效十六进制值。",
            )
        expected = 0
        for item in content[1:]:
            expected ^= item
        if actual != expected:
            return self._result(
                content,
                FrameStatus.INVALID_CHECKSUM,
                f"NMEA checksum 不匹配：expected={expected:02X} actual={actual:02X}。",
            )
        return self._result(content, FrameStatus.VALID, None)

    def _calculate_checksum(self, payload: bytes) -> bytes:
        kind = self._config.checksum
        if kind is ChecksumKind.XOR8:
            value = 0
            for item in payload:
                value ^= item
        elif kind is ChecksumKind.CRC16_MODBUS:
            value = 0xFFFF
            for item in payload:
                value ^= item
                for _ in range(8):
                    value = (value >> 1) ^ 0xA001 if value & 1 else value >> 1
        elif kind is ChecksumKind.CRC32:
            value = binascii.crc32(payload) & 0xFFFFFFFF
        else:
            value = 0
        return value.to_bytes(self._config.checksum_bytes, self._config.checksum_byteorder)

    def _result(
        self,
        payload: bytes,
        status: FrameStatus,
        error: str | None,
    ) -> DecodedFrame:
        self._sequence += 1
        if status is FrameStatus.VALID:
            self._frames_valid += 1
        elif status is FrameStatus.INCOMPLETE:
            self._frames_incomplete += 1
        elif status is not FrameStatus.UNVERIFIED:
            self._frames_invalid += 1
        return DecodedFrame(self._sequence, payload, status, error)

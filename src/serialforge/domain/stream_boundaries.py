"""Protocol-specific stream boundaries kept outside transport and codecs.

These decoders are intentionally small and stateful per source.  They do not
own timers, threads, sockets, or Qt objects.  The application worker supplies
one ingress unit at a time and the decoder returns bounded frame results.
"""

from __future__ import annotations

from typing import Final

from .errors import ConfigurationError, ProtocolInputError
from .mavlink import (
    MAVLINK_CRC_BYTES,
    MAVLINK_MAX_PACKET_BYTES,
    MAVLINK_MAX_PAYLOAD_BYTES,
    MAVLINK_MAX_V1_PACKET_BYTES,
    MAVLINK_MAX_V2_PACKET_BYTES,
    MAVLINK_V1_HEADER_BYTES,
    MAVLINK_V1_MAGIC,
    MAVLINK_V2_HEADER_BYTES,
    MAVLINK_V2_MAGIC,
    MAVLINK_V2_SIGNATURE_BYTES,
    MAVLINK_V2_SIGNED_FLAG,
    decode_mavlink,
)
from .modbus import decode_modbus_rtu
from .protocols import (
    MAX_PROTOCOL_BATCH_FRAMES,
    MAX_PROTOCOL_BUFFER_BYTES,
    DecodedFrame,
    FrameStatus,
    FramingKind,
    IngressFrameDecoderPort,
    ProtocolConfig,
    ProtocolIngressUnit,
    ProtocolStats,
)
from .timing import GapObservation, TimingQuality

MAVLINK_MAGIC_BYTES: Final = bytes((MAVLINK_V1_MAGIC, MAVLINK_V2_MAGIC))


class _BoundaryDecoderBase:
    """Shared bounded counters for protocol-specific stream decoders."""

    def __init__(self, config: ProtocolConfig) -> None:
        if not isinstance(config, ProtocolConfig):
            raise ConfigurationError("stream boundary decoder 只接受 ProtocolConfig。")
        self._config = config
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

    @staticmethod
    def _payload(value: bytes | bytearray | memoryview) -> bytes:
        if not isinstance(value, (bytes, bytearray, memoryview)):
            raise ProtocolInputError("stream boundary payload 必须是 bytes-like。")
        return bytes(value)


class MavlinkStreamDecoder(_BoundaryDecoderBase, IngressFrameDecoderPort):
    """Extract MAVLink v1/v2 packets from a noisy byte stream.

    The length field is used only for structural extraction.  CRC_EXTRA and
    signature authentication remain in ``domain.mavlink`` and the component
    codec, so a structurally complete packet is emitted as ``UNVERIFIED`` here
    and can become a validated or integrity-failed result downstream.
    """

    def __init__(self, config: ProtocolConfig) -> None:
        if config.framing is not FramingKind.MAVLINK_STREAM:
            raise ConfigurationError("MAVLink stream decoder 需要 MAVLINK_STREAM framing。")
        super().__init__(config)

    def feed_unit(self, unit: ProtocolIngressUnit) -> tuple[DecodedFrame, ...]:
        if not isinstance(unit, ProtocolIngressUnit):
            raise ConfigurationError("MAVLink stream decoder 只接受 ProtocolIngressUnit。")
        return self.feed(unit.payload)

    def feed(self, payload: bytes) -> tuple[DecodedFrame, ...]:
        value = self._payload(payload)
        if not value:
            return ()
        self._bytes_in += len(value)
        self._buffer.extend(value)
        if len(self._buffer) > MAX_PROTOCOL_BUFFER_BYTES:
            dropped = len(self._buffer)
            self._buffer.clear()
            self._dropped_bytes += dropped
            self._resyncs += 1
            return (
                self._result(
                    b"",
                    FrameStatus.OVERSIZE,
                    "MAVLink stream buffer 超过 256 KiB，已丢弃并重新同步。",
                ),
            )

        frames: list[DecodedFrame] = []
        while len(frames) < MAX_PROTOCOL_BATCH_FRAMES:
            anchor = self._find_magic()
            if anchor < 0:
                dropped = len(self._buffer)
                self._buffer.clear()
                if dropped:
                    self._dropped_bytes += dropped
                    self._resyncs += 1
                break
            if anchor:
                del self._buffer[:anchor]
                self._dropped_bytes += anchor
                self._resyncs += 1
            if len(self._buffer) < 2:
                break
            magic = self._buffer[0]
            if magic == MAVLINK_V1_MAGIC:
                header_bytes = MAVLINK_V1_HEADER_BYTES
                maximum = MAVLINK_MAX_V1_PACKET_BYTES
            else:
                header_bytes = MAVLINK_V2_HEADER_BYTES
                maximum = MAVLINK_MAX_V2_PACKET_BYTES
            if len(self._buffer) < header_bytes:
                break
            payload_length = self._buffer[1]
            if payload_length > MAVLINK_MAX_PAYLOAD_BYTES:
                del self._buffer[0]
                self._dropped_bytes += 1
                self._resyncs += 1
                continue
            signed_bytes = (
                MAVLINK_V2_SIGNATURE_BYTES
                if magic == MAVLINK_V2_MAGIC and self._buffer[2] & MAVLINK_V2_SIGNED_FLAG
                else 0
            )
            total = header_bytes + payload_length + MAVLINK_CRC_BYTES + signed_bytes
            if total > maximum or total > MAVLINK_MAX_PACKET_BYTES:
                del self._buffer[0]
                self._dropped_bytes += 1
                self._resyncs += 1
                continue
            if len(self._buffer) < total:
                alternative = self._find_complete_resync_anchor()
                if alternative is not None:
                    del self._buffer[:alternative]
                    self._dropped_bytes += alternative
                    self._resyncs += 1
                    continue
                break
            candidate = bytes(self._buffer[:total])
            del self._buffer[:total]
            structural = decode_mavlink(candidate)
            if structural.status in {
                FrameStatus.INVALID_FORMAT,
                FrameStatus.INVALID_LENGTH,
                FrameStatus.OVERSIZE,
            }:
                frames.append(self._result(candidate, structural.status, structural.error))
            else:
                # Structural extraction is complete, but the stream layer has
                # no dialect-specific CRC_EXTRA or signing key.  Preserve the
                # packet and let the explicit component profile perform the
                # second-stage validation without claiming wire integrity.
                frames.append(self._result(candidate, FrameStatus.UNVERIFIED, None))
        return tuple(frames)

    def finish(self) -> tuple[DecodedFrame, ...]:
        if not self._buffer:
            return ()
        payload = bytes(self._buffer)
        self._buffer.clear()
        self._dropped_bytes += len(payload)
        return (
            self._result(
                payload,
                FrameStatus.INCOMPLETE,
                "source 在完整 MAVLink packet 前结束。",
            ),
        )

    def _find_magic(self) -> int:
        positions = [
            position
            for magic in MAVLINK_MAGIC_BYTES
            if (position := self._buffer.find(bytes((magic,)))) >= 0
        ]
        return min(positions) if positions else -1

    def _find_complete_resync_anchor(self) -> int | None:
        """Recover when a corrupt length field blocks a later complete packet.

        This is deliberately a conservative structural heuristic.  It only
        re-anchors on a later magic byte when that candidate is complete and
        structurally valid without CRC_EXTRA; payload bytes that merely look
        like magic remain part of the current candidate otherwise.
        """

        for anchor in range(1, len(self._buffer)):
            if self._buffer[anchor] not in MAVLINK_MAGIC_BYTES:
                continue
            total = self._packet_total_at(anchor)
            if total is None or len(self._buffer) - anchor < total:
                continue
            candidate = bytes(self._buffer[anchor : anchor + total])
            structural = decode_mavlink(candidate)
            if structural.status in {FrameStatus.VALID, FrameStatus.UNVERIFIED}:
                return anchor
        return None

    def _packet_total_at(self, anchor: int) -> int | None:
        remaining = len(self._buffer) - anchor
        if remaining < 2:
            return None
        magic = self._buffer[anchor]
        if magic == MAVLINK_V1_MAGIC:
            header_bytes = MAVLINK_V1_HEADER_BYTES
            maximum = MAVLINK_MAX_V1_PACKET_BYTES
        elif magic == MAVLINK_V2_MAGIC:
            header_bytes = MAVLINK_V2_HEADER_BYTES
            maximum = MAVLINK_MAX_V2_PACKET_BYTES
        else:
            return None
        if remaining < header_bytes:
            return None
        payload_length = self._buffer[anchor + 1]
        signed_bytes = (
            MAVLINK_V2_SIGNATURE_BYTES
            if magic == MAVLINK_V2_MAGIC and self._buffer[anchor + 2] & MAVLINK_V2_SIGNED_FLAG
            else 0
        )
        total = header_bytes + payload_length + MAVLINK_CRC_BYTES + signed_bytes
        return total if total <= maximum else None


class ModbusRtuStreamDecoder(_BoundaryDecoderBase, IngressFrameDecoderPort):
    """Split Modbus RTU ADUs only on an explicit observed silent interval."""

    def __init__(self, config: ProtocolConfig) -> None:
        if config.framing is not FramingKind.MODBUS_RTU_TIMED:
            raise ConfigurationError("Modbus RTU decoder 需要 MODBUS_RTU_TIMED framing。")
        if config.modbus_timing is None:
            raise ConfigurationError("Modbus RTU decoder 缺少 timing。")
        super().__init__(config)

    def feed_unit(self, unit: ProtocolIngressUnit) -> tuple[DecodedFrame, ...]:
        if not isinstance(unit, ProtocolIngressUnit):
            raise ConfigurationError("Modbus RTU decoder 只接受 ProtocolIngressUnit。")
        return self._consume(unit.payload, unit.timing)

    def feed(self, payload: bytes) -> tuple[DecodedFrame, ...]:
        value = self._payload(payload)
        if not value:
            return ()
        return self._consume(value, GapObservation())

    def finish(self) -> tuple[DecodedFrame, ...]:
        if not self._buffer:
            return ()
        payload = bytes(self._buffer)
        self._buffer.clear()
        self._dropped_bytes += len(payload)
        return (
            self._result(
                payload,
                FrameStatus.INCOMPLETE,
                "source 在 RTU silent boundary 前结束。",
            ),
        )

    def _consume(
        self,
        payload: bytes,
        observation: GapObservation,
    ) -> tuple[DecodedFrame, ...]:
        self._bytes_in += len(payload)
        frames: list[DecodedFrame] = []
        timing = self._config.modbus_timing
        assert timing is not None
        gap_before = (
            observation.gap_before
            if observation.quality in {TimingQuality.HOST_READ_GAP, TimingQuality.DEVICE_TIMESTAMP}
            else None
        )
        if gap_before is not None and self._buffer:
            if gap_before >= timing.t3_5_seconds:
                self._gap_boundaries += 1
                frames.append(self._finalize_current())
            elif gap_before > timing.t1_5_seconds:
                self._gap_boundaries += 1
                self._resyncs += 1
                frames.append(self._discard_partial())
        self._buffer.extend(payload)
        if len(self._buffer) > 256:
            oversized = bytes(self._buffer)
            self._buffer.clear()
            self._dropped_bytes += len(oversized)
            self._resyncs += 1
            frames.append(
                self._result(
                    oversized,
                    FrameStatus.OVERSIZE,
                    "Modbus RTU ADU 超过 256 B，已丢弃并等待下一个 silent boundary。",
                )
            )
        return tuple(frames)

    def _finalize_current(self) -> DecodedFrame:
        payload = bytes(self._buffer)
        self._buffer.clear()
        decoded = decode_modbus_rtu(payload)
        return self._result(payload, decoded.status, decoded.error)

    def _discard_partial(self) -> DecodedFrame:
        payload = bytes(self._buffer)
        self._buffer.clear()
        self._dropped_bytes += len(payload)
        return self._result(
            payload,
            FrameStatus.INCOMPLETE,
            "RTU frame 内 gap 超过 t1.5，候选 ADU 已丢弃。",
        )


__all__ = ["MavlinkStreamDecoder", "ModbusRtuStreamDecoder"]

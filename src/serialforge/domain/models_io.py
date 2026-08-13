"""Stream, datagram, and accepted-peer value objects.

These objects preserve boundaries between stream, datagram, BLE, and server
operations without depending on presentation or infrastructure modules.
"""

from __future__ import annotations

from dataclasses import dataclass
from enum import StrEnum
from uuid import UUID

from .errors import ConfigurationError, ErrorInfo
from .models_transport import (
    MAX_BLE_WRITE_BYTES,
    MAX_BLE_WRITE_WITH_RESPONSE_BYTES,
    MAX_STREAM_PAYLOAD_BYTES,
    MAX_UDP_DATAGRAM_BYTES,
    BleGattCharacteristicRef,
    BleGattTransportConfig,
    BleGattWriteMode,
    PeerAddress,
    PeerId,
    RttTransportConfig,
    ServerReadKind,
    ServerRejectReason,
    TcpServerTransportConfig,
    TcpTransportConfig,
    UartTransportConfig,
    UdpTransportConfig,
)


@dataclass(frozen=True, slots=True)
class ServerReadResult:
    """Typed listener/client outcome; writes and peer failures stay observable."""

    kind: ServerReadKind
    peer: PeerAddress | None = None
    chunk: StreamChunk | None = None
    reason: ServerRejectReason | None = None
    peer_id: PeerId | None = None
    error: ErrorInfo | None = None

    def __post_init__(self) -> None:
        if not isinstance(self.kind, ServerReadKind):
            raise ConfigurationError("TCP Server read 结果必须使用 ServerReadKind。")
        if self.kind is ServerReadKind.TIMEOUT:
            if (
                self.peer is not None
                or self.chunk is not None
                or self.reason is not None
                or self.peer_id is not None
                or self.error is not None
            ):
                raise ConfigurationError(
                    "TIMEOUT TCP Server read 不能包含 peer/chunk/reason/peer_id/error。"
                )
        elif self.kind in {ServerReadKind.DATA, ServerReadKind.WRITE_COMPLETED}:
            if (
                self.peer is None
                or self.chunk is None
                or self.reason is not None
                or self.peer_id is None
                or self.error is not None
            ):
                raise ConfigurationError(
                    "DATA/WRITE_COMPLETED TCP Server read 必须包含 peer、peer_id 和 payload。"
                )
        elif self.kind is ServerReadKind.PEER_REJECTED:
            if (
                self.peer is None
                or self.chunk is not None
                or self.reason is None
                or self.peer_id is not None
                or self.error is not None
            ):
                raise ConfigurationError("PEER_REJECTED 必须包含 peer 和 reason。")
        elif self.kind is ServerReadKind.CLIENT_ERROR:
            if (
                self.peer is None
                or self.chunk is not None
                or self.reason is not None
                or self.peer_id is None
                or self.error is None
            ):
                raise ConfigurationError(
                    "CLIENT_ERROR TCP Server read 必须包含 peer、peer_id 和 error。"
                )
        else:
            if (
                self.peer is None
                or self.chunk is not None
                or self.reason is not None
                or self.peer_id is None
                or self.error is not None
            ):
                raise ConfigurationError("TCP Server client 状态结果必须包含 peer 和 peer_id。")
        if self.peer is not None and not isinstance(self.peer, PeerAddress):
            raise ConfigurationError("TCP Server read peer 必须使用 PeerAddress。")
        if self.chunk is not None and not isinstance(self.chunk, StreamChunk):
            raise ConfigurationError("TCP Server read chunk 必须使用 StreamChunk。")
        if self.reason is not None and not isinstance(self.reason, ServerRejectReason):
            raise ConfigurationError("TCP Server reject reason 必须使用 ServerRejectReason。")
        if self.peer_id is not None and not isinstance(self.peer_id, UUID):
            raise ConfigurationError("TCP Server read peer_id 必须使用 UUID。")
        if self.error is not None and not isinstance(self.error, ErrorInfo):
            raise ConfigurationError("TCP Server read error 必须使用 ErrorInfo。")

    @classmethod
    def timeout(cls) -> ServerReadResult:
        return cls(ServerReadKind.TIMEOUT)

    @classmethod
    def client_connected(cls, peer_id: PeerId, peer: PeerAddress) -> ServerReadResult:
        return cls(ServerReadKind.CLIENT_CONNECTED, peer=peer, peer_id=peer_id)

    @classmethod
    def data(cls, peer_id: PeerId, peer: PeerAddress, payload: bytes) -> ServerReadResult:
        return cls(ServerReadKind.DATA, peer=peer, chunk=StreamChunk(payload), peer_id=peer_id)

    @classmethod
    def client_eof(cls, peer_id: PeerId, peer: PeerAddress) -> ServerReadResult:
        return cls(ServerReadKind.CLIENT_EOF, peer=peer, peer_id=peer_id)

    @classmethod
    def write_completed(
        cls,
        peer_id: PeerId,
        peer: PeerAddress,
        payload: bytes,
    ) -> ServerReadResult:
        return cls(
            ServerReadKind.WRITE_COMPLETED,
            peer=peer,
            chunk=StreamChunk(payload),
            peer_id=peer_id,
        )

    @classmethod
    def client_error(
        cls,
        peer_id: PeerId,
        peer: PeerAddress,
        error: ErrorInfo,
    ) -> ServerReadResult:
        return cls(
            ServerReadKind.CLIENT_ERROR,
            peer=peer,
            peer_id=peer_id,
            error=error,
        )

    @classmethod
    def rejected(cls, peer: PeerAddress, reason: ServerRejectReason) -> ServerReadResult:
        return cls(ServerReadKind.PEER_REJECTED, peer=peer, reason=reason)


type TransportConfig = (
    UartTransportConfig
    | TcpTransportConfig
    | RttTransportConfig
    | UdpTransportConfig
    | TcpServerTransportConfig
    | BleGattTransportConfig
)


@dataclass(frozen=True, slots=True)
class StreamChunk:
    """A bounded unit of stream data kept as raw bytes."""

    payload: bytes

    def __post_init__(self) -> None:
        if not isinstance(self.payload, (bytes, bytearray, memoryview)):
            raise ConfigurationError("StreamChunk payload 必须是 bytes-like。")
        payload = bytes(self.payload)
        if not payload:
            raise ConfigurationError("StreamChunk payload 不能为空。")
        if len(payload) > MAX_STREAM_PAYLOAD_BYTES:
            raise ConfigurationError("StreamChunk payload 超过 1 MiB 上限。")
        object.__setattr__(self, "payload", payload)


@dataclass(frozen=True, slots=True)
class StreamWrite:
    """An outbound stream operation; datagram/GATT semantics stay separate."""

    payload: bytes

    def __post_init__(self) -> None:
        if not isinstance(self.payload, (bytes, bytearray, memoryview)):
            raise ConfigurationError("StreamWrite payload 必须是 bytes-like。")
        payload = bytes(self.payload)
        if not payload:
            raise ConfigurationError("StreamWrite payload 不能为空。")
        if len(payload) > MAX_STREAM_PAYLOAD_BYTES:
            raise ConfigurationError("StreamWrite payload 超过 1 MiB 上限。")
        object.__setattr__(self, "payload", payload)


@dataclass(frozen=True, slots=True)
class RttDownWrite:
    """An explicit write addressed to the RTT channel bound to this session."""

    channel: int
    write: StreamWrite

    def __post_init__(self) -> None:
        if isinstance(self.channel, bool) or self.channel not in {0, 1}:
            raise ConfigurationError("RTT Down channel 只能是 0 或 1。")
        if not isinstance(self.write, StreamWrite):
            raise ConfigurationError("RTT Down write 必须使用 StreamWrite。")

    @property
    def payload(self) -> bytes:
        """Expose the bounded payload for queue accounting and raw recording."""

        return self.write.payload


@dataclass(frozen=True, slots=True)
class TcpServerSend:
    """An explicit stream write addressed to one accepted TCP Server peer."""

    peer_id: PeerId
    write: StreamWrite

    def __post_init__(self) -> None:
        if not isinstance(self.peer_id, UUID):
            raise ConfigurationError("TCP Server send peer_id 必须使用 UUID。")
        if not isinstance(self.write, StreamWrite):
            raise ConfigurationError("TCP Server send write 必须使用 StreamWrite。")


@dataclass(frozen=True, slots=True)
class BleGattRead:
    """Read one discovered characteristic instance."""

    characteristic: BleGattCharacteristicRef

    def __post_init__(self) -> None:
        if not isinstance(self.characteristic, BleGattCharacteristicRef):
            raise ConfigurationError("BLE read characteristic 类型无效。")


@dataclass(frozen=True, slots=True)
class BleGattWrite:
    """Write bytes with an explicit GATT response mode."""

    characteristic: BleGattCharacteristicRef
    mode: BleGattWriteMode
    payload: bytes

    def __post_init__(self) -> None:
        if not isinstance(self.characteristic, BleGattCharacteristicRef):
            raise ConfigurationError("BLE write characteristic 类型无效。")
        if not isinstance(self.mode, BleGattWriteMode):
            raise ConfigurationError("BLE write mode 类型无效。")
        if not isinstance(self.payload, (bytes, bytearray, memoryview)):
            raise ConfigurationError("BLE write payload 必须是 bytes-like。")
        payload = bytes(self.payload)
        if not payload:
            raise ConfigurationError("BLE write payload 不能为空。")
        if len(payload) > MAX_BLE_WRITE_BYTES:
            raise ConfigurationError(f"BLE write payload 不能超过 {MAX_BLE_WRITE_BYTES} 字节。")
        if (
            self.mode is BleGattWriteMode.WITH_RESPONSE
            and len(payload) > MAX_BLE_WRITE_WITH_RESPONSE_BYTES
        ):
            raise ConfigurationError(
                f"BLE with-response payload 不能超过 {MAX_BLE_WRITE_WITH_RESPONSE_BYTES} "
                "字节；当前不自动分片。"
            )
        object.__setattr__(self, "payload", payload)


@dataclass(frozen=True, slots=True)
class BleGattNotify:
    """Enable or disable notification/indication on one characteristic."""

    characteristic: BleGattCharacteristicRef
    enabled: bool

    def __post_init__(self) -> None:
        if not isinstance(self.characteristic, BleGattCharacteristicRef):
            raise ConfigurationError("BLE notify characteristic 类型无效。")
        if not isinstance(self.enabled, bool):
            raise ConfigurationError("BLE notify enabled 必须是 bool。")


type BleGattCommand = BleGattRead | BleGattWrite | BleGattNotify


class StreamReadKind(StrEnum):
    """Outcome of one bounded stream read."""

    DATA = "data"
    TIMEOUT = "timeout"
    EOF = "eof"


@dataclass(frozen=True, slots=True)
class StreamReadResult:
    """Typed stream read outcome; EOF is never represented as empty data."""

    kind: StreamReadKind
    chunk: StreamChunk | None = None
    peer: PeerAddress | None = None

    def __post_init__(self) -> None:
        if not isinstance(self.kind, StreamReadKind):
            raise ConfigurationError("stream read 结果必须使用 StreamReadKind。")
        if self.kind is StreamReadKind.DATA:
            if self.chunk is None:
                raise ConfigurationError("DATA stream read 必须包含 payload。")
        elif self.chunk is not None:
            raise ConfigurationError("TIMEOUT/EOF stream read 不能包含 payload。")
        if self.peer is not None and not isinstance(self.peer, PeerAddress):
            raise ConfigurationError("stream read peer 必须使用 PeerAddress。")

    @classmethod
    def data(cls, payload: bytes, peer: PeerAddress | None = None) -> StreamReadResult:
        return cls(StreamReadKind.DATA, StreamChunk(payload), peer)

    @classmethod
    def timeout(cls, peer: PeerAddress | None = None) -> StreamReadResult:
        return cls(StreamReadKind.TIMEOUT, peer=peer)

    @classmethod
    def eof(cls, peer: PeerAddress | None = None) -> StreamReadResult:
        return cls(StreamReadKind.EOF, peer=peer)


@dataclass(frozen=True, slots=True)
class Datagram:
    """One complete inbound UDP datagram with its source peer."""

    peer: PeerAddress
    payload: bytes

    def __post_init__(self) -> None:
        if not isinstance(self.peer, PeerAddress):
            raise ConfigurationError("UDP datagram peer 必须使用 PeerAddress。")
        if not isinstance(self.payload, (bytes, bytearray, memoryview)):
            raise ConfigurationError("UDP datagram payload 必须是 bytes-like。")
        payload = bytes(self.payload)
        if len(payload) > MAX_UDP_DATAGRAM_BYTES:
            raise ConfigurationError(
                f"UDP datagram payload 超过 {MAX_UDP_DATAGRAM_BYTES} 字节硬上限。"
            )
        object.__setattr__(self, "payload", payload)


@dataclass(frozen=True, slots=True)
class DatagramSend:
    """One complete outbound UDP datagram addressed to an explicit peer."""

    peer: PeerAddress
    payload: bytes

    def __post_init__(self) -> None:
        if not isinstance(self.peer, PeerAddress):
            raise ConfigurationError("UDP send peer 必须使用 PeerAddress。")
        if not isinstance(self.payload, (bytes, bytearray, memoryview)):
            raise ConfigurationError("UDP send payload 必须是 bytes-like。")
        payload = bytes(self.payload)
        if len(payload) > MAX_UDP_DATAGRAM_BYTES:
            raise ConfigurationError(f"UDP send payload 超过 {MAX_UDP_DATAGRAM_BYTES} 字节硬上限。")
        object.__setattr__(self, "payload", payload)


class DatagramReadKind(StrEnum):
    """Outcome of one bounded UDP receive operation."""

    DATA = "data"
    TIMEOUT = "timeout"
    DROPPED = "dropped"


class DatagramDropReason(StrEnum):
    """Observable reasons a UDP datagram was not delivered to the terminal."""

    UNAUTHORIZED_PEER = "unauthorized_peer"
    PAYLOAD_TOO_LARGE = "payload_too_large"


@dataclass(frozen=True, slots=True)
class DatagramReadResult:
    """Typed UDP result preserving datagram boundaries and drop causes."""

    kind: DatagramReadKind
    datagram: Datagram | None = None
    peer: PeerAddress | None = None
    reason: DatagramDropReason | None = None

    def __post_init__(self) -> None:
        if not isinstance(self.kind, DatagramReadKind):
            raise ConfigurationError("UDP read 结果必须使用 DatagramReadKind。")
        if self.kind is DatagramReadKind.DATA:
            if self.datagram is None or self.peer != self.datagram.peer:
                raise ConfigurationError("DATA UDP read 必须包含匹配 peer 的 datagram。")
            if self.reason is not None:
                raise ConfigurationError("DATA UDP read 不能包含丢弃原因。")
        elif self.kind is DatagramReadKind.TIMEOUT:
            if self.datagram is not None or self.peer is not None or self.reason is not None:
                raise ConfigurationError("TIMEOUT UDP read 不能包含 datagram/peer/reason。")
        else:
            if self.datagram is not None or self.peer is None or self.reason is None:
                raise ConfigurationError("DROPPED UDP read 必须包含 peer 和 reason。")

    @classmethod
    def data(cls, datagram: Datagram) -> DatagramReadResult:
        return cls(DatagramReadKind.DATA, datagram=datagram, peer=datagram.peer)

    @classmethod
    def timeout(cls) -> DatagramReadResult:
        return cls(DatagramReadKind.TIMEOUT)

    @classmethod
    def dropped(cls, peer: PeerAddress, reason: DatagramDropReason) -> DatagramReadResult:
        return cls(DatagramReadKind.DROPPED, peer=peer, reason=reason)

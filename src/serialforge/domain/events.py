"""Immutable events crossing the worker, application, and presentation layers."""

from __future__ import annotations

from dataclasses import dataclass, field
from uuid import UUID

from .codecs import ComponentConfiguration
from .components import ComponentFrameRow, ComponentStats
from .datasets import DatasetConfig, DatasetSample, DatasetStats
from .errors import ErrorInfo
from .models import (
    BleGattCharacteristicRef,
    BleGattService,
    BleGattWrite,
    Datagram,
    DatagramDropReason,
    DatagramSend,
    Endpoint,
    PeerAddress,
    PeerId,
    RecordingSnapshot,
    SessionId,
    SessionState,
    StreamChunk,
    StreamWrite,
    TcpServerClientState,
    TcpServerPeerSnapshot,
)
from .protocols import DecodedFrame, ProtocolSource, ProtocolStats
from .replay import ReplayRecord, ReplaySnapshot
from .timing import GapObservation


@dataclass(frozen=True, slots=True)
class SessionStateChangedEvent:
    """A lifecycle transition for one session."""

    session_id: SessionId
    endpoint: Endpoint
    state: SessionState
    occurred_at: float
    error: ErrorInfo | None = None


@dataclass(frozen=True, slots=True)
class SessionErrorEvent:
    """A structured failure associated with a session."""

    session_id: SessionId
    endpoint: Endpoint
    error: ErrorInfo
    occurred_at: float
    peer: PeerAddress | None = None
    peer_id: PeerId | None = None
    characteristic: BleGattCharacteristicRef | None = None


@dataclass(frozen=True, slots=True)
class TcpServerClientChangedEvent:
    """A TCP Server listener accepted or lost one active client."""

    session_id: SessionId
    endpoint: Endpoint
    state: TcpServerClientState
    peer_id: PeerId
    peer: PeerAddress
    occurred_at: float
    error: ErrorInfo | None = None
    peers: tuple[TcpServerPeerSnapshot, ...] = ()


@dataclass(frozen=True, slots=True)
class StreamDataReceivedEvent:
    """Raw inbound stream bytes; parsing is a later application concern."""

    session_id: SessionId
    endpoint: Endpoint
    chunk: StreamChunk
    occurred_at: float
    peer: PeerAddress | None = None
    peer_id: PeerId | None = None
    timing: GapObservation = field(default_factory=GapObservation)


@dataclass(frozen=True, slots=True)
class StreamDataSentEvent:
    """Raw outbound stream bytes acknowledged by the transport adapter."""

    session_id: SessionId
    endpoint: Endpoint
    write: StreamWrite
    occurred_at: float
    peer: PeerAddress | None = None
    peer_id: PeerId | None = None


@dataclass(frozen=True, slots=True)
class BleGattServicesChangedEvent:
    """The discovered service/characteristic capability snapshot for one BLE session."""

    session_id: SessionId
    endpoint: Endpoint
    services: tuple[BleGattService, ...]
    mtu_size: int | None
    occurred_at: float


@dataclass(frozen=True, slots=True)
class BleGattNotificationEvent:
    """One notification/indication payload copied out of the backend callback."""

    session_id: SessionId
    endpoint: Endpoint
    characteristic: BleGattCharacteristicRef
    payload: bytes
    occurred_at: float


@dataclass(frozen=True, slots=True)
class BleGattReadCompletedEvent:
    """One explicit characteristic read completion."""

    session_id: SessionId
    endpoint: Endpoint
    characteristic: BleGattCharacteristicRef
    payload: bytes
    occurred_at: float


@dataclass(frozen=True, slots=True)
class BleGattWriteCompletedEvent:
    """One write accepted by the BLE backend, not an application-level ACK."""

    session_id: SessionId
    endpoint: Endpoint
    write: BleGattWrite
    occurred_at: float


@dataclass(frozen=True, slots=True)
class BleGattSubscriptionChangedEvent:
    """A notification/indication subscription state transition."""

    session_id: SessionId
    endpoint: Endpoint
    characteristic: BleGattCharacteristicRef
    enabled: bool
    occurred_at: float


@dataclass(frozen=True, slots=True)
class ProtocolFramesDecodedEvent:
    """A bounded batch of parsed RX frames for one source identity."""

    source: ProtocolSource
    frames: tuple[DecodedFrame, ...]
    stats: ProtocolStats
    occurred_at: float
    generation: int = 0


@dataclass(frozen=True, slots=True)
class ProtocolBackpressureEvent:
    """A parser input drop that must not affect raw recording."""

    source: ProtocolSource
    dropped_units: int
    dropped_bytes: int
    occurred_at: float
    generation: int = 0


@dataclass(frozen=True, slots=True)
class ComponentFramesDecodedEvent:
    """A bounded batch of component rows derived from protocol frames."""

    source: ProtocolSource
    profile: ComponentConfiguration
    rows: tuple[ComponentFrameRow, ...]
    stats: ComponentStats
    occurred_at: float
    generation: int = 0
    protocol_generation: int = 0


@dataclass(frozen=True, slots=True)
class ComponentBackpressureEvent:
    """A component decode drop that does not affect protocol/raw data."""

    source: ProtocolSource
    dropped_frames: int
    dropped_bytes: int
    occurred_at: float
    generation: int = 0


@dataclass(frozen=True, slots=True)
class DatasetBatchEvent:
    """A bounded batch of transformed samples derived from component rows."""

    source: ProtocolSource
    samples: tuple[DatasetSample, ...]
    stats: DatasetStats
    occurred_at: float
    config: DatasetConfig | None = None
    generation: int = 0
    component_generation: int = 0
    protocol_generation: int = 0


@dataclass(frozen=True, slots=True)
class DatasetBackpressureEvent:
    """A dataset transform queue drop that leaves raw/component data intact."""

    source: ProtocolSource
    dropped_frames: int
    dropped_bytes: int
    occurred_at: float
    generation: int = 0


@dataclass(frozen=True, slots=True)
class ReplayStateChangedEvent:
    """A historical replay lifecycle/counter snapshot for presentation."""

    snapshot: ReplaySnapshot
    occurred_at: float


@dataclass(frozen=True, slots=True)
class ReplayDataEvent:
    """One historical RX record copied to the raw preview without transport I/O."""

    replay_id: UUID
    record: ReplayRecord
    occurred_at: float


@dataclass(frozen=True, slots=True)
class DatagramDataReceivedEvent:
    """One complete inbound UDP datagram with its source peer."""

    session_id: SessionId
    endpoint: Endpoint
    datagram: Datagram
    occurred_at: float


@dataclass(frozen=True, slots=True)
class DatagramDataSentEvent:
    """One complete outbound UDP datagram acknowledged by the adapter."""

    session_id: SessionId
    endpoint: Endpoint
    write: DatagramSend
    occurred_at: float


@dataclass(frozen=True, slots=True)
class DatagramDroppedEvent:
    """A UDP datagram rejected without stopping the active session."""

    session_id: SessionId
    endpoint: Endpoint
    peer: PeerAddress
    reason: DatagramDropReason
    occurred_at: float


@dataclass(frozen=True, slots=True)
class EventBackpressureEvent:
    """Observable preview/control event loss from a bounded event queue."""

    dropped_stream_events: int
    dropped_control_events: int
    occurred_at: float


@dataclass(frozen=True, slots=True)
class RecordingStateChangedEvent:
    """A recorder lifecycle or counter snapshot for presentation."""

    snapshot: RecordingSnapshot
    occurred_at: float


@dataclass(frozen=True, slots=True)
class RecordingBackpressureEvent:
    """A coalesced notification that the raw-record queue rejected data."""

    dropped_records: int
    occurred_at: float


type SessionEvent = (
    SessionStateChangedEvent
    | SessionErrorEvent
    | TcpServerClientChangedEvent
    | StreamDataReceivedEvent
    | StreamDataSentEvent
    | BleGattServicesChangedEvent
    | BleGattNotificationEvent
    | BleGattReadCompletedEvent
    | BleGattWriteCompletedEvent
    | BleGattSubscriptionChangedEvent
    | ProtocolFramesDecodedEvent
    | ProtocolBackpressureEvent
    | ComponentFramesDecodedEvent
    | ComponentBackpressureEvent
    | DatasetBatchEvent
    | DatasetBackpressureEvent
    | ReplayStateChangedEvent
    | ReplayDataEvent
    | DatagramDataReceivedEvent
    | DatagramDataSentEvent
    | DatagramDroppedEvent
    | EventBackpressureEvent
    | RecordingStateChangedEvent
    | RecordingBackpressureEvent
)

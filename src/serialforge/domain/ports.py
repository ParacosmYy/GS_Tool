"""Explicit ports used to keep device I/O, orchestration, and UI replaceable."""

from __future__ import annotations

from collections.abc import Callable
from typing import Protocol
from uuid import UUID

from .codecs import ComponentConfiguration
from .commands import (
    CommandBatch,
    CommandBatchId,
    CommandBatchRequest,
    CommandBatchSnapshot,
)
from .components import ComponentFrameRow, ComponentStats
from .datasets import DatasetConfig, DatasetSample, DatasetStats
from .events import ComponentFramesDecodedEvent, SessionEvent
from .models import (
    BleGattCharacteristicRef,
    BleGattCommand,
    BleGattDevice,
    BleGattDiscoveryConfig,
    BleGattService,
    BleGattTransportConfig,
    BleGattWrite,
    CommandEntry,
    DatagramReadResult,
    DatagramSend,
    Endpoint,
    PeerAddress,
    RawRecord,
    RecordingSnapshot,
    RttDownWrite,
    ServerReadResult,
    SessionId,
    SessionSnapshot,
    StreamReadResult,
    StreamWrite,
    TcpServerPeerSnapshot,
    TcpServerSend,
    TcpServerTransportConfig,
    TransportConfig,
    TransportKind,
)
from .protocols import (
    DecodedFrame,
    ProtocolConfig,
    ProtocolIngressUnit,
    ProtocolSource,
    ProtocolStats,
)
from .replay import ReplayOptions, ReplaySnapshot


class StreamTransportPort(Protocol):
    """A blocking byte-stream API used by UART and TCP."""

    @property
    def kind(self) -> TransportKind:
        """Return the transport family while preserving its wire semantics."""

    @property
    def endpoint(self) -> Endpoint:
        """Return the endpoint represented by this adapter."""

    @property
    def peer(self) -> PeerAddress | None:
        """Return the configured stream peer, or ``None`` for UART."""

    def open(self) -> None:
        """Open the underlying handle; bounded I/O begins after this call."""

    def receive(self, max_bytes: int) -> StreamReadResult:
        """Read one typed stream result without collapsing timeout and EOF."""

    def send(self, write: StreamWrite) -> None:
        """Send one stream operation while preserving stream semantics."""

    def close(self) -> None:
        """Close the handle idempotently and release adapter resources."""


class DatagramTransportPort(Protocol):
    """A blocking datagram API that preserves peer and packet boundaries."""

    @property
    def kind(self) -> TransportKind:
        """Return the datagram transport family."""

    @property
    def endpoint(self) -> Endpoint:
        """Return the configured local/remote endpoint identity."""

    def open(self) -> None:
        """Bind the local socket and prepare bounded I/O."""

    def receive(self, max_bytes: int) -> DatagramReadResult:
        """Read one complete datagram or one observable timeout/drop result."""

    def send(self, write: DatagramSend) -> None:
        """Send one complete datagram to its explicit peer."""

    def close(self) -> None:
        """Close the datagram socket idempotently."""


class TcpServerTransportPort(Protocol):
    """A listener plus bounded accepted clients, separate from client streams."""

    @property
    def kind(self) -> TransportKind:
        """Return the TCP Server transport family."""

    @property
    def endpoint(self) -> Endpoint:
        """Return the listener endpoint identity."""

    @property
    def peer(self) -> PeerAddress | None:
        """Return the only peer when exactly one is connected, else ``None``."""

    @property
    def peers(self) -> tuple[TcpServerPeerSnapshot, ...]:
        """Return the stable sorted list of connected peers."""

    def open(self) -> None:
        """Bind and start listening without accepting a client yet."""

    def receive(self, max_bytes: int) -> ServerReadResult:
        """Accept, reject, receive, or observe EOF for one bounded operation."""

    def send(self, write: TcpServerSend) -> bool:
        """Start one explicitly addressed write; False means its peer is busy."""

    def wake(self) -> None:
        """Wake the server worker after a command or cancellation request."""

    def close(self) -> None:
        """Close listener, client, and cancellation wake resources idempotently."""


type BleGattNotificationHandler = Callable[[BleGattCharacteristicRef, bytes], None]
type BleGattDisconnectHandler = Callable[[], None]


class BleGattTransportPort(Protocol):
    """Async BLE GATT operations kept behind a dedicated worker event loop."""

    @property
    def endpoint(self) -> Endpoint:
        """Return the stable BLE device endpoint."""

    @property
    def mtu_size(self) -> int | None:
        """Return the negotiated ATT MTU when the backend exposes it."""

    def set_handlers(
        self,
        notification: BleGattNotificationHandler,
        disconnected: BleGattDisconnectHandler,
    ) -> None:
        """Register UI-safe callbacks before opening the client."""

    async def open(self) -> tuple[BleGattService, ...]:
        """Connect and discover services on the BLE worker loop."""

    async def read(self, characteristic: BleGattCharacteristicRef) -> bytes:
        """Read one discovered characteristic."""

    async def write(self, command: BleGattWrite) -> None:
        """Execute one typed GATT write."""

    async def set_notify(self, characteristic: BleGattCharacteristicRef, enabled: bool) -> None:
        """Start or stop notification/indication delivery."""

    async def close(self) -> None:
        """Stop notifications, disconnect, and release backend handles."""


class BleGattTransportFactoryPort(Protocol):
    """Create a lazy BLE adapter without importing Bleak on the caller's path."""

    def create(self, config: BleGattTransportConfig) -> BleGattTransportPort:
        """Create a BLE adapter; actual Windows I/O begins on the worker."""


class BleGattDiscoveryPort(Protocol):
    """Discover BLE advertisements without exposing backend objects."""

    def discover(self, config: BleGattDiscoveryConfig) -> tuple[BleGattDevice, ...]:
        """Run one bounded manual scan and return immutable device snapshots."""


class ProtocolPipelinePort(Protocol):
    """Decode bounded raw bytes without owning a transport or widget."""

    @property
    def config(self) -> ProtocolConfig:
        """Return the active parser configuration."""

    @property
    def stats(self) -> ProtocolStats:
        """Return bounded parser counters."""

    @property
    def generation(self) -> int:
        """Return the current parser configuration generation."""

    def configure(self, config: ProtocolConfig) -> None:
        """Replace the parser configuration and clear partial state."""

    def offer(self, unit: ProtocolIngressUnit) -> bool:
        """Offer one raw ingress unit without blocking its producer.

        The worker observes ``unit.segment_id`` in FIFO order and owns any
        parser-state transition required at a segment boundary.
        """

    def finish(self, source: ProtocolSource | None = None) -> None:
        """Flush one source or all partial states and publish frame events."""

    def reset(self, source: ProtocolSource | None = None) -> None:
        """Clear partial state and counters."""

    def shutdown(self, timeout: float | None = 3.0) -> None:
        """Stop parser worker within a bounded shutdown budget."""


class ComponentCodecPort(Protocol):
    """Decode one protocol frame using a declarative component profile."""

    def decode(
        self,
        frame: DecodedFrame,
        profile: ComponentConfiguration,
        source: ProtocolSource,
        occurred_at: float,
    ) -> ComponentFrameRow:
        """Return one bounded component row without transport or Qt access."""


class ComponentPipelinePort(Protocol):
    """Run bounded field decoding away from the Qt thread."""

    @property
    def profile(self) -> ComponentConfiguration:
        """Return the active declarative component profile or codec config."""

    @property
    def stats(self) -> ComponentStats:
        """Return bounded component counters."""

    @property
    def generation(self) -> int:
        """Return the current component profile generation."""

    def configure(
        self,
        profile: ComponentConfiguration,
        *,
        protocol_generation: int | None = None,
    ) -> None:
        """Replace the profile and fence older protocol generations."""

    def offer(self, event: SessionEvent) -> bool:
        """Offer one protocol event without blocking the producer."""

    def reset(self, *, protocol_generation: int | None = None) -> None:
        """Clear pending rows/counters and fence older protocol generations."""

    def shutdown(self, timeout: float | None = 3.0) -> None:
        """Stop component worker within a bounded shutdown budget."""


class ComponentProfileStorePort(Protocol):
    """Load a declarative profile without exposing filesystem details to Qt."""

    def load(self, path: str) -> ComponentConfiguration:
        """Load and validate one bounded profile file."""


class DatasetPipelinePort(Protocol):
    """Apply declarative transforms and retain a bounded sample window."""

    @property
    def config(self) -> DatasetConfig:
        """Return the active independent dataset configuration."""

    @property
    def stats(self) -> DatasetStats:
        """Return bounded dataset counters."""

    @property
    def generation(self) -> int:
        """Return the current dataset configuration generation."""

    @property
    def samples(self) -> tuple[DatasetSample, ...]:
        """Return the retained immutable sample window."""

    def configure(
        self,
        config: DatasetConfig,
        *,
        component_generation: int | None = None,
        protocol_generation: int | None = None,
    ) -> None:
        """Replace dataset config and fence older component/protocol generations."""

    def offer(self, event: ComponentFramesDecodedEvent) -> bool:
        """Offer decoded component rows without blocking the producer."""

    def reset(
        self,
        *,
        component_generation: int | None = None,
        protocol_generation: int | None = None,
    ) -> None:
        """Clear samples and fence older component/protocol generations."""

    def shutdown(self, timeout: float | None = 3.0) -> None:
        """Stop the dataset worker within a bounded shutdown budget."""


class DatasetConfigStorePort(Protocol):
    """Load a bounded declarative dataset file without exposing filesystem details."""

    def load(self, path: str) -> DatasetConfig:
        """Load and validate one dataset configuration file."""


class ReplayPipelinePort(Protocol):
    """Replay bounded historical RX JSONL through the existing protocol pipeline."""

    def start(self, path: str, options: ReplayOptions | None = None) -> UUID:
        """Start one historical run and return its opaque replay identity."""

    def pause(self) -> None:
        """Pause the replay clock without dropping the current record."""

    def resume(self) -> None:
        """Resume a paused replay."""

    def stop(self) -> None:
        """Stop replay and close its file without touching a live transport."""

    def snapshot(self) -> ReplaySnapshot:
        """Return the latest immutable replay state and counters."""

    def shutdown(self, timeout: float | None = 3.0) -> None:
        """Stop the replay worker within a bounded shutdown budget."""


type TransportPort = StreamTransportPort | DatagramTransportPort
type TransportWrite = StreamWrite | RttDownWrite | DatagramSend | TcpServerSend | BleGattCommand


class TransportFactoryPort(Protocol):
    """Create an adapter without opening device I/O on the caller's thread."""

    def create(self, config: TransportConfig) -> TransportPort:
        """Create a transport for a validated configuration."""


class TcpServerTransportFactoryPort(Protocol):
    """Create the dedicated TCP Server adapter without opening its socket."""

    def create(self, config: TcpServerTransportConfig) -> TcpServerTransportPort:
        """Create a listener adapter for a validated server configuration."""


class EndpointDiscoveryPort(Protocol):
    """Enumerate immutable endpoints without exposing library handles."""

    def discover(self) -> tuple[Endpoint, ...]:
        """Return the currently visible endpoints."""


class SessionPort(Protocol):
    """Non-blocking session commands consumed by presentation code."""

    def open(self, config: TransportConfig) -> SessionId:
        """Schedule opening and return immediately with a session identity."""

    def close(self, session_id: SessionId) -> None:
        """Request cancellation; cleanup and the CLOSED event happen in the worker."""

    def send(self, session_id: SessionId, write: TransportWrite) -> None:
        """Enqueue one outbound operation without waiting for device I/O."""

    def shutdown(self, timeout: float | None = 3.0) -> None:
        """Request worker cancellation and wait only up to the supplied bound."""

    def snapshot(self) -> SessionSnapshot | None:
        """Return the latest immutable snapshot, if a session has existed."""


class EventSinkPort(Protocol):
    """Non-blocking event publication from workers."""

    def publish(self, event: SessionEvent) -> bool:
        """Publish without waiting; return false when a bounded queue rejects it."""


class EventSourcePort(Protocol):
    """Bounded, batch-oriented event consumption for presentation."""

    def drain(self, max_events: int = 64) -> tuple[SessionEvent, ...]:
        """Return at most ``max_events`` events without blocking."""


class EventPort(EventSinkPort, EventSourcePort, Protocol):
    """The in-process event port used by the composition root."""


class RawRecorderPort(Protocol):
    """A non-blocking bounded sink for complete raw stream records."""

    def start(self, path: str) -> None:
        """Start a bounded append-only recording at ``path``."""

    def stop(self) -> None:
        """Request recording shutdown without blocking the caller."""

    def record(self, record: RawRecord) -> bool:
        """Enqueue one record without waiting; return false if inactive/full."""

    def snapshot(self) -> RecordingSnapshot:
        """Return an immutable bounded recorder snapshot."""

    def shutdown(self, timeout: float | None = 3.0) -> None:
        """Stop the writer within a bounded application-shutdown budget."""


class CommandHistoryPort(Protocol):
    """Bounded in-memory history and quick-command storage."""

    def add_history(self, entry: CommandEntry) -> None:
        """Add a sent command, keeping only the configured recent entries."""

    def history(self) -> tuple[CommandEntry, ...]:
        """Return recent history, newest first."""

    def add_quick_command(self, entry: CommandEntry) -> None:
        """Add or replace a bounded quick command."""

    def quick_commands(self) -> tuple[CommandEntry, ...]:
        """Return quick commands in stable display order."""

    def remove_quick_command(self, index: int) -> None:
        """Remove one quick command by bounded list index."""

    def clear_history(self) -> None:
        """Remove all send history."""


class CommandBatchPort(Protocol):
    """Catalog and execute finite command batches without device knowledge."""

    def add_batch(self, batch: CommandBatch) -> None:
        """Add or replace one in-memory batch definition."""

    def batches(self) -> tuple[CommandBatch, ...]:
        """Return bounded batches in stable display order."""

    def get_batch(self, batch_id: CommandBatchId) -> CommandBatch | None:
        """Return one batch by immutable ID."""

    def remove_batch(self, batch_id: CommandBatchId) -> None:
        """Remove one batch when no run is active."""

    def start_batch(self, request: CommandBatchRequest) -> CommandBatchId:
        """Start one prevalidated finite run and return its batch ID."""

    def cancel_batch(self, batch_id: CommandBatchId | None = None) -> None:
        """Request cancellation; accepted writes cannot be recalled."""

    def snapshot(self) -> CommandBatchSnapshot:
        """Return the latest immutable run snapshot."""

    def shutdown(self, timeout: float | None = 3.0) -> None:
        """Cancel and join the bounded batch worker."""


class EndpointIdentityPort(Protocol):
    """Remember stable endpoint identities without owning reconnect policy."""

    def remember(self, endpoint: Endpoint) -> None:
        """Remember a discovered endpoint by its stable identity."""

    def resolve(self, identity: str) -> Endpoint | None:
        """Resolve a previously seen stable identity, if still known."""

    def recent(self) -> tuple[Endpoint, ...]:
        """Return bounded identities, most recently seen first."""

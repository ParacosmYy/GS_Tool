"""Qt-facing state and command adapter for the application ports."""

from __future__ import annotations

from pathlib import Path
from threading import Event

from ..domain.codecs import (
    ComponentCodecConfig,
    ComponentConfiguration,
)
from ..domain.commands import (
    CommandBatch,
    CommandBatchId,
    CommandBatchSnapshot,
    CommandBatchState,
)
from ..domain.components import (
    ComponentFrameRow,
    ComponentStats,
)
from ..domain.datasets import DatasetConfig, DatasetSample, DatasetStats
from ..domain.errors import (
    ErrorCode,
    ErrorInfo,
)
from ..domain.events import (
    ComponentFramesDecodedEvent,
    DatasetBatchEvent,
    ProtocolFramesDecodedEvent,
)
from ..domain.models import (
    MAX_PREVIEW_BYTES,
    BleGattCharacteristicRef,
    BleGattDevice,
    BleGattDiscoveryConfig,
    BleGattService,
    BleGattTransportConfig,
    BleGattWriteMode,
    CommandEntry,
    Endpoint,
    PeerId,
    RecordingSnapshot,
    RttTransportConfig,
    SessionId,
    SessionState,
    TcpServerPeerSnapshot,
    TcpServerTransportConfig,
    TcpTransportConfig,
    TransportConfig,
    TransportKind,
    UartTransportConfig,
    UdpTransportConfig,
)
from ..domain.ports import (
    BleGattDiscoveryPort,
    CommandBatchPort,
    CommandHistoryPort,
    ComponentPipelinePort,
    ComponentProfileStorePort,
    DatasetConfigStorePort,
    DatasetPipelinePort,
    EndpointDiscoveryPort,
    EndpointIdentityPort,
    EventSourcePort,
    ProtocolPipelinePort,
    RawRecorderPort,
    ReplayPipelinePort,
    SessionPort,
)
from ..domain.protocols import (
    DataOrigin,
    ProtocolConfig,
    ProtocolStats,
)
from ..domain.replay import ReplaySnapshot, ReplayState
from .qt import QObject, QThreadPool, QTimer, Signal, Slot
from .viewmodel_events import (
    _drain_events,
    _on_ble_scan_completed,
    _on_ble_scan_failed,
    _on_discovery_completed,
    _on_discovery_failed,
    _refresh_hotplug,
)
from .viewmodel_jobs import _BleScanJob, _DiscoveryJob
from .viewmodel_operations import (
    _connect,
)
from .viewmodel_operations import (
    add_quick_command as _add_quick_command,
)
from .viewmodel_operations import (
    clear_error as _clear_error,
)
from .viewmodel_operations import (
    clear_history as _clear_history,
)
from .viewmodel_operations import (
    clear_preview as _clear_preview,
)
from .viewmodel_operations import (
    connect_ble as _connect_ble,
)
from .viewmodel_operations import (
    connect_rtt as _connect_rtt,
)
from .viewmodel_operations import (
    connect_server as _connect_server,
)
from .viewmodel_operations import (
    connect_tcp as _connect_tcp,
)
from .viewmodel_operations import (
    connect_uart as _connect_uart,
)
from .viewmodel_operations import (
    connect_udp as _connect_udp,
)
from .viewmodel_operations import (
    disconnect as _disconnect,
)
from .viewmodel_operations import (
    export_component_csv as _export_component_csv,
)
from .viewmodel_operations import (
    pause_replay as _pause_replay,
)
from .viewmodel_operations import (
    read_ble_characteristic as _read_ble_characteristic,
)
from .viewmodel_operations import (
    remove_command_batch as _remove_command_batch,
)
from .viewmodel_operations import (
    remove_quick_command as _remove_quick_command,
)
from .viewmodel_operations import (
    request_shutdown as _request_shutdown,
)
from .viewmodel_operations import (
    resume_replay as _resume_replay,
)
from .viewmodel_operations import (
    save_command_batch as _save_command_batch,
)
from .viewmodel_operations import (
    send_entry as _send_entry,
)
from .viewmodel_operations import (
    set_ble_notifications as _set_ble_notifications,
)
from .viewmodel_operations import (
    set_preview_paused as _set_preview_paused,
)
from .viewmodel_operations import (
    show_local_error as _show_local_error,
)
from .viewmodel_operations import (
    start_command_batch as _start_command_batch,
)
from .viewmodel_operations import (
    start_recording as _start_recording,
)
from .viewmodel_operations import (
    start_replay as _start_replay,
)
from .viewmodel_operations import (
    stop_command_batch as _stop_command_batch,
)
from .viewmodel_operations import (
    stop_recording as _stop_recording,
)
from .viewmodel_operations import (
    stop_replay as _stop_replay,
)
from .viewmodel_pipeline import (
    _reset_dataset,
)
from .viewmodel_pipeline import (
    configure_component_codec as _configure_component_codec,
)
from .viewmodel_pipeline import (
    configure_component_profile as _configure_component_profile,
)
from .viewmodel_pipeline import (
    configure_dataset as _configure_dataset,
)
from .viewmodel_pipeline import (
    configure_protocol as _configure_protocol,
)
from .viewmodel_pipeline import (
    export_dataset_csv as _export_dataset_csv,
)
from .viewmodel_pipeline import (
    load_component_codec as _load_component_codec,
)
from .viewmodel_pipeline import (
    load_component_profile as _load_component_profile,
)
from .viewmodel_pipeline import (
    load_dataset_config as _load_dataset_config,
)
from .viewmodel_pipeline import (
    reset_protocol as _reset_protocol,
)


class SessionViewModel(QObject):
    """Translate immutable application events into small Qt signals."""

    status_changed = Signal(str)
    error_changed = Signal(object)
    discovery_busy_changed = Signal(bool)
    ble_scan_busy_changed = Signal(bool)
    data_batch_received = Signal(bytes)
    data_activity_changed = Signal(int)
    preview_snapshot_changed = Signal(bytes)
    paused_bytes_changed = Signal(int)
    endpoints_changed = Signal(object)
    session_state_changed = Signal(object)
    recording_changed = Signal(object)
    history_changed = Signal(object)
    quick_commands_changed = Signal(object)
    send_ready_changed = Signal(bool)
    server_peers_changed = Signal(object)
    ble_devices_changed = Signal(object)
    ble_services_changed = Signal(object)
    ble_subscription_changed = Signal(object)
    protocol_config_changed = Signal(object)
    protocol_frames_changed = Signal(object)
    protocol_stats_changed = Signal(object)
    component_profile_changed = Signal(object)
    component_rows_changed = Signal(object)
    component_stats_changed = Signal(object)
    dataset_config_changed = Signal(object)
    dataset_samples_changed = Signal(object)
    dataset_stats_changed = Signal(object)
    replay_changed = Signal(object)
    command_batches_changed = Signal(object)
    command_batch_changed = Signal(object)

    def __init__(
        self,
        *,
        session: SessionPort,
        events: EventSourcePort,
        discovery: EndpointDiscoveryPort,
        recorder: RawRecorderPort,
        history: CommandHistoryPort,
        identities: EndpointIdentityPort,
        ble_discovery: BleGattDiscoveryPort,
        protocol: ProtocolPipelinePort,
        components: ComponentPipelinePort,
        component_profiles: ComponentProfileStorePort,
        dataset: DatasetPipelinePort,
        dataset_configs: DatasetConfigStorePort,
        replay: ReplayPipelinePort,
        commands: CommandBatchPort,
        default_record_path: Path,
        parent: QObject | None = None,
    ) -> None:
        super().__init__(parent)
        self._session = session
        self._events = events
        self._discovery = discovery
        self._recorder = recorder
        self._history = history
        self._identities = identities
        self._ble_discovery = ble_discovery
        self._protocol = protocol
        self._components = components
        self._component_profiles = component_profiles
        self._dataset = dataset
        self._dataset_configs = dataset_configs
        self._replay = replay
        self._commands = commands
        self._default_record_path = default_record_path
        self._protocol_generation = protocol.generation
        self._component_generation = components.generation
        self._dataset_generation = dataset.generation
        self._status = "就绪 · 未连接"
        self._error_info: ErrorInfo | None = None
        self._state = SessionState.CLOSED
        self._session_id: SessionId | None = None
        self._session_endpoint: Endpoint | None = None
        self._active_config: TransportConfig | None = None
        self._discovery_busy = False
        self._ble_scan_busy = False
        self._closing = False
        self._known_endpoints: dict[str, Endpoint] = {}
        self._preview_paused = False
        self._paused_bytes = 0
        self._preview_buffer = bytearray()
        self._send_ready = False
        self._server_peers: tuple[TcpServerPeerSnapshot, ...] = ()
        self._recording = recorder.snapshot()
        self._thread_pool = QThreadPool.globalInstance()
        self._discovery_cancelled = Event()
        self._ble_scan_cancelled = Event()
        self._ble_devices: tuple[BleGattDevice, ...] = ()
        self._ble_services: tuple[BleGattService, ...] = ()
        self._ble_mtu_size: int | None = None
        self._component_rows: tuple[ComponentFrameRow, ...] = ()
        self._dataset_samples: tuple[DatasetSample, ...] = ()
        self._replay_snapshot = ReplaySnapshot()
        self._batch_snapshot = commands.snapshot()
        self._historical_replay_id = None
        self._historical_replay_accepting = False
        self._poller = QTimer(self)
        self._poller.setInterval(50)
        self._poller.timeout.connect(self._drain_events)
        self._poller.start()
        self._hotplug_watcher = QTimer(self)
        self._hotplug_watcher.setInterval(2_000)
        self._hotplug_watcher.timeout.connect(self._refresh_hotplug)
        self._hotplug_watcher.start()

    @property
    def status(self) -> str:
        """Return the latest status-bar text."""

        return self._status

    @property
    def error_message(self) -> str:
        """Return the persistent actionable error summary."""

        return self._error_info.message if self._error_info is not None else ""

    @property
    def error_info(self) -> ErrorInfo | None:
        """Return the current structured presentation error, if any."""

        return self._error_info

    @property
    def state(self) -> SessionState:
        """Return the last state observed by the view model."""

        return self._state

    @property
    def is_active(self) -> bool:
        """Return whether the single active session is opening or open."""

        return self._state in {
            SessionState.OPENING,
            SessionState.OPEN,
            SessionState.CLOSING,
        }

    @property
    def recording(self) -> RecordingSnapshot:
        """Return the latest recorder snapshot."""

        return self._recording

    @property
    def default_record_path(self) -> Path:
        """Return the application-owned default JSONL path."""

        return self._default_record_path

    @property
    def history_entries(self) -> tuple[CommandEntry, ...]:
        """Return recent sends for initial presentation hydration."""

        return self._history.history()

    @property
    def quick_command_entries(self) -> tuple[CommandEntry, ...]:
        """Return saved quick commands for initial presentation hydration."""

        return self._history.quick_commands()

    @property
    def command_batches(self) -> tuple[CommandBatch, ...]:
        """Return the bounded in-memory batch catalog for initial UI hydration."""

        return self._commands.batches()

    @property
    def command_batch_snapshot(self) -> CommandBatchSnapshot:
        """Return the latest enqueue-level batch progress snapshot."""

        return self._batch_snapshot

    @property
    def batch_active(self) -> bool:
        """Return whether one finite batch is currently submitting steps."""

        return self._batch_snapshot.state is CommandBatchState.RUNNING

    @property
    def preview_paused(self) -> bool:
        return self._preview_paused

    @property
    def preview_snapshot(self) -> bytes:
        """Return the current bounded raw preview window for presentation."""

        return bytes(self._preview_buffer)

    @property
    def can_send(self) -> bool:
        """Return whether the active transport currently has a writable peer."""

        return self._send_ready

    @property
    def server_peers(self) -> tuple[TcpServerPeerSnapshot, ...]:
        """Return the current TCP Server peer identities in stable order."""

        return self._server_peers

    @property
    def ble_devices(self) -> tuple[BleGattDevice, ...]:
        """Return the latest manual BLE scan snapshots."""

        return self._ble_devices

    @property
    def discovery_busy(self) -> bool:
        """Return whether a manual or hotplug UART discovery job is running."""

        return self._discovery_busy

    @property
    def ble_scan_busy(self) -> bool:
        """Return whether an explicit BLE discovery job is running."""

        return self._ble_scan_busy

    @property
    def ble_services(self) -> tuple[BleGattService, ...]:
        """Return the latest GATT service/characteristic snapshot."""

        return self._ble_services

    @property
    def ble_mtu_size(self) -> int | None:
        """Return the latest negotiated ATT MTU, if the backend exposed one."""

        return self._ble_mtu_size

    @property
    def protocol_config(self) -> ProtocolConfig:
        """Return the current framing/checksum configuration."""

        return self._protocol.config

    @property
    def protocol_stats(self) -> ProtocolStats:
        """Return the current bounded parser counters."""

        return self._protocol.stats

    @property
    def component_profile(self) -> ComponentConfiguration:
        """Return the active legacy profile or structured codec config."""

        return self._components.profile

    @property
    def component_rows(self) -> tuple[ComponentFrameRow, ...]:
        """Return the bounded component table snapshot."""

        return self._component_rows

    @property
    def component_stats(self) -> ComponentStats:
        """Return the current bounded component counters."""

        return self._components.stats

    @property
    def dataset_config(self) -> DatasetConfig:
        """Return the independent transform/dataset configuration."""

        return self._dataset.config

    @property
    def dataset_samples(self) -> tuple[DatasetSample, ...]:
        """Return the bounded transformed sample window for presentation."""

        return self._dataset_samples

    @property
    def dataset_stats(self) -> DatasetStats:
        """Return bounded transform/dataset counters."""

        return self._dataset.stats

    @property
    def replay_snapshot(self) -> ReplaySnapshot:
        """Return the current historical replay lifecycle and counters."""

        return self._replay_snapshot

    @property
    def replay_active(self) -> bool:
        return self._replay_snapshot.state in {ReplayState.PLAYING, ReplayState.PAUSED}

    def stop(self) -> None:
        """Stop Qt polling before the window and event loop are destroyed."""

        self._poller.stop()
        self._hotplug_watcher.stop()
        self._discovery_cancelled.set()
        self._ble_scan_cancelled.set()
        if self._discovery_busy:
            self._discovery_busy = False
            self.discovery_busy_changed.emit(False)
        if self._ble_scan_busy:
            self._ble_scan_busy = False
            self.ble_scan_busy_changed.emit(False)

    def refresh_ports(self, silent: bool = False) -> None:
        """Enumerate UART endpoints on the global Qt worker pool."""

        if self._discovery_busy or self._closing:
            return
        self._discovery_busy = True
        self.discovery_busy_changed.emit(True)
        if not silent:
            self._set_status("正在枚举 UART 端口…")
        job = _DiscoveryJob(self._discovery, self._discovery_cancelled)
        job.signals.completed.connect(self._on_discovery_completed)
        job.signals.failed.connect(self._on_discovery_failed)
        self._thread_pool.start(job)

    def scan_ble(self, config: BleGattDiscoveryConfig) -> None:
        """Run one explicit bounded BLE scan; never scan or reconnect automatically."""

        if self._ble_scan_busy or self._closing:
            return
        self._ble_scan_busy = True
        self.ble_scan_busy_changed.emit(True)
        self._set_status("正在扫描 BLE 设备…")
        job = _BleScanJob(self._ble_discovery, config, self._ble_scan_cancelled)
        job.signals.completed.connect(self._on_ble_scan_completed)
        job.signals.failed.connect(self._on_ble_scan_failed)
        self._thread_pool.start(job)

    def configure_protocol(self, config: ProtocolConfig) -> None:
        _configure_protocol(self, config)

    def reset_protocol(self) -> None:
        _reset_protocol(self)

    def _reset_dataset(self) -> None:
        _reset_dataset(self)

    def configure_component_profile(self, profile: ComponentConfiguration) -> bool:
        return _configure_component_profile(self, profile)

    def load_component_profile(self, path: str) -> None:
        _load_component_profile(self, path)

    def configure_component_codec(self, configuration: ComponentCodecConfig) -> bool:
        return _configure_component_codec(self, configuration)

    def load_component_codec(
        self,
        path: str,
        *,
        transport: TransportKind | None = None,
    ) -> None:
        _load_component_codec(self, path, transport=transport)

    def configure_dataset(self, config: DatasetConfig) -> bool:
        return _configure_dataset(self, config)

    def load_dataset_config(self, path: str) -> None:
        _load_dataset_config(self, path)

    def export_dataset_csv(self, path: str) -> None:
        _export_dataset_csv(self, path)

    def save_command_batch(self, batch: CommandBatch) -> None:
        _save_command_batch(self, batch)

    def remove_command_batch(self, batch_id: CommandBatchId) -> None:
        _remove_command_batch(self, batch_id)

    def start_command_batch(
        self,
        batch_id: CommandBatchId,
        *,
        target_peer_id: PeerId | None = None,
        ble_characteristic: BleGattCharacteristicRef | None = None,
        ble_write_mode: BleGattWriteMode | None = None,
    ) -> bool:
        return _start_command_batch(
            self,
            batch_id,
            target_peer_id=target_peer_id,
            ble_characteristic=ble_characteristic,
            ble_write_mode=ble_write_mode,
        )

    def stop_command_batch(self) -> None:
        _stop_command_batch(self)

    def start_replay(self, path: str, speed: float = 1.0) -> bool:
        return _start_replay(self, path, speed)

    def pause_replay(self) -> None:
        _pause_replay(self)

    def resume_replay(self) -> None:
        _resume_replay(self)

    def stop_replay(self) -> None:
        _stop_replay(self)

    def export_component_csv(self, path: str) -> None:
        _export_component_csv(self, path)

    def connect_uart(self, config: UartTransportConfig) -> None:
        _connect_uart(self, config)

    def connect_tcp(self, config: TcpTransportConfig) -> None:
        _connect_tcp(self, config)

    def connect_rtt(self, config: RttTransportConfig) -> None:
        _connect_rtt(self, config)

    def connect_udp(self, config: UdpTransportConfig) -> None:
        _connect_udp(self, config)

    def connect_server(self, config: TcpServerTransportConfig) -> None:
        _connect_server(self, config)

    def connect_ble(self, config: BleGattTransportConfig) -> None:
        _connect_ble(self, config)

    def read_ble_characteristic(self, characteristic: BleGattCharacteristicRef) -> None:
        _read_ble_characteristic(self, characteristic)

    def set_ble_notifications(
        self,
        characteristic: BleGattCharacteristicRef,
        enabled: bool,
    ) -> None:
        _set_ble_notifications(self, characteristic, enabled)

    def _connect(self, config: TransportConfig, label: str) -> None:
        _connect(self, config, label)

    def disconnect(self) -> None:
        _disconnect(self)

    def send_entry(
        self,
        entry: CommandEntry,
        *,
        target_peer_id: PeerId | None = None,
        ble_characteristic: BleGattCharacteristicRef | None = None,
        ble_write_mode: BleGattWriteMode | None = None,
    ) -> bool:
        return _send_entry(
            self,
            entry,
            target_peer_id=target_peer_id,
            ble_characteristic=ble_characteristic,
            ble_write_mode=ble_write_mode,
        )

    def start_recording(self, path: str) -> None:
        _start_recording(self, path)

    def stop_recording(self) -> None:
        _stop_recording(self)

    def add_quick_command(self, entry: CommandEntry) -> None:
        _add_quick_command(self, entry)

    def remove_quick_command(self, index: int) -> None:
        _remove_quick_command(self, index)

    def clear_history(self) -> None:
        _clear_history(self)

    def set_preview_paused(self, paused: bool) -> None:
        _set_preview_paused(self, paused)

    def clear_preview(self) -> None:
        _clear_preview(self)

    def clear_error(self) -> None:
        _clear_error(self)

    def show_local_error(self, message: str) -> None:
        _show_local_error(self, message)

    def request_shutdown(self) -> None:
        _request_shutdown(self)

    def _refresh_hotplug(self) -> None:
        _refresh_hotplug(self)

    @Slot(object)
    def _on_ble_scan_completed(self, devices: object) -> None:
        _on_ble_scan_completed(self, devices)

    @Slot(object)
    def _on_ble_scan_failed(self, info: object) -> None:
        _on_ble_scan_failed(self, info)

    @Slot(object)
    def _on_discovery_completed(self, endpoints: object) -> None:
        _on_discovery_completed(self, endpoints)

    @Slot(object)
    def _on_discovery_failed(self, info: object) -> None:
        _on_discovery_failed(self, info)

    @Slot()
    def _drain_events(self) -> None:
        _drain_events(self)

    def _accept_source(self, source: object) -> bool:
        if not hasattr(source, "session_id") or not hasattr(source, "origin"):
            return False
        if source.origin is DataOrigin.HISTORICAL:
            return (
                self._historical_replay_accepting
                and source.session_id == self._historical_replay_id
            )
        return source.session_id == self._session_id

    def _sync_pipeline_generations(self) -> None:
        """Capture worker generations after each local reconfiguration fence."""

        self._protocol_generation = self._protocol.generation
        self._component_generation = self._components.generation
        self._dataset_generation = self._dataset.generation

    def _accept_protocol_event(self, event: ProtocolFramesDecodedEvent) -> bool:
        return event.generation == self._protocol_generation

    def _accept_component_event(self, event: ComponentFramesDecodedEvent) -> bool:
        return (
            event.generation == self._component_generation
            and event.protocol_generation == self._protocol_generation
        )

    def _accept_dataset_event(self, event: DatasetBatchEvent) -> bool:
        return (
            event.generation == self._dataset_generation
            and event.component_generation == self._component_generation
            and event.protocol_generation == self._protocol_generation
        )

    def _refresh_batch_snapshot(self) -> None:
        snapshot = self._commands.snapshot()
        if snapshot == self._batch_snapshot:
            return
        self._batch_snapshot = snapshot
        self.command_batch_changed.emit(snapshot)
        if snapshot.accepted_steps > 0:
            self.history_changed.emit(self._history.history())
        if snapshot.error is not None:
            self._show_error(snapshot.error)
        elif snapshot.state is CommandBatchState.COMPLETED:
            self._set_status(snapshot.message)

    def _append_preview(self, payload: bytes) -> None:
        self._preview_buffer.extend(payload)
        overflow = len(self._preview_buffer) - MAX_PREVIEW_BYTES
        if overflow > 0:
            del self._preview_buffer[:overflow]

    def _append_ble_payload(
        self,
        payload: bytes,
        channel: str,
        batch: bytearray,
    ) -> None:
        self._append_preview(payload)
        if self._preview_paused:
            self._paused_bytes += len(payload)
            self.paused_bytes_changed.emit(self._paused_bytes)
            return
        remaining = 65_536 - len(batch)
        if remaining > 0:
            batch.extend(payload[:remaining])
        self._set_status(f"BLE 数据 · {channel} · {len(payload)} B")

    def _show_error(self, info: ErrorInfo) -> None:
        if not isinstance(info, ErrorInfo):
            return
        if info != self._error_info:
            self._error_info = info
            self.error_changed.emit(info)
        self._set_status(info.message)

    def _check_active_identity(self) -> None:
        endpoint = self._session_endpoint
        if self._state not in {SessionState.OPEN, SessionState.CLOSING} or endpoint is None:
            return
        if endpoint.transport != TransportKind.UART:
            return
        if isinstance(self._active_config, UartTransportConfig) and (
            self._active_config.endpoint_identity is None
        ):
            return
        current = self._known_endpoints.get(endpoint.identity)
        if current is None:
            self._show_error(
                ErrorInfo(
                    code=ErrorCode.ENDPOINT_MISSING,
                    message="当前 UART 设备未被系统发现；请检查连接后手动刷新并重新连接。",
                    recoverable=True,
                    detail=f"identity={endpoint.identity}",
                )
            )
        elif current.address != endpoint.address:
            self._show_error(
                ErrorInfo(
                    code=ErrorCode.ENDPOINT_CHANGED,
                    message=(
                        f"设备身份仍匹配，但端口已从 {endpoint.address} 变为 "
                        f"{current.address}；请先断开，再手动重新连接。"
                    ),
                    recoverable=True,
                    detail=f"identity={endpoint.identity}",
                )
            )
        elif self._error_info is not None and self._error_info.code in {
            ErrorCode.ENDPOINT_MISSING,
            ErrorCode.ENDPOINT_CHANGED,
        }:
            self._clear_error()

    def _clear_error(self) -> None:
        if self._error_info is None:
            return
        self._error_info = None
        self.error_changed.emit(None)

    def _set_status(self, status: str) -> None:
        if status == self._status:
            return
        self._status = status
        self.status_changed.emit(status)

    def _set_send_ready(self, ready: bool) -> None:
        if self._send_ready == ready:
            return
        self._send_ready = ready
        self.send_ready_changed.emit(ready)

    def _set_server_peers(self, peers: tuple[TcpServerPeerSnapshot, ...]) -> None:
        unique = {peer.peer_id: peer for peer in peers}
        normalized = tuple(
            sorted(
                unique.values(),
                key=lambda item: (item.address.host, item.address.port, str(item.peer_id)),
            )
        )
        if self._server_peers == normalized:
            return
        self._server_peers = normalized
        self.server_peers_changed.emit(normalized)

    def _set_ble_services(
        self,
        services: tuple[BleGattService, ...],
        mtu_size: int | None,
    ) -> None:
        normalized = tuple(service for service in services if isinstance(service, BleGattService))
        if self._ble_services == normalized and self._ble_mtu_size == mtu_size:
            return
        self._ble_services = normalized
        self._ble_mtu_size = mtu_size
        self.ble_services_changed.emit((normalized, mtu_size))

    @staticmethod
    def _state_text(state: SessionState, address: str, transport: TransportKind) -> str:
        labels = {
            SessionState.DISCOVERED: "已发现",
            SessionState.OPENING: "连接中",
            SessionState.OPEN: "已连接",
            SessionState.CLOSING: "断开中",
            SessionState.CLOSED: "已断开",
            SessionState.ERROR: "会话错误",
        }
        if transport == TransportKind.UDP_DATAGRAM:
            labels.update(
                {
                    SessionState.OPENING: "绑定中",
                    SessionState.OPEN: "已绑定",
                    SessionState.CLOSING: "停止中",
                    SessionState.CLOSED: "已停止",
                }
            )
        elif transport == TransportKind.TCP_SERVER:
            labels.update(
                {
                    SessionState.OPENING: "启动中",
                    SessionState.OPEN: "监听中",
                    SessionState.CLOSING: "停止中",
                    SessionState.CLOSED: "已停止",
                }
            )
        suffix = f" · {address}" if state in {SessionState.OPEN, SessionState.OPENING} else ""
        return labels[state] + suffix


def _csv_scalar(value: object) -> str:
    if value is None:
        return ""
    if isinstance(value, bool):
        return "true" if value else "false"
    return str(value)

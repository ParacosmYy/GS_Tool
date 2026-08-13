"""Event polling and discovery/BLE result projection for SessionViewModel."""

from __future__ import annotations

from ..domain.components import (
    MAX_COMPONENT_ROWS,
)
from ..domain.datasets import DatasetStats
from ..domain.errors import (
    ErrorCode,
    ErrorInfo,
)
from ..domain.events import (
    BleGattNotificationEvent,
    BleGattReadCompletedEvent,
    BleGattServicesChangedEvent,
    BleGattSubscriptionChangedEvent,
    BleGattWriteCompletedEvent,
    ComponentBackpressureEvent,
    ComponentFramesDecodedEvent,
    DatagramDataReceivedEvent,
    DatagramDroppedEvent,
    DatasetBackpressureEvent,
    DatasetBatchEvent,
    EventBackpressureEvent,
    ProtocolBackpressureEvent,
    ProtocolFramesDecodedEvent,
    RecordingBackpressureEvent,
    RecordingStateChangedEvent,
    ReplayDataEvent,
    ReplayStateChangedEvent,
    SessionErrorEvent,
    SessionStateChangedEvent,
    StreamDataReceivedEvent,
    TcpServerClientChangedEvent,
)
from ..domain.models import (
    BleGattDevice,
    Endpoint,
    SessionState,
    TcpServerClientState,
    TcpServerPeerSnapshot,
    TransportKind,
)
from ..domain.replay import ReplayState


def _refresh_hotplug(view_model) -> None:
    """Poll discovery for safe identity diagnostics, never auto-reconnect."""

    view_model.refresh_ports(silent=True)


def _on_ble_scan_completed(view_model, devices: object) -> None:
    """On ble scan completed."""
    view_model._ble_scan_busy = False
    view_model.ble_scan_busy_changed.emit(False)
    if view_model._closing:
        return
    normalized = tuple(device for device in devices if isinstance(device, BleGattDevice))
    if view_model._ble_devices != normalized:
        view_model._ble_devices = normalized
        view_model.ble_devices_changed.emit(normalized)
    view_model._set_status(f"已发现 {len(normalized)} 个 BLE 设备")


def _on_ble_scan_failed(view_model, info: object) -> None:
    """On ble scan failed."""
    view_model._ble_scan_busy = False
    view_model.ble_scan_busy_changed.emit(False)
    if view_model._closing:
        return
    if isinstance(info, ErrorInfo):
        view_model._show_error(info)
    else:
        view_model._show_error(
            ErrorInfo(
                code=ErrorCode.UNKNOWN,
                message=f"BLE 扫描失败 · {info}",
                recoverable=True,
            )
        )


def _on_discovery_completed(view_model, endpoints: object) -> None:
    """On discovery completed."""
    view_model._discovery_busy = False
    view_model.discovery_busy_changed.emit(False)
    if view_model._closing:
        return
    normalized = tuple(endpoint for endpoint in endpoints if isinstance(endpoint, Endpoint))
    view_model._known_endpoints = {
        endpoint.identity: endpoint for endpoint in normalized if endpoint.identity is not None
    }
    for endpoint in normalized:
        view_model._identities.remember(endpoint)
    view_model.endpoints_changed.emit(normalized)
    if not view_model.is_active:
        view_model._set_status(f"已发现 {len(normalized)} 个 UART 端口")
    view_model._check_active_identity()


def _on_discovery_failed(view_model, info: object) -> None:
    """On discovery failed."""
    view_model._discovery_busy = False
    view_model.discovery_busy_changed.emit(False)
    if view_model._closing:
        return
    if isinstance(info, ErrorInfo):
        view_model._show_error(info)
    else:
        view_model._show_error(
            ErrorInfo(
                code=ErrorCode.UNKNOWN,
                message=f"端口枚举失败 · {info}",
                recoverable=True,
            )
        )


def _drain_events(view_model) -> None:
    """Drain events."""
    batch = bytearray()
    activity_bytes = 0
    dataset_changed = False
    dataset_stats_update: DatasetStats | None = None
    for event in view_model._events.drain(max_events=64):
        if isinstance(event, SessionStateChangedEvent):
            if event.session_id != view_model._session_id:
                continue
            view_model._state = event.state
            view_model._session_endpoint = event.endpoint
            view_model.session_state_changed.emit(event.state)
            view_model._set_send_ready(
                event.state is SessionState.OPEN
                and event.endpoint.transport != TransportKind.TCP_SERVER
            )
            view_model._set_status(
                view_model._state_text(
                    event.state,
                    event.endpoint.address,
                    event.endpoint.transport,
                )
            )
            if event.error is not None:
                view_model._show_error(event.error)
            if event.state in {SessionState.CLOSED, SessionState.ERROR}:
                view_model._commands.cancel_batch()
                view_model._set_send_ready(False)
                view_model._set_server_peers(())
                view_model._session_id = None
                view_model._session_endpoint = None
                view_model._active_config = None
                view_model._set_ble_services((), None)
                view_model.reset_protocol()
        elif isinstance(event, TcpServerClientChangedEvent):
            if event.session_id != view_model._session_id:
                continue
            peers = event.peers or (
                (TcpServerPeerSnapshot(peer_id=event.peer_id, address=event.peer))
                if event.state is TcpServerClientState.CONNECTED
                else ()
            )
            view_model._set_server_peers(peers)
            if event.state is TcpServerClientState.CONNECTED:
                view_model._set_send_ready(bool(peers))
                view_model._set_status(
                    f"监听中 · 已接入 {event.peer.display} · {len(peers)} 个 client"
                )
            else:
                view_model._set_send_ready(bool(peers))
                view_model._set_status(
                    f"监听中 · {len(peers)} 个 client · 已断开 {event.peer.display}"
                )
        elif isinstance(event, SessionErrorEvent):
            if event.session_id == view_model._session_id:
                view_model._commands.cancel_batch()
                view_model._show_error(event.error)
        elif isinstance(event, BleGattServicesChangedEvent):
            if event.session_id != view_model._session_id:
                continue
            view_model._set_ble_services(event.services, event.mtu_size)
            view_model._set_status(
                f"BLE 已连接 · {len(event.services)} 个 service"
                + (f" · MTU {event.mtu_size}" if event.mtu_size else "")
            )
        elif isinstance(event, BleGattNotificationEvent):
            if event.session_id != view_model._session_id:
                continue
            activity_bytes += len(event.payload)
            view_model._append_ble_payload(
                event.payload,
                event.characteristic.key,
                batch,
            )
        elif isinstance(event, BleGattReadCompletedEvent):
            if event.session_id != view_model._session_id:
                continue
            activity_bytes += len(event.payload)
            view_model._append_ble_payload(
                event.payload,
                event.characteristic.key,
                batch,
            )
            view_model._set_status(
                f"BLE read 完成 · {event.characteristic.key} · {len(event.payload)} B"
            )
        elif isinstance(event, BleGattWriteCompletedEvent):
            if event.session_id != view_model._session_id:
                continue
            view_model._set_status(
                f"BLE write 完成 · {event.write.characteristic.key} · {len(event.write.payload)} B"
            )
        elif isinstance(event, BleGattSubscriptionChangedEvent):
            if event.session_id != view_model._session_id:
                continue
            view_model.ble_subscription_changed.emit(event)
            view_model._set_status(
                f"BLE 通知{'已开启' if event.enabled else '已关闭'} · {event.characteristic.key}"
            )
        elif isinstance(event, ReplayStateChangedEvent):
            if event.snapshot.replay_id != view_model._historical_replay_id:
                continue
            view_model._replay_snapshot = event.snapshot
            view_model.replay_changed.emit(event.snapshot)
            if event.snapshot.error is not None:
                view_model._show_error(event.snapshot.error)
            else:
                replay_labels = {
                    ReplayState.PLAYING: "历史回放 · 播放中",
                    ReplayState.PAUSED: "历史回放 · 已暂停",
                    ReplayState.EOF: "历史回放 · 已结束",
                    ReplayState.STOPPED: "历史回放 · 已停止",
                }
                label = replay_labels.get(event.snapshot.state)
                if label:
                    view_model._set_status(f"{label} · {event.snapshot.records_emitted} 条")
        elif isinstance(event, ReplayDataEvent):
            if (
                not view_model._historical_replay_accepting
                or event.replay_id != view_model._historical_replay_id
            ):
                continue
            payload = event.record.raw.payload
            activity_bytes += len(payload)
            view_model._append_preview(payload)
            if view_model._preview_paused:
                view_model._paused_bytes += len(payload)
                view_model.paused_bytes_changed.emit(view_model._paused_bytes)
            else:
                remaining = 65_536 - len(batch)
                if remaining > 0:
                    batch.extend(payload[:remaining])
        elif isinstance(event, ProtocolFramesDecodedEvent):
            if not view_model._accept_source(event.source) or not view_model._accept_protocol_event(
                event
            ):
                continue
            view_model.protocol_frames_changed.emit(event.frames)
            view_model.protocol_stats_changed.emit(event.stats)
        elif isinstance(event, ProtocolBackpressureEvent):
            if (
                view_model._accept_source(event.source)
                and event.generation == view_model._protocol_generation
            ):
                view_model._set_status(
                    f"组件解析队列已丢弃 · {event.dropped_bytes} B；原始记录不受影响"
                )
        elif isinstance(event, ComponentFramesDecodedEvent):
            if not view_model._accept_source(
                event.source
            ) or not view_model._accept_component_event(event):
                continue
            if event.profile != view_model._components.profile:
                continue
            view_model._component_rows = (*view_model._component_rows, *event.rows)[
                -MAX_COMPONENT_ROWS:
            ]
            view_model.component_rows_changed.emit(view_model._component_rows)
            view_model.component_stats_changed.emit(event.stats)
        elif isinstance(event, ComponentBackpressureEvent):
            if (
                view_model._accept_source(event.source)
                and event.generation == view_model._component_generation
            ):
                view_model._set_status(
                    f"组件字段解析队列已丢弃 · {event.dropped_bytes} B；raw/协议帧不受影响"
                )
        elif isinstance(event, DatasetBatchEvent):
            if not view_model._accept_source(event.source) or not view_model._accept_dataset_event(
                event
            ):
                continue
            if event.config is not None and event.config != view_model._dataset.config:
                continue
            capacity = view_model._dataset.config.capacity
            view_model._dataset_samples = (
                *view_model._dataset_samples,
                *event.samples,
            )[-capacity:]
            dataset_changed = True
            dataset_stats_update = event.stats
        elif isinstance(event, DatasetBackpressureEvent):
            if (
                view_model._accept_source(event.source)
                and event.generation == view_model._dataset_generation
            ):
                view_model._set_status(
                    f"dataset 队列已丢弃 · {event.dropped_bytes} B；raw/组件帧不受影响"
                )
        elif isinstance(event, EventBackpressureEvent):
            dropped = event.dropped_stream_events + event.dropped_control_events
            view_model._set_status(f"预览队列有界丢弃 · {dropped} 个事件")
        elif isinstance(event, RecordingStateChangedEvent):
            view_model._recording = event.snapshot
            view_model.recording_changed.emit(event.snapshot)
            if event.snapshot.error is not None:
                view_model._show_error(event.snapshot.error)
        elif isinstance(event, RecordingBackpressureEvent):
            view_model._set_status(f"原始记录队列已丢弃 · {event.dropped_records} 条")
        elif isinstance(event, StreamDataReceivedEvent):
            if event.session_id != view_model._session_id:
                continue
            activity_bytes += len(event.chunk.payload)
            view_model._append_preview(event.chunk.payload)
            if view_model._preview_paused:
                view_model._paused_bytes += len(event.chunk.payload)
                view_model.paused_bytes_changed.emit(view_model._paused_bytes)
            else:
                remaining = 65_536 - len(batch)
                if remaining > 0:
                    batch.extend(event.chunk.payload[:remaining])
        elif isinstance(event, DatagramDataReceivedEvent):
            if event.session_id != view_model._session_id:
                continue
            payload = event.datagram.payload
            activity_bytes += len(payload)
            view_model._append_preview(payload)
            if view_model._preview_paused:
                view_model._paused_bytes += len(payload)
                view_model.paused_bytes_changed.emit(view_model._paused_bytes)
            else:
                remaining = 65_536 - len(batch)
                if remaining > 0:
                    batch.extend(payload[:remaining])
        elif isinstance(event, DatagramDroppedEvent):
            if event.session_id == view_model._session_id:
                view_model._set_status(f"UDP 已丢弃 · {event.peer.display} · {event.reason.value}")

    if dataset_changed:
        view_model.dataset_samples_changed.emit(view_model._dataset_samples)
        view_model.dataset_stats_changed.emit(
            dataset_stats_update if dataset_stats_update is not None else view_model._dataset.stats
        )
    view_model._refresh_batch_snapshot()
    if batch and not view_model._preview_paused:
        view_model.data_batch_received.emit(bytes(batch))
    if activity_bytes:
        view_model.data_activity_changed.emit(activity_bytes)

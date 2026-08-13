"""Command, replay, transport, recording, and shutdown operations."""

from __future__ import annotations

import csv
import io
from pathlib import Path

from ..application.commands import build_transport_write
from ..domain.commands import (
    CommandBatch,
    CommandBatchId,
    CommandBatchRequest,
)
from ..domain.errors import (
    ErrorCode,
    ErrorInfo,
    SerialForgeError,
)
from ..domain.models import (
    BleGattCharacteristicRef,
    BleGattNotify,
    BleGattRead,
    BleGattTransportConfig,
    BleGattWriteMode,
    CommandEntry,
    PeerId,
    RttTransportConfig,
    SessionState,
    TcpServerTransportConfig,
    TcpTransportConfig,
    TransportConfig,
    UartTransportConfig,
    UdpTransportConfig,
)
from ..domain.replay import ReplayOptions


def save_command_batch(view_model, batch: CommandBatch) -> None:
    """Add or replace one declarative batch without touching an active session."""

    try:
        view_model._commands.add_batch(batch)
        view_model.command_batches_changed.emit(view_model._commands.batches())
        view_model._clear_error()
        view_model._set_status(f"已保存批量命令 · {batch.name} · {len(batch.steps)} 步")
    except SerialForgeError as exc:
        view_model._show_error(exc.info)


def remove_command_batch(view_model, batch_id: CommandBatchId) -> None:
    """Remove one in-memory batch definition."""

    try:
        view_model._commands.remove_batch(batch_id)
        view_model.command_batches_changed.emit(view_model._commands.batches())
        view_model._clear_error()
    except SerialForgeError as exc:
        view_model._show_error(exc.info)


def start_command_batch(
    view_model,
    batch_id: CommandBatchId,
    *,
    target_peer_id: PeerId | None = None,
    ble_characteristic: BleGattCharacteristicRef | None = None,
    ble_write_mode: BleGattWriteMode | None = None,
) -> bool:
    """Start one batch against a frozen active-session target."""

    if view_model.batch_active:
        view_model._show_error(
            ErrorInfo(
                code=ErrorCode.CONFIGURATION,
                message="已有批量命令正在执行，请先停止。",
                recoverable=True,
            )
        )
        return False
    if view_model.replay_active:
        view_model._show_error(
            ErrorInfo(
                code=ErrorCode.REPLAY,
                message="历史回放期间不能执行批量命令；回放仅支持 RX。",
                recoverable=True,
            )
        )
        return False
    if (
        view_model._session_id is None
        or view_model._active_config is None
        or not view_model._send_ready
    ):
        view_model._show_error(
            ErrorInfo(
                code=ErrorCode.SESSION_NOT_OPEN,
                message="批量命令执行失败 · 尚未连接可写传输。",
                recoverable=True,
            )
        )
        return False
    batch = view_model._commands.get_batch(batch_id)
    if batch is None:
        view_model._show_error(
            ErrorInfo(
                code=ErrorCode.CONFIGURATION,
                message="批量命令不存在或已被删除。",
                recoverable=True,
            )
        )
        return False
    if isinstance(view_model._active_config, RttTransportConfig):
        view_model._show_error(
            ErrorInfo(
                code=ErrorCode.CONFIGURATION,
                message="J-Link RTT 批量命令留到最后阶段，当前仅支持单条发送。",
                recoverable=True,
            )
        )
        return False
    request = CommandBatchRequest(
        session_id=view_model._session_id,
        config=view_model._active_config,
        batch=batch,
        target_peer_id=target_peer_id,
        ble_characteristic=ble_characteristic,
        ble_write_mode=ble_write_mode,
    )
    try:
        view_model._commands.start_batch(request)
        view_model._refresh_batch_snapshot()
        view_model._clear_error()
        view_model._set_status(f"批量命令已开始 · {batch.name} · {len(batch.steps)} 步")
        return True
    except SerialForgeError as exc:
        view_model._show_error(exc.info)
        view_model._refresh_batch_snapshot()
        return False


def stop_command_batch(view_model) -> None:
    """Request cancellation without closing the transport session."""

    view_model._commands.cancel_batch(view_model._batch_snapshot.batch_id)
    view_model._refresh_batch_snapshot()


def start_replay(view_model, path: str, speed: float = 1.0) -> bool:
    """Replay historical RX through the current framing/component/dataset config."""

    if view_model.is_active:
        view_model._show_error(
            ErrorInfo(
                code=ErrorCode.REPLAY,
                message="连接活动期间不能开始历史回放。",
                recoverable=True,
            )
        )
        return False
    if view_model.replay_active:
        view_model._show_error(
            ErrorInfo(
                code=ErrorCode.REPLAY_BUSY,
                message="已有历史回放正在运行。",
                recoverable=True,
            )
        )
        return False
    try:
        view_model._historical_replay_id = None
        view_model._historical_replay_accepting = False
        view_model.reset_protocol()
        replay_id = view_model._replay.start(path, ReplayOptions(speed=speed))
        view_model._historical_replay_id = replay_id
        view_model._historical_replay_accepting = True
        view_model._replay_snapshot = view_model._replay.snapshot()
        view_model.replay_changed.emit(view_model._replay_snapshot)
        view_model._clear_error()
        view_model._set_status("历史回放 · 不连接设备 · 使用当前协议/组件/Dataset 配置")
        return True
    except SerialForgeError as exc:
        view_model._show_error(exc.info)
        return False


def pause_replay(view_model) -> None:
    """Pause replay."""
    view_model._replay.pause()


def resume_replay(view_model) -> None:
    """Resume replay."""
    view_model._replay.resume()


def stop_replay(view_model) -> None:
    """Stop replay."""
    view_model._historical_replay_accepting = False
    view_model._replay.stop()


def export_component_csv(view_model, path: str) -> None:
    """Export the current bounded component rows as a flat CSV view."""

    if not path.strip() or len(path) > 4_096:
        view_model._show_error(
            ErrorInfo(
                code=ErrorCode.COMPONENT_PROFILE,
                message="组件 CSV 导出路径无效。",
                recoverable=True,
            )
        )
        return
    output = io.StringIO(newline="")
    writer = csv.writer(output, lineterminator="\n")
    writer.writerow(("time_monotonic", "sequence", "status", "source", "fields", "raw_hex"))
    for row in view_model._component_rows:
        fields = "; ".join(f"{field.name}={field.display}" for field in row.fields)
        writer.writerow(
            (
                f"{row.occurred_at:.6f}",
                row.sequence,
                row.status.value,
                row.source.display,
                fields,
                row.payload_hex,
            )
        )
    try:
        Path(path).write_text(output.getvalue(), encoding="utf-8", newline="\n")
    except OSError as exc:
        view_model._show_error(
            ErrorInfo(
                code=ErrorCode.COMPONENT_PROFILE,
                message="组件 CSV 导出失败。",
                recoverable=True,
                detail=str(exc),
            )
        )
        return
    view_model._set_status(f"组件 CSV 已导出 · {len(view_model._component_rows)} 行")


def connect_uart(view_model, config: UartTransportConfig) -> None:
    """Schedule a UART connection without performing device I/O on Qt."""

    view_model._connect(config, config.port)


def connect_tcp(view_model, config: TcpTransportConfig) -> None:
    """Schedule a TCP client connection without blocking the Qt thread."""

    view_model._connect(config, config.remote_peer.display)


def connect_rtt(view_model, config: RttTransportConfig) -> None:
    """Attach to an already-running J-Link RTT Telnet bridge."""

    view_model._connect(config, config.endpoint.label)


def connect_udp(view_model, config: UdpTransportConfig) -> None:
    """Schedule a UDP unicast bind without blocking the Qt thread."""

    view_model._connect(config, config.remote_peer.display)


def connect_server(view_model, config: TcpServerTransportConfig) -> None:
    """Schedule an IPv4 TCP listener; sending waits for its first client."""

    view_model._connect(config, config.endpoint.address)


def connect_ble(view_model, config: BleGattTransportConfig) -> None:
    """Schedule a single-device BLE GATT connection."""

    view_model._connect(config, config.endpoint.label)


def read_ble_characteristic(view_model, characteristic: BleGattCharacteristicRef) -> None:
    """Queue one explicit GATT read on the dedicated BLE worker."""

    if view_model._session_id is None:
        return
    try:
        view_model._session.send(view_model._session_id, BleGattRead(characteristic))
    except SerialForgeError as exc:
        view_model._show_error(exc.info)


def set_ble_notifications(
    view_model,
    characteristic: BleGattCharacteristicRef,
    enabled: bool,
) -> None:
    """Queue one explicit notification/indication subscription change."""

    if view_model._session_id is None:
        return
    try:
        view_model._session.send(
            view_model._session_id,
            BleGattNotify(characteristic=characteristic, enabled=enabled),
        )
    except SerialForgeError as exc:
        view_model._show_error(exc.info)


def _connect(view_model, config: TransportConfig, label: str) -> None:
    """Open one typed transport while keeping the terminal transport-neutral."""

    if view_model.is_active:
        view_model._set_status("当前会话仍在运行。")
        return
    if view_model.replay_active:
        view_model._show_error(
            ErrorInfo(
                code=ErrorCode.REPLAY,
                message="历史回放活动期间不能连接实时传输。",
                recoverable=True,
            )
        )
        return
    view_model._historical_replay_accepting = False
    view_model._historical_replay_id = None
    view_model._set_send_ready(False)
    view_model._set_server_peers(())
    view_model._set_ble_services((), None)
    view_model.reset_protocol()
    try:
        view_model._session_id = view_model._session.open(config)
        view_model._session_endpoint = config.endpoint
        view_model._active_config = config
        view_model._identities.remember(config.endpoint)
        view_model._clear_error()
        view_model._set_status(f"连接请求已发送 · {label}")
    except SerialForgeError as exc:
        view_model._show_error(exc.info)


def disconnect(view_model) -> None:
    """Request worker cancellation without waiting on device I/O."""

    view_model._commands.cancel_batch()
    if view_model._session_id is None:
        return
    try:
        view_model._session.close(view_model._session_id)
    except SerialForgeError as exc:
        view_model._show_error(exc.info)


def send_entry(
    view_model,
    entry: CommandEntry,
    *,
    target_peer_id: PeerId | None = None,
    ble_characteristic: BleGattCharacteristicRef | None = None,
    ble_write_mode: BleGattWriteMode | None = None,
) -> bool:
    """Send one command and add the UI-level entry to bounded history."""

    if view_model.batch_active:
        view_model._show_error(
            ErrorInfo(
                code=ErrorCode.CONFIGURATION,
                message="批量命令执行期间不能插入单条发送。",
                recoverable=True,
            )
        )
        return False
    if view_model._closing:
        view_model._show_error(
            ErrorInfo(
                code=ErrorCode.SESSION_NOT_OPEN,
                message="发送失败 · 应用正在关闭。",
                recoverable=True,
            )
        )
        return False
    if view_model.replay_active:
        view_model._show_error(
            ErrorInfo(
                code=ErrorCode.REPLAY,
                message="历史回放期间不能发送；回放仅支持 RX。",
                recoverable=True,
            )
        )
        return False
    if view_model._state is not SessionState.OPEN or not view_model._send_ready:
        view_model._show_error(
            ErrorInfo(
                code=ErrorCode.SESSION_NOT_OPEN,
                message="发送失败 · 尚未连接可写传输。",
                recoverable=True,
            )
        )
        return False
    if view_model._session_id is None:
        view_model._show_error(
            ErrorInfo(
                code=ErrorCode.SESSION_NOT_OPEN,
                message="发送失败 · 尚未连接传输。",
                recoverable=True,
            )
        )
        return False
    if view_model._active_config is None:
        view_model._show_error(
            ErrorInfo(
                code=ErrorCode.SESSION_NOT_OPEN,
                message="发送失败 · 当前传输配置不存在。",
                recoverable=True,
            )
        )
        return False
    payload = entry.payload + (b"\r\n" if entry.append_newline else b"")
    try:
        write = build_transport_write(
            view_model._active_config,
            payload,
            target_peer_id=target_peer_id,
            ble_characteristic=ble_characteristic,
            ble_write_mode=ble_write_mode,
        )
        view_model._session.send(view_model._session_id, write)
        view_model._history.add_history(entry)
        view_model.history_changed.emit(view_model._history.history())
        view_model._clear_error()
        return True
    except SerialForgeError as exc:
        view_model._show_error(exc.info)
        return False


def start_recording(view_model, path: str) -> None:
    """Start raw JSONL recording; file I/O stays in the recorder worker."""

    try:
        view_model._recorder.start(path)
        view_model._clear_error()
    except SerialForgeError as exc:
        view_model._show_error(exc.info)


def stop_recording(view_model) -> None:
    """Request recording flush without blocking the Qt thread."""

    view_model._recorder.stop()


def add_quick_command(view_model, entry: CommandEntry) -> None:
    """Store a bounded named command without executing scripts."""

    try:
        view_model._history.add_quick_command(entry)
        view_model.quick_commands_changed.emit(view_model._history.quick_commands())
    except SerialForgeError as exc:
        view_model._show_error(exc.info)


def remove_quick_command(view_model, index: int) -> None:
    """Remove quick command."""
    try:
        view_model._history.remove_quick_command(index)
        view_model.quick_commands_changed.emit(view_model._history.quick_commands())
    except SerialForgeError as exc:
        view_model._show_error(exc.info)


def clear_history(view_model) -> None:
    """Clear history."""
    view_model._history.clear_history()
    view_model.history_changed.emit(())


def set_preview_paused(view_model, paused: bool) -> None:
    """Freeze only the preview; worker and recorder continue receiving."""

    if view_model._preview_paused == paused:
        return
    view_model._preview_paused = paused
    if not paused:
        view_model._paused_bytes = 0
        view_model.paused_bytes_changed.emit(0)
        view_model.preview_snapshot_changed.emit(bytes(view_model._preview_buffer))
    view_model._set_status("预览已暂停" if paused else "预览已恢复")


def clear_preview(view_model) -> None:
    """Clear preview."""
    view_model._preview_buffer.clear()
    view_model._paused_bytes = 0
    view_model.paused_bytes_changed.emit(0)
    view_model.preview_snapshot_changed.emit(b"")


def clear_error(view_model) -> None:
    """Clear error."""
    view_model._clear_error()


def show_local_error(view_model, message: str) -> None:
    """Route a presentation validation error through the shared error state."""

    if not isinstance(message, str) or not message.strip():
        return
    view_model._show_error(
        ErrorInfo(
            code=ErrorCode.CONFIGURATION,
            message=message.strip(),
            recoverable=True,
        )
    )


def request_shutdown(view_model) -> None:
    """Request session/recorder shutdown without blocking the Qt thread."""

    view_model._closing = True
    view_model._set_send_ready(False)
    view_model.stop_command_batch()
    view_model.stop_replay()
    view_model.disconnect()
    view_model.stop()

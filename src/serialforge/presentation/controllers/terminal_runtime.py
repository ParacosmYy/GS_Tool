"""Terminal observation, recording, history, and endpoint view controller.

This controller owns raw-preview rendering and user-facing terminal state.
It delegates capture and transport operations through the view-model port.
"""

from __future__ import annotations

import time

from ...domain.commands import CommandEntry
from ...domain.errors import ConfigurationError
from ...domain.models import (
    BleGattCharacteristic,
    BleGattCharacteristicRef,
    BleGattDevice,
    BleGattService,
    CommandMode,
    Endpoint,
    RecordingState,
    SessionState,
    TcpServerPeerSnapshot,
)
from ..command_selection import current_send_mode
from ..connection_bindings import ble_bindings_for, network_bindings_for, uart_bindings_for
from ..data_activity_surface import DataActivityProjection
from ..file_dialog_surface import get_save_file_name
from ..formatters import endpoint_label, history_label
from ..property_refresh import refresh_dynamic_property
from ..protocol_scope import derived_source_supported
from ..qt import QInputDialog, Qt
from ..terminal_bindings import terminal_bindings_for
from ..workspace_bindings import workspace_bindings_for
from .lifecycle import set_data_activity_motion
from .workspace_runtime import leave_connection_onboarding


def toggle_recording(window) -> None:
    """Toggle recording."""
    if window._view_model.recording.state in {
        RecordingState.STARTING,
        RecordingState.ACTIVE,
        RecordingState.STOPPING,
    }:
        window._view_model.stop_recording()
        return
    path, _ = get_save_file_name(
        window,
        "选择原始记录文件",
        str(window._view_model.default_record_path),
        "JSON Lines (*.jsonl);;所有文件 (*.*)",
    )
    if path:
        window._view_model.start_recording(path)


def save_current_quick(window) -> None:
    """Save current quick."""
    try:
        entry = current_entry(window)
    except (ValueError, ConfigurationError) as exc:
        window._show_local_error(f"快捷命令格式无效 · {exc}")
        return
    name, accepted = QInputDialog.getText(window, "保存快捷命令", "名称")
    if accepted and name.strip():
        window._view_model.add_quick_command(
            CommandEntry(
                name=name,
                payload=entry.payload,
                mode=entry.mode,
                append_newline=entry.append_newline,
                created_at=entry.created_at,
            )
        )


def current_entry(window) -> CommandEntry:
    """Current entry."""
    terminal = terminal_bindings_for(window)
    if terminal is None:
        raise ConfigurationError("终端控件尚未初始化。")
    text = terminal.send_input.text()
    if not text:
        raise ConfigurationError("命令内容不能为空。")
    mode = current_send_mode(window)
    payload = (
        bytes.fromhex("".join(text.split())) if mode == CommandMode.HEX else text.encode("utf-8")
    )
    return CommandEntry(
        name=text[:128],
        payload=payload,
        mode=mode,
        append_newline=terminal.newline_check.isChecked(),
        created_at=time.monotonic(),
    )


def on_endpoints_changed(window, endpoints: object) -> None:
    """On endpoints changed."""
    if window._closing:
        return
    uart = uart_bindings_for(window)
    if uart is None:
        return
    window._update_connection_context()
    normalized = tuple(endpoint for endpoint in endpoints if isinstance(endpoint, Endpoint))
    previous = uart.port_combo.currentData(Qt.ItemDataRole.UserRole)
    previous_identity = previous.identity if isinstance(previous, Endpoint) else None
    manual_text = uart.port_combo.currentText()
    restore_text = manual_text
    if isinstance(previous, Endpoint) and manual_text == endpoint_label(previous):
        restore_text = previous.address
    window._endpoints = normalized
    uart.port_combo.blockSignals(True)
    uart.port_combo.clear()
    selected_index = -1
    for index, endpoint in enumerate(normalized):
        uart.port_combo.addItem(endpoint_label(endpoint), endpoint)
        if endpoint.identity == previous_identity:
            selected_index = index
    if selected_index >= 0:
        uart.port_combo.setCurrentIndex(selected_index)
    elif restore_text:
        uart.port_combo.setCurrentText(restore_text)
    uart.port_combo.blockSignals(False)
    window._refresh_connection_controls(window._view_model.state)
    window._update_connection_context()


def on_ble_devices_changed(window, devices: object) -> None:
    """On ble devices changed."""
    if window._closing:
        return
    ble = ble_bindings_for(window)
    if ble is None:
        return
    normalized = tuple(device for device in devices if isinstance(device, BleGattDevice))
    previous = ble.device_combo.currentData(Qt.ItemDataRole.UserRole)
    previous_id = previous.device_id if isinstance(previous, BleGattDevice) else None
    ble.device_combo.blockSignals(True)
    ble.device_combo.clear()
    selected_index = -1
    for index, device in enumerate(normalized):
        ble.device_combo.addItem(device.display, device)
        if device.device_id == previous_id:
            selected_index = index
    if selected_index >= 0:
        ble.device_combo.setCurrentIndex(selected_index)
    else:
        ble.device_combo.setCurrentIndex(-1)
    ble.device_combo.blockSignals(False)
    window._refresh_connection_controls(window._view_model.state)
    window._update_connection_context()


def on_ble_services_changed(window, payload: object) -> None:
    """On ble services changed."""
    if window._closing:
        return
    ble = ble_bindings_for(window)
    if ble is None:
        return
    services: tuple[BleGattService, ...] = ()
    mtu_size: int | None = None
    if isinstance(payload, tuple) and len(payload) == 2:
        candidate_services, candidate_mtu = payload
        services = tuple(
            service for service in candidate_services if isinstance(service, BleGattService)
        )
        mtu_size = candidate_mtu if isinstance(candidate_mtu, int) else None
    previous_notification_ref = window._ble_notification_ref
    if previous_notification_ref is not None and window._view_model.state is SessionState.OPEN:
        window._request_ble_notifications(previous_notification_ref, False)
    window._ble_notification_timer.stop()
    window._ble_notification_ref = None
    window._ble_notification_pending = None
    ble.characteristic_combo.blockSignals(True)
    previous = ble.characteristic_combo.currentData(Qt.ItemDataRole.UserRole)
    previous_key = previous.ref.key if isinstance(previous, BleGattCharacteristic) else None
    ble.characteristic_combo.clear()
    characteristics: list[BleGattCharacteristic] = []
    for service in services:
        for characteristic in service.characteristics:
            characteristics.append(characteristic)
            ble.characteristic_combo.addItem(
                f"{service.uuid} · {characteristic.display}",
                characteristic,
            )
    selected_index = next(
        (
            index
            for index, characteristic in enumerate(characteristics)
            if characteristic.ref.key == previous_key
        ),
        -1,
    )
    if selected_index >= 0:
        ble.characteristic_combo.setCurrentIndex(selected_index)
    else:
        ble.characteristic_combo.setCurrentIndex(-1)
    ble.characteristic_combo.blockSignals(False)
    if selected_index >= 0:
        window._on_ble_characteristic_changed(selected_index)
    else:
        description = f"未选择特征{f' · MTU {mtu_size}' if mtu_size else ''}"
        ble.characteristic_properties.setText(description)
        ble.characteristic_properties.setAccessibleDescription(description)
        ble.characteristic_properties.setToolTip(description)
        ble.notify_check.blockSignals(True)
        ble.notify_check.setChecked(False)
        ble.notify_check.blockSignals(False)
    window._refresh_connection_controls(window._view_model.state)


def on_ble_subscription_changed(window, event: object) -> None:
    """On ble subscription changed."""
    if window._closing:
        return
    ble = ble_bindings_for(window)
    if ble is None:
        return
    characteristic = getattr(event, "characteristic", None)
    if not isinstance(characteristic, BleGattCharacteristicRef):
        return
    enabled = bool(getattr(event, "enabled", False))
    pending = window._ble_notification_pending
    if pending is None:
        return
    if pending != (characteristic, enabled):
        window._refresh_connection_controls(window._view_model.state)
        return
    window._ble_notification_timer.stop()
    window._ble_notification_pending = None
    selected = ble.characteristic_combo.currentData(Qt.ItemDataRole.UserRole)
    if not isinstance(selected, BleGattCharacteristic) or selected.ref != characteristic:
        if not enabled and window._ble_notification_ref == characteristic:
            window._ble_notification_ref = None
        window._refresh_connection_controls(window._view_model.state)
        return
    window._ble_notification_ref = characteristic if enabled else None
    ble.notify_check.blockSignals(True)
    ble.notify_check.setChecked(enabled)
    ble.notify_check.blockSignals(False)
    window._refresh_connection_controls(window._view_model.state)


def on_server_peers_changed(window, peers: object) -> None:
    """On server peers changed."""
    if window._closing:
        return
    network = network_bindings_for(window)
    if network is None:
        return
    normalized = tuple(peer for peer in peers if isinstance(peer, TcpServerPeerSnapshot))
    previous = network.server_peer_combo.currentData(Qt.ItemDataRole.UserRole)
    previous_peer_id = previous.peer_id if isinstance(previous, TcpServerPeerSnapshot) else None
    had_explicit = window._server_target_explicit
    preserve_explicit = had_explicit and any(
        peer.peer_id == previous_peer_id for peer in normalized
    )
    network.server_peer_combo.blockSignals(True)
    network.server_peer_combo.clear()
    for peer in normalized:
        network.server_peer_combo.addItem(peer.display, peer)
    if preserve_explicit:
        network.server_peer_combo.setCurrentIndex(
            next(index for index, peer in enumerate(normalized) if peer.peer_id == previous_peer_id)
        )
    elif had_explicit:
        window._server_target_explicit = False
        network.server_peer_combo.setCurrentIndex(-1)
    elif len(normalized) == 1:
        window._server_target_explicit = False
        network.server_peer_combo.setCurrentIndex(0)
    else:
        window._server_target_explicit = False
        network.server_peer_combo.setCurrentIndex(-1)
    network.server_peer_combo.blockSignals(False)
    window._refresh_connection_controls(window._view_model.state)


def on_server_target_changed(window, _index: int = -1) -> None:
    """On server target changed."""
    network = network_bindings_for(window)
    if network is None:
        return
    window._server_target_explicit = network.server_peer_combo.currentIndex() >= 0
    window._refresh_connection_controls(window._view_model.state)


def schedule_preview_render(window) -> None:
    """Coalesce terminal and derived preview document rebuilds."""

    window._preview_render_pending = True
    if window._closing or window.isHidden() or window.isMinimized():
        window._preview_render_timer.stop()
        return
    if not window._preview_render_timer.isActive():
        window._preview_render_timer.start()


def flush_preview_renders(window) -> None:
    """Flush preview renders."""
    if not window._preview_render_pending:
        return
    if window._closing or window.isHidden() or window.isMinimized():
        window._preview_render_timer.stop()
        return
    window._preview_render_pending = False
    window._rerender_preview()
    workspace = workspace_bindings_for(window)
    if workspace is not None and workspace.tabs.currentIndex() == window._protocol_tab_index:
        window._rerender_component_preview()
        window._rerender_dataset_preview()


def on_data_received(window, payload: bytes) -> None:
    """On data received."""
    if window._closing:
        return
    if window._view_model.state is SessionState.OPEN:
        window._motion_controller.request_activity()
    # The ViewModel owns the single bounded window. Copying its snapshot
    # avoids rebuilding a different window from a possibly truncated batch.
    window._preview_buffer = bytearray(window._view_model.preview_snapshot)
    # Re-render from the bounded raw preview so the Qt document cannot
    # grow without limit when the device sends a long line without '\n'.
    window._schedule_preview_render()


def on_data_activity_changed(window, count: int) -> None:
    """On data activity changed."""
    if window._closing:
        return
    window._last_data_activity_bytes = max(0, int(count))
    if not window._view_model.preview_paused:
        window._preview_buffer = bytearray(window._view_model.preview_snapshot)
        window._schedule_preview_render()
    window._update_data_activity()


def on_preview_snapshot(window, payload: bytes) -> None:
    """On preview snapshot."""
    if window._closing:
        return
    window._preview_buffer = bytearray(payload)
    window._update_data_activity()
    window._schedule_preview_render()


def rerender_preview(window) -> None:
    """Rerender preview."""
    if window._closing:
        return
    terminal = terminal_bindings_for(window)
    if terminal is None:
        return
    terminal.terminal.setPlainText(render(window, bytes(window._preview_buffer)))
    scrollbar = terminal.terminal.verticalScrollBar()
    scrollbar.setValue(scrollbar.maximum())
    refresh_terminal_empty_state(window)


def refresh_terminal_empty_state(window) -> None:
    """Project existing source/session facts onto the terminal empty layer."""

    terminal = terminal_bindings_for(window)
    if terminal is None or window._closing:
        return
    empty_state = terminal.terminal_empty_state
    if window._preview_buffer:
        empty_state.setVisible(False)
        return

    if window._history_source_active or window._history_file_selected:
        state = "history"
        title = "历史回放等待数据"
        hint = "当前历史文件没有可显示的 RX；选择其他记录，或切回实时链路。"
    elif window._view_model.preview_paused:
        state = "paused"
        title = "终端显示已暂停"
        hint = "接收和原始记录仍会继续；恢复显示后，新的预览会出现在这里。"
    elif window._view_model.state in {SessionState.OPENING, SessionState.CLOSING}:
        state = "transition"
        title = "链路正在切换"
        hint = "连接流程完成后，接收数据会自动显示；当前不会改变发送或记录语义。"
    elif window._view_model.state is SessionState.OPEN:
        state = "waiting"
        title = "链路已就绪，等待 RX"
        hint = "已建立会话；等待设备发送第一批数据。暂停显示不会停止接收。"
    else:
        state = "idle"
        title = "等待链路数据"
        hint = "选择连接方式并点击“连接”；接收数据会出现在这里。"

    empty_state.set_context(state, title, hint)
    empty_state.setVisible(True)
    empty_state.raise_()


def reset_data_activity(window) -> None:
    """Reset data activity."""
    window._last_data_activity_bytes = 0
    set_data_activity_motion(window, False)
    if terminal_bindings_for(window) is not None:
        window._update_data_activity()


def update_data_activity(window) -> None:
    """Update data activity."""
    terminal = terminal_bindings_for(window)
    if terminal is None:
        return
    source = "历史" if window._history_source_active or window._history_file_selected else "实时"
    window_bytes = len(window._view_model.preview_snapshot)
    text = f"{source} · 最近 +{window._last_data_activity_bytes} B · 窗口 {window_bytes} B"
    terminal.data_activity_label.setText(text)
    terminal.data_activity_label.setAccessibleDescription(
        f"{text}。窗口受有界预览限制；暂停显示不会停止接收。"
    )
    set_projection = getattr(terminal.data_activity_label, "set_projection", None)
    if callable(set_projection):
        set_projection(
            DataActivityProjection(
                source=source,
                latest_bytes=window._last_data_activity_bytes,
                window_bytes=window_bytes,
            )
        )
    refresh_terminal_empty_state(window)


def render(window, payload: bytes) -> str:
    """Render."""
    terminal = terminal_bindings_for(window)
    if terminal is not None and terminal.display_mode.currentData() == CommandMode.HEX.value:
        return payload.hex(" ").upper()
    return payload.decode("utf-8", errors="replace")


def on_paused_bytes_changed(window, count: int) -> None:
    """On paused bytes changed."""
    if window._closing:
        return
    terminal = terminal_bindings_for(window)
    if terminal is None:
        return
    text = f"暂停 · {count} 字节未显示" if count else ""
    state = "paused" if count else "idle"
    refresh_dynamic_property(terminal.paused_label, "state", state)
    terminal.paused_label.setText(text)
    terminal.paused_label.setAccessibleDescription(text)
    window._refresh_live_observation_state()


def on_recording_changed(window, snapshot: object) -> None:
    """On recording changed."""
    if window._closing:
        return
    terminal = terminal_bindings_for(window)
    if terminal is None:
        return
    if not hasattr(snapshot, "state"):
        return
    state = snapshot.state
    recording_busy = state in {
        RecordingState.STARTING,
        RecordingState.ACTIVE,
        RecordingState.STOPPING,
    }
    terminal.record_button.set_busy(recording_busy)
    if state in {RecordingState.STARTING, RecordingState.ACTIVE, RecordingState.STOPPING}:
        terminal.record_button.setText("停止原始记录")
        text = f"REC · {snapshot.written_records} 条 · {snapshot.bytes_written} B"
        record_state = "stopping" if state is RecordingState.STOPPING else "active"
        terminal.record_label.setText(text)
        record_hint = "停止当前原始记录。"
    elif state is RecordingState.ERROR:
        terminal.record_button.setText("开始原始记录")
        text = "记录错误"
        record_state = "error"
        terminal.record_label.setText(text)
        record_hint = "当前原始记录发生错误；可重新开始记录。"
    else:
        terminal.record_button.setText("开始原始记录")
        text = "未记录"
        record_state = "idle"
        terminal.record_label.setText(text)
        record_hint = "选择文件并开始原始记录。"
    terminal.record_button.setAccessibleDescription(record_hint)
    terminal.record_button.setToolTip(record_hint)
    refresh_dynamic_property(terminal.record_label, "state", record_state)
    terminal.record_label.setAccessibleDescription(text)
    window._refresh_live_observation_state()


def on_history_changed(window, entries: object) -> None:
    """On history changed."""
    if window._closing:
        return
    terminal = terminal_bindings_for(window)
    if terminal is None:
        return
    window._history_entries = tuple(entry for entry in entries if isinstance(entry, CommandEntry))
    terminal.history_combo.blockSignals(True)
    terminal.history_combo.clear()
    if window._history_entries:
        terminal.history_combo.addItems(
            [history_label(entry) for entry in window._history_entries]
        )
    else:
        terminal.history_combo.addItem("暂无发送历史 · 发送一条命令后可复用", None)
    terminal.history_combo.blockSignals(False)
    refresh_terminal_empty_state(window)


def on_quick_commands_changed(window, entries: object) -> None:
    """On quick commands changed."""
    if window._closing:
        return
    terminal = terminal_bindings_for(window)
    if terminal is None:
        return
    window._quick_entries = tuple(entry for entry in entries if isinstance(entry, CommandEntry))
    terminal.quick_menu.clear()
    if not window._quick_entries:
        action = terminal.quick_menu.addAction("暂无快捷命令")
        action.setEnabled(False)
        action.setToolTip("暂无已保存快捷命令；在发送区保存后可从此处载入。")
        return
    for entry in window._quick_entries:
        action = terminal.quick_menu.addAction(entry.name)
        action.setToolTip(f"将快捷命令“{entry.name}”填入发送区；不会自动发送。")
        action.triggered.connect(lambda _checked=False, item=entry: window._load_entry(item))


def load_history(window, index: int) -> None:
    """Load history."""
    if 0 <= index < len(window._history_entries):
        window._load_entry(window._history_entries[index])


def load_entry(window, entry: CommandEntry) -> None:
    """Load entry."""
    terminal = terminal_bindings_for(window)
    if terminal is None:
        return
    index = terminal.send_mode.findData(entry.mode.value)
    if index < 0:
        index = terminal.send_mode.findData(entry.mode)
    terminal.send_mode.setCurrentIndex(index)
    text = (
        entry.payload.hex(" ").upper()
        if entry.mode is CommandMode.HEX
        else entry.payload.decode("utf-8", errors="replace")
    )
    terminal.send_input.setText(text)
    terminal.newline_check.setChecked(entry.append_newline)


def on_session_state_changed(window, state: object) -> None:
    """On session state changed."""
    if window._closing:
        return
    if isinstance(state, SessionState):
        ble = ble_bindings_for(window)
        window._reset_data_activity()
        if state is SessionState.OPEN:
            leave_connection_onboarding(window)
        if state in {SessionState.CLOSING, SessionState.CLOSED, SessionState.ERROR}:
            window._ble_notification_timer.stop()
            window._ble_notification_pending = None
            window._ble_notification_ref = None
            if ble is not None:
                ble.notify_check.blockSignals(True)
                ble.notify_check.setChecked(False)
                ble.notify_check.blockSignals(False)
        if (
            state in {SessionState.OPENING, SessionState.OPEN}
            and not window._view_model.replay_active
        ):
            was_history_context = window._history_source_active or window._history_file_selected
            window._history_source_active = False
            window._history_file_selected = False
            if was_history_context:
                window._update_protocol_scope()
                window._set_protocol_controls_enabled(derived_source_supported(window))
                window._set_derived_controls_enabled(derived_source_supported(window))
                window._update_component_status()
                window._update_dataset_status()
                window._update_dataset_curve_status(window._dataset_curve.snapshot)
        window._update_state_badge(state)
        window._update_source_badge()
        window._update_pipeline_summary()
    window._refresh_connection_controls(state)

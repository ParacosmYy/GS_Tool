"""Command entry, quick-command, and batch execution controller.

Only the explicit window feature surface is consumed here; transport and
protocol implementation stay behind the view-model/application ports.
"""

from __future__ import annotations

import time

from ...domain.commands import CommandBatch, CommandBatchSnapshot, CommandBatchState
from ...domain.errors import ConfigurationError
from ...domain.models import (
    BleGattCharacteristic,
    BleGattCharacteristicRef,
    BleGattWriteMode,
    CommandEntry,
    CommandMode,
    TcpServerPeerSnapshot,
    TransportKind,
)
from ..ble_selection import selected_ble_write_mode
from ..command_batch_editor import CommandBatchEditorDialog
from ..command_batch_surface import CommandBatchSurfaceProjection
from ..command_bindings import command_batch_bindings_for
from ..command_selection import current_send_mode, selected_command_batch
from ..connection_bindings import (
    ble_bindings_for,
    connection_shell_bindings_for,
    network_bindings_for,
)
from ..property_refresh import refresh_dynamic_property
from ..qt import Qt, QTableWidgetItem
from ..terminal_bindings import terminal_bindings_for

COMMAND_BATCH_ACTIVITY_MS = 520
COMMAND_BATCH_TERMINAL_ACTIVITY_MS = 480


def _request_command_batch_activity(window, duration_ms: int) -> None:
    """Acknowledge visible batch transitions through the shared motion clock."""

    if window._closing or not window.isVisible() or window.isMinimized():
        return
    motion_controller = getattr(window, "_motion_controller", None)
    if motion_controller is not None:
        motion_controller.request_activity(duration_ms)


def send_current(window) -> None:
    """Send current."""
    terminal = terminal_bindings_for(window)
    if terminal is None:
        window._show_local_error("发送失败 · 终端控件尚未初始化。")
        return
    text = terminal.send_input.text()
    if not text:
        window._show_local_error("命令内容不能为空。")
        return
    mode = current_send_mode(window)
    try:
        payload = (
            bytes.fromhex("".join(text.split()))
            if mode == CommandMode.HEX
            else text.encode("utf-8")
        )
        if not payload:
            window._show_local_error("命令内容不能为空。")
            return
        entry = CommandEntry(
            name=text[:128],
            payload=payload,
            mode=mode,
            append_newline=terminal.newline_check.isChecked(),
            created_at=time.monotonic(),
        )
    except (ValueError, ConfigurationError) as exc:
        window._show_local_error(f"发送格式无效 · {exc}")
        return
    shell = connection_shell_bindings_for(window)
    if shell is None:
        window._show_local_error("发送失败 · 连接控件尚未初始化。")
        return
    kind = TransportKind(shell.transport_combo.currentData())
    target_peer_id = None
    ble_characteristic: BleGattCharacteristicRef | None = None
    ble_write_mode: BleGattWriteMode | None = None
    if kind is TransportKind.TCP_SERVER:
        network = network_bindings_for(window)
        if network is None:
            window._show_local_error("发送失败 · 网络控件尚未初始化。")
            return
        selected = network.server_peer_combo.currentData(Qt.ItemDataRole.UserRole)
        target_peer_id = selected.peer_id if isinstance(selected, TcpServerPeerSnapshot) else None
    elif kind is TransportKind.BLE_GATT:
        ble = ble_bindings_for(window)
        if ble is None:
            window._show_local_error("发送失败 · BLE 控件尚未初始化。")
            return
        selected = ble.characteristic_combo.currentData(Qt.ItemDataRole.UserRole)
        if not isinstance(selected, BleGattCharacteristic):
            window._show_local_error("发送失败 · 请先选择 BLE GATT 特征。")
            return
        ble_characteristic = selected.ref
        selected_mode = selected_ble_write_mode(window)
        if selected_mode is None:
            window._show_local_error("发送失败 · BLE 写模式无效。")
            return
        ble_write_mode = selected_mode
    if window._view_model.send_entry(
        entry,
        target_peer_id=target_peer_id,
        ble_characteristic=ble_characteristic,
        ble_write_mode=ble_write_mode,
    ):
        window._motion_controller.request_activity()
        terminal.send_input.clear()


def send_current_action(window, *_args: object, **_kwargs: object) -> None:
    """Absorb Qt signal payload at the command owner boundary."""

    send_current(window)


def on_command_batches_changed(window, batches: object) -> None:
    """On command batches changed."""
    if window._closing:
        return
    controls = command_batch_bindings_for(window)
    if controls is None:
        return
    normalized = tuple(batch for batch in batches if isinstance(batch, CommandBatch))
    selected = selected_command_batch(window)
    selected_id = selected.batch_id if selected is not None else None
    window._command_batches = normalized
    controls.combo.blockSignals(True)
    controls.combo.clear()
    if not normalized:
        controls.combo.addItem("暂无批量命令 · 点击新建", None)
    else:
        for batch in normalized:
            controls.combo.addItem(
                f"{batch.name} · {len(batch.steps)} 步",
                batch,
            )
        restored = next(
            (index for index, batch in enumerate(normalized) if batch.batch_id == selected_id),
            0,
        )
        controls.combo.setCurrentIndex(restored)
    controls.combo.blockSignals(False)
    on_command_batch_selection_changed(window)


def on_command_batch_selection_changed(window, _index: int = -1) -> None:
    """On command batch selection changed."""
    if window._closing:
        return
    render_command_batch_results(window, window._view_model.command_batch_snapshot)
    window._refresh_connection_controls(window._view_model.state)


def on_command_batch_changed(window, snapshot: object) -> None:
    """On command batch changed."""
    if window._closing:
        return
    if not isinstance(snapshot, CommandBatchSnapshot):
        return
    window._command_batch_snapshot = snapshot
    if snapshot.state is CommandBatchState.RUNNING:
        _request_command_batch_activity(window, COMMAND_BATCH_ACTIVITY_MS)
    elif snapshot.state in {
        CommandBatchState.COMPLETED,
        CommandBatchState.STOPPED,
        CommandBatchState.FAILED,
    }:
        _request_command_batch_activity(window, COMMAND_BATCH_TERMINAL_ACTIVITY_MS)
    render_command_batch_results(window, snapshot)
    window._refresh_connection_controls(window._view_model.state)


def _command_batch_surface_state(
    batch: CommandBatch | None,
    snapshot: CommandBatchSnapshot,
) -> str:
    """Project only the selected batch's snapshot into a visual state."""

    if batch is None or snapshot.batch_id != batch.batch_id:
        return "empty" if batch is None else "ready"
    if snapshot.state is CommandBatchState.IDLE:
        return "ready"
    return {
        CommandBatchState.RUNNING: "running",
        CommandBatchState.COMPLETED: "completed",
        CommandBatchState.STOPPED: "stopped",
        CommandBatchState.FAILED: "failed",
    }[snapshot.state]


def render_command_batch_results(window, snapshot: CommandBatchSnapshot) -> None:
    """Render command batch results."""
    if window._closing:
        return
    controls = command_batch_bindings_for(window)
    if controls is None:
        return
    batch = selected_command_batch(window)
    selected_step = None
    current_row = controls.results.currentRow()
    if current_row >= 0:
        current_item = controls.results.item(current_row, 0)
        if current_item is not None:
            selected_step = current_item.text()
    scroll_value = controls.results.verticalScrollBar().value()
    controls.results.setRowCount(0)
    surface_state = _command_batch_surface_state(batch, snapshot)
    refresh_dynamic_property(controls.status, "state", surface_state)
    matching_snapshot = batch is not None and snapshot.batch_id == batch.batch_id
    controls.status.set_projection(
        CommandBatchSurfaceProjection(
            state=surface_state,
            step_count=len(batch.steps) if batch is not None else 0,
            accepted_steps=snapshot.accepted_steps if matching_snapshot else 0,
            current_step=snapshot.current_step if matching_snapshot else None,
            failed_step=snapshot.failed_step if matching_snapshot else None,
        )
    )
    controls.status.setVisible(batch is not None)
    controls.results.setVisible(batch is not None)
    if batch is None:
        text = "暂无批量命令结果 · 先新建一个批量命令。"
        controls.empty_state.set_content(
            "还没有批量命令",
            "新建一个批量命令后，可按固定顺序执行并查看每一步结果。",
        )
        controls.status.setToolTip(text)
        controls.empty_state.setVisible(True)
        text = "固定顺序 · 无循环/脚本/广播；RTT 批量命令留到最后阶段"
        controls.status.setText(text)
        controls.status.setAccessibleDescription(text)
        return
    controls.empty_state.setVisible(False)
    if snapshot.batch_id != batch.batch_id or snapshot.state is CommandBatchState.IDLE:
        text = f"就绪 · {len(batch.steps)} 步 · 总等待 {batch.total_delay_ms} ms"
    else:
        text = snapshot.message
    controls.status.setText(text)
    controls.status.setToolTip(text)
    controls.status.setAccessibleDescription(text)
    for index, _step in enumerate(batch.steps, start=1):
        if snapshot.batch_id != batch.batch_id:
            state = "待执行"
            note = ""
        elif index <= snapshot.accepted_steps:
            state = "已提交"
            note = "已进入传输队列"
        elif snapshot.failed_step == index:
            state = "失败"
            note = snapshot.error.message if snapshot.error is not None else "发送失败"
        elif snapshot.state is CommandBatchState.STOPPED:
            state = "未执行"
            note = "批量命令已停止"
        elif snapshot.state is CommandBatchState.RUNNING and snapshot.current_step == index:
            state = "执行中"
            note = "等待本地发送队列接受"
        else:
            state = "待执行"
            note = ""
        controls.results.insertRow(index - 1)
        for column, value in enumerate((str(index), state, note)):
            item = QTableWidgetItem(value)
            item.setData(Qt.ItemDataRole.AccessibleTextRole, value)
            controls.results.setItem(index - 1, column, item)
    if selected_step is not None:
        for row_index in range(controls.results.rowCount()):
            item = controls.results.item(row_index, 0)
            if item is not None and item.text() == selected_step:
                controls.results.setCurrentCell(row_index, 0)
                controls.results.selectRow(row_index)
                break
    controls.results.verticalScrollBar().setValue(scroll_value)


def new_command_batch(window) -> None:
    """New command batch."""
    dialog = CommandBatchEditorDialog(quick_entries=window._quick_entries, parent=window)
    if dialog.exec() and dialog.result_batch is not None:
        window._view_model.save_command_batch(dialog.result_batch)


def new_command_batch_action(window, *_args: object, **_kwargs: object) -> None:
    """Absorb Qt signal payload before opening the batch editor."""

    new_command_batch(window)


def edit_command_batch(window) -> None:
    """Edit command batch."""
    batch = selected_command_batch(window)
    if batch is None:
        window._show_local_error("请先选择一个批量命令。")
        return
    if window._view_model.batch_active:
        window._show_local_error("批量命令执行期间不能编辑。")
        return
    dialog = CommandBatchEditorDialog(
        batch=batch,
        quick_entries=window._quick_entries,
        parent=window,
    )
    if dialog.exec() and dialog.result_batch is not None:
        window._view_model.save_command_batch(dialog.result_batch)


def edit_command_batch_action(window, *_args: object, **_kwargs: object) -> None:
    """Absorb Qt signal payload before editing the selected batch."""

    edit_command_batch(window)


def delete_command_batch(window) -> None:
    """Delete command batch."""
    batch = selected_command_batch(window)
    if batch is None:
        return
    if window._view_model.batch_active:
        window._show_local_error("批量命令执行期间不能删除。")
        return
    window._view_model.remove_command_batch(batch.batch_id)


def delete_command_batch_action(window, *_args: object, **_kwargs: object) -> None:
    """Absorb Qt signal payload before deleting the selected batch."""

    delete_command_batch(window)


def run_command_batch(window) -> None:
    """Run command batch."""
    batch = selected_command_batch(window)
    if batch is None:
        window._show_local_error("请先新建或选择一个批量命令。")
        return
    shell = connection_shell_bindings_for(window)
    if shell is None:
        window._show_local_error("批量命令执行失败 · 连接控件尚未初始化。")
        return
    kind = TransportKind(shell.transport_combo.currentData())
    target_peer_id = None
    ble_characteristic: BleGattCharacteristicRef | None = None
    ble_write_mode: BleGattWriteMode | None = None
    if kind is TransportKind.TCP_SERVER:
        network = network_bindings_for(window)
        if network is None:
            window._show_local_error("批量命令执行失败 · 网络控件尚未初始化。")
            return
        selected = network.server_peer_combo.currentData(Qt.ItemDataRole.UserRole)
        target_peer_id = selected.peer_id if isinstance(selected, TcpServerPeerSnapshot) else None
    elif kind is TransportKind.BLE_GATT:
        ble = ble_bindings_for(window)
        if ble is None:
            window._show_local_error("批量命令执行失败 · BLE 控件尚未初始化。")
            return
        selected = ble.characteristic_combo.currentData(Qt.ItemDataRole.UserRole)
        if not isinstance(selected, BleGattCharacteristic):
            window._show_local_error("批量命令执行失败 · 请先选择 BLE GATT 特征。")
            return
        ble_characteristic = selected.ref
        selected_mode = selected_ble_write_mode(window)
        if selected_mode is None:
            window._show_local_error("批量命令执行失败 · BLE 写模式无效。")
            return
        ble_write_mode = selected_mode
    window._view_model.start_command_batch(
        batch.batch_id,
        target_peer_id=target_peer_id,
        ble_characteristic=ble_characteristic,
        ble_write_mode=ble_write_mode,
    )


def run_command_batch_action(window, *_args: object, **_kwargs: object) -> None:
    """Absorb Qt signal payload before starting the selected batch."""

    run_command_batch(window)

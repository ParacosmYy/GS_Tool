"""Connection surface state and affordance controller.

This module owns connection-related presentation policy while the composition
root supplies the concrete widgets and application view-model port.
"""

from __future__ import annotations

import ipaddress

from ...domain.models import (
    MAX_SERVER_ALLOWLIST_ENTRIES,
    BleGattCharacteristic,
    BleGattDevice,
    CommandMode,
    SessionState,
    TcpServerPeerSnapshot,
    TransportKind,
)
from ..ble_selection import selected_ble_write_mode, supported_ble_write_modes
from ..command_bindings import command_batch_bindings_for
from ..command_selection import current_send_mode, selected_command_batch
from ..connection_bindings import (
    ble_bindings_for,
    connection_shell_bindings_for,
    network_bindings_for,
    uart_bindings_for,
)
from ..property_refresh import refresh_dynamic_property
from ..qt import Qt
from ..terminal_bindings import terminal_bindings_for
from .send_context import refresh_send_context


def tcp_server_readiness(window) -> tuple[bool, str]:
    """Return a local explanation for TCP Server listener readiness."""

    network = network_bindings_for(window)
    if network is None:
        return False, "网络控件尚未初始化。"
    host = network.local_host.text().strip()
    try:
        bind_ip = ipaddress.ip_address(host)
    except ValueError:
        return False, "TCP Server 监听主机必须是 IPv4 字面量。"
    if bind_ip.version != 4:
        return False, "TCP Server 当前仅支持 IPv4 监听地址。"
    if network.local_port.value() <= 0:
        return False, "TCP Server 监听端口必须大于 0。"
    entries = tuple(
        item.strip()
        for item in network.server_allowlist.toPlainText().splitlines()
        if item.strip()
    )
    if not bind_ip.is_loopback and not network.server_lan_confirm.isChecked():
        return False, "非回环监听前请勾选“我确认允许 LAN 监听”。"
    if not bind_ip.is_loopback and not entries:
        return False, "非回环监听前请填写至少一条 IPv4/CIDR allowlist。"
    normalized_entries: set[str] = set()
    for entry in entries:
        try:
            network = ipaddress.ip_network(entry, strict=False)
        except ValueError:
            return False, f"allowlist 项无效：{entry}；请输入 IPv4 或 CIDR。"
        if network.version != 4:
            return False, f"allowlist 项不是 IPv4/CIDR：{entry}。"
        normalized_entries.add(str(network))
    if len(normalized_entries) > MAX_SERVER_ALLOWLIST_ENTRIES:
        return False, (f"allowlist 归一化后最多允许 {MAX_SERVER_ALLOWLIST_ENTRIES} 项。")
    if bind_ip.is_loopback:
        return True, "回环监听就绪；client 连接后选择发送目标。"
    return True, "LAN 监听条件已满足；allowlist 将限制接入 client。"


def update_connection_controls(window, state: SessionState) -> None:
    """Update connection controls."""
    uart = uart_bindings_for(window)
    network = network_bindings_for(window)
    ble = ble_bindings_for(window)
    terminal = terminal_bindings_for(window)
    batch = command_batch_bindings_for(window)
    shell = connection_shell_bindings_for(window)
    if (
        uart is None
        or network is None
        or ble is None
        or terminal is None
        or batch is None
        or shell is None
    ):
        return
    active = state in {SessionState.OPENING, SessionState.OPEN, SessionState.CLOSING}
    historical_active = window._view_model.replay_active
    batch_active = window._view_model.batch_active
    connected = state is SessionState.OPEN and window._view_model.can_send and not historical_active
    discovery_busy = bool(getattr(window._view_model, "discovery_busy", False))
    ble_scan_busy = bool(getattr(window._view_model, "ble_scan_busy", False))
    for widget in (
        uart.baud_combo,
        uart.data_bits,
        uart.parity,
        uart.stop_bits,
        uart.flow_control,
        uart.read_timeout,
        uart.write_timeout,
        uart.inter_byte_timeout,
        uart.exclusive,
        uart.dtr,
        uart.rts,
        network.remote_host,
        network.remote_port,
        network.local_host,
        network.local_port,
        network.connect_timeout,
        network.read_timeout,
        network.write_timeout,
        network.udp_limit,
        network.server_allowlist,
        network.server_lan_confirm,
        network.server_max_clients,
        network.rtt_channel,
        ble.scan_timeout,
        ble.name_filter,
        ble.service_filter,
        ble.connect_timeout,
        ble.pair,
        ble.cached_services,
    ):
        widget.setEnabled(not active and not historical_active)
    shell.transport_combo.setEnabled(not active and not historical_active)
    shell.preset_combo.setEnabled(not active and not historical_active)
    custom_preset_selected = bool(shell.preset_combo.property("customSelected"))
    shell.save_preset_button.setEnabled(not active and not historical_active)
    shell.delete_preset_button.setEnabled(
        not active and not historical_active and custom_preset_selected
    )
    uart.port_combo.setEnabled(not active and not historical_active)
    uart.refresh_button.setEnabled(not active and not historical_active)
    kind = TransportKind(shell.transport_combo.currentData())
    selected_ble_device = ble.device_combo.currentData(Qt.ItemDataRole.UserRole)
    uart_ready = kind is not TransportKind.UART or bool(uart.port_combo.currentText().strip())
    server_ready, server_hint = tcp_server_readiness(window)
    connection_busy = state in {SessionState.OPENING, SessionState.CLOSING}
    shell.connect_button.set_busy(connection_busy)
    refresh_hint = "正在枚举 UART 端口…" if discovery_busy else "刷新可用 UART 端点；不会自动连接。"
    uart.refresh_button.setText("刷新中…" if discovery_busy else "刷新端口")
    uart.refresh_button.set_busy(discovery_busy)
    uart.refresh_button.setToolTip(refresh_hint)
    uart.refresh_button.setAccessibleDescription(refresh_hint)
    uart.refresh_button.setEnabled(not active and not historical_active and not discovery_busy)
    ble.scan_button.setText("扫描中…" if ble_scan_busy else "扫描 BLE")
    ble.scan_button.set_busy(ble_scan_busy)
    if historical_active:
        hint = "回放中 · 停止回放后可连接"
    elif active and state is SessionState.OPEN and kind is TransportKind.TCP_SERVER:
        peer_count = len(window._view_model.server_peers)
        hint = (
            f"监听中 · 已接入 {peer_count} 个 client · 选择发送目标后可发送"
            if peer_count
            else "监听中 · 等待 client 接入"
        )
    elif active:
        hint = "会话已建立 · 顶部状态区显示当前阶段"
    elif kind is TransportKind.UART and not uart_ready:
        hint = "下一步：选择或输入 UART 端口"
    elif kind is TransportKind.BLE_GATT and not isinstance(selected_ble_device, BleGattDevice):
        hint = "下一步：扫描并选择 BLE 设备"
    elif kind is TransportKind.RTT:
        hint = "下一步：确认外部 RTT Telnet 服务已启动，再点击连接"
    elif kind is TransportKind.TCP_SERVER:
        hint = (
            server_hint if not server_ready else "下一步：点击开始监听；client 接入后选择发送目标"
        )
    elif kind is TransportKind.UDP_DATAGRAM:
        hint = "下一步：确认远端地址，再点击连接绑定 UDP"
    elif kind is TransportKind.TCP_STREAM:
        hint = "下一步：确认远端地址，再点击连接建立 TCP"
    else:
        hint = "就绪 · 点击连接开始会话"
    shell.hint_label.setText(hint)
    shell.hint_label.setAccessibleDescription(hint)
    shell.hint_label.setToolTip(hint)
    can_start = (
        not window._view_model.is_active
        and not historical_active
        and uart_ready
        and (kind is not TransportKind.TCP_SERVER or server_ready)
        and (kind is not TransportKind.BLE_GATT or isinstance(selected_ble_device, BleGattDevice))
    )
    can_stop = active and state is not SessionState.CLOSING
    shell.connect_button.setEnabled(can_start or can_stop)
    if kind is TransportKind.TCP_SERVER:
        server_target_selected = isinstance(
            network.server_peer_combo.currentData(Qt.ItemDataRole.UserRole),
            TcpServerPeerSnapshot,
        )
        connected = connected and server_target_selected
    else:
        server_target_selected = True
    selected_ble = ble.characteristic_combo.currentData(Qt.ItemDataRole.UserRole)
    ble_capability = selected_ble if isinstance(selected_ble, BleGattCharacteristic) else None
    selected_ble_mode = selected_ble_write_mode(window)
    ble_notification_pending = window._ble_notification_pending is not None
    supported_ble_modes = supported_ble_write_modes(ble_capability)
    ble_can_write = bool(
        ble_capability
        and selected_ble_mode is not None
        and selected_ble_mode in supported_ble_modes
    )
    if kind is not TransportKind.BLE_GATT:
        ble_mode_hint = "仅 BLE GATT 连接时可选择写入模式。"
    elif historical_active:
        ble_mode_hint = "历史回放期间不可使用 BLE 写入。"
    elif not connected:
        ble_mode_hint = "建立 BLE 连接并选择 GATT characteristic 后可选择写入模式。"
    elif ble_capability is None:
        ble_mode_hint = "选择 BLE GATT characteristic 后可选择写入模式。"
    elif not supported_ble_modes:
        ble_mode_hint = "当前 characteristic 不支持写入。"
    elif not ble_can_write:
        ble_mode_hint = "当前 characteristic 不支持所选 BLE 写入模式；已自动选择可用模式。"
    else:
        ble_mode_hint = "当前 BLE 写入模式可用。"
    ble.write_mode.setAccessibleDescription(ble_mode_hint)
    ble.write_mode.setToolTip(ble_mode_hint)
    if kind is not TransportKind.BLE_GATT:
        ble_read_hint = "仅 BLE GATT 连接时可读取特征。"
        ble_notify_hint = "仅 BLE GATT 连接时可订阅通知或指示。"
    elif not connected:
        ble_read_hint = "建立 BLE 连接并选择 GATT characteristic 后可读取。"
        ble_notify_hint = "建立 BLE 连接并选择支持 notify/indicate 的 characteristic 后可订阅。"
    elif ble_capability is None:
        ble_read_hint = "选择 BLE GATT characteristic 后可读取。"
        ble_notify_hint = "选择支持 notify/indicate 的 BLE GATT characteristic 后可订阅。"
    else:
        ble_read_hint = (
            "当前 characteristic 支持读取。"
            if ble_capability.supports("read")
            else "当前 characteristic 不支持读取。"
        )
        ble_notify_hint = (
            "当前 characteristic 支持通知或指示。"
            if ble_capability.supports("notify") or ble_capability.supports("indicate")
            else "当前 characteristic 不支持通知或指示。"
        )
    for widget, description in (
        (ble.read_button, ble_read_hint),
        (ble.notify_check, ble_notify_hint),
    ):
        widget.setAccessibleDescription(description)
        widget.setToolTip(description)
    shell.connect_button.setText(
        "停止监听"
        if active and kind is TransportKind.TCP_SERVER
        else ("断开" if active else "连接")
    )
    shell.connect_button.setAccessibleDescription(
        "停止当前监听。"
        if active and kind is TransportKind.TCP_SERVER
        else ("断开当前会话。" if active else hint)
    )
    shell.connect_button.setToolTip(shell.connect_button.accessibleDescription())
    network.server_peer_combo.setEnabled(
        kind is TransportKind.TCP_SERVER
        and active
        and not historical_active
        and not batch_active
        and state is not SessionState.CLOSING
        and bool(window._view_model.server_peers)
    )
    ble.scan_button.setEnabled(
        kind is TransportKind.BLE_GATT
        and not active
        and not historical_active
        and not ble_scan_busy
    )
    ble.device_combo.setEnabled(not active and not historical_active)
    ble.characteristic_combo.setEnabled(
        kind is TransportKind.BLE_GATT
        and state is SessionState.OPEN
        and not batch_active
        and not ble_notification_pending
        and bool(ble.characteristic_combo.count())
    )
    if kind is not TransportKind.BLE_GATT:
        ble_scan_hint = "仅 BLE GATT 传输可扫描设备。"
        ble_device_hint = "仅 BLE GATT 传输可选择设备。"
        ble_characteristic_hint = "仅 BLE GATT 连接后可选择 characteristic。"
    elif historical_active:
        ble_scan_hint = "历史回放期间不可扫描 BLE。"
        ble_device_hint = "历史回放期间不可更换 BLE 设备。"
        ble_characteristic_hint = "历史回放期间不可选择 BLE characteristic。"
    elif active:
        ble_scan_hint = "会话活动期间不可重新扫描 BLE。"
        ble_device_hint = "会话活动期间不可更换 BLE 设备。"
        ble_characteristic_hint = (
            "通知订阅确认中，完成或超时后才能更换 characteristic。"
            if ble_notification_pending
            else "会话关闭后才能重新选择 BLE characteristic。"
        )
    elif state is not SessionState.OPEN:
        ble_scan_hint = "正在扫描 BLE 设备…" if ble_scan_busy else "建立 BLE 会话前可扫描设备。"
        ble_device_hint = "扫描后选择 BLE 设备。"
        ble_characteristic_hint = "建立 BLE 连接后才能选择 characteristic。"
    else:
        ble_scan_hint = "正在扫描 BLE 设备…" if ble_scan_busy else "扫描附近 BLE GATT 设备。"
        ble_device_hint = "选择要连接的 BLE GATT 设备。"
        ble_characteristic_hint = "选择当前 BLE 会话中的 GATT characteristic。"
    for widget, description in (
        (ble.scan_button, ble_scan_hint),
        (ble.device_combo, ble_device_hint),
        (ble.characteristic_combo, ble_characteristic_hint),
    ):
        widget.setAccessibleDescription(description)
        widget.setToolTip(description)
    ble.read_button.setEnabled(
        kind is TransportKind.BLE_GATT
        and connected
        and not batch_active
        and not ble_notification_pending
        and bool(ble_capability)
        and ble_capability.supports("read")
    )
    ble.notify_check.setEnabled(
        kind is TransportKind.BLE_GATT
        and connected
        and not batch_active
        and not ble_notification_pending
        and bool(ble_capability)
        and (ble_capability.supports("notify") or ble_capability.supports("indicate"))
    )
    ble.write_mode.setEnabled(
        kind is TransportKind.BLE_GATT
        and connected
        and not batch_active
        and not ble_notification_pending
        and bool(ble_capability)
        and (ble_capability.supports("write") or ble_capability.supports("write-without-response"))
    )
    send_available = (
        connected
        and not batch_active
        and not (kind is TransportKind.BLE_GATT and ble_notification_pending)
        and (kind is not TransportKind.BLE_GATT or ble_can_write)
    )
    send_text = terminal.send_input.text()
    send_has_payload = bool(send_text)
    if send_has_payload and current_send_mode(window) is CommandMode.HEX:
        try:
            send_has_payload = bool(bytes.fromhex("".join(send_text.split())))
        except ValueError:
            # Keep malformed Hex enabled so the submit path can explain it.
            send_has_payload = True
    send_enabled = send_available and send_has_payload
    terminal.send_input.setEnabled(send_available)
    terminal.send_mode.setEnabled(send_available)
    terminal.newline_check.setEnabled(send_available)
    terminal.send_button.setEnabled(send_enabled)
    if historical_active:
        send_band_state = "history"
    elif batch_active or ble_notification_pending:
        send_band_state = "busy"
    elif send_available:
        send_band_state = "ready"
    elif connected:
        send_band_state = "waiting"
    else:
        send_band_state = "blocked"
    refresh_dynamic_property(terminal.send_control_band, "state", send_band_state)
    terminal.send_input.set_surface_state(send_band_state)
    refresh_send_context(window)
    if historical_active:
        send_hint = "历史回放期间不可发送。"
    elif batch_active:
        send_hint = "批量命令执行中；完成或停止后可发送。"
    elif kind is TransportKind.BLE_GATT and ble_notification_pending:
        send_hint = "BLE 通知订阅确认中；完成或超时后可发送。"
    elif kind is TransportKind.BLE_GATT and not ble_can_write:
        send_hint = ble_mode_hint
    elif (
        kind is TransportKind.TCP_SERVER
        and state is SessionState.OPEN
        and not server_target_selected
    ):
        send_hint = "TCP Server 已监听 · 先选择发送目标 client。"
    elif not connected:
        send_hint = "建立可发送会话后才能发送。"
    elif not send_has_payload:
        send_hint = "输入内容后才能发送。"
    else:
        send_hint = "输入内容后发送。"
    if historical_active:
        send_state_text = "历史回放"
    elif batch_active or ble_notification_pending:
        send_state_text = "处理中"
    elif send_enabled:
        send_state_text = "可发送"
    elif send_available:
        send_state_text = "输入内容"
    elif (
        kind is TransportKind.TCP_SERVER
        and state is SessionState.OPEN
        and not server_target_selected
    ):
        send_state_text = "等待目标"
    else:
        send_state_text = "等待连接"
    terminal.send_state_label.setText(send_state_text)
    refresh_dynamic_property(terminal.send_state_label, "state", send_band_state)
    terminal.send_state_label.setAccessibleDescription(send_hint)
    terminal.send_state_label.setToolTip(send_hint)
    for widget, hint in (
        (
            terminal.send_input,
            f"发送内容输入框：输入文本或十六进制内容；{send_hint}",
        ),
        (
            terminal.send_mode,
            f"发送格式选择：文本或十六进制；{send_hint}",
        ),
        (
            terminal.newline_check,
            f"追加 CRLF 只改变发送 payload；{send_hint}",
        ),
        (terminal.send_button, send_hint),
    ):
        widget.setAccessibleDescription(hint)
        widget.setToolTip(hint)
    batch_edit_locked = (
        batch_active
        or historical_active
        or (kind is TransportKind.BLE_GATT and ble_notification_pending)
    )
    batch.combo.setEnabled(not batch_edit_locked)
    batch.new_button.setEnabled(not batch_edit_locked)
    batch.empty_action_button.setEnabled(not batch_edit_locked)
    batch.edit_button.setEnabled(
        not batch_edit_locked and selected_command_batch(window) is not None
    )
    batch.delete_button.setEnabled(
        not batch_edit_locked and selected_command_batch(window) is not None
    )
    run_batch_enabled = (
        connected
        and not batch_active
        and not historical_active
        and kind is not TransportKind.RTT
        and not (kind is TransportKind.BLE_GATT and ble_notification_pending)
        and (kind is not TransportKind.BLE_GATT or ble_can_write)
        and selected_command_batch(window) is not None
    )
    batch.run_button.setEnabled(run_batch_enabled)
    batch.stop_button.setEnabled(batch_active)
    batch.stop_button.set_busy(batch_active)
    selected_batch = selected_command_batch(window) is not None
    if historical_active:
        batch_hint = "历史回放期间不可编辑批量命令。"
    elif batch_active:
        batch_hint = "批量命令执行中；完成或停止后可编辑。"
    elif ble_notification_pending:
        batch_hint = "BLE 通知订阅确认中；完成或超时后可编辑。"
    else:
        batch_hint = "选择批量命令后可编辑、删除或执行。"
    batch.combo.setAccessibleDescription(batch_hint)
    new_batch_hint = (
        "当前状态暂不可新建批量命令。"
        if batch_edit_locked
        else "新建批量命令。"
    )
    batch.new_button.setAccessibleDescription(new_batch_hint)
    batch.empty_action_button.setAccessibleDescription(new_batch_hint)
    batch.edit_button.setAccessibleDescription(
        "先选择批量命令。"
        if not selected_batch
        else ("当前状态不可编辑。" if historical_active or batch_active else "编辑当前批量命令。")
    )
    batch.delete_button.setAccessibleDescription(
        "先选择批量命令。"
        if not selected_batch
        else ("当前状态不可删除。" if historical_active or batch_active else "删除当前批量命令。")
    )
    batch.run_button.setAccessibleDescription(
        "RTT 暂不支持批量命令。"
        if kind is TransportKind.RTT
        else (
            ble_mode_hint
            if kind is TransportKind.BLE_GATT and not ble_can_write
            else (
                "TCP Server 已监听 · 先选择发送目标 client。"
                if (
                    kind is TransportKind.TCP_SERVER
                    and state is SessionState.OPEN
                    and not server_target_selected
                )
                else (
                    "先建立可发送会话并选择批量命令。"
                    if not connected or not selected_batch
                    else (
                        "当前状态不可执行。"
                        if batch_active or historical_active
                        else "执行当前批量命令。"
                    )
                )
            )
        )
    )
    batch.stop_button.setAccessibleDescription(
        "停止正在执行的批量命令。" if batch_active else "当前没有正在执行的批量命令。"
    )
    for button in (
        batch.new_button,
        batch.empty_action_button,
        batch.edit_button,
        batch.delete_button,
        batch.run_button,
        batch.stop_button,
    ):
        button.setToolTip(button.accessibleDescription())

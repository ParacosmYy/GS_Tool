"""Apply and persist safe connection presets through the presentation form."""

from __future__ import annotations

from ...domain.models import (
    TransportKind,
    UartFlowControl,
    UartParity,
    UartStopBits,
)
from ..connection_bindings import (
    ble_bindings_for,
    connection_shell_bindings_for,
    network_bindings_for,
    uart_bindings_for,
)
from ..connection_preset_editor import ConnectionPresetEditorDialog, ConnectionPresetMetadata
from ..connection_preset_surface import refresh_connection_preset_combo
from ..connection_presets import (
    BUILTIN_CONNECTION_PRESET_KEYS,
    DEFAULT_CONNECTION_PRESET_KEY,
    BleConnectionPresetValues,
    ConnectionPreset,
    ConnectionPresetCatalog,
    NetworkConnectionPresetValues,
    RttConnectionPresetValues,
    TcpServerConnectionPresetValues,
    UartConnectionPresetValues,
    UdpConnectionPresetValues,
    custom_connection_presets,
    merge_connection_preset_catalog,
)
from ..qt import QDialog, Qt


def _set_combo_data(combo, value: object) -> None:
    index = combo.findData(value, Qt.ItemDataRole.UserRole)
    if index >= 0:
        combo.setCurrentIndex(index)


def _apply_uart_values(window, values: UartConnectionPresetValues) -> None:
    uart = uart_bindings_for(window)
    if uart is None:
        return
    _set_combo_data(uart.baud_combo, values.baud_rate)
    _set_combo_data(uart.data_bits, values.data_bits)
    _set_combo_data(uart.parity, values.parity)
    _set_combo_data(uart.stop_bits, values.stop_bits)
    _set_combo_data(uart.flow_control, values.flow_control)


def _apply_network_values(window, values: NetworkConnectionPresetValues) -> None:
    network = network_bindings_for(window)
    if network is None:
        return
    network.remote_host.setText(values.host)
    network.remote_port.setValue(values.port)


def _apply_server_values(window, values: TcpServerConnectionPresetValues) -> None:
    network = network_bindings_for(window)
    if network is None:
        return
    network.local_host.setText(values.bind_host)
    network.local_port.setValue(values.listen_port)
    network.server_max_clients.setValue(values.max_clients)
    network.server_allowlist.clear()
    network.server_lan_confirm.setChecked(False)


def _apply_udp_values(window, values: UdpConnectionPresetValues) -> None:
    network = network_bindings_for(window)
    if network is None:
        return
    network.local_host.setText(values.local_host)
    network.local_port.setValue(values.local_port)
    network.remote_host.setText(values.remote_host)
    network.remote_port.setValue(values.remote_port)
    network.udp_limit.setValue(values.max_datagram_size)


def _apply_rtt_values(window, values: RttConnectionPresetValues) -> None:
    network = network_bindings_for(window)
    if network is None:
        return
    network.remote_host.setText(values.host)
    network.remote_port.setValue(values.port)
    _set_combo_data(network.rtt_channel, values.channel)


def _apply_ble_values(window, values: BleConnectionPresetValues) -> None:
    ble = ble_bindings_for(window)
    if ble is None:
        return
    ble.name_filter.setText(values.name_filter)
    ble.service_filter.setText(values.service_filter)


def apply_connection_preset(window, preset: ConnectionPreset) -> None:
    """Fill controls from a preset; never start a session implicitly."""

    if window._closing:
        return
    if window._view_model.is_active or window._view_model.replay_active:
        window._show_local_error("当前会话或历史回放活动中，暂不能应用连接快速配置。")
        return
    shell = connection_shell_bindings_for(window)
    if shell is None:
        window._show_local_error("快速配置失败 · 连接控件尚未初始化。")
        return
    transport_index = shell.transport_combo.findData(
        preset.transport,
        Qt.ItemDataRole.UserRole,
    )
    if transport_index < 0:
        window._show_local_error(f"快速配置不可用 · 未找到 {preset.transport.value} 传输。")
        return
    transport_signals_blocked = shell.transport_combo.blockSignals(True)
    try:
        shell.transport_combo.setCurrentIndex(transport_index)
        match preset.values:
            case UartConnectionPresetValues():
                _apply_uart_values(window, preset.values)
            case NetworkConnectionPresetValues():
                _apply_network_values(window, preset.values)
            case TcpServerConnectionPresetValues():
                _apply_server_values(window, preset.values)
            case UdpConnectionPresetValues():
                _apply_udp_values(window, preset.values)
            case RttConnectionPresetValues():
                _apply_rtt_values(window, preset.values)
            case BleConnectionPresetValues():
                _apply_ble_values(window, preset.values)
    finally:
        shell.transport_combo.blockSignals(transport_signals_blocked)
    window._on_transport_changed()
    hint = f"已填入快速配置 · {preset.label}；确认端点后点击连接。"
    shell.hint_label.setText(hint)
    shell.hint_label.setAccessibleDescription(hint)
    shell.hint_label.setToolTip(preset.description)


def hydrate_recommended_connection_preset(window) -> None:
    """Prefill the safe first-run profile without starting a session."""

    shell = connection_shell_bindings_for(window)
    catalog = getattr(window, "_connection_preset_catalog", None)
    if (
        shell is None
        or not isinstance(catalog, ConnectionPresetCatalog)
        or window._closing
        or window._view_model.is_active
        or window._view_model.replay_active
        or shell.preset_combo.currentData(Qt.ItemDataRole.UserRole) is not None
    ):
        return
    preset = next(
        (candidate for candidate in catalog if candidate.key == DEFAULT_CONNECTION_PRESET_KEY),
        None,
    )
    if preset is None:
        return
    index = shell.preset_combo.findData(preset, Qt.ItemDataRole.UserRole)
    if index >= 0:
        shell.preset_combo.setCurrentIndex(index)


def _preset_mutation_allowed(window) -> bool:
    if window._closing:
        return False
    if window._view_model.is_active or window._view_model.replay_active:
        window._show_local_error("当前会话或历史回放活动中，暂不能保存或删除连接配置。")
        return False
    return True


def _current_custom_preset(window) -> ConnectionPreset | None:
    shell = connection_shell_bindings_for(window)
    if shell is None:
        return None
    value = shell.preset_combo.currentData(Qt.ItemDataRole.UserRole)
    if isinstance(value, ConnectionPreset) and value.key not in BUILTIN_CONNECTION_PRESET_KEYS:
        return value
    return None


def _combo_value(combo, label: str) -> object:
    value = combo.currentData(Qt.ItemDataRole.UserRole)
    if value is None:
        raise ValueError(f"{label}未选择")
    return value


def build_connection_preset_from_window(
    window,
    metadata: ConnectionPresetMetadata,
) -> ConnectionPreset:
    """Snapshot only normalized, non-secret values from the active form."""

    shell = connection_shell_bindings_for(window)
    if shell is None:
        raise ValueError("连接控件尚未初始化")
    try:
        transport = TransportKind(_combo_value(shell.transport_combo, "传输方式"))
    except ValueError as exc:
        raise ValueError("传输方式无效") from exc
    match transport:
        case TransportKind.UART:
            uart = uart_bindings_for(window)
            if uart is None:
                raise ValueError("UART 控件尚未初始化")
            values = UartConnectionPresetValues(
                baud_rate=_combo_value(uart.baud_combo, "波特率"),
                data_bits=_combo_value(uart.data_bits, "数据位"),
                parity=UartParity(_combo_value(uart.parity, "校验")),
                stop_bits=UartStopBits(_combo_value(uart.stop_bits, "停止位")),
                flow_control=UartFlowControl(_combo_value(uart.flow_control, "流控")),
            )
        case TransportKind.TCP_STREAM:
            network = network_bindings_for(window)
            if network is None:
                raise ValueError("网络控件尚未初始化")
            values = NetworkConnectionPresetValues(
                host=network.remote_host.text().strip(),
                port=network.remote_port.value(),
            )
        case TransportKind.TCP_SERVER:
            network = network_bindings_for(window)
            if network is None:
                raise ValueError("网络控件尚未初始化")
            values = TcpServerConnectionPresetValues(
                bind_host=network.local_host.text().strip(),
                listen_port=network.local_port.value(),
                max_clients=network.server_max_clients.value(),
            )
        case TransportKind.UDP_DATAGRAM:
            network = network_bindings_for(window)
            if network is None:
                raise ValueError("网络控件尚未初始化")
            values = UdpConnectionPresetValues(
                local_host=network.local_host.text().strip(),
                local_port=network.local_port.value(),
                remote_host=network.remote_host.text().strip(),
                remote_port=network.remote_port.value(),
                max_datagram_size=network.udp_limit.value(),
            )
        case TransportKind.RTT:
            network = network_bindings_for(window)
            if network is None:
                raise ValueError("网络控件尚未初始化")
            values = RttConnectionPresetValues(
                host=network.remote_host.text().strip(),
                port=network.remote_port.value(),
                channel=_combo_value(network.rtt_channel, "RTT channel"),
            )
        case TransportKind.BLE_GATT:
            ble = ble_bindings_for(window)
            if ble is None:
                raise ValueError("BLE 控件尚未初始化")
            values = BleConnectionPresetValues(
                name_filter=ble.name_filter.text().strip(),
                service_filter=ble.service_filter.text().strip(),
            )
        case _:
            raise ValueError("当前传输方式不支持自定义连接配置")
    return ConnectionPreset(
        key=metadata.key,
        label=metadata.label,
        description=metadata.description,
        transport=transport,
        values=values,
    )


def _catalog_with_preset(
    catalog: ConnectionPresetCatalog,
    preset: ConnectionPreset,
) -> ConnectionPresetCatalog:
    custom = list(custom_connection_presets(catalog))
    for index, current in enumerate(custom):
        if current.key == preset.key:
            custom[index] = preset
            break
    else:
        custom.append(preset)
    return merge_connection_preset_catalog(custom)


def _set_connection_preset_hint(window, text: str, *, description: str | None = None) -> None:
    shell = connection_shell_bindings_for(window)
    if shell is None:
        return
    shell.hint_label.setText(text)
    shell.hint_label.setAccessibleDescription(text)
    shell.hint_label.setToolTip(description or text)
    shell.preset_combo.setToolTip(description or text)


def save_custom_connection_preset(window) -> None:
    """Save the current normalized form as a user-owned preset."""

    if not _preset_mutation_allowed(window):
        return
    shell = connection_shell_bindings_for(window)
    if shell is None:
        window._show_local_error("保存失败 · 连接控件尚未初始化。")
        return
    dialog = ConnectionPresetEditorDialog(window, existing=_current_custom_preset(window))
    if dialog.exec() != QDialog.DialogCode.Accepted:
        return
    try:
        preset = build_connection_preset_from_window(window, dialog.metadata())
        current = _current_custom_preset(window)
        if current is None and preset.key in {
            item.key for item in custom_connection_presets(window._connection_preset_catalog)
        }:
            raise ValueError("自定义配置标识冲突，请重新保存")
        candidate = _catalog_with_preset(window._connection_preset_catalog, preset)
    except (TypeError, ValueError) as exc:
        window._show_local_error(f"自定义连接配置无效 · {exc}")
        return
    try:
        persisted = bool(window._connection_preset_store.save(candidate))
    except Exception:
        persisted = False
    window._connection_preset_catalog = candidate
    refresh_connection_preset_combo(
        shell.preset_combo,
        candidate,
        selected_key=preset.key,
        context_surface=shell.preset_context,
    )
    window._refresh_connection_controls(window._view_model.state)
    if persisted:
        hint = f"已保存自定义配置 · {preset.label}；确认端点后点击连接。"
    else:
        hint = f"已加入本次会话 · {preset.label}；本地保存失败，退出后可能丢失。"
    _set_connection_preset_hint(window, hint, description=preset.description)


def delete_custom_connection_preset(window) -> None:
    """Delete only the selected custom preset; built-ins are immutable."""

    if not _preset_mutation_allowed(window):
        return
    shell = connection_shell_bindings_for(window)
    if shell is None:
        window._show_local_error("删除失败 · 连接控件尚未初始化。")
        return
    current = _current_custom_preset(window)
    if current is None:
        window._show_local_error("请先选择一个自定义连接配置。")
        return
    remaining = tuple(
        preset
        for preset in custom_connection_presets(window._connection_preset_catalog)
        if preset.key != current.key
    )
    try:
        candidate = merge_connection_preset_catalog(remaining)
        persisted = bool(window._connection_preset_store.save(candidate))
    except (TypeError, ValueError):
        persisted = False
        candidate = window._connection_preset_catalog
    except Exception:
        persisted = False
        candidate = window._connection_preset_catalog
    if not persisted:
        window._show_local_error("自定义连接配置保存失败 · 未删除当前配置。")
        return
    window._connection_preset_catalog = candidate
    refresh_connection_preset_combo(
        shell.preset_combo,
        candidate,
        context_surface=shell.preset_context,
    )
    window._refresh_connection_controls(window._view_model.state)
    _set_connection_preset_hint(window, f"已删除自定义配置 · {current.label}。")


__all__ = [
    "apply_connection_preset",
    "build_connection_preset_from_window",
    "delete_custom_connection_preset",
    "hydrate_recommended_connection_preset",
    "save_custom_connection_preset",
]

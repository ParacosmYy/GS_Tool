"""Transport connection actions and typed configuration builders.

This controller owns transport-specific validation and UI affordance routing.
Concrete sessions remain behind the application view-model ports.
"""

from __future__ import annotations

from ...domain.errors import ConfigurationError
from ...domain.models import (
    DEFAULT_RTT_TELNET_PORT,
    BleGattDevice,
    BleGattDiscoveryConfig,
    BleGattTransportConfig,
    Endpoint,
    PeerAddress,
    RttTransportConfig,
    TcpServerTransportConfig,
    TcpTransportConfig,
    TransportKind,
    UartFlowControl,
    UartParity,
    UartStopBits,
    UartTransportConfig,
    UdpTransportConfig,
)
from ..connection_bindings import (
    ble_bindings_for,
    connection_shell_bindings_for,
    network_bindings_for,
    uart_bindings_for,
)
from ..connection_presets import ConnectionPreset
from ..formatters import endpoint_label
from ..protocol_scope import derived_source_supported
from ..qt import Qt
from ..transport_panel_transition import start_transport_panel_transition


def toggle_connection(window, *args: object, **kwargs: object) -> None:
    """Toggle the session; extra Qt signal payloads are intentionally ignored."""

    if window._view_model.is_active:
        window._view_model.disconnect()
        return
    try:
        shell = connection_shell_bindings_for(window)
        if shell is None:
            raise ConfigurationError("连接控件尚未初始化。")
        kind = TransportKind(shell.transport_combo.currentData())
        if kind == TransportKind.UART:
            uart = uart_bindings_for(window)
            if uart is None or not uart.port_combo.currentText().strip():
                window._show_local_error("请先选择或输入 UART 端口。")
                return
            window._view_model.connect_uart(build_uart_config(window))
        elif kind == TransportKind.TCP_STREAM:
            window._view_model.connect_tcp(build_tcp_config(window))
        elif kind == TransportKind.TCP_SERVER:
            window._view_model.connect_server(build_server_config(window))
        elif kind == TransportKind.UDP_DATAGRAM:
            window._view_model.connect_udp(build_udp_config(window))
        elif kind == TransportKind.BLE_GATT:
            window._view_model.connect_ble(build_ble_config(window))
        elif kind == TransportKind.RTT:
            window._view_model.connect_rtt(build_rtt_config(window))
        else:
            raise ConfigurationError("当前传输类型尚未实现。")
    except ConfigurationError as exc:
        window._show_local_error(str(exc))


def build_uart_config(window) -> UartTransportConfig:
    """Build uart config."""
    uart = uart_bindings_for(window)
    if uart is None:
        raise ConfigurationError("UART 控件尚未初始化。")
    selected = uart.port_combo.currentData(Qt.ItemDataRole.UserRole)
    visible_text = uart.port_combo.currentText().strip()
    selected_display = endpoint_label(selected) if isinstance(selected, Endpoint) else ""
    selected_matches = isinstance(selected, Endpoint) and visible_text in {
        selected.address,
        selected_display,
    }
    matching_endpoint = next(
        (endpoint for endpoint in window._endpoints if endpoint.address == visible_text),
        None,
    )
    if selected_matches:
        port = selected.address.strip()
        identity = selected.identity
    elif matching_endpoint is not None:
        port = matching_endpoint.address
        identity = matching_endpoint.identity
    else:
        port = visible_text
        identity = None
    read_timeout = uart.read_timeout.value()
    write_timeout = uart.write_timeout.value()
    inter_byte_timeout = uart.inter_byte_timeout.value() or None
    return UartTransportConfig(
        port=port,
        baud_rate=int(uart.baud_combo.currentData(Qt.ItemDataRole.UserRole)),
        data_bits=int(uart.data_bits.currentData()),
        parity=UartParity(uart.parity.currentData()),
        stop_bits=UartStopBits(uart.stop_bits.currentData()),
        flow_control=UartFlowControl(uart.flow_control.currentData()),
        read_timeout=read_timeout,
        write_timeout=write_timeout,
        inter_byte_timeout=inter_byte_timeout,
        exclusive=True if uart.exclusive.isChecked() else None,
        dtr=uart.dtr.isChecked(),
        rts=uart.rts.isChecked(),
        endpoint_identity=identity,
    )


def build_tcp_config(window) -> TcpTransportConfig:
    """Build tcp config."""
    network = network_bindings_for(window)
    if network is None:
        raise ConfigurationError("网络控件尚未初始化。")
    return TcpTransportConfig(
        remote_peer=PeerAddress(
            network.remote_host.text(),
            int(network.remote_port.value()),
        ),
        connect_timeout=network.connect_timeout.value(),
        read_timeout=network.read_timeout.value(),
        write_timeout=network.write_timeout.value(),
    )


def build_rtt_config(window) -> RttTransportConfig:
    """Build rtt config."""
    network = network_bindings_for(window)
    if network is None:
        raise ConfigurationError("网络控件尚未初始化。")
    return RttTransportConfig(
        remote_peer=PeerAddress(
            network.remote_host.text(),
            int(network.remote_port.value()),
        ),
        channel=int(network.rtt_channel.currentData(Qt.ItemDataRole.UserRole)),
        connect_timeout=network.connect_timeout.value(),
        read_timeout=network.read_timeout.value(),
        write_timeout=network.write_timeout.value(),
    )


def build_udp_config(window) -> UdpTransportConfig:
    """Build udp config."""
    network = network_bindings_for(window)
    if network is None:
        raise ConfigurationError("网络控件尚未初始化。")
    return UdpTransportConfig(
        local_host=network.local_host.text(),
        local_port=int(network.local_port.value()),
        remote_peer=PeerAddress(
            network.remote_host.text(),
            int(network.remote_port.value()),
        ),
        read_timeout=network.read_timeout.value(),
        write_timeout=network.write_timeout.value(),
        max_datagram_size=int(network.udp_limit.value()),
    )


def build_server_config(window) -> TcpServerTransportConfig:
    """Build server config."""
    network = network_bindings_for(window)
    if network is None:
        raise ConfigurationError("网络控件尚未初始化。")
    allowlist = tuple(
        item.strip()
        for item in network.server_allowlist.toPlainText().splitlines()
        if item.strip()
    )
    return TcpServerTransportConfig(
        bind_host=network.local_host.text(),
        listen_port=int(network.local_port.value()),
        accept_timeout=network.connect_timeout.value(),
        read_timeout=network.read_timeout.value(),
        write_timeout=network.write_timeout.value(),
        max_clients=int(network.server_max_clients.value()),
        allowlist=allowlist,
        lan_confirmed=network.server_lan_confirm.isChecked(),
    )


def invalidate_server_lan_authorization(window) -> None:
    """Invalidate LAN approval whenever the listener identity changes."""

    network = network_bindings_for(window)
    if network is None:
        return
    allowlist = network.server_allowlist
    lan_confirm = network.server_lan_confirm
    allowlist_signals_blocked = allowlist.blockSignals(True)
    confirm_signals_blocked = lan_confirm.blockSignals(True)
    try:
        allowlist.clear()
        lan_confirm.setChecked(False)
    finally:
        allowlist.blockSignals(allowlist_signals_blocked)
        lan_confirm.blockSignals(confirm_signals_blocked)


def build_ble_discovery_config(window) -> BleGattDiscoveryConfig:
    """Build ble discovery config."""
    ble = ble_bindings_for(window)
    if ble is None:
        raise ConfigurationError("BLE 控件尚未初始化。")
    return BleGattDiscoveryConfig(
        scan_timeout=ble.scan_timeout.value(),
        name_filter=ble.name_filter.text(),
        service_uuids=parse_ble_uuid_filter(ble.service_filter.text()),
    )


def build_ble_config(window) -> BleGattTransportConfig:
    """Build ble config."""
    ble = ble_bindings_for(window)
    if ble is None:
        raise ConfigurationError("BLE 控件尚未初始化。")
    device = ble.device_combo.currentData(Qt.ItemDataRole.UserRole)
    if not isinstance(device, BleGattDevice):
        raise ConfigurationError("请先扫描并选择 BLE 设备。")
    return BleGattTransportConfig(
        device_id=device.device_id,
        device_name=device.name,
        connect_timeout=ble.connect_timeout.value(),
        pair=ble.pair.isChecked(),
        use_cached_services=ble.cached_services.currentData(Qt.ItemDataRole.UserRole),
        service_uuids=parse_ble_uuid_filter(ble.service_filter.text()),
    )


@staticmethod
def parse_ble_uuid_filter(value: str) -> tuple[str, ...]:
    """Parse ble uuid filter."""
    return tuple(item.strip() for item in value.replace(",", "\n").splitlines() if item.strip())


def on_transport_changed(window, _index: int = -1) -> None:
    """On transport changed."""
    if window._closing:
        return
    uart = uart_bindings_for(window)
    network = network_bindings_for(window)
    ble = ble_bindings_for(window)
    shell = connection_shell_bindings_for(window)
    if uart is None or network is None or ble is None or shell is None:
        return
    invalidate_server_lan_authorization(window)
    window._reset_data_activity()
    kind = TransportKind(shell.transport_combo.currentData())
    selected_preset = shell.preset_combo.currentData(Qt.ItemDataRole.UserRole)
    if isinstance(selected_preset, ConnectionPreset) and selected_preset.transport is not kind:
        shell.preset_combo.setCurrentIndex(0)
    shell.transport_mode_surface.set_mode(kind.value)
    is_uart = kind == TransportKind.UART
    is_tcp = kind == TransportKind.TCP_STREAM
    is_server = kind == TransportKind.TCP_SERVER
    is_udp = kind == TransportKind.UDP_DATAGRAM
    is_ble = kind == TransportKind.BLE_GATT
    is_rtt = kind == TransportKind.RTT
    uart.panel.setVisible(is_uart)
    network.panel.setVisible(not is_uart and not is_ble)
    ble.panel.setVisible(is_ble)
    uart.title.setVisible(is_uart)
    network.title.setVisible(not is_uart and not is_ble)
    ble.title.setVisible(is_ble)
    active_panel = (
        uart.panel
        if is_uart
        else ble.panel
        if is_ble
        else network.panel
    )
    start_transport_panel_transition(window, active_panel)
    is_remote = is_tcp or is_udp or is_rtt
    network.remote_host_label.setVisible(is_remote)
    network.remote_host.setVisible(is_remote)
    network.remote_port_label.setVisible(is_remote)
    network.remote_port.setVisible(is_remote)
    for widget in (
        network.local_host_label,
        network.local_host,
        network.local_port_label,
        network.local_port,
    ):
        widget.setVisible(is_udp or is_server)
    for widget in (
        network.udp_limit_label,
        network.udp_limit,
    ):
        widget.setVisible(is_udp)
    network.rtt_channel_label.setVisible(is_rtt)
    network.rtt_channel.setVisible(is_rtt)
    network.rtt_hint.setVisible(is_rtt)
    network.server_allowlist_label.setVisible(is_server)
    network.server_allowlist.setVisible(is_server)
    network.server_lan_confirm.setVisible(is_server)
    for widget in (network.server_max_clients_label, network.server_max_clients):
        widget.setVisible(is_server)
    network.server_peer_label.setVisible(is_server)
    network.server_peer_combo.setVisible(is_server)
    if is_server:
        if not window._server_defaults_applied:
            network.local_host.setText("127.0.0.1")
            if network.local_port.value() == 0:
                network.local_port.setValue(9_000)
            window._server_defaults_applied = True
        network.local_host_label.setText("监听主机")
        network.local_port_label.setText("监听端口")
    else:
        network.local_host_label.setText("本地绑定")
        network.local_port_label.setText("本地端口")
    if is_rtt and not window._rtt_defaults_applied:
        network.remote_host.setText("127.0.0.1")
        if network.remote_port.value() == 9_000:
            network.remote_port.setValue(DEFAULT_RTT_TELNET_PORT)
        window._rtt_defaults_applied = True
    network.connect_timeout_label.setText("接收超时" if is_server else "连接超时")
    network.connect_timeout_label.setVisible(is_tcp or is_server or is_rtt)
    network.connect_timeout.setVisible(is_tcp or is_server or is_rtt)
    network.hint.setText(
        "TCP 是连续字节流；一次读取不代表消息边界。"
        if is_tcp
        else (
            "TCP Server 默认只监听 127.0.0.1；非回环监听必须确认 LAN 并填写 allowlist。"
            if is_server
            else (
                "RTT 是已有 J-Link Telnet 服务的原始字节桥接；不启动 SEGGER 工具。"
                if is_rtt
                else "UDP 是固定远端单播；每次接收保留一个 datagram 边界。"
            )
        )
    )
    network.hint.setAccessibleDescription(network.hint.text())
    network.hint.setToolTip(network.hint.text())
    window._update_protocol_scope()
    source_supported = derived_source_supported(window)
    window._set_protocol_controls_enabled(source_supported)
    window._set_derived_controls_enabled(source_supported)
    window._update_component_status()
    window._update_dataset_status()
    window._update_dataset_curve_status(window._dataset_curve.snapshot)
    window._update_connection_context()
    window._update_pipeline_summary()
    window._refresh_connection_controls(window._view_model.state)

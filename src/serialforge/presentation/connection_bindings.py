"""Typed presentation wiring for the UART connection surface."""

from __future__ import annotations

from dataclasses import dataclass

from .action_surface import ActionRailButton, BusyActionButton
from .bounded_value_combo import BoundedFloatCombo, BoundedIntCombo
from .connection_preset_context_surface import ConnectionPresetContextSurface
from .connection_status_surface import ConnectionStatusRail
from .qt import (
    QCheckBox,
    QComboBox,
    QLabel,
    QLineEdit,
    QPlainTextEdit,
    QSpinBox,
    QWidget,
)
from .transport_mode_surface import TransportModeSurface
from .uart_timing_surface import UartTimingSummarySurface


@dataclass(frozen=True, slots=True)
class UartControlBindings:
    """Immutable references for one composed UART form."""

    panel: QWidget
    title: QLabel
    port_combo: QComboBox
    refresh_button: BusyActionButton
    baud_combo: QComboBox
    data_bits: QComboBox
    parity: QComboBox
    stop_bits: QComboBox
    flow_control: QComboBox
    timing_summary: UartTimingSummarySurface
    read_timeout: BoundedFloatCombo
    write_timeout: BoundedFloatCombo
    inter_byte_timeout: BoundedFloatCombo
    exclusive: QCheckBox
    dtr: QCheckBox
    rts: QCheckBox


@dataclass(frozen=True, slots=True)
class NetworkControlBindings:
    """Immutable references for the shared TCP/UDP/RTT form."""

    panel: QWidget
    title: QLabel
    remote_host_label: QLabel
    remote_host: QLineEdit
    remote_port_label: QLabel
    remote_port: QSpinBox
    local_host_label: QLabel
    local_host: QLineEdit
    local_port_label: QLabel
    local_port: QSpinBox
    connect_timeout_label: QLabel
    connect_timeout: BoundedFloatCombo
    read_timeout: BoundedFloatCombo
    write_timeout: BoundedFloatCombo
    udp_limit_label: QLabel
    udp_limit: BoundedIntCombo
    hint: QLabel
    rtt_channel_label: QLabel
    rtt_channel: QComboBox
    rtt_hint: QLabel
    server_allowlist_label: QLabel
    server_allowlist: QPlainTextEdit
    server_lan_confirm: QCheckBox
    server_max_clients_label: QLabel
    server_max_clients: BoundedIntCombo
    server_peer_label: QLabel
    server_peer_combo: QComboBox


@dataclass(frozen=True, slots=True)
class BleControlBindings:
    """Immutable references for the composed BLE GATT form."""

    panel: QWidget
    title: QLabel
    hint: QLabel
    scan_timeout: BoundedFloatCombo
    scan_button: BusyActionButton
    name_filter: QLineEdit
    service_filter: QLineEdit
    device_combo: QComboBox
    connect_timeout: BoundedFloatCombo
    pair: QCheckBox
    cached_services: QComboBox
    characteristic_combo: QComboBox
    characteristic_properties: QLabel
    read_button: ActionRailButton
    notify_check: QCheckBox
    write_mode: QComboBox


@dataclass(frozen=True, slots=True)
class ConnectionShellBindings:
    """Immutable references for the shared transport/preset shell."""

    control_band: QWidget
    transport_mode_surface: TransportModeSurface
    transport_combo: QComboBox
    preset_combo: QComboBox
    preset_context: ConnectionPresetContextSurface
    save_preset_button: ActionRailButton
    delete_preset_button: ActionRailButton
    status_rail: ConnectionStatusRail
    connect_button: BusyActionButton

    @property
    def hint_label(self) -> QLabel:
        """Expose the existing connection hint contract from its owner surface."""

        return self.preset_context.hint_label


def uart_bindings_for(window: object) -> UartControlBindings | None:
    """Read the typed UART bundle without importing the MainWindow class."""

    bindings = getattr(window, "_uart_bindings", None)
    return bindings if isinstance(bindings, UartControlBindings) else None


def network_bindings_for(window: object) -> NetworkControlBindings | None:
    """Read the typed network bundle without importing the MainWindow class."""

    bindings = getattr(window, "_network_bindings", None)
    return bindings if isinstance(bindings, NetworkControlBindings) else None


def ble_bindings_for(window: object) -> BleControlBindings | None:
    """Read the typed BLE bundle without importing the MainWindow class."""

    bindings = getattr(window, "_ble_bindings", None)
    return bindings if isinstance(bindings, BleControlBindings) else None


def connection_shell_bindings_for(window: object) -> ConnectionShellBindings | None:
    """Read the typed transport/preset shell without importing MainWindow."""

    bindings = getattr(window, "_connection_shell_bindings", None)
    return bindings if isinstance(bindings, ConnectionShellBindings) else None


__all__ = [
    "BleControlBindings",
    "ConnectionShellBindings",
    "NetworkControlBindings",
    "UartControlBindings",
    "ble_bindings_for",
    "connection_shell_bindings_for",
    "network_bindings_for",
    "uart_bindings_for",
]

"""Composition-root helpers for the desktop workspace.

The main window owns lifecycle and dependency wiring.  This module owns the
repeatable Qt composition details so the shell stays small and easy to audit.
Feature controllers receive the window boundary explicitly during the staged
migration; they do not create transports or reach into infrastructure.
"""

from __future__ import annotations

from functools import partial
from typing import TYPE_CHECKING

from ..bounded_value_combo import BoundedFloatCombo
from ..chrome_bindings import header_chrome_bindings_for
from ..command_bindings import command_batch_bindings_for
from ..connection_bindings import (
    ble_bindings_for,
    connection_shell_bindings_for,
    network_bindings_for,
    uart_bindings_for,
)
from ..contracts import ProtocolPanelCallbacks
from ..qt import (
    QComboBox,
    QFrame,
    QKeySequence,
    QShortcut,
    QSizePolicy,
    Qt,
    QVBoxLayout,
    QWidget,
)
from ..responsive_scroll_area import ResponsiveScrollArea, ShortPageVerticalRhythm
from ..terminal_bindings import terminal_bindings_for
from ..workspace_bindings import workspace_bindings_for
from .commands import send_current_action
from .protocol import build_protocol_panel

if TYPE_CHECKING:
    from ..main_window import MainWindow


def enum_combo(
    values: tuple[object, ...],
    *,
    labels: tuple[str, ...],
) -> QComboBox:
    """Build a non-editable option combo from a typed value/label pair."""

    combo = QComboBox()
    for value, label in zip(values, labels, strict=True):
        combo.addItem(label, value)
    return combo


def timeout_combo(
    value: float,
    maximum: float,
    *,
    minimum: float = 0.0,
) -> BoundedFloatCombo:
    """Build a non-editable timeout selector with stable second values."""

    combo = BoundedFloatCombo(minimum, maximum)
    combo.setValue(value)
    if minimum == 0.0:
        combo.setAccessibleDescription("可选择超时；选择“未设置”表示不额外限制该项超时。")
    return combo


def scroll_page(
    window: MainWindow,
    layout: QVBoxLayout,
    object_name: str,
    *,
    vertical_rhythm: ShortPageVerticalRhythm = ShortPageVerticalRhythm.CENTER,
) -> ResponsiveScrollArea:
    """Wrap a settings layout in the shared themed scroll surface."""

    content = QWidget()
    content.setObjectName(object_name)
    content.setProperty("role", "scrollContent")
    content.setSizePolicy(
        QSizePolicy.Policy.Expanding,
        QSizePolicy.Policy.Preferred,
    )
    layout.setContentsMargins(14, 14, 14, 14)
    layout.setSpacing(10)
    layout.setAlignment(Qt.AlignmentFlag.AlignTop)
    content.setLayout(layout)

    scroll = ResponsiveScrollArea(
        layout,
        window,
        vertical_rhythm=vertical_rhythm,
    )
    scroll.setObjectName("settingsScroll")
    scroll.setWidgetResizable(True)
    scroll.setFrameShape(QFrame.Shape.NoFrame)
    scroll.setHorizontalScrollBarPolicy(Qt.ScrollBarPolicy.ScrollBarAlwaysOff)
    viewport = scroll.viewport()
    viewport.setObjectName("settingsViewport")
    viewport.setProperty("role", "scrollViewport")
    scroll.setWidget(content)
    for widget in (viewport, content):
        style = widget.style()
        style.unpolish(widget)
        style.polish(widget)
    return scroll


def build_protocol_panel_for_window(window: MainWindow) -> QVBoxLayout:
    """Compose protocol, derived-data, and replay controls for the shell."""

    panel = build_protocol_panel(
        callbacks=ProtocolPanelCallbacks(
            on_protocol_preset_changed=window._on_protocol_preset_changed,
            on_protocol_framing_changed=window._on_protocol_framing_changed,
            on_apply_protocol_config=window._apply_protocol_config,
            on_reset_protocol=window._reset_protocol,
            on_mark_protocol_editor_dirty=window._mark_protocol_editor_dirty,
            on_load_component_codec=window._load_component_codec,
            on_component_filter_changed=window._on_component_filter_changed,
            on_export_component_csv=window._export_component_csv,
            on_load_dataset_config=window._load_dataset_config,
            on_export_dataset_csv=window._export_dataset_csv,
            on_curve_series_changed=window._on_curve_series_changed,
            on_start_replay=window._start_replay,
            on_toggle_replay_pause=window._toggle_replay_pause,
            on_stop_replay=window._view_model.stop_replay,
        ),
        register_status_surface=window._status_surfaces,
    )
    window._protocol_panel = panel
    for name in (
        "protocol_preset",
        "protocol_framing",
        "protocol_checksum",
        "protocol_max_frame",
        "protocol_apply_button",
        "protocol_reset_button",
        "protocol_status",
        "protocol_config_context",
        "pipeline_summary",
        "protocol_delimiter",
        "protocol_length_bytes",
        "protocol_byteorder",
        "protocol_scope",
        "protocol_timing_hint",
        "protocol_detail_layout",
        "component_profile_label",
        "load_profile_button",
        "component_filter",
        "export_component_button",
        "component_status",
        "component_table",
        "component_empty",
        "component_preview",
        "dataset_config_label",
        "load_dataset_button",
        "export_dataset_button",
        "dataset_status",
        "dataset_preview",
        "dataset_curve_series",
        "dataset_curve_status",
        "dataset_curve",
        "replay_speed",
        "replay_start_button",
        "replay_pause_button",
        "replay_stop_button",
        "replay_status",
    ):
        setattr(window, f"_{name}", getattr(panel, name))
    window._on_protocol_framing_changed()
    return panel.layout


def install_shortcuts(window: MainWindow) -> None:
    """Install the stable keyboard affordances for the workspace shell."""

    terminal = terminal_bindings_for(window)
    if terminal is None:
        return
    send_shortcut = QShortcut(QKeySequence("Ctrl+Enter"), terminal.send_input)
    send_shortcut.setContext(Qt.ShortcutContext.WidgetWithChildrenShortcut)
    send_shortcut.activated.connect(partial(send_current_action, window))
    QShortcut(QKeySequence("F5"), window, window._refresh_ports)
    QShortcut(QKeySequence("Ctrl+L"), window, window._view_model.clear_preview)
    QShortcut(QKeySequence("Ctrl+Shift+R"), window, window._toggle_recording)


def install_tab_order(window: MainWindow) -> None:
    """Keep the primary keyboard path stable across the dense workspace."""

    workspace = workspace_bindings_for(window)
    uart = uart_bindings_for(window)
    network = network_bindings_for(window)
    ble = ble_bindings_for(window)
    shell = connection_shell_bindings_for(window)
    chrome = header_chrome_bindings_for(window)
    terminal = terminal_bindings_for(window)
    batch = command_batch_bindings_for(window)
    if (
        workspace is None
        or uart is None
        or network is None
        or ble is None
        or shell is None
        or chrome is None
        or terminal is None
        or batch is None
    ):
        return
    order = (
        shell.transport_combo,
        shell.preset_combo,
        uart.port_combo,
        uart.refresh_button,
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
        network.rtt_channel,
        network.udp_limit,
        network.server_allowlist,
        network.server_lan_confirm,
        network.server_max_clients,
        network.server_peer_combo,
        ble.scan_timeout,
        ble.scan_button,
        ble.name_filter,
        ble.service_filter,
        ble.device_combo,
        ble.connect_timeout,
        ble.pair,
        ble.cached_services,
        ble.characteristic_combo,
        ble.read_button,
        ble.notify_check,
        ble.write_mode,
        shell.connect_button,
        window._clear_error_button,
        chrome.motion_check,
        chrome.motion_pause_check,
        chrome.theme_combo,
        workspace.tabs,
        workspace.focus_button,
        window._protocol_preset,
        window._protocol_framing,
        window._protocol_checksum,
        window._protocol_max_frame,
        window._protocol_apply_button,
        window._protocol_reset_button,
        window._protocol_delimiter,
        window._protocol_length_bytes,
        window._protocol_byteorder,
        window._load_profile_button,
        window._component_filter,
        window._export_component_button,
        window._component_table,
        window._component_preview,
        window._load_dataset_button,
        window._export_dataset_button,
        window._dataset_curve_series,
        window._dataset_preview,
        window._dataset_curve,
        window._replay_speed,
        window._replay_start_button,
        window._replay_pause_button,
        window._replay_stop_button,
        terminal.history_combo,
        batch.combo,
        terminal.clear_history_button,
        batch.new_button,
        batch.edit_button,
        batch.delete_button,
        batch.run_button,
        batch.stop_button,
        batch.empty_action_button,
        terminal.display_mode,
        terminal.pause_check,
        terminal.clear_terminal_button,
        terminal.record_button,
        terminal.terminal,
        terminal.send_mode,
        terminal.send_input,
        terminal.newline_check,
        terminal.send_button,
        terminal.quick_button,
        terminal.save_quick_button,
    )
    for first, second in zip(order, order[1:], strict=False):
        window.setTabOrder(first, second)

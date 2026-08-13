"""Application-window bootstrap and signal wiring.

The bootstrap boundary is deliberately boring: construct shell state, compose
the view, connect ViewModel signals, then perform the initial render.  Feature
behavior remains in the dedicated controllers and application services.
"""

from __future__ import annotations

from functools import partial
from typing import TYPE_CHECKING

from ...domain.commands import CommandBatch
from ...domain.components import ComponentFrameRow
from ...domain.datasets import DatasetSample
from ...domain.models import BleGattCharacteristicRef, CommandEntry, Endpoint, SessionState
from ...domain.protocols import DecodedFrame
from ..command_bindings import CommandBatchControlBindings
from ..connection_preset_store import ConnectionPresetCatalogStore
from ..connection_presets import ConnectionPresetCatalog
from ..motion_transition import MotionDrivenAnimationGroup
from ..popup_surface import refresh_combo_popup_themes
from ..preferences import PreferenceStore
from ..property_refresh import refresh_dynamic_property
from ..qt import (
    QFrame,
    QGraphicsOpacityEffect,
    QGridLayout,
    QTimer,
    QVBoxLayout,
    QWidget,
)
from ..status_footer_surface import StatusFooterSurface
from ..terminal_bindings import TerminalControlBindings
from ..terminal_surface import TerminalEmptyState, TerminalViewport
from ..theme import apply_theme, theme_key_for_widget
from ..widgets import MotionController
from .ble import (
    on_ble_characteristic_changed,
    on_ble_device_changed,
    on_ble_notification_timeout,
    on_ble_write_mode_changed,
    read_ble_characteristic,
    request_ble_notifications,
    scan_ble,
    toggle_ble_notifications,
)
from .commands import on_command_batch_changed, on_command_batches_changed
from .composition import install_shortcuts, install_tab_order
from .connection import update_connection_controls
from .connection_presets import hydrate_recommended_connection_preset
from .connection_runtime import on_transport_changed
from .derived_data import (
    export_component_csv,
    export_dataset_csv,
    flush_component_rows,
    load_component_codec,
    load_dataset_config,
    on_component_filter_changed,
    on_component_profile_changed,
    on_component_rows_changed,
    on_component_stats_changed,
    on_curve_series_changed,
    on_dataset_config_changed,
    on_dataset_curve_snapshot_changed,
    on_dataset_samples_changed,
    on_dataset_stats_changed,
    on_protocol_config_changed,
    on_protocol_frames_changed,
    on_protocol_stats_changed,
    refresh_dataset_curve,
    renderers_suspended,
    rerender_component_preview,
    rerender_component_table,
    rerender_dataset_preview,
    update_component_status,
    update_dataset_curve_status,
    update_dataset_status,
)
from .lifecycle import (
    on_ble_scan_busy_changed,
    on_connection_endpoint_changed,
    on_discovery_busy_changed,
    on_error_changed,
    on_port_text_changed,
    on_send_input_changed,
    on_send_ready_changed,
    on_status_changed,
    refresh_live_observation_state,
    refresh_ports,
    refresh_station_band_source,
    rollback_ble_notification,
    show_local_error,
    update_connection_context,
    update_pipeline_summary,
    update_source_badge,
    update_state_badge,
)
from .protocol_config import (
    apply_protocol_config,
    confirm_protocol_change,
    current_modbus_timing,
    mark_protocol_editor_dirty,
    on_protocol_framing_changed,
    on_protocol_preset_changed,
    on_uart_timing_changed,
    reset_protocol,
    select_protocol_preset,
    set_derived_controls_enabled,
    set_protocol_controls_enabled,
    set_protocol_editor,
    set_protocol_status,
    set_protocol_timing_text,
    update_protocol_scope,
    update_protocol_timing_hint,
)
from .replay import on_replay_changed, start_replay, toggle_replay_pause
from .send_bar_builder import build_send_bar
from .status_surfaces import StatusSurfaceController, status_surface_source
from .terminal import build_error_bar
from .terminal_runtime import (
    flush_preview_renders,
    load_entry,
    load_history,
    on_ble_devices_changed,
    on_ble_services_changed,
    on_ble_subscription_changed,
    on_data_activity_changed,
    on_data_received,
    on_endpoints_changed,
    on_history_changed,
    on_paused_bytes_changed,
    on_preview_snapshot,
    on_quick_commands_changed,
    on_recording_changed,
    on_server_peers_changed,
    on_server_target_changed,
    on_session_state_changed,
    rerender_preview,
    reset_data_activity,
    save_current_quick,
    schedule_preview_render,
    toggle_recording,
    update_data_activity,
)
from .terminal_toolbar_builder import build_terminal_toolbar
from .workspace import build_app_header, build_workspace_tabs
from .workspace_runtime import open_connection_setup, start_connection_onboarding

if TYPE_CHECKING:
    from ..main_window import MainWindow
    from ..viewmodels import SessionViewModel


def _bind_protocol_callbacks(window: MainWindow) -> None:
    """Bind protocol owner functions without exposing MainWindow facades."""

    window._on_protocol_framing_changed = partial(on_protocol_framing_changed, window)
    window._update_protocol_scope = partial(update_protocol_scope, window)
    window._set_protocol_controls_enabled = partial(set_protocol_controls_enabled, window)
    window._set_derived_controls_enabled = partial(set_derived_controls_enabled, window)
    window._reset_protocol = partial(reset_protocol, window)
    window._set_protocol_status = partial(set_protocol_status, window)
    window._confirm_protocol_change = partial(confirm_protocol_change, window)
    window._mark_protocol_editor_dirty = partial(mark_protocol_editor_dirty, window)
    window._current_modbus_timing = partial(current_modbus_timing, window)
    window._on_uart_timing_changed = partial(on_uart_timing_changed, window)
    window._update_protocol_timing_hint = partial(update_protocol_timing_hint, window)
    window._set_protocol_timing_text = partial(set_protocol_timing_text, window)
    window._on_protocol_preset_changed = partial(on_protocol_preset_changed, window)
    window._set_protocol_editor = partial(set_protocol_editor, window)
    window._select_protocol_preset = partial(select_protocol_preset, window)
    window._apply_protocol_config = partial(apply_protocol_config, window)


def _bind_derived_callbacks(window: MainWindow) -> None:
    """Bind derived-data owner functions at the composition boundary."""

    window._on_protocol_config_changed = partial(on_protocol_config_changed, window)
    window._on_protocol_frames_changed = partial(on_protocol_frames_changed, window)
    window._rerender_component_preview = partial(rerender_component_preview, window)
    window._on_protocol_stats_changed = partial(on_protocol_stats_changed, window)
    window._load_component_codec = partial(load_component_codec, window)
    window._export_component_csv = partial(export_component_csv, window)
    window._load_dataset_config = partial(load_dataset_config, window)
    window._export_dataset_csv = partial(export_dataset_csv, window)
    window._on_component_profile_changed = partial(on_component_profile_changed, window)
    window._on_component_rows_changed = partial(on_component_rows_changed, window)
    window._renderers_suspended = partial(renderers_suspended, window)
    window._on_component_filter_changed = partial(on_component_filter_changed, window)
    window._flush_component_rows = partial(flush_component_rows, window)
    window._on_component_stats_changed = partial(on_component_stats_changed, window)
    window._update_component_status = partial(update_component_status, window)
    window._on_dataset_config_changed = partial(on_dataset_config_changed, window)
    window._on_dataset_samples_changed = partial(on_dataset_samples_changed, window)
    window._on_dataset_stats_changed = partial(on_dataset_stats_changed, window)
    window._update_dataset_status = partial(update_dataset_status, window)
    window._rerender_dataset_preview = partial(rerender_dataset_preview, window)
    window._on_curve_series_changed = partial(on_curve_series_changed, window)
    window._refresh_dataset_curve = partial(refresh_dataset_curve, window)
    window._on_dataset_curve_snapshot_changed = partial(on_dataset_curve_snapshot_changed, window)
    window._update_dataset_curve_status = partial(update_dataset_curve_status, window)
    window._rerender_component_table = partial(rerender_component_table, window)


def _bind_replay_callbacks(window: MainWindow) -> None:
    """Bind replay owner functions at the composition boundary."""

    window._start_replay = partial(start_replay, window)
    window._toggle_replay_pause = partial(toggle_replay_pause, window)
    window._on_replay_changed = partial(on_replay_changed, window)


def _bind_ble_callbacks(window: MainWindow) -> None:
    """Bind BLE owner functions at the composition boundary."""

    window._scan_ble = partial(scan_ble, window)
    window._read_ble_characteristic = partial(read_ble_characteristic, window)
    window._toggle_ble_notifications = partial(toggle_ble_notifications, window)
    window._request_ble_notifications = partial(request_ble_notifications, window)
    window._on_ble_notification_timeout = partial(on_ble_notification_timeout, window)
    window._on_ble_device_changed = partial(on_ble_device_changed, window)
    window._on_ble_characteristic_changed = partial(on_ble_characteristic_changed, window)
    window._on_ble_write_mode_changed = partial(on_ble_write_mode_changed, window)


def _bind_terminal_callbacks(window: MainWindow) -> None:
    """Bind terminal and recording owner functions at the composition boundary."""

    window._toggle_recording = partial(toggle_recording, window)
    window._save_current_quick = partial(save_current_quick, window)
    window._on_endpoints_changed = partial(on_endpoints_changed, window)
    window._on_ble_devices_changed = partial(on_ble_devices_changed, window)
    window._on_ble_services_changed = partial(on_ble_services_changed, window)
    window._on_ble_subscription_changed = partial(on_ble_subscription_changed, window)
    window._on_server_peers_changed = partial(on_server_peers_changed, window)
    window._on_server_target_changed = partial(on_server_target_changed, window)
    window._schedule_preview_render = partial(schedule_preview_render, window)
    window._flush_preview_renders = partial(flush_preview_renders, window)
    window._on_data_received = partial(on_data_received, window)
    window._on_data_activity_changed = partial(on_data_activity_changed, window)
    window._on_preview_snapshot = partial(on_preview_snapshot, window)
    window._rerender_preview = partial(rerender_preview, window)
    window._reset_data_activity = partial(reset_data_activity, window)
    window._update_data_activity = partial(update_data_activity, window)
    window._on_paused_bytes_changed = partial(on_paused_bytes_changed, window)
    window._on_recording_changed = partial(on_recording_changed, window)
    window._on_history_changed = partial(on_history_changed, window)
    window._on_quick_commands_changed = partial(on_quick_commands_changed, window)
    window._load_history = partial(load_history, window)
    window._load_entry = partial(load_entry, window)
    window._on_session_state_changed = partial(on_session_state_changed, window)


def _bind_lifecycle_callbacks(window: MainWindow) -> None:
    """Bind shell/lifecycle owner functions at the composition boundary."""

    window._on_send_ready_changed = partial(on_send_ready_changed, window)
    window._on_send_input_changed = partial(on_send_input_changed, window)
    window._update_source_badge = partial(update_source_badge, window)
    window._refresh_station_band_source = partial(refresh_station_band_source, window)
    window._refresh_live_observation_state = partial(refresh_live_observation_state, window)
    window._update_pipeline_summary = partial(update_pipeline_summary, window)
    window._update_connection_context = partial(update_connection_context, window)
    window._update_state_badge = partial(update_state_badge, window)
    window._refresh_ports = partial(refresh_ports, window)
    window._on_status_changed = partial(on_status_changed, window)
    window._on_discovery_busy_changed = partial(on_discovery_busy_changed, window)
    window._on_ble_scan_busy_changed = partial(on_ble_scan_busy_changed, window)
    window._on_connection_endpoint_changed = partial(on_connection_endpoint_changed, window)
    window._on_port_text_changed = partial(on_port_text_changed, window)
    window._rollback_ble_notification = partial(rollback_ble_notification, window)
    window._on_error_changed = partial(on_error_changed, window)
    window._show_local_error = partial(show_local_error, window)


def _bind_controller_callbacks(window: MainWindow) -> None:
    """Install explicit feature callbacks before any Qt signal can fire."""

    window._on_transport_changed = partial(on_transport_changed, window)
    window._refresh_connection_controls = partial(update_connection_controls, window)
    _bind_protocol_callbacks(window)
    _bind_derived_callbacks(window)
    _bind_replay_callbacks(window)
    _bind_ble_callbacks(window)
    _bind_terminal_callbacks(window)
    _bind_lifecycle_callbacks(window)


def initialize_window(
    window: MainWindow,
    view_model: SessionViewModel,
    preference_store: PreferenceStore,
    preset_catalog: ConnectionPresetCatalog,
    preset_store: ConnectionPresetCatalogStore,
) -> None:
    """Initialize the desktop shell around an already-created ViewModel."""

    window._view_model = view_model
    window._preference_store = preference_store
    window._connection_preset_catalog = preset_catalog
    window._connection_preset_store = preset_store
    window._presentation_preferences = preference_store.load()
    window._preferences_hydrating = True
    window._last_non_error_status = view_model.status if view_model.error_info is None else ""
    window._endpoints: tuple[Endpoint, ...] = ()
    window._history_entries: tuple[CommandEntry, ...] = ()
    window._quick_entries: tuple[CommandEntry, ...] = ()
    window._command_batches: tuple[CommandBatch, ...] = ()
    window._command_batch_snapshot = view_model.command_batch_snapshot
    window._preview_buffer = bytearray()
    window._last_data_activity_bytes = 0
    window._closing = False
    _bind_controller_callbacks(window)
    window._server_defaults_applied = False
    window._rtt_defaults_applied = False
    window._server_target_explicit = False
    window._ble_notification_ref: BleGattCharacteristicRef | None = None
    window._ble_notification_pending: tuple[BleGattCharacteristicRef, bool] | None = None
    window._ble_notification_timer = QTimer(window)
    window._ble_notification_timer.setSingleShot(True)
    window._ble_notification_timer.setInterval(3_000)
    window._ble_notification_timer.timeout.connect(window._on_ble_notification_timeout)
    window._dataset_curve_field: str | None = None
    window._dataset_curve_samples: tuple[DatasetSample, ...] = ()
    window._protocol_frames_latest: tuple[DecodedFrame, ...] = ()
    window._derived_pipeline_enabled: bool | None = None
    window._dataset_stats_latest = view_model.dataset_stats
    window._component_rows_pending: tuple[ComponentFrameRow, ...] | None = None
    window._component_stats_latest = view_model.component_stats
    window._component_render_timer = QTimer(window)
    window._component_render_timer.setSingleShot(True)
    window._component_render_timer.setInterval(100)
    window._component_render_timer.timeout.connect(window._flush_component_rows)
    window._preview_render_pending = False
    window._preview_render_timer = QTimer(window)
    window._preview_render_timer.setSingleShot(True)
    window._preview_render_timer.setInterval(80)
    window._preview_render_timer.timeout.connect(window._flush_preview_renders)
    window._history_source_active = False
    window._history_file_selected = False
    window._protocol_editor_dirty = False
    window._protocol_editor_syncing = False
    window._status_surfaces = StatusSurfaceController(
        source=lambda: status_surface_source(window),
        refresh_property=refresh_dynamic_property,
    )
    window._theme_transition: MotionDrivenAnimationGroup | None = None
    window._theme_transition_effect: QGraphicsOpacityEffect | None = None
    window._theme_transition_host: QWidget | None = None
    window._workspace_transition: MotionDrivenAnimationGroup | None = None
    window._workspace_transition_effect: QGraphicsOpacityEffect | None = None
    window._workspace_focus_transition: MotionDrivenAnimationGroup | None = None
    window._workspace_focus_mode = False
    window._connection_focus_override: bool | None = None
    window._connection_onboarding = False
    window._transport_panel_transition: MotionDrivenAnimationGroup | None = None
    window._transport_panel_transition_effect: QGraphicsOpacityEffect | None = None
    window._motion_controller = MotionController(window)

    apply_theme(window, window._presentation_preferences.theme_key)
    window.setWindowTitle("SerialForge · 星轨串行实验室")
    window.resize(1_240, 820)

    root = QWidget(window)
    root.setObjectName("appRoot")
    window._app_root = root
    root_layout = QVBoxLayout(root)
    root_layout.setContentsMargins(16, 16, 16, 12)
    root_layout.setSpacing(12)
    header = build_app_header(window)
    window._theme_transition_host = header
    root_layout.addWidget(header)
    root_layout.addWidget(build_error_bar(window))
    workspace_bindings = build_workspace_tabs(window)
    window._command_batch_bindings = CommandBatchControlBindings(
        combo=window._command_batch_combo,
        new_button=window._new_batch_button,
        edit_button=window._edit_batch_button,
        delete_button=window._delete_batch_button,
        run_button=window._run_batch_button,
        stop_button=window._stop_batch_button,
        status=window._command_batch_status,
        results=window._command_batch_results,
        empty_state=window._command_batch_empty,
    )
    root_layout.addWidget(workspace_bindings.shell)
    root_layout.addWidget(build_terminal_toolbar(window))

    terminal_surface = QFrame(root)
    terminal_surface.setObjectName("terminalSurface")
    terminal_layout = QGridLayout(terminal_surface)
    terminal_layout.setContentsMargins(0, 0, 0, 0)
    window._terminal = TerminalViewport(terminal_surface)
    window._terminal.setObjectName("terminal")
    window._terminal.setReadOnly(True)
    window._terminal.setAccessibleName("串口终端预览")
    window._terminal.setAccessibleDescription("显示有界的接收预览；暂停显示不会停止接收或记录。")
    window._terminal.setPlaceholderText("")
    window._terminal.setMaximumBlockCount(10_000)
    terminal_layout.addWidget(window._terminal, 0, 0)
    window._terminal_empty_state = TerminalEmptyState(terminal_surface)
    terminal_layout.addWidget(window._terminal_empty_state, 0, 0)
    window._terminal_empty_state.raise_()
    window._terminal_empty_state.connection_requested.connect(
        partial(open_connection_setup, window)
    )
    window._terminal_surface = terminal_surface
    root_layout.addWidget(terminal_surface, stretch=1)
    root_layout.addWidget(build_send_bar(window))
    window._terminal_bindings = TerminalControlBindings(
        live_observation_band=window._live_observation_band,
        terminal_surface=terminal_surface,
        terminal=window._terminal,
        terminal_empty_state=window._terminal_empty_state,
        display_mode=window._display_mode,
        pause_check=window._pause_check,
        paused_label=window._paused_label,
        clear_terminal_button=window._clear_terminal_button,
        record_button=window._record_button,
        record_label=window._record_label,
        data_activity_label=window._data_activity_label,
        send_control_band=window._send_control_band,
        send_mode=window._send_mode,
        send_input=window._send_input,
        newline_check=window._newline_check,
        send_button=window._send_button,
        send_state_label=window._send_state_label,
        send_context_surface=window._send_context_surface,
        quick_button=window._quick_button,
        quick_menu=window._quick_menu,
        save_quick_button=window._save_quick_button,
        history_combo=window._history_combo,
        clear_history_button=window._clear_history_button,
    )
    window.setCentralWidget(root)
    refresh_combo_popup_themes(window, theme_key_for_widget(window))
    window._on_transport_changed()
    hydrate_recommended_connection_preset(window)

    status_bar = window.statusBar()
    window._status_footer_surface = StatusFooterSurface(status_bar)
    window._status_footer_surface.setObjectName("statusFooterSurface")
    window._status_footer_surface.set_state(view_model.state)
    status_bar.addPermanentWidget(window._status_footer_surface)
    status_bar.showMessage(view_model.status)
    view_model.status_changed.connect(window._on_status_changed)
    view_model.error_changed.connect(window._on_error_changed)
    view_model.discovery_busy_changed.connect(window._on_discovery_busy_changed)
    view_model.ble_scan_busy_changed.connect(window._on_ble_scan_busy_changed)
    view_model.endpoints_changed.connect(window._on_endpoints_changed)
    view_model.data_batch_received.connect(window._on_data_received)
    view_model.data_activity_changed.connect(window._on_data_activity_changed)
    view_model.preview_snapshot_changed.connect(window._on_preview_snapshot)
    view_model.paused_bytes_changed.connect(window._on_paused_bytes_changed)
    view_model.session_state_changed.connect(window._on_session_state_changed)
    view_model.send_ready_changed.connect(window._on_send_ready_changed)
    view_model.server_peers_changed.connect(window._on_server_peers_changed)
    view_model.ble_devices_changed.connect(window._on_ble_devices_changed)
    view_model.ble_services_changed.connect(window._on_ble_services_changed)
    view_model.ble_subscription_changed.connect(window._on_ble_subscription_changed)
    view_model.protocol_config_changed.connect(window._on_protocol_config_changed)
    view_model.protocol_frames_changed.connect(window._on_protocol_frames_changed)
    view_model.protocol_stats_changed.connect(window._on_protocol_stats_changed)
    view_model.component_profile_changed.connect(window._on_component_profile_changed)
    view_model.component_rows_changed.connect(window._on_component_rows_changed)
    view_model.component_stats_changed.connect(window._on_component_stats_changed)
    view_model.dataset_config_changed.connect(window._on_dataset_config_changed)
    view_model.dataset_samples_changed.connect(window._on_dataset_samples_changed)
    view_model.dataset_stats_changed.connect(window._on_dataset_stats_changed)
    view_model.replay_changed.connect(window._on_replay_changed)
    view_model.recording_changed.connect(window._on_recording_changed)
    view_model.history_changed.connect(window._on_history_changed)
    view_model.quick_commands_changed.connect(window._on_quick_commands_changed)
    view_model.command_batches_changed.connect(partial(on_command_batches_changed, window))
    view_model.command_batch_changed.connect(partial(on_command_batch_changed, window))

    window._on_recording_changed(view_model.recording)
    window._on_history_changed(view_model.history_entries)
    window._on_quick_commands_changed(view_model.quick_command_entries)
    on_command_batches_changed(window, view_model.command_batches)
    on_command_batch_changed(window, view_model.command_batch_snapshot)
    window._on_server_peers_changed(view_model.server_peers)
    window._on_ble_devices_changed(view_model.ble_devices)
    window._on_ble_services_changed((view_model.ble_services, view_model.ble_mtu_size))
    window._on_protocol_config_changed(view_model.protocol_config)
    window._on_protocol_stats_changed(view_model.protocol_stats)
    window._on_component_profile_changed(view_model.component_profile)
    window._on_component_rows_changed(view_model.component_rows)
    window._on_component_stats_changed(view_model.component_stats)
    window._on_dataset_config_changed(view_model.dataset_config)
    window._on_dataset_samples_changed(view_model.dataset_samples)
    window._on_dataset_stats_changed(view_model.dataset_stats)
    window._on_replay_changed(view_model.replay_snapshot)
    window._on_error_changed(view_model.error_info)
    window._update_data_activity()
    install_shortcuts(window)
    install_tab_order(window)
    window._refresh_connection_controls(view_model.state)
    window._update_state_badge(view_model.state)
    window._refresh_ports()
    window._preferences_hydrating = False
    if view_model.state is SessionState.CLOSED:
        start_connection_onboarding(window)

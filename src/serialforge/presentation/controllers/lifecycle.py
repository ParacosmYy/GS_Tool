"""Shell status, theme, lifecycle, and connection context controller.

This controller owns presentation-only status aggregation and window lifecycle
hooks. Session shutdown remains delegated to the application view-model port.
"""

from __future__ import annotations

import logging
from dataclasses import replace
from functools import partial

from ...domain.errors import ErrorCode, ErrorInfo
from ...domain.models import (
    BleGattCharacteristic,
    BleGattDevice,
    RecordingState,
    SessionState,
    TransportKind,
)
from ...domain.protocols import FramingKind
from ...domain.replay import ReplayState
from ..chrome_bindings import header_chrome_bindings_for
from ..connection_bindings import (
    ble_bindings_for,
    connection_shell_bindings_for,
    network_bindings_for,
    uart_bindings_for,
)
from ..preferences import PresentationPreferences
from ..property_refresh import refresh_dynamic_property
from ..protocol_scope import derived_source_supported
from ..qt import QEvent, QMainWindow, Qt, QTimer
from ..terminal_bindings import terminal_bindings_for
from ..theme import DEFAULT_THEME_KEY, apply_theme, theme_spec
from ..theme_transition import start_theme_transition, stop_theme_transition
from ..transition_coordinator import stop_shell_transitions
from ..workspace_bindings import workspace_bindings_for
from ..workspace_focus_transition import (
    refresh_workspace_focus_layout,
)
from ..workspace_tab_icons import refresh_workspace_tab_icons
from .connection_runtime import invalidate_server_lan_authorization
from .lifecycle_motion import (
    _stop_motion_surfaces,
    invalidate_motion_surface_snapshot,
    on_motion_frame,
    set_data_activity_motion,
)
from .workspace_runtime import (
    on_workspace_tab_changed,
    sync_compact_connection_focus,
    workspace_motion_enabled,
)

MAX_UI_DETAIL_CHARS = 8_192
LOGGER = logging.getLogger(__name__)

def _clip_text(value: object, limit: int) -> str:
    text = value if isinstance(value, str) else str(value)
    if len(text) <= limit:
        return text
    return f"{text[: max(1, limit - 1)]}…"


def _framing_display(window, value: FramingKind) -> str:
    """Resolve a protocol enum to the localized selector label."""

    index = window._protocol_framing.findData(value, Qt.ItemDataRole.UserRole)
    if index >= 0:
        return window._protocol_framing.itemText(index)
    return value.value


def _save_preferences(window, **changes: object) -> None:
    """Persist a typed snapshot without making storage part of lifecycle logic."""

    if getattr(window, "_preferences_hydrating", False):
        return
    current: PresentationPreferences = window._presentation_preferences
    try:
        updated = replace(current, **changes)
    except (TypeError, ValueError):
        return
    window._presentation_preferences = updated
    try:
        window._preference_store.save(updated)
    except (OSError, RuntimeError, TypeError, ValueError):
        LOGGER.warning("SerialForge presentation preference save failed", exc_info=True)


def on_send_ready_changed(window, _ready: bool) -> None:
    if window._closing:
        return
    window._refresh_connection_controls(window._view_model.state)


def on_motion_toggled(window, reduced: bool) -> None:
    """Pause only decorative motion; terminal and transport semantics stay intact."""

    window._motion_controller.set_motion_enabled(not reduced)
    _save_preferences(window, reduced_motion=reduced)
    if not workspace_motion_enabled(window):
        stop_shell_transitions(window)
        set_data_activity_motion(window, False)
    if reduced:
        stop_theme_transition(window)
        _stop_motion_surfaces(window)


def on_theme_changed(window, _index: int = -1) -> None:
    """Apply a named palette without changing transport or ViewModel state."""

    chrome = header_chrome_bindings_for(window)
    if chrome is None:
        return
    key = chrome.theme_combo.currentData(Qt.ItemDataRole.UserRole)
    theme_key = key if isinstance(key, str) else DEFAULT_THEME_KEY
    theme = theme_spec(theme_key)
    stop_shell_transitions(window, keep=None)
    apply_theme(window, theme.key)
    workspace = workspace_bindings_for(window)
    refresh_workspace_tab_icons(workspace.tabs if workspace is not None else None, theme)
    if (
        not window._closing
        and window.isVisible()
        and not window.isMinimized()
        and workspace_motion_enabled(window)
    ):
        start_theme_transition(
            window,
            target=getattr(window, "_app_root", None),
            sweep_host=getattr(window, "_theme_transition_host", None),
        )
    else:
        stop_theme_transition(window)
    _save_preferences(window, theme_key=theme.key)
    description = f"当前主题：{theme.label}。{theme.description}"
    chrome.theme_combo.setToolTip(description)
    chrome.theme_combo.setAccessibleDescription(description)


def on_motion_pause_toggled(window, paused: bool) -> None:
    """Keep the explicit animation pause separate from reduced-motion mode."""

    window._motion_controller.set_paused(paused)
    _save_preferences(window, motion_paused=paused)
    if not workspace_motion_enabled(window):
        stop_shell_transitions(window)
        set_data_activity_motion(window, False)
    if paused:
        stop_shell_transitions(window)
        _stop_motion_surfaces(window)


def on_send_input_changed(window, _text: str) -> None:
    """Refresh send affordances without changing wire payload semantics."""

    if not window._closing:
        window._refresh_connection_controls(window._view_model.state)


def update_source_badge(window) -> None:
    """Make the origin of terminal and derived data visible at a glance."""

    state = window._view_model.state
    if window._history_source_active:
        text = "历史回放"
        description = "当前终端与派生数据可能来自历史 JSONL；回放不会连接设备。"
    elif window._history_file_selected:
        text = "历史 · 无可用 RX"
        description = "已选择历史 JSONL，但没有可进入派生管线的 UART/TCP Client RX 记录。"
    elif state is SessionState.OPEN:
        if derived_source_supported(window):
            text = "实时会话"
            description = "当前终端与派生数据来自实时传输会话。"
        else:
            text = "实时 · 仅原始终端"
            description = (
                "当前实时传输仅进入原始终端和记录；协议、组件、Dataset、曲线不适用。"
            )
    elif state in {SessionState.OPENING, SessionState.CLOSING}:
        text = "实时 · 连接流程"
        description = "当前正在建立或关闭实时传输会话。"
    else:
        text = "实时 · 未连接"
        description = "尚未连接实时传输，也未选择历史回放。"
    chrome = header_chrome_bindings_for(window)
    if chrome is None:
        return
    chrome.source_badge.setText(text)
    source = "history" if window._history_source_active or window._history_file_selected else "live"
    refresh_dynamic_property(chrome.source_badge, "source", source)
    source_description = f"{text}。{description}"
    chrome.source_badge.setAccessibleDescription(source_description)
    chrome.source_badge.setToolTip(source_description)
    window._refresh_station_band_source()


def refresh_station_band_source(window) -> None:
    """Repaint station bands only when the existing source context changes."""

    source = "history" if window._history_source_active or window._history_file_selected else "live"
    terminal = terminal_bindings_for(window)
    bands = tuple(
        band
        for band in (
            terminal.live_observation_band if terminal is not None else None,
            terminal.send_control_band if terminal is not None else None,
        )
        if band is not None
    )
    for band in bands:
        refresh_dynamic_property(band, "source", source)

    window._refresh_live_observation_state()


def refresh_live_observation_state(window) -> None:
    """Project existing source and display facts onto the live station band."""

    terminal = terminal_bindings_for(window)
    if terminal is None or window._closing:
        return
    band = terminal.live_observation_band
    if window._history_source_active or window._history_file_selected:
        state = "history"
    elif window._view_model.state in {SessionState.OPENING, SessionState.CLOSING}:
        state = "transition"
    elif window._view_model.preview_paused:
        state = "paused"
    else:
        recording = window._view_model.recording.state
        if recording in {
            RecordingState.STARTING,
            RecordingState.ACTIVE,
            RecordingState.STOPPING,
        }:
            state = "recording"
        elif recording is RecordingState.ERROR:
            state = "error"
        elif terminal.data_activity_label.property("state") == "active":
            state = "active"
        else:
            state = "idle"
    refresh_dynamic_property(band, "state", state)


def update_pipeline_summary(window) -> None:
    """Summarize the presentation-only Protocol → Component → Dataset flow."""

    config = window._view_model.protocol_config
    profile = window._view_model.component_profile
    dataset = window._view_model.dataset_config
    parser_supported = derived_source_supported(window)
    replay_labels = {
        ReplayState.EMPTY: "未加载",
        ReplayState.PLAYING: "播放中",
        ReplayState.PAUSED: "已暂停",
        ReplayState.EOF: "已结束",
        ReplayState.STOPPED: "已停止",
        ReplayState.ERROR: "错误",
    }
    profile_fields = len(getattr(profile, "fields", ()))
    profile_text = "未配置字段" if profile_fields == 0 else f"{profile_fields} 字段"
    dataset_text = "未配置序列" if not dataset.series else f"{len(dataset.series)} 个序列"
    replay_text = replay_labels.get(window._view_model.replay_snapshot.state, "未知")
    try:
        editor_framing = FramingKind(window._protocol_framing.currentData())
    except (TypeError, ValueError):
        editor_framing = config.framing
    protocol_text = (
        _framing_display(window, config.framing)
        if parser_supported
        else ("无可用 RX · 不适用" if window._history_file_selected else "仅原始终端 · 不适用")
    )
    if parser_supported and window._protocol_editor_dirty:
        editor_framing_text = _framing_display(window, editor_framing)
        applied_framing_text = _framing_display(window, config.framing)
        protocol_text = (
            f"草稿 {editor_framing_text} / 已应用 {applied_framing_text}"
            if editor_framing != config.framing
            else f"草稿待应用 · {editor_framing_text}"
        )
    if not parser_supported:
        profile_text = "不适用"
        dataset_text = "不适用"
        curve_text = "不适用"
    else:
        curve_snapshot = window._dataset_curve.snapshot
        if curve_snapshot.field_name is None:
            curve_text = "未选择序列"
        elif curve_snapshot.sample_count == 0:
            curve_text = f"{curve_snapshot.field_name} · 等待样本"
        else:
            curve_text = f"{curve_snapshot.field_name} · {len(curve_snapshot.points)} 点"
    if window._history_source_active:
        input_text = f"历史回放 · {replay_text}"
    elif window._history_file_selected:
        input_text = "历史回放 · 无可用 RX"
    elif window._view_model.state is SessionState.OPEN:
        input_text = "实时会话" if parser_supported else "实时 · 仅原始终端"
    elif window._view_model.state in {SessionState.OPENING, SessionState.CLOSING}:
        input_text = "实时 · 连接流程"
    else:
        input_text = "实时 · 未连接"
    text = (
        f"输入 {input_text}  →  协议 {protocol_text}  →  "
        f"组件 {profile_text}  →  Dataset {dataset_text}  →  曲线 {curve_text}"
    )
    source = "history" if window._history_source_active or window._history_file_selected else "live"
    if source == "history":
        summary_state = "history"
    elif window._view_model.state in {SessionState.OPENING, SessionState.CLOSING}:
        summary_state = "transition"
    elif not parser_supported:
        summary_state = "blocked"
    elif window._protocol_editor_dirty:
        summary_state = "draft"
    elif window._view_model.state is SessionState.OPEN:
        summary_state = "active"
    else:
        summary_state = "idle"
    refresh_dynamic_property(window._pipeline_summary, "source", source)
    refresh_dynamic_property(window._pipeline_summary, "state", summary_state)
    window._pipeline_summary.setText(text)
    window._pipeline_summary.setToolTip(text)
    window._pipeline_summary.setAccessibleDescription(text)


def update_connection_context(window) -> None:
    chrome = header_chrome_bindings_for(window)
    shell = connection_shell_bindings_for(window)
    if chrome is None or shell is None:
        return
    uart = uart_bindings_for(window)
    network = network_bindings_for(window)
    ble = ble_bindings_for(window)
    kind = TransportKind(shell.transport_combo.currentData())
    labels = {
        TransportKind.UART: "UART",
        TransportKind.TCP_STREAM: "TCP Client",
        TransportKind.TCP_SERVER: "TCP Server",
        TransportKind.UDP_DATAGRAM: "UDP 单播",
        TransportKind.BLE_GATT: "BLE GATT",
        TransportKind.RTT: "J-Link RTT",
    }
    endpoint = "未配置端点"
    if kind is TransportKind.UART:
        endpoint = uart.port_combo.currentText().strip() if uart is not None else ""
        endpoint = endpoint or "未选择端口"
    elif kind is TransportKind.BLE_GATT and ble is not None:
        device = ble.device_combo.currentData(Qt.ItemDataRole.UserRole)
        endpoint = device.display if isinstance(device, BleGattDevice) else "未选择设备"
    elif network is not None and kind is TransportKind.TCP_SERVER:
        endpoint = f"{network.local_host.text()}:{network.local_port.value()}"
    elif network is not None:
        endpoint = f"{network.remote_host.text()}:{network.remote_port.value()}"
    text = f"{labels[kind]} · {endpoint}"
    chrome.context_label.setText(text)
    chrome.context_label.setAccessibleDescription(text)


def update_state_badge(window, state: SessionState) -> None:
    labels = {
        SessionState.DISCOVERED: "已发现端点",
        SessionState.OPENING: "正在连接",
        SessionState.OPEN: "已连接",
        SessionState.CLOSING: "正在断开",
        SessionState.CLOSED: "未连接",
        SessionState.ERROR: "连接错误",
    }
    window._motion_controller.set_transition_active(
        state in {SessionState.OPENING, SessionState.CLOSING}
    )
    chrome = header_chrome_bindings_for(window)
    if chrome is None:
        return
    chrome.state_indicator.set_state(state)
    shell = connection_shell_bindings_for(window)
    footer = getattr(window, "_status_footer_surface", None)
    if footer is not None:
        footer.set_state(state)
    if shell is not None:
        shell.status_rail.set_state(state.value)
        refresh_dynamic_property(shell.control_band, "state", state.value)
    refresh_dynamic_property(chrome.status_cluster, "state", state.value)
    text = labels.get(state, state.value)
    chrome.state_label.setProperty("state", state.value)
    style = chrome.state_label.style()
    style.unpolish(chrome.state_label)
    style.polish(chrome.state_label)
    chrome.state_label.update()
    chrome.state_label.setText(text)
    chrome.state_label.setAccessibleDescription(text)
    window._update_source_badge()


def refresh_ports(window) -> None:
    """Start one explicit UART discovery and immediately expose its busy state."""

    if getattr(window._view_model, "discovery_busy", False):
        return
    window._view_model.refresh_ports()
    window._refresh_connection_controls(window._view_model.state)


def on_status_changed(window, status: object) -> None:
    if window._closing:
        return
    text = status if isinstance(status, str) else str(status)
    if window._view_model.error_info is None:
        window._last_non_error_status = text
    window.statusBar().showMessage(text)
    window._refresh_connection_controls(window._view_model.state)


def on_discovery_busy_changed(window, _busy: bool) -> None:
    if not window._closing:
        window._refresh_connection_controls(window._view_model.state)


def on_ble_scan_busy_changed(window, _busy: bool) -> None:
    if not window._closing:
        window._refresh_connection_controls(window._view_model.state)


def on_connection_endpoint_changed(window, *_args: object) -> None:
    """Keep the visible endpoint summary and listener gate live while editing."""

    network = network_bindings_for(window)
    if network is not None and window.sender() in {network.local_host, network.local_port}:
        invalidate_server_lan_authorization(window)
    window._update_connection_context()
    if not window._view_model.is_active:
        window._refresh_connection_controls(window._view_model.state)


def on_port_text_changed(window, _text: str) -> None:
    window._update_connection_context()
    if not window._view_model.is_active:
        window._refresh_connection_controls(window._view_model.state)


def rollback_ble_notification(window) -> None:
    pending = window._ble_notification_pending
    window._ble_notification_timer.stop()
    window._ble_notification_pending = None
    if window._closing:
        window._ble_notification_ref = None
        return
    if pending is None:
        return
    ble = ble_bindings_for(window)
    if ble is None:
        return
    characteristic, enabled = pending
    selected = ble.characteristic_combo.currentData(Qt.ItemDataRole.UserRole)
    if not isinstance(selected, BleGattCharacteristic) or selected.ref != characteristic:
        window._refresh_connection_controls(window._view_model.state)
        return
    restored = not enabled
    window._ble_notification_ref = characteristic if restored else None
    ble.notify_check.blockSignals(True)
    ble.notify_check.setChecked(restored)
    ble.notify_check.blockSignals(False)
    window._refresh_connection_controls(window._view_model.state)


def on_error_changed(window, info: object) -> None:
    if window._closing:
        return
    if info is None:
        signal = getattr(window, "_error_signal", None)
        if signal is not None:
            signal.set_active(False)
        footer = getattr(window, "_status_footer_surface", None)
        if footer is not None:
            footer.set_fault(False)
        window._error_label.clear()
        window._error_label.setAccessibleDescription("")
        window._error_label.setToolTip("")
        window._error_container.setAccessibleDescription("当前没有应用错误。")
        window._error_container.setVisible(False)
        if window._last_non_error_status:
            window.statusBar().showMessage(window._last_non_error_status)
        else:
            window.statusBar().clearMessage()
        return
    if not isinstance(info, ErrorInfo):
        return
    motion_controller = getattr(window, "_motion_controller", None)
    if window.isVisible() and motion_controller is not None:
        motion_controller.request_activity(520)
    signal = getattr(window, "_error_signal", None)
    if signal is not None:
        signal.set_active(True)
    footer = getattr(window, "_status_footer_surface", None)
    if footer is not None:
        footer.set_fault(True)
    if info.code is ErrorCode.DISCOVERY:
        window._refresh_connection_controls(window._view_model.state)
    pending = window._ble_notification_pending
    if info.code == ErrorCode.BLE_NOTIFY and pending is not None and pending[0].key in info.message:
        window._rollback_ble_notification()
    summary = _clip_text(info.message, 2_048)
    detail = _clip_text(info.detail, MAX_UI_DETAIL_CHARS) if info.detail else ""
    message = summary
    if detail:
        message = f"{summary}\n详情：{detail}"
    window._error_label.setText(message)
    window._error_label.setAccessibleDescription(message)
    window._error_label.setToolTip(detail)
    window._error_container.setAccessibleDescription(message)
    window._error_container.setVisible(bool(info.message))
    window.statusBar().showMessage(summary)


def show_local_error(window, message: str) -> None:
    window._view_model.show_local_error(message)


def closeEvent(window, event: object) -> None:
    """Request background shutdown before Qt-owned objects are destroyed."""

    if not window._closing:
        window._closing = True
        stop_shell_transitions(window)
        set_data_activity_motion(window, False)
        window._component_render_timer.stop()
        window._preview_render_timer.stop()
        window._preview_render_pending = False
        window._ble_notification_timer.stop()
        window._ble_notification_pending = None
        window._ble_notification_ref = None
        window._component_rows_pending = None
        window._dataset_curve.shutdown()
        window._motion_controller.set_ambient_active(False)
        window._motion_controller.close()
        _stop_motion_surfaces(window)
        window._view_model.request_shutdown()
    QMainWindow.closeEvent(window, event)  # type: ignore[arg-type]


def resizeEvent(window, event: object) -> None:
    """Settle one-shot presentation motion before host geometry changes."""

    stop_shell_transitions(window)
    QMainWindow.resizeEvent(window, event)  # type: ignore[arg-type]
    invalidate_motion_surface_snapshot(window)
    refresh_workspace_focus_layout(window)
    sync_compact_connection_focus(window)


def hideEvent(window, event: object) -> None:
    invalidate_motion_surface_snapshot(window)
    stop_shell_transitions(window)
    set_data_activity_motion(window, False)
    window._component_render_timer.stop()
    window._preview_render_timer.stop()
    window._dataset_curve.set_suspended(True)
    window._motion_controller.set_suspended(True)
    _stop_motion_surfaces(window)
    QMainWindow.hideEvent(window, event)  # type: ignore[arg-type]


def _rearm_motion_after_show(window, hop: int = 0) -> None:
    """Resume the shared clock after the native show path has settled."""

    if hop == 0:
        try:
            if window._closing or not window.isVisible() or window.isMinimized():
                window._motion_rearm_pending = False
                return
        except RuntimeError:
            window._motion_rearm_pending = False
            return
        # One extra queued hop crosses the native restore boundary on Windows
        # without creating another animation clock or blocking the UI thread.
        QTimer.singleShot(
            0,
            partial(_rearm_motion_after_show, window, 1),
        )
        return

    window._motion_rearm_pending = False
    try:
        if window._closing or not window.isVisible() or window.isMinimized():
            return
    except RuntimeError:
        return
    controller = getattr(window, "_motion_controller", None)
    rearm_after_show = getattr(controller, "rearm_after_show", None)
    if not callable(rearm_after_show):
        return
    rearm_after_show()
    controller.set_ambient_active(True)
    controller.set_suspended(False)


def _queue_motion_rearm(window) -> None:
    """Defer timer rearming past the native show event's delivery boundary."""

    if getattr(window, "_motion_rearm_pending", False):
        return
    window._motion_rearm_pending = True
    QTimer.singleShot(0, partial(_rearm_motion_after_show, window))


def showEvent(window, event: object) -> None:
    QMainWindow.showEvent(window, event)  # type: ignore[arg-type]
    if window._closing:
        return
    invalidate_motion_surface_snapshot(window)
    workspace = workspace_bindings_for(window)
    if workspace is not None:
        on_workspace_tab_changed(window, workspace.tabs.currentIndex())
    window._rerender_preview()
    window._preview_render_pending = False
    # A preset hydration callback may request activity before the first show.
    # Restart the existing shared timer after the native window is visible so
    # Windows does not retain the pre-show timer's coarse delivery cadence.
    window._motion_controller.set_suspended(True)
    window._motion_controller.set_ambient_active(True)
    _queue_motion_rearm(window)
    window._update_state_badge(window._view_model.state)


def changeEvent(window, event: object) -> None:
    if window._closing:
        QMainWindow.changeEvent(window, event)  # type: ignore[arg-type]
        return
    if hasattr(event, "type") and event.type() == QEvent.Type.WindowStateChange:
        invalidate_motion_surface_snapshot(window)
        if window.isMinimized():
            stop_shell_transitions(window)
            set_data_activity_motion(window, False)
            window._component_render_timer.stop()
            window._preview_render_timer.stop()
            window._dataset_curve.set_suspended(True)
            window._motion_controller.set_suspended(True)
            _stop_motion_surfaces(window)
        elif window.isVisible():
            workspace = workspace_bindings_for(window)
            if workspace is not None:
                on_workspace_tab_changed(window, workspace.tabs.currentIndex())
            window._rerender_preview()
            window._preview_render_pending = False
            _queue_motion_rearm(window)

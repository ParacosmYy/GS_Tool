"""Protocol editor and pipeline configuration controller.

The protocol controller owns editor state, preset selection, parser gates,
and timing hints. It does not own transport or derived-data workers.
"""

from __future__ import annotations

from dataclasses import replace

from ...domain.datasets import DatasetConfig
from ...domain.errors import ConfigurationError
from ...domain.mavlink import MAVLINK_MAX_PACKET_BYTES
from ...domain.modbus import MAX_MODBUS_RTU_FRAME_BYTES
from ...domain.models import TransportKind, UartParity, UartStopBits
from ...domain.protocol_presets import ProtocolPreset, builtin_protocol_presets, preset_for_config
from ...domain.protocols import ChecksumKind, FramingKind, ProtocolConfig
from ...domain.timing import ModbusRtuTiming
from ..connection_bindings import connection_shell_bindings_for, uart_bindings_for
from ..curve import CurveSnapshot
from ..dialog_surface import configure_confirmation_dialog
from ..protocol_action_hints import PROTOCOL_DERIVED_ACTION_HINTS
from ..protocol_scope import derived_source_supported, derived_source_unavailable_text
from ..qt import QMessageBox, Qt
from .protocol_context import refresh_protocol_config_context


def update_protocol_scope(window) -> None:
    """Keep protocol scope text aligned with replay and transport state."""

    shell = connection_shell_bindings_for(window)
    try:
        kind = (
            TransportKind(shell.transport_combo.currentData())
            if shell is not None
            else TransportKind.UART
        )
    except (AttributeError, TypeError, ValueError):
        kind = TransportKind.UART
    if window._history_source_active:
        text = (
            "历史回放 RX 可进入协议 → 组件 → Dataset；"
            "当前实时传输选择不改变这次回放的解析来源。"
        )
    elif window._history_file_selected:
        text = "已选择历史文件但没有可派生 RX；请换一个文件，或停止回放后连接 UART/TCP Client。"
    elif kind is TransportKind.UART:
        text = (
            "UART 接收支持 MAVLink stream 与 Modbus RTU timed；"
            "Modbus 使用带质量的 host read gap 作为诊断，不等于线缆时间戳。"
        )
    elif kind is TransportKind.TCP_STREAM:
        text = (
            "TCP Client 接收支持 MAVLink stream；Modbus RTU timed 只允许 UART，"
            "当前仍可保持 raw/recording。"
        )
    else:
        text = "当前实时传输仅进入原始终端和记录；历史回放仍可使用协议组件配置。"
    window._protocol_scope.setText(text)
    window._protocol_scope.setAccessibleDescription(text)
    window._protocol_scope.setToolTip(text)


def set_protocol_controls_enabled(window, enabled: bool) -> None:
    """Keep the protocol editor visibly scoped to supported RX transports."""

    for widget in (
        window._protocol_preset,
        window._protocol_framing,
        window._protocol_checksum,
        window._protocol_max_frame,
        window._protocol_apply_button,
        window._protocol_reset_button,
        window._protocol_delimiter,
        window._protocol_length_bytes,
        window._protocol_byteorder,
    ):
        widget.setEnabled(enabled)
    if enabled:
        for widget, hint in (
            (window._protocol_preset, "预设只填入配置编辑区；点击应用后才会更新解析状态。"),
            (window._protocol_framing, "选择当前协议帧格式。"),
            (window._protocol_apply_button, "应用编辑区配置并更新解析状态。"),
            (window._protocol_reset_button, "清空当前协议、组件、Dataset 派生状态。"),
        ):
            widget.setToolTip(hint)
            widget.setAccessibleDescription(hint)
        window._protocol_editor_syncing = True
        try:
            window._on_protocol_framing_changed()
        finally:
            window._protocol_editor_syncing = False
        if window._protocol_editor_dirty:
            window._set_protocol_status("有未应用协议草稿 · 点击“应用”更新解析状态")
        elif window._history_source_active:
            window._set_protocol_status("协议配置可用 · 当前为历史回放解析")
        elif window._history_file_selected:
            window._set_protocol_status("协议配置可用 · 等待有效历史 RX 或下一次实时连接")
        else:
            window._set_protocol_status("协议配置可用 · 当前传输支持组件 RX")
        return
    window._protocol_timing_hint.clear()
    reason = (
        "当前历史文件没有可派生 RX · 协议配置暂不可用"
        if window._history_file_selected
        else "当前传输仅原始终端 · 协议组件配置不可用"
    )
    for widget in (
        window._protocol_preset,
        window._protocol_framing,
        window._protocol_checksum,
        window._protocol_max_frame,
        window._protocol_apply_button,
        window._protocol_reset_button,
        window._protocol_delimiter,
        window._protocol_length_bytes,
        window._protocol_byteorder,
    ):
        widget.setToolTip(reason)
        widget.setAccessibleDescription(reason)
    window._set_protocol_status(reason)


def set_derived_controls_enabled(window, enabled: bool) -> None:
    """Scope profile/Dataset controls to a source that can produce derived RX data."""

    was_enabled = window._derived_pipeline_enabled
    window._derived_pipeline_enabled = enabled
    if enabled:
        hint = "当前来源支持协议 → 组件 → Dataset → 曲线。"
    elif window._history_file_selected:
        hint = "当前历史文件没有可派生的 UART/TCP Client RX；组件、Dataset 和曲线不适用。"
    else:
        hint = "当前实时传输仅支持原始终端/记录；组件、Dataset 和曲线不适用。"
    for widget in (
        window._component_filter,
        window._dataset_curve_series,
        window._dataset_curve,
        window._component_table,
        window._component_preview,
        window._dataset_preview,
    ):
        widget.setEnabled(enabled)
        widget.setToolTip(hint)
        widget.setAccessibleDescription(hint)
    for widget, action_hint in (
        (
            window._load_profile_button,
            PROTOCOL_DERIVED_ACTION_HINTS.load_profile,
        ),
        (
            window._export_component_button,
            PROTOCOL_DERIVED_ACTION_HINTS.export_component,
        ),
        (
            window._load_dataset_button,
            PROTOCOL_DERIVED_ACTION_HINTS.load_dataset,
        ),
        (
            window._export_dataset_button,
            PROTOCOL_DERIVED_ACTION_HINTS.export_dataset,
        ),
    ):
        widget.setEnabled(enabled)
        effective_hint = (
            action_hint
            if enabled
            else f"{action_hint} 当前不可用：{hint}"
        )
        widget.setToolTip(effective_hint)
        widget.setAccessibleDescription(effective_hint)
    for widget in (window._component_table, window._component_preview, window._dataset_preview):
        widget.setFocusPolicy(Qt.FocusPolicy.StrongFocus if enabled else Qt.FocusPolicy.NoFocus)
    window._component_empty.set_action_enabled(enabled)
    window._dataset_curve.setFocusPolicy(
        Qt.FocusPolicy.StrongFocus if enabled else Qt.FocusPolicy.NoFocus
    )
    window._dataset_curve.set_empty_message("未选择数值序列" if enabled else "当前来源不适用")
    if not enabled:
        window._component_render_timer.stop()
        window._component_rows_pending = None
        window._component_table.setRowCount(0)
        window._component_table.setVisible(False)
        unavailable = derived_source_unavailable_text(window)
        next_step = (
            "请换一个历史文件，或停止回放后连接 UART/TCP Client。"
            if window._history_file_selected
            else "请切换到 UART/TCP Client 或选择有效历史回放。"
        )
        window._component_empty.setText(f"{unavailable} · 组件解析不适用。{next_step}")
        window._component_empty.setAccessibleDescription(window._component_empty.text())
        window._component_empty.setToolTip(window._component_empty.text())
        window._component_empty.setVisible(True)
        window._component_preview.clear()
        window._component_preview.setPlaceholderText(window._component_empty.text())
        window._component_preview.setAccessibleDescription(window._component_empty.text())
        window._component_preview.setToolTip(window._component_empty.text())
        window._dataset_curve_samples = ()
        window._dataset_curve_field = None
        window._dataset_curve_series.blockSignals(True)
        window._dataset_curve_series.setCurrentIndex(0)
        window._dataset_curve_series.blockSignals(False)
        window._dataset_preview.clear()
        window._dataset_preview.setPlaceholderText(hint)
        window._dataset_preview.setAccessibleDescription(hint)
        window._dataset_preview.setToolTip(hint)
        window._dataset_curve.set_snapshot(CurveSnapshot())
        window._dataset_curve.setAccessibleDescription(hint)
        window._dataset_curve.setToolTip(hint)
    elif enabled and was_enabled is False:
        window._component_empty.setText("暂无组件帧 · 等待下一批接收数据或选择历史回放。")
        window._component_empty.setAccessibleDescription(window._component_empty.text())
        window._component_empty.setToolTip(window._component_empty.text())
        window._component_empty.setVisible(True)
        window._dataset_preview.setAccessibleDescription(
            "尚无 Dataset 样本；请先接收有效组件帧。"
        )
        window._dataset_preview.setPlaceholderText("尚无 Dataset 样本 · 请先接收有效组件帧。")
        window._dataset_preview.setToolTip(window._dataset_preview.accessibleDescription())
        config = window._view_model.dataset_config
        if isinstance(config, DatasetConfig):
            window._on_dataset_config_changed(config)
        samples = window._view_model.dataset_samples
        if isinstance(samples, tuple):
            window._on_dataset_samples_changed(samples)
        window._on_protocol_frames_changed(window._protocol_frames_latest)
        window._on_component_rows_changed(window._view_model.component_rows)
    if enabled:
        window._component_table.setVisible(bool(window._view_model.component_rows))
        window._component_table.setAccessibleDescription(
            "只读显示最近的组件帧；过滤器可切换全部、有效或错误行。"
        )
        window._component_preview.setAccessibleDescription(
            "显示最近的协议帧预览；尚无数据时会提示下一步。"
        )
        window._dataset_preview.setAccessibleDescription(
            "显示最近的 Dataset 样本；尚无数据时会提示下一步。"
        )


def reset_protocol(window) -> None:
    """Reset protocol."""
    if not derived_source_supported(window):
        window._show_local_error("当前来源没有可派生 RX，暂不能重置协议解析。")
        return
    if not window._confirm_protocol_change("重置"):
        return
    window._view_model.reset_protocol()


def set_protocol_status(window, text: str) -> None:
    """Set protocol status."""
    if not derived_source_supported(window):
        state = "blocked"
    elif window._protocol_editor_dirty:
        state = "draft"
    elif window._view_model.protocol_stats.bytes_in > 0:
        state = "active"
    else:
        state = "waiting"
    window._status_surfaces.set_state("protocol", state)
    window._protocol_status.setText(text)
    window._protocol_status.setToolTip(text)
    window._protocol_status.setAccessibleDescription(text)
    refresh_protocol_config_context(window)


def confirm_protocol_change(window, action: str) -> bool:
    """Ask before clearing derived data while keeping raw preview untouched."""

    has_derived_state = bool(
        window._protocol_frames_latest
        or window._view_model.component_rows
        or window._view_model.dataset_samples
    )
    if not has_derived_state and not window._protocol_editor_dirty:
        return True
    dialog = QMessageBox(window)
    dialog.setIcon(QMessageBox.Icon.Warning)
    dialog.setWindowTitle(f"{action}协议解析")
    dialog.setText(f"{action}协议配置会清空当前派生状态。")
    dialog.setInformativeText(
        "将清空协议帧、组件和 Dataset 派生缓存；原始终端、原始记录和历史文件不受影响。"
    )
    dialog.setStandardButtons(QMessageBox.StandardButton.Cancel | QMessageBox.StandardButton.Ok)
    dialog.setDefaultButton(QMessageBox.StandardButton.Cancel)
    configure_confirmation_dialog(
        dialog,
        accessible_name=f"确认{action}协议解析",
        accessible_description=(
            "这是一个确认操作。确认后会清空协议帧、组件和 Dataset 派生缓存，"
            "不会清空原始终端、原始记录或历史文件。默认焦点为取消。"
        ),
        confirm_text=f"确认{action}",
        confirm_description=f"确认{action}协议配置并清空当前派生状态。",
        cancel_text="取消",
        cancel_description="取消当前操作，保留协议配置和当前派生状态。",
    )
    return dialog.exec() == QMessageBox.StandardButton.Ok


def on_protocol_framing_changed(window, _index: int = -1) -> None:
    """On protocol framing changed."""
    if not window._protocol_editor_syncing:
        window._mark_protocol_editor_dirty()
    value = window._protocol_framing.currentData(Qt.ItemDataRole.UserRole)
    try:
        kind = FramingKind(value)
    except (TypeError, ValueError):
        kind = FramingKind.RAW
    delimiter_enabled = kind is FramingKind.DELIMITER
    length_enabled = kind is FramingKind.LENGTH_PREFIXED
    protocol_specific = kind in {
        FramingKind.MAVLINK_STREAM,
        FramingKind.MODBUS_RTU_TIMED,
    }
    fixed_limit = {
        FramingKind.MAVLINK_STREAM: MAVLINK_MAX_PACKET_BYTES,
        FramingKind.MODBUS_RTU_TIMED: MAX_MODBUS_RTU_FRAME_BYTES,
    }.get(kind)
    if fixed_limit is None:
        window._protocol_max_frame.setRange(1, 65_536)
        window._protocol_max_frame.setEnabled(True)
    else:
        window._protocol_max_frame.setRange(1, fixed_limit)
        window._protocol_max_frame.setValue(min(window._protocol_max_frame.value(), fixed_limit))
        window._protocol_max_frame.setEnabled(False)
    window._protocol_delimiter.setEnabled(delimiter_enabled)
    window._protocol_length_bytes.setEnabled(length_enabled)
    window._protocol_byteorder.setEnabled(length_enabled)
    for widget, hint in (
        (
            window._protocol_delimiter,
            "仅“自定义分隔符”帧格式使用分隔符；当前帧格式不需要时不可编辑。",
        ),
        (
            window._protocol_length_bytes,
            "仅“长度前缀”帧格式使用长度字节数；当前帧格式不需要时不可编辑。",
        ),
        (
            window._protocol_byteorder,
            "仅“长度前缀”帧格式使用字节序；当前帧格式不需要时不可编辑。",
        ),
        (
            window._protocol_max_frame,
            "当前帧格式使用固定最大帧长度。"
            if fixed_limit is not None
            else "限制单帧最大字节数。",
        ),
    ):
        widget.setToolTip(hint)
        widget.setAccessibleDescription(hint)
    if kind is FramingKind.RAW or protocol_specific:
        window._protocol_checksum.setCurrentIndex(0)
        window._protocol_checksum.setEnabled(False)
        checksum_hint = "原始字节流、MAVLink、Modbus 帧格式不使用额外校验。"
    else:
        window._protocol_checksum.setEnabled(True)
        checksum_hint = "选择协议帧的附加校验方式。"
    window._protocol_checksum.setToolTip(checksum_hint)
    window._protocol_checksum.setAccessibleDescription(checksum_hint)
    if kind is FramingKind.MODBUS_RTU_TIMED and window._history_source_active:
        window._set_protocol_timing_text(
            "历史回放不提供物理线缆静默间隔；Modbus RTU timed 不适用。"
        )
    elif kind is FramingKind.MODBUS_RTU_TIMED:
        window._update_protocol_timing_hint()
    else:
        window._set_protocol_timing_text("")
    refresh_protocol_config_context(window)


def mark_protocol_editor_dirty(window, *_args: object) -> None:
    """Mark protocol editor dirty."""
    if window._protocol_editor_syncing:
        return
    window._protocol_editor_dirty = True
    try:
        framing = FramingKind(window._protocol_framing.currentData())
    except (TypeError, ValueError):
        framing = window._view_model.protocol_config.framing
    framing_label = window._protocol_framing.currentText() or framing.value
    window._set_protocol_status(f"草稿 · {framing_label} · 点击“应用”更新解析状态")
    window._update_pipeline_summary()


def current_modbus_timing(window) -> ModbusRtuTiming:
    """Derive RTU timing from the visible UART wire-format controls."""

    uart = uart_bindings_for(window)
    if uart is None:
        raise ConfigurationError("UART 控件尚未初始化。")
    return ModbusRtuTiming(
        baud_rate=int(uart.baud_combo.currentData(Qt.ItemDataRole.UserRole)),
        data_bits=int(uart.data_bits.currentData(Qt.ItemDataRole.UserRole)),
        parity=UartParity(uart.parity.currentData(Qt.ItemDataRole.UserRole)),
        stop_bits=UartStopBits(uart.stop_bits.currentData(Qt.ItemDataRole.UserRole)),
    )


def on_uart_timing_changed(window, *_args: object) -> None:
    """Refresh the pending Modbus timing preview without applying it."""

    if window._protocol_framing.currentData(Qt.ItemDataRole.UserRole) == (
        FramingKind.MODBUS_RTU_TIMED
    ):
        window._mark_protocol_editor_dirty()
        if window._history_source_active:
            window._set_protocol_timing_text(
                "历史回放不提供物理线缆静默间隔；Modbus RTU timed 不适用。"
            )
        else:
            window._update_protocol_timing_hint()


def update_protocol_timing_hint(window) -> None:
    """Update protocol timing hint."""
    try:
        timing = window._current_modbus_timing()
    except (TypeError, ValueError, ConfigurationError):
        window._set_protocol_timing_text("Modbus RTU 定时 · 当前 UART 参数无效")
        return
    window._set_protocol_timing_text(
        "Modbus RTU 定时 · "
        f"{timing.baud_rate} 波特 · {timing.data_bits} 数据位 · "
        f"{timing.parity.value} 校验 · {timing.stop_bits.value} 停止位 · "
        f"t1.5 {timing.t1_5_seconds * 1_000:.3f} ms · "
        f"t3.5 {timing.t3_5_seconds * 1_000:.3f} ms · "
        "仅使用带质量的 host read gap；不等于线缆逐字节时间戳。"
    )


def set_protocol_timing_text(window, text: str) -> None:
    """Set protocol timing text."""
    window._protocol_timing_hint.setText(text)
    window._protocol_timing_hint.setToolTip(text)
    window._protocol_timing_hint.setAccessibleDescription(text)


def on_protocol_preset_changed(window, _index: int = -1) -> None:
    """Load a preset into the editor without applying parser state."""

    value = window._protocol_preset.currentData(Qt.ItemDataRole.UserRole)
    if not isinstance(value, ProtocolPreset):
        window._mark_protocol_editor_dirty()
        window._set_protocol_status("自定义草稿 · 可编辑后点击应用")
        return
    config = value.config
    if config.framing is FramingKind.MODBUS_RTU_TIMED:
        config = replace(config, modbus_timing=window._current_modbus_timing())
    window._set_protocol_editor(config)
    window._protocol_editor_dirty = True
    preset_label = window._protocol_preset.currentText() or value.label
    window._set_protocol_status(f"{preset_label} · 已载入编辑区，点击应用")
    window._update_pipeline_summary()


def set_protocol_editor(window, config: ProtocolConfig) -> None:
    """Reflect one immutable config in the protocol editor controls."""

    window._protocol_editor_syncing = True
    try:
        for combo, value in (
            (window._protocol_framing, config.framing),
            (window._protocol_checksum, config.checksum),
            (window._protocol_byteorder, config.byteorder),
        ):
            combo.blockSignals(True)
            index = combo.findData(value)
            if index >= 0:
                combo.setCurrentIndex(index)
            combo.blockSignals(False)
        window._protocol_delimiter.blockSignals(True)
        window._protocol_delimiter.setText(config.delimiter.hex(" ").upper())
        window._protocol_delimiter.blockSignals(False)
        window._protocol_length_bytes.blockSignals(True)
        length_index = window._protocol_length_bytes.findData(config.length_bytes)
        if length_index >= 0:
            window._protocol_length_bytes.setCurrentIndex(length_index)
        window._protocol_length_bytes.blockSignals(False)
        window._protocol_max_frame.blockSignals(True)
        window._protocol_max_frame.setValue(config.max_frame_bytes)
        window._protocol_max_frame.blockSignals(False)
        window._on_protocol_framing_changed()
    finally:
        window._protocol_editor_syncing = False


def select_protocol_preset(window, config: ProtocolConfig) -> None:
    """Mark the editor as a matching built-in preset or custom config."""

    preset = preset_for_config(config)
    if preset is None and config.framing is FramingKind.MODBUS_RTU_TIMED:
        modbus_preset = next(
            (
                candidate
                for candidate in builtin_protocol_presets()
                if candidate.config.framing is FramingKind.MODBUS_RTU_TIMED
            ),
            None,
        )
        if (
            modbus_preset is not None
            and replace(
                modbus_preset.config,
                modbus_timing=config.modbus_timing,
            )
            == config
        ):
            preset = modbus_preset
    index = 0
    if preset is not None:
        index = window._protocol_preset.findData(preset)
        if index < 0:
            index = 0
    window._protocol_preset.blockSignals(True)
    window._protocol_preset.setCurrentIndex(index)
    window._protocol_preset.blockSignals(False)


def apply_protocol_config(window) -> None:
    """Apply protocol config."""
    if not derived_source_supported(window):
        window._show_local_error(
            "当前历史文件没有可派生 RX，协议组件配置不适用。"
            if window._history_file_selected
            else "当前实时传输仅支持原始终端/记录，协议组件配置不适用。"
        )
        return
    framing_value = window._protocol_framing.currentData(Qt.ItemDataRole.UserRole)
    checksum_value = window._protocol_checksum.currentData(Qt.ItemDataRole.UserRole)
    try:
        framing = FramingKind(framing_value)
        checksum = ChecksumKind(checksum_value)
    except (TypeError, ValueError):
        window._show_local_error("协议配置无效 · 帧格式/校验类型错误。")
        return
    shell = connection_shell_bindings_for(window)
    try:
        transport = (
            TransportKind(shell.transport_combo.currentData())
            if shell is not None
            else None
        )
    except (TypeError, ValueError):
        transport = None
    if framing is FramingKind.MODBUS_RTU_TIMED and window._history_source_active:
        window._show_local_error(
            "协议配置无效 · 历史回放没有物理线缆静默间隔，Modbus RTU timed 不适用。"
        )
        return
    if framing is FramingKind.MODBUS_RTU_TIMED and transport is not TransportKind.UART:
        window._show_local_error(
            "协议配置无效 · Modbus RTU timed 只允许 UART；"
            "TCP 的 host gap 不能代表 RTU 线缆静默间隔。"
        )
        return
    try:
        delimiter_text = window._protocol_delimiter.text().strip()
        delimiter = bytes.fromhex("".join(delimiter_text.split()))
        config = ProtocolConfig(
            framing=framing,
            checksum=checksum,
            delimiter=delimiter,
            length_bytes=int(window._protocol_length_bytes.currentData()),
            byteorder=str(window._protocol_byteorder.currentData()),
            checksum_byteorder=str(window._protocol_byteorder.currentData()),
            max_frame_bytes=int(window._protocol_max_frame.value()),
            modbus_timing=(
                window._current_modbus_timing() if framing is FramingKind.MODBUS_RTU_TIMED else None
            ),
        )
    except (TypeError, ValueError, ConfigurationError) as exc:
        window._show_local_error(f"协议配置无效 · {exc}")
        return
    if not window._confirm_protocol_change("应用"):
        return
    window._select_protocol_preset(config)
    window._view_model.configure_protocol(config)

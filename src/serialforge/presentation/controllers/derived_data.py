"""Component, Dataset, Curve, and derived preview controller.

This controller owns derived-pipeline rendering and configuration handoff.
Protocol parsing remains an application concern; raw terminal rendering is
kept separate so each surface can be throttled independently.
"""

from __future__ import annotations

from ...domain.codecs import ComponentCodecConfig
from ...domain.components import ComponentFrameRow, ComponentStats
from ...domain.datasets import DatasetConfig, DatasetSample, DatasetStats
from ...domain.models import TransportKind
from ...domain.protocols import DecodedFrame, FrameStatus, ProtocolConfig, ProtocolStats
from ..connection_bindings import connection_shell_bindings_for
from ..curve import CurveSnapshot, build_curve_snapshot
from ..file_dialog_surface import get_open_file_name, get_save_file_name
from ..protocol_scope import derived_source_supported, derived_source_unavailable_text
from ..qt import Qt, QTableWidgetItem
from ..workspace_bindings import workspace_bindings_for

MAX_UI_SUMMARY_CHARS = 4_096
MAX_UI_DETAIL_CHARS = 8_192
DERIVED_ACTIVITY_MS = 360


def _selected_transport(window) -> TransportKind | None:
    shell = connection_shell_bindings_for(window)
    if shell is None:
        return None
    try:
        return TransportKind(shell.transport_combo.currentData())
    except (TypeError, ValueError):
        return None


def _clip_text(value: object, limit: int) -> str:
    text = value if isinstance(value, str) else str(value)
    if len(text) <= limit:
        return text
    return f"{text[: max(1, limit - 1)]}…"


def _protocol_tab_active(window) -> bool:
    workspace = workspace_bindings_for(window)
    return workspace is not None and workspace.tabs.currentIndex() == window._protocol_tab_index


def _request_derived_activity(window) -> None:
    """Acknowledge visible derived output through the shared motion clock."""

    if (
        window._closing
        or not window.isVisible()
        or window.isMinimized()
        or not _protocol_tab_active(window)
    ):
        return
    motion_controller = getattr(window, "_motion_controller", None)
    if motion_controller is not None:
        motion_controller.request_activity(DERIVED_ACTIVITY_MS)


def on_protocol_config_changed(window, config: object) -> None:
    if window._closing:
        return
    if not isinstance(config, ProtocolConfig):
        return
    window._set_protocol_editor(config)
    window._select_protocol_preset(config)
    window._protocol_editor_dirty = False
    window._update_pipeline_summary()


def on_protocol_frames_changed(window, frames: object) -> None:
    if window._closing:
        return
    if not isinstance(frames, tuple):
        return
    if not derived_source_supported(window):
        if not frames:
            window._protocol_frames_latest = ()
        window._component_preview.clear()
        window._component_preview.setVisible(False)
        return
    window._protocol_frames_latest = tuple(
        frame for frame in frames if isinstance(frame, DecodedFrame)
    )[-120:]
    window._schedule_preview_render()


def rerender_component_preview(window) -> None:
    if window._closing:
        return
    if not derived_source_supported(window):
        description = f"{derived_source_unavailable_text(window)} · 组件帧预览不适用。"
        window._component_preview.clear()
        window._component_preview.setVisible(False)
        window._component_preview.setPlaceholderText(description)
        window._component_preview.setAccessibleDescription(description)
        window._component_preview.setToolTip(description)
        return
    window._component_preview.clear()
    if not window._protocol_frames_latest:
        description = "尚无可显示的协议帧 · 接收 UART/TCP Client RX 后会在这里显示。"
        window._component_preview.setVisible(False)
        window._component_preview.setPlaceholderText(description)
        window._component_preview.setAccessibleDescription(description)
        window._component_preview.setToolTip(description)
        return
    window._component_preview.setVisible(True)
    window._component_preview.setPlaceholderText("")
    status_labels = {
        FrameStatus.VALID: "通过",
        FrameStatus.UNVERIFIED: "未验证",
        FrameStatus.INVALID_CHECKSUM: "校验失败",
        FrameStatus.INVALID_FORMAT: "格式失败",
        FrameStatus.INVALID_LENGTH: "长度失败",
        FrameStatus.OVERSIZE: "超限",
        FrameStatus.INCOMPLETE: "未完成",
    }
    for frame in window._protocol_frames_latest:
        source = f" [{_clip_text(frame.source.display, 128)}]" if frame.source is not None else ""
        error = f" · {_clip_text(frame.error, 512)}" if frame.error else ""
        window._component_preview.appendPlainText(
            _clip_text(
                f"#{frame.sequence}{source} · {status_labels[frame.status]} · "
                f"{len(frame.payload)} B · {frame.payload[:256].hex(' ').upper()}{error}",
                MAX_UI_SUMMARY_CHARS,
            )
        )
    description = f"显示最近 {len(window._protocol_frames_latest)} 条协议帧预览。"
    window._component_preview.setAccessibleDescription(description)
    window._component_preview.setToolTip(description)


def on_protocol_stats_changed(window, stats: object) -> None:
    if window._closing:
        return
    if not isinstance(stats, ProtocolStats):
        return
    if not derived_source_supported(window):
        window._set_protocol_status("当前来源没有可派生 RX · 协议组件状态不适用")
        return
    config = window._view_model.protocol_config
    draft_note = " · 有未应用草稿" if window._protocol_editor_dirty else ""
    framing_label = window._protocol_framing.currentText() or config.framing.value
    window._set_protocol_status(
        f"{framing_label} · 接收 {stats.bytes_in} B · "
        f"有效 {stats.frames_valid} · 无效 {stats.frames_invalid} · "
        f"未完成 {stats.frames_incomplete} · 缓存 {stats.buffered_bytes} B · "
        f"解析丢弃 {stats.dropped_bytes} B · 间隔边界 {stats.gap_boundaries} · "
        f"重同步 {stats.resyncs}{draft_note}"
    )


def load_component_codec(window) -> None:
    if not derived_source_supported(window):
        window._show_local_error("当前实时传输仅支持原始终端/记录，组件解析不适用。")
        return
    path, _ = get_open_file_name(
        window,
        "加载组件配置 / Codec",
        "",
        "JSON Profile / Codec (*.json);;All files (*)",
    )
    if path:
        if window._history_source_active:
            transport = None
        else:
            transport = _selected_transport(window)
        window._view_model.load_component_codec(path, transport=transport)


def export_component_csv(window) -> None:
    if not derived_source_supported(window):
        window._show_local_error("当前实时传输仅支持原始终端/记录，组件导出不适用。")
        return
    path, _ = get_save_file_name(
        window,
        "导出组件 CSV",
        "component-preview.csv",
        "CSV (*.csv);;All files (*)",
    )
    if path:
        window._view_model.export_component_csv(path)


def load_dataset_config(window) -> None:
    if not derived_source_supported(window):
        window._show_local_error("当前实时传输仅支持原始终端/记录，Dataset 不适用。")
        return
    path, _ = get_open_file_name(
        window,
        "加载 Dataset 配置",
        "",
        "Dataset JSON (*.json);;All files (*)",
    )
    if path:
        window._view_model.load_dataset_config(path)


def export_dataset_csv(window) -> None:
    if not derived_source_supported(window):
        window._show_local_error("当前实时传输仅支持原始终端/记录，Dataset 导出不适用。")
        return
    path, _ = get_save_file_name(
        window,
        "导出 Dataset CSV",
        "dataset-preview.csv",
        "CSV (*.csv);;All files (*)",
    )
    if path:
        window._view_model.export_dataset_csv(path)


def on_component_profile_changed(window, profile: object) -> None:
    if window._closing:
        return
    text = window._component_profile_label.text()
    if isinstance(profile, ComponentCodecConfig):
        text = (
            f"{profile.name} · {profile.codec_kind.value} v{profile.codec.version} · "
            f"{len(profile.fields)} 字段"
        )
    elif hasattr(profile, "name") and hasattr(profile, "fields"):
        text = f"{profile.name} · legacy v1 · {len(profile.fields)} 字段"
    window._component_profile_label.setText(text)
    window._component_profile_label.setToolTip(text)
    window._component_profile_label.setAccessibleDescription(text)
    window._update_pipeline_summary()


def on_component_rows_changed(window, rows: object) -> None:
    if window._closing or not derived_source_supported(window):
        window._component_rows_pending = None
        window._component_render_timer.stop()
        return
    if not isinstance(rows, tuple) or not all(isinstance(row, ComponentFrameRow) for row in rows):
        return
    if rows:
        _request_derived_activity(window)
    window._component_rows_pending = rows
    if window._renderers_suspended():
        return
    if not window._component_render_timer.isActive():
        window._component_render_timer.start()


def renderers_suspended(window) -> bool:
    return (
        window._closing
        or window.isHidden()
        or window.isMinimized()
        or not _protocol_tab_active(window)
    )


def on_component_filter_changed(window, _index: int = -1) -> None:
    """Apply the newest immutable row snapshot before changing the filter."""

    if window._renderers_suspended():
        return
    had_pending = window._component_rows_pending is not None
    window._flush_component_rows()
    if not had_pending:
        window._rerender_component_table(window._view_model.component_rows)


def flush_component_rows(window) -> None:
    if window._closing or not derived_source_supported(window):
        window._component_rows_pending = None
        window._component_render_timer.stop()
        return
    if (
        window.isHidden()
        or window.isMinimized()
        or not _protocol_tab_active(window)
    ):
        window._component_render_timer.stop()
        return
    pending = window._component_rows_pending
    window._component_rows_pending = None
    window._component_render_timer.stop()
    if pending is not None:
        window._rerender_component_table(pending)


def on_component_stats_changed(window, stats: object) -> None:
    if window._closing:
        return
    if not isinstance(stats, ComponentStats):
        return
    window._component_stats_latest = stats
    window._update_component_status()


def update_component_status(window, *, cached_count: int | None = None) -> None:
    stats = window._component_stats_latest
    if cached_count is None:
        cached_count = len(window._view_model.component_rows)
    if not derived_source_supported(window):
        state = "blocked"
        text = f"{derived_source_unavailable_text(window)} · 组件解析不适用"
    elif stats.field_errors or stats.codec_errors:
        state = "error"
        text = (
            f"字段错误 {stats.field_errors} · codec 错误 {stats.codec_errors} · "
            f"显示 {window._component_table.rowCount()} / 缓存 {cached_count} · "
            f"丢弃 {stats.dropped_frames}"
        )
    elif cached_count == 0:
        state = "waiting"
        text = (
            f"字段错误 {stats.field_errors} · codec 错误 {stats.codec_errors} · "
            f"显示 {window._component_table.rowCount()} / 缓存 {cached_count} · "
            f"丢弃 {stats.dropped_frames}"
        )
    else:
        state = "active"
        text = (
            f"字段错误 {stats.field_errors} · codec 错误 {stats.codec_errors} · "
            f"显示 {window._component_table.rowCount()} / 缓存 {cached_count} · "
            f"丢弃 {stats.dropped_frames}"
        )
    window._status_surfaces.set_state("component", state)
    window._component_status.setText(text)
    window._component_status.setToolTip(text)
    window._component_status.setAccessibleDescription(text)


def on_dataset_config_changed(window, config: object) -> None:
    if window._closing or not derived_source_supported(window):
        return
    if not isinstance(config, DatasetConfig):
        return
    text = f"{config.name} · {len(config.series)} 个序列 · 容量 {config.capacity}"
    window._dataset_config_label.setText(text)
    window._dataset_config_label.setToolTip(text)
    window._dataset_config_label.setAccessibleDescription(text)
    current = window._dataset_curve_series.currentData(Qt.ItemDataRole.UserRole)
    window._dataset_curve_series.blockSignals(True)
    window._dataset_curve_series.clear()
    window._dataset_curve_series.addItem("选择数值序列", None)
    for series in config.series:
        label = series.field_name + (f" ({series.unit})" if series.unit else "")
        window._dataset_curve_series.addItem(label, series.field_name)
    if isinstance(current, str):
        index = window._dataset_curve_series.findData(current, Qt.ItemDataRole.UserRole)
        if index >= 0:
            window._dataset_curve_series.setCurrentIndex(index)
    window._dataset_curve_series.blockSignals(False)
    window._on_curve_series_changed()
    window._schedule_preview_render()
    window._update_dataset_status()
    window._update_pipeline_summary()


def on_dataset_samples_changed(window, samples: object) -> None:
    if window._closing or not derived_source_supported(window):
        window._dataset_curve_samples = ()
        window._dataset_preview.clear()
        window._dataset_preview.setVisible(False)
        return
    if isinstance(samples, tuple) and all(isinstance(sample, DatasetSample) for sample in samples):
        if samples:
            _request_derived_activity(window)
        window._dataset_curve_samples = samples
        if window._renderers_suspended():
            return
        window._schedule_preview_render()
        window._refresh_dataset_curve()


def on_dataset_stats_changed(window, stats: object) -> None:
    if window._closing:
        return
    if not isinstance(stats, DatasetStats):
        return
    window._dataset_stats_latest = stats
    window._update_dataset_status()


def update_dataset_status(window) -> None:
    stats = window._dataset_stats_latest
    config = window._view_model.dataset_config
    if not derived_source_supported(window):
        state = "blocked"
        text = f"{derived_source_unavailable_text(window)} · Dataset 不适用"
    elif not config.series:
        state = "empty"
        text = "未配置序列 · 加载 Dataset 后等待组件帧"
    elif stats.frames_in == 0 and stats.samples_out == 0:
        state = "waiting"
        text = f"已配置 {len(config.series)} 个序列 · 等待有效组件帧"
    elif stats.transform_errors:
        state = "error"
        text = (
            f"帧 {stats.frames_in} · 样本 {stats.samples_out} · "
            f"保留 {stats.retained_samples} · 错误 {stats.transform_errors} · "
            f"丢弃 {stats.dropped_batches}"
        )
    else:
        state = "active"
        text = (
            f"帧 {stats.frames_in} · 样本 {stats.samples_out} · "
            f"保留 {stats.retained_samples} · 错误 {stats.transform_errors} · "
            f"丢弃 {stats.dropped_batches}"
        )
    window._status_surfaces.set_state("dataset", state)
    window._dataset_status.setText(text)
    window._dataset_status.setToolTip(text)
    window._dataset_status.setAccessibleDescription(text)


def rerender_dataset_preview(window) -> None:
    if window._closing:
        return
    scrollbar = window._dataset_preview.verticalScrollBar()
    previous_scroll = scrollbar.value()
    follow_latest = previous_scroll >= scrollbar.maximum()
    if not derived_source_supported(window):
        description = f"{derived_source_unavailable_text(window)} · Dataset 预览不适用。"
        window._dataset_preview.clear()
        window._dataset_preview.setVisible(False)
        window._dataset_preview.setPlaceholderText(description)
        window._dataset_preview.setAccessibleDescription(description)
        window._dataset_preview.setToolTip(description)
        return
    lines: list[str] = []
    for sample in window._view_model.dataset_samples[-100:]:
        values = " · ".join(
            f"{_clip_text(value.field_name, 128)}={_clip_text(value.display, 512)}"
            + (f" {_clip_text(value.unit, 64)}" if value.unit else "")
            + (" [clip]" if value.clipped else "")
            + (f" [!{_clip_text(value.warning, 256)}]" if value.warning else "")
            + (f" [error: {_clip_text(value.error, 256)}]" if value.error else "")
            for value in sample.values
        )
        lines.append(
            _clip_text(
                f"#{sample.sequence} [{_clip_text(sample.source.display, 128)}] "
                f"{_clip_text(values or '—', MAX_UI_DETAIL_CHARS)}",
                MAX_UI_DETAIL_CHARS,
            )
        )
    window._dataset_preview.setPlainText("\n".join(lines))
    window._dataset_preview.setVisible(bool(lines))
    if lines:
        description = f"显示最近 {len(lines)} 条 Dataset 样本。"
        placeholder = ""
    elif not window._view_model.dataset_config.series:
        description = "尚未配置 Dataset 序列；请加载 Dataset 配置。"
        placeholder = description
    else:
        description = "尚无 Dataset 样本；请先接收有效组件帧。"
        placeholder = description
    window._dataset_preview.setPlaceholderText(placeholder)
    window._dataset_preview.setAccessibleDescription(description)
    window._dataset_preview.setToolTip(description)
    if follow_latest:
        scrollbar.setValue(scrollbar.maximum())
    else:
        scrollbar.setValue(min(previous_scroll, scrollbar.maximum()))


def on_curve_series_changed(window, _index: int = -1) -> None:
    value = window._dataset_curve_series.currentData(Qt.ItemDataRole.UserRole)
    window._dataset_curve_field = value if isinstance(value, str) else None
    window._refresh_dataset_curve(immediate=not window._renderers_suspended())


def refresh_dataset_curve(window, *, immediate: bool = False) -> None:
    if window._closing or not derived_source_supported(window):
        description = f"{derived_source_unavailable_text(window)} · Dataset 曲线不适用。"
        window._dataset_curve.set_snapshot(CurveSnapshot())
        window._dataset_curve.setAccessibleDescription(description)
        window._dataset_curve.setToolTip(description)
        if immediate and not window._renderers_suspended():
            window._dataset_curve.flush()
        return
    snapshot = build_curve_snapshot(
        window._dataset_curve_samples,
        field_name=window._dataset_curve_field,
        capacity=window._view_model.dataset_config.capacity,
    )
    window._dataset_curve.set_snapshot(snapshot)
    if immediate:
        window._dataset_curve.flush()
    window._update_dataset_curve_status(snapshot)
    window._update_pipeline_summary()


def on_dataset_curve_snapshot_changed(window, snapshot: object) -> None:
    if window._closing or not isinstance(snapshot, CurveSnapshot):
        return
    window._update_pipeline_summary()


def update_dataset_curve_status(window, snapshot: CurveSnapshot) -> None:
    if not derived_source_supported(window):
        state = "blocked"
        text = f"{derived_source_unavailable_text(window)} · Dataset 曲线不适用"
    elif snapshot.field_name is None:
        state = "empty"
        text = "未选择数值序列 · 先加载 Dataset 并选择序列"
    elif snapshot.sample_count == 0:
        state = "waiting"
        text = f"已选择 {snapshot.field_name} · 等待有效样本"
    elif not snapshot.points:
        state = "error"
        text = f"未绘制 {snapshot.field_name} · {snapshot.skipped_points} 个样本无有限数值"
    else:
        state = "active"
        origin = (
            "历史"
            if snapshot.origin is not None and snapshot.origin.value == "historical"
            else "实时"
        )
        latest = "—" if snapshot.latest_value is None else f"{snapshot.latest_value:.6g}"
        minimum = min(point.value for point in snapshot.points)
        maximum = max(point.value for point in snapshot.points)
        unit = f" {snapshot.unit}" if snapshot.unit else ""
        text = (
            f"{origin} · {len(snapshot.points)}/{snapshot.sample_count} 点 · "
            f"范围 {minimum:.6g}~{maximum:.6g}{unit} · 最新 {latest}{unit} · "
            f"跳过 {snapshot.skipped_points} · "
            f"丢弃 {snapshot.truncated_points}"
        )
    window._status_surfaces.set_state("curve", state)
    window._dataset_curve_status.setText(text)
    window._dataset_curve_status.setToolTip(text)
    window._dataset_curve_status.setAccessibleDescription(text)


def rerender_component_table(
    window,
    rows: object = None,
    _index: int = -1,
) -> None:
    if not derived_source_supported(window):
        window._component_table.setRowCount(0)
        window._component_table.setVisible(False)
        window._component_empty.setVisible(True)
        return
    normalized = (
        rows
        if isinstance(rows, tuple) and all(isinstance(row, ComponentFrameRow) for row in rows)
        else window._view_model.component_rows
    )
    selected = window._component_filter.currentData(Qt.ItemDataRole.UserRole)
    if selected == "valid":
        visible = tuple(
            row
            for row in normalized
            if row.status is FrameStatus.VALID
            and row.codec_error is None
            and row.error is None
            and not any(field.error is not None for field in row.fields)
        )
    elif selected == "error":
        visible = tuple(
            row
            for row in normalized
            if row.status is not FrameStatus.VALID
            or row.codec_error is not None
            or row.error is not None
            or any(field.error is not None for field in row.fields)
        )
    else:
        visible = normalized
    status_labels = {
        FrameStatus.VALID: "通过",
        FrameStatus.UNVERIFIED: "未验证",
        FrameStatus.INVALID_CHECKSUM: "校验失败",
        FrameStatus.INVALID_FORMAT: "格式失败",
        FrameStatus.INVALID_LENGTH: "长度失败",
        FrameStatus.OVERSIZE: "超限",
        FrameStatus.INCOMPLETE: "未完成",
    }
    visible = visible[-200:]
    if visible:
        window._component_table.setVisible(True)
        window._component_empty.clear()
        window._component_empty.setVisible(False)
    elif not normalized:
        window._component_table.setVisible(False)
        transport = _selected_transport(window) or TransportKind.UART
        if (
            transport not in {TransportKind.UART, TransportKind.TCP_STREAM}
            and not window._history_source_active
        ):
            text = "当前传输仅支持原始终端/记录 · 组件解析不适用。"
        else:
            text = (
                "暂无组件帧 · 接收 UART/TCP Client RX 或选择历史回放；"
                "加载组件配置 / Codec 可解析字段。"
            )
        window._component_empty.setText(text)
        window._component_empty.setVisible(True)
    elif selected in {"valid", "error"}:
        window._component_table.setVisible(False)
        window._component_empty.setText("当前筛选无匹配 · 切换“全部”查看缓存的组件帧。")
        window._component_empty.setVisible(True)
    else:
        window._component_table.setVisible(False)
        window._component_empty.setText("暂无可显示组件帧 · 等待下一批接收数据。")
        window._component_empty.setVisible(True)
    window._component_empty.setAccessibleDescription(window._component_empty.text())
    window._component_empty.setToolTip(window._component_empty.text())
    selected_sequence = None
    selected_column = 0
    current_row = window._component_table.currentRow()
    if current_row >= 0:
        current_item = window._component_table.item(current_row, 1)
        if current_item is not None:
            selected_sequence = current_item.text()
            selected_column = window._component_table.currentColumn()
    scroll_value = window._component_table.verticalScrollBar().value()
    window._component_table.setRowCount(0)
    window._component_table.setRowCount(len(visible))
    for index, row in enumerate(visible):
        fields = " · ".join(
            f"{_clip_text(field.name, 128)}={_clip_text(field.display, 512)}"
            + (f" (!{_clip_text(field.error, 256)})" if field.error else "")
            for field in row.fields
        )
        if not fields:
            fields = "—"
        if row.error:
            fields = f"{fields} · frame: {_clip_text(row.error, 512)}"
        if row.codec_error:
            fields = f"{fields} · codec: {_clip_text(row.codec_error, 512)}"
        fields = _clip_text(fields, MAX_UI_DETAIL_CHARS)
        fields_display = _clip_text(fields, 256)
        raw_display = _clip_text(row.payload_hex, 1_024)
        values = (
            _clip_text(f"{row.occurred_at:.3f}", 64),
            str(row.sequence),
            status_labels[row.status],
            _clip_text(row.source.display, 128),
            fields_display,
            raw_display,
        )
        for column, value in enumerate(values):
            item = QTableWidgetItem(value)
            item.setData(Qt.ItemDataRole.AccessibleTextRole, value)
            if column == 4:
                item.setToolTip(_clip_text(fields, MAX_UI_DETAIL_CHARS))
                item.setData(
                    Qt.ItemDataRole.AccessibleTextRole,
                    _clip_text(fields, MAX_UI_DETAIL_CHARS),
                )
            elif column == 5:
                raw_detail = _clip_text(row.payload_hex, MAX_UI_DETAIL_CHARS)
                item.setToolTip(raw_detail)
                item.setData(Qt.ItemDataRole.AccessibleTextRole, raw_detail)
            window._component_table.setItem(index, column, item)
    if selected_sequence is not None:
        for row_index, row in enumerate(visible):
            if str(row.sequence) == selected_sequence:
                window._component_table.setCurrentCell(
                    row_index,
                    min(max(selected_column, 0), window._component_table.columnCount() - 1),
                )
                window._component_table.selectRow(row_index)
                break
    window._component_table.verticalScrollBar().setValue(scroll_value)
    window._update_component_status(cached_count=len(normalized))

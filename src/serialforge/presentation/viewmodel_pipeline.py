"""Protocol, component, dataset, and replay configuration operations.

Each function receives the owning SessionViewModel explicitly; the facade
keeps Qt signals and lifecycle ownership in one place.
"""

from __future__ import annotations

import csv
import io
from pathlib import Path

from ..domain.codecs import (
    ComponentCodecConfig,
    ComponentConfiguration,
    MavlinkCodecConfig,
    ModbusRtuCodecConfig,
)
from ..domain.datasets import DatasetConfig
from ..domain.errors import (
    ErrorCode,
    ErrorInfo,
    SerialForgeError,
)
from ..domain.models import (
    TransportKind,
)
from ..domain.protocols import (
    ProtocolConfig,
)


def _csv_scalar(value: object) -> str:
    """Csv scalar."""
    if value is None:
        return ""
    if isinstance(value, bool):
        return "true" if value else "false"
    return str(value)


def configure_protocol(view_model, config: ProtocolConfig) -> None:
    """Apply parser settings and clear only protocol partial state/counters."""

    try:
        view_model._protocol.configure(config)
        view_model._sync_pipeline_generations()
        view_model._components.reset(protocol_generation=view_model._protocol_generation)
        view_model._sync_pipeline_generations()
        view_model._reset_dataset()
        view_model._component_rows = ()
        view_model.protocol_config_changed.emit(config)
        view_model.protocol_frames_changed.emit(())
        view_model.protocol_stats_changed.emit(view_model._protocol.stats)
        view_model.component_rows_changed.emit(view_model._component_rows)
        view_model.component_stats_changed.emit(view_model._components.stats)
        view_model._clear_error()
    except SerialForgeError as exc:
        view_model._show_error(exc.info)


def reset_protocol(view_model) -> None:
    """Reset parser state without changing the selected framing configuration."""

    view_model._protocol.reset()
    view_model._sync_pipeline_generations()
    view_model._components.reset(protocol_generation=view_model._protocol_generation)
    view_model._sync_pipeline_generations()
    view_model._reset_dataset()
    view_model._component_rows = ()
    view_model.protocol_frames_changed.emit(())
    view_model.protocol_stats_changed.emit(view_model._protocol.stats)
    view_model.component_rows_changed.emit(view_model._component_rows)
    view_model.component_stats_changed.emit(view_model._components.stats)


def _reset_dataset(view_model) -> None:
    """Discard derived samples when their upstream component rows change."""

    view_model._dataset.reset(
        component_generation=view_model._component_generation,
        protocol_generation=view_model._protocol_generation,
    )
    view_model._sync_pipeline_generations()
    view_model._dataset_samples = ()
    view_model.dataset_samples_changed.emit(view_model._dataset_samples)
    view_model.dataset_stats_changed.emit(view_model._dataset.stats)


def configure_component_profile(view_model, profile: ComponentConfiguration) -> bool:
    """Apply a profile/codec without reconnecting or changing framing."""

    try:
        view_model._components.configure(
            profile,
            protocol_generation=view_model._protocol_generation,
        )
        view_model._sync_pipeline_generations()
        view_model._reset_dataset()
        view_model._component_rows = ()
        view_model.component_profile_changed.emit(profile)
        view_model.component_rows_changed.emit(view_model._component_rows)
        view_model.component_stats_changed.emit(view_model._components.stats)
        view_model._clear_error()
        return True
    except SerialForgeError as exc:
        view_model._show_error(exc.info)
        return False


def load_component_profile(view_model, path: str) -> None:
    """Load one bounded legacy profile or schema v2 codec file."""

    try:
        if view_model.configure_component_profile(view_model._component_profiles.load(path)):
            view_model._set_status(f"已加载组件 profile · {Path(path).name}")
    except SerialForgeError as exc:
        view_model._show_error(exc.info)


def configure_component_codec(view_model, configuration: ComponentCodecConfig) -> bool:
    """Apply a schema v2 RX codec without changing transport or raw data."""

    if not isinstance(configuration, ComponentCodecConfig):
        view_model._show_error(
            ErrorInfo(
                code=ErrorCode.COMPONENT_PROFILE,
                message="组件 codec 配置类型无效。",
                recoverable=True,
            )
        )
        return False
    return view_model.configure_component_profile(configuration)


def load_component_codec(
    view_model,
    path: str,
    *,
    transport: TransportKind | None = None,
) -> None:
    """Load a schema v2 codec, applying explicit transport scope rules."""

    try:
        configuration = view_model._component_profiles.load(path)
        if (
            isinstance(configuration, ComponentCodecConfig)
            and isinstance(
                configuration.codec,
                (ModbusRtuCodecConfig, MavlinkCodecConfig),
            )
            and transport is not None
            and transport is not TransportKind.UART
        ):
            view_model._show_error(
                ErrorInfo(
                    code=ErrorCode.COMPONENT_PROFILE,
                    message="Modbus RTU/MAVLink codec 当前只允许 UART 已分帧 RX。",
                    recoverable=True,
                )
            )
            return
        if view_model.configure_component_profile(configuration):
            view_model._set_status(f"已加载组件 codec · {Path(path).name}")
    except SerialForgeError as exc:
        view_model._show_error(exc.info)


def configure_dataset(view_model, config: DatasetConfig) -> bool:
    """Apply an independent transform chain and bounded retention policy."""

    if not isinstance(config, DatasetConfig):
        view_model._show_error(
            ErrorInfo(
                code=ErrorCode.DATASET_CONFIGURATION,
                message="dataset 配置类型无效。",
                recoverable=True,
            )
        )
        return False
    try:
        view_model._dataset.configure(
            config,
            component_generation=view_model._component_generation,
            protocol_generation=view_model._protocol_generation,
        )
        view_model._sync_pipeline_generations()
        view_model._dataset_samples = ()
        view_model.dataset_config_changed.emit(config)
        view_model.dataset_samples_changed.emit(view_model._dataset_samples)
        view_model.dataset_stats_changed.emit(view_model._dataset.stats)
        view_model._clear_error()
        view_model._set_status(f"已应用 dataset · {len(config.series)} 条 series")
        return True
    except SerialForgeError as exc:
        view_model._show_error(exc.info)
        return False


def load_dataset_config(view_model, path: str) -> None:
    """Load one bounded dataset JSON file without changing component decoding."""

    try:
        config = view_model._dataset_configs.load(path)
        if view_model.configure_dataset(config):
            view_model._set_status(f"已加载 dataset · {Path(path).name}")
    except SerialForgeError as exc:
        view_model._show_error(exc.info)


def export_dataset_csv(view_model, path: str) -> None:
    """Export the retained transformed values as one bounded flat CSV."""

    if not path.strip() or len(path) > 4_096:
        view_model._show_error(
            ErrorInfo(
                code=ErrorCode.DATASET_CONFIGURATION,
                message="dataset CSV 导出路径无效。",
                recoverable=True,
            )
        )
        return
    output = io.StringIO(newline="")
    writer = csv.writer(output, lineterminator="\n")
    writer.writerow(
        (
            "time_monotonic",
            "sequence",
            "source",
            "raw_hex",
            "field",
            "display",
            "unit",
            "input_value",
            "value",
            "clipped",
            "warning",
            "error",
        )
    )
    for sample in view_model._dataset_samples:
        for value in sample.values:
            writer.writerow(
                (
                    f"{sample.occurred_at:.6f}",
                    sample.sequence,
                    sample.source.display,
                    sample.payload_hex,
                    value.field_name,
                    value.display,
                    value.unit,
                    _csv_scalar(value.input_value),
                    _csv_scalar(value.value),
                    "1" if value.clipped else "0",
                    value.warning or "",
                    value.error or "",
                )
            )
    try:
        Path(path).write_text(output.getvalue(), encoding="utf-8", newline="\n")
    except OSError as exc:
        view_model._show_error(
            ErrorInfo(
                code=ErrorCode.DATASET_CONFIGURATION,
                message="dataset CSV 导出失败。",
                recoverable=True,
                detail=str(exc),
            )
        )
        return
    view_model._set_status(f"dataset CSV 已导出 · {len(view_model._dataset_samples)} 个 sample")

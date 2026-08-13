"""Bounded in-memory dataset contracts for transformed component values."""

from __future__ import annotations

import json
import math
from dataclasses import dataclass, field
from typing import Final

from .components import (
    MAX_COMPONENT_DISPLAY_LENGTH,
    MAX_COMPONENT_PREVIEW_BYTES,
    ComponentFrameRow,
    ScalarValue,
)
from .errors import ConfigurationError
from .protocols import ProtocolSource
from .transforms import (
    EnumMapping,
    TransformChain,
    TransformKind,
    TransformSpec,
    format_scalar,
)

MAX_DATASET_CONFIG_BYTES: Final = 64 * 1024
MAX_DATASET_SERIES: Final = 32
MAX_DATASET_RETAINED_SAMPLES: Final = 1_024
DEFAULT_DATASET_RETAINED_SAMPLES: Final = 256


@dataclass(frozen=True, slots=True)
class DatasetSeriesConfig:
    """Select one component field and apply a bounded ordered chain."""

    field_name: str
    transforms: TransformChain = field(default_factory=TransformChain)
    unit: str = ""

    def __post_init__(self) -> None:
        if not isinstance(self.field_name, str) or not self.field_name.strip():
            raise ConfigurationError("dataset field 必须是非空字符串。")
        if len(self.field_name.strip()) > 64:
            raise ConfigurationError("dataset field 名称超过长度上限。")
        if not isinstance(self.transforms, TransformChain):
            raise ConfigurationError("dataset transforms 类型无效。")
        if not isinstance(self.unit, str) or len(self.unit.strip()) > 32:
            raise ConfigurationError("dataset unit 超过长度上限。")
        object.__setattr__(self, "field_name", self.field_name.strip())
        object.__setattr__(self, "unit", self.unit.strip())


@dataclass(frozen=True, slots=True)
class DatasetConfig:
    """Independent schema v1 dataset window configuration."""

    name: str = "Dataset preview"
    schema_version: int = 1
    capacity: int = DEFAULT_DATASET_RETAINED_SAMPLES
    series: tuple[DatasetSeriesConfig, ...] = ()

    def __post_init__(self) -> None:
        if not isinstance(self.name, str) or not self.name.strip() or len(self.name.strip()) > 64:
            raise ConfigurationError("dataset name 必须为 1 到 64 个字符。")
        if (
            isinstance(self.schema_version, bool)
            or not isinstance(self.schema_version, int)
            or self.schema_version != 1
        ):
            raise ConfigurationError("不支持的 dataset schema 版本。")
        if (
            isinstance(self.capacity, bool)
            or not isinstance(self.capacity, int)
            or not 1 <= self.capacity <= MAX_DATASET_RETAINED_SAMPLES
        ):
            raise ConfigurationError(
                f"dataset capacity 必须在 1 到 {MAX_DATASET_RETAINED_SAMPLES} 之间。"
            )
        series = tuple(self.series)
        if len(series) > MAX_DATASET_SERIES:
            raise ConfigurationError(f"dataset series 不能超过 {MAX_DATASET_SERIES} 项。")
        if any(not isinstance(item, DatasetSeriesConfig) for item in series):
            raise ConfigurationError("dataset series 类型无效。")
        names = [item.field_name.casefold() for item in series]
        if len(names) != len(set(names)):
            raise ConfigurationError("dataset field 不能重复。")
        object.__setattr__(self, "name", self.name.strip())
        object.__setattr__(self, "series", series)


@dataclass(frozen=True, slots=True)
class DatasetValue:
    """One transformed field value, retaining its input and visible status."""

    field_name: str
    input_value: ScalarValue
    value: ScalarValue
    display: str
    unit: str = ""
    clipped: bool = False
    warning: str | None = None
    error: str | None = None

    def __post_init__(self) -> None:
        if not isinstance(self.field_name, str) or not self.field_name.strip():
            raise ConfigurationError("dataset value field_name 无效。")
        for value in (self.input_value, self.value):
            if value is not None and not isinstance(value, (str, int, float, bool)):
                raise ConfigurationError("dataset value 必须是 scalar。")
            if isinstance(value, float) and not math.isfinite(value):
                raise ConfigurationError("dataset value 必须是有限 scalar。")
        if not isinstance(self.display, str) or len(self.display) > MAX_COMPONENT_DISPLAY_LENGTH:
            raise ConfigurationError("dataset value display 超过上限。")
        if not isinstance(self.unit, str) or len(self.unit) > 32:
            raise ConfigurationError("dataset value unit 超过上限。")
        if self.warning is not None and not self.warning.strip():
            raise ConfigurationError("dataset value warning 不能为空。")
        if self.error is not None and not self.error.strip():
            raise ConfigurationError("dataset value error 不能为空。")


@dataclass(frozen=True, slots=True)
class DatasetSample:
    """One bounded sample generated from one component row."""

    source: ProtocolSource
    sequence: int
    occurred_at: float
    payload_hex: str
    values: tuple[DatasetValue, ...] = ()

    def __post_init__(self) -> None:
        if not isinstance(self.source, ProtocolSource):
            raise ConfigurationError("dataset sample source 类型无效。")
        if (
            isinstance(self.sequence, bool)
            or not isinstance(self.sequence, int)
            or self.sequence < 1
        ):
            raise ConfigurationError("dataset sample sequence 必须是正整数。")
        if not isinstance(self.occurred_at, (int, float)) or not math.isfinite(self.occurred_at):
            raise ConfigurationError("dataset sample occurred_at 必须是有限数字。")
        if len(self.payload_hex) > MAX_COMPONENT_PREVIEW_BYTES * 2 + 3:
            raise ConfigurationError("dataset sample payload_hex 超过预览上限。")
        values = tuple(self.values)
        if len(values) > MAX_DATASET_SERIES:
            raise ConfigurationError("dataset sample values 超过上限。")
        if any(not isinstance(value, DatasetValue) for value in values):
            raise ConfigurationError("dataset sample values 类型无效。")
        object.__setattr__(self, "values", values)


@dataclass(frozen=True, slots=True)
class DatasetStats:
    """Bounded dataset worker counters."""

    frames_in: int = 0
    samples_out: int = 0
    transform_errors: int = 0
    dropped_batches: int = 0
    dropped_samples: int = 0
    retained_samples: int = 0
    buffered_bytes: int = 0


class DatasetConfigurationCodec:
    """Strict JSON loader/dumper for independent dataset schema v1."""

    @classmethod
    def loads(cls, text: str) -> DatasetConfig:
        """Loads."""
        if not isinstance(text, str):
            raise ConfigurationError("dataset 配置内容必须是文本。")
        if len(text.encode("utf-8")) > MAX_DATASET_CONFIG_BYTES:
            raise ConfigurationError("dataset 配置超过 64 KiB 上限。")
        try:
            value = json.loads(
                text,
                object_pairs_hook=_reject_duplicate_keys,
                parse_constant=_reject_constant,
            )
        except (json.JSONDecodeError, ValueError) as exc:
            raise ConfigurationError(f"dataset 配置 JSON 无效：{exc}。") from exc
        if not isinstance(value, dict) or set(value) != {
            "name",
            "schema_version",
            "capacity",
            "series",
        }:
            raise ConfigurationError("dataset 配置顶层字段不受支持。")
        raw_series = value["series"]
        if not isinstance(raw_series, list):
            raise ConfigurationError("dataset series 必须是数组。")
        series: list[DatasetSeriesConfig] = []
        for raw in raw_series:
            if not isinstance(raw, dict) or set(raw) != {"field", "unit", "transforms"}:
                raise ConfigurationError("dataset series 字段不受支持。")
            transforms = raw["transforms"]
            if not isinstance(transforms, list):
                raise ConfigurationError("dataset transforms 必须是数组。")
            series.append(
                DatasetSeriesConfig(
                    field_name=raw["field"],
                    unit=raw["unit"],
                    transforms=TransformChain(
                        tuple(cls._load_transform(item) for item in transforms)
                    ),
                )
            )
        return DatasetConfig(
            name=value["name"],
            schema_version=value["schema_version"],
            capacity=value["capacity"],
            series=tuple(series),
        )

    @staticmethod
    def dumps(config: DatasetConfig) -> str:
        """Dumps."""
        if not isinstance(config, DatasetConfig):
            raise ConfigurationError("只能序列化 DatasetConfig。")
        value = {
            "name": config.name,
            "schema_version": config.schema_version,
            "capacity": config.capacity,
            "series": [
                {
                    "field": item.field_name,
                    "unit": item.unit,
                    "transforms": [
                        _dump_transform(transform) for transform in item.transforms.specs
                    ],
                }
                for item in config.series
            ],
        }
        text = json.dumps(value, ensure_ascii=False, indent=2, sort_keys=True) + "\n"
        if len(text.encode("utf-8")) > MAX_DATASET_CONFIG_BYTES:
            raise ConfigurationError("dataset 配置序列化结果超过 64 KiB。")
        return text

    @staticmethod
    def _load_transform(value: object) -> TransformSpec:
        """Load transform."""
        if not isinstance(value, dict) or "kind" not in value:
            raise ConfigurationError("transform 定义必须声明 kind。")
        try:
            kind = TransformKind(value["kind"])
        except (TypeError, ValueError) as exc:
            raise ConfigurationError("transform kind 不受支持。") from exc
        if kind in {TransformKind.SCALE, TransformKind.OFFSET}:
            if set(value) != {"kind", "value"}:
                raise ConfigurationError("scale/offset transform 字段不受支持。")
            return TransformSpec(kind=kind, value=value["value"])
        if kind is TransformKind.CLAMP:
            if set(value) != {"kind", "minimum", "maximum"}:
                raise ConfigurationError("clamp transform 字段不受支持。")
            return TransformSpec(
                kind=kind,
                minimum=value["minimum"],
                maximum=value["maximum"],
            )
        if set(value) != {"kind", "mapping"} or not isinstance(value["mapping"], dict):
            raise ConfigurationError("enum transform 字段不受支持。")
        mapping = tuple(
            EnumMapping(key=key, label=label) for key, label in value["mapping"].items()
        )
        return TransformSpec(kind=kind, mappings=mapping)


def default_dataset_config() -> DatasetConfig:
    """Return an installed-but-disabled dataset configuration."""

    return DatasetConfig()


def sample_from_row(row: ComponentFrameRow, config: DatasetConfig) -> DatasetSample:
    """Project one component row into a bounded transformed sample."""

    fields = {field.name.casefold(): field for field in row.fields}
    values: list[DatasetValue] = []
    for series in config.series:
        field = fields.get(series.field_name.casefold())
        if field is None:
            values.append(
                DatasetValue(
                    field_name=series.field_name,
                    input_value=None,
                    value=None,
                    display="<缺少字段>",
                    unit=series.unit,
                    error="component row 中不存在该字段。",
                )
            )
            continue
        if field.error is not None:
            values.append(
                DatasetValue(
                    field_name=series.field_name,
                    input_value=field.value,
                    value=None,
                    display="<字段错误>",
                    unit=series.unit,
                    error=field.error,
                )
            )
            continue
        result = series.transforms.apply(field.value)
        values.append(
            DatasetValue(
                field_name=series.field_name,
                input_value=field.value,
                value=result.value,
                display=format_scalar(result.value),
                unit=series.unit,
                clipped=result.clipped,
                warning=result.warning,
                error=result.error,
            )
        )
    return DatasetSample(
        source=row.source,
        sequence=row.sequence,
        occurred_at=row.occurred_at,
        payload_hex=row.payload_hex,
        values=tuple(values),
    )


def _dump_transform(spec: TransformSpec) -> dict[str, object]:
    """Dump transform."""
    if spec.kind in {TransformKind.SCALE, TransformKind.OFFSET}:
        return {"kind": spec.kind.value, "value": spec.value}
    if spec.kind is TransformKind.CLAMP:
        return {"kind": spec.kind.value, "minimum": spec.minimum, "maximum": spec.maximum}
    return {
        "kind": spec.kind.value,
        "mapping": {item.key: item.label for item in spec.mappings},
    }


def _reject_duplicate_keys(pairs: list[tuple[str, object]]) -> dict[str, object]:
    """Reject duplicate keys."""
    result: dict[str, object] = {}
    for key, value in pairs:
        if key in result:
            raise ValueError(f"重复 JSON key：{key}")
        result[key] = value
    return result


def _reject_constant(value: str) -> None:
    """Reject constant."""
    raise ValueError(f"JSON 不允许特殊数字：{value}")


__all__ = [
    "DEFAULT_DATASET_RETAINED_SAMPLES",
    "MAX_DATASET_CONFIG_BYTES",
    "MAX_DATASET_RETAINED_SAMPLES",
    "MAX_DATASET_SERIES",
    "DatasetConfig",
    "DatasetConfigurationCodec",
    "DatasetSample",
    "DatasetSeriesConfig",
    "DatasetStats",
    "DatasetValue",
    "default_dataset_config",
    "sample_from_row",
]

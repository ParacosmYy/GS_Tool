"""Small, declarative transform chains with no executable user content."""

from __future__ import annotations

import math
from dataclasses import dataclass
from enum import StrEnum
from typing import Final

from .components import ScalarValue
from .errors import ConfigurationError

MAX_TRANSFORMS_PER_CHAIN: Final = 8
MAX_ENUM_ENTRIES: Final = 32
MAX_TRANSFORM_LABEL_LENGTH: Final = 128


class TransformKind(StrEnum):
    """Built-in transform operations; no user callable is accepted."""

    SCALE = "scale"
    OFFSET = "offset"
    CLAMP = "clamp"
    ENUM = "enum"


@dataclass(frozen=True, slots=True)
class EnumMapping:
    """One finite scalar key to bounded display label mapping."""

    key: str
    label: str

    def __post_init__(self) -> None:
        if not isinstance(self.key, str) or not self.key.strip():
            raise ConfigurationError("enum mapping key 必须是非空字符串。")
        if len(self.key) > MAX_TRANSFORM_LABEL_LENGTH:
            raise ConfigurationError("enum mapping key 超过长度上限。")
        if not isinstance(self.label, str) or not self.label.strip():
            raise ConfigurationError("enum mapping label 必须是非空字符串。")
        if len(self.label) > MAX_TRANSFORM_LABEL_LENGTH:
            raise ConfigurationError("enum mapping label 超过长度上限。")
        object.__setattr__(self, "key", self.key.strip())
        object.__setattr__(self, "label", self.label.strip())


@dataclass(frozen=True, slots=True)
class TransformSpec:
    """One statically validated transform declaration."""

    kind: TransformKind
    value: float | None = None
    minimum: float | None = None
    maximum: float | None = None
    mappings: tuple[EnumMapping, ...] = ()

    def __post_init__(self) -> None:
        if not isinstance(self.kind, TransformKind):
            raise ConfigurationError("transform kind 类型无效。")
        value = _finite_optional(self.value, "transform value")
        minimum = _finite_optional(self.minimum, "transform minimum")
        maximum = _finite_optional(self.maximum, "transform maximum")
        mappings = tuple(self.mappings)
        if self.kind in {TransformKind.SCALE, TransformKind.OFFSET}:
            if value is None or minimum is not None or maximum is not None or mappings:
                raise ConfigurationError("scale/offset transform 只允许 value。")
        elif self.kind is TransformKind.CLAMP:
            if value is not None or (minimum is None and maximum is None) or mappings:
                raise ConfigurationError("clamp transform 必须声明 minimum 或 maximum。")
            if minimum is not None and maximum is not None and minimum > maximum:
                raise ConfigurationError("clamp minimum 不能大于 maximum。")
        elif self.kind is TransformKind.ENUM:
            if value is not None or minimum is not None or maximum is not None:
                raise ConfigurationError("enum transform 只允许 mappings。")
            if not 1 <= len(mappings) <= MAX_ENUM_ENTRIES:
                raise ConfigurationError(f"enum mapping 数量必须在 1 到 {MAX_ENUM_ENTRIES} 之间。")
            if any(not isinstance(item, EnumMapping) for item in mappings):
                raise ConfigurationError("enum mappings 类型无效。")
            keys = [item.key.casefold() for item in mappings]
            if len(keys) != len(set(keys)):
                raise ConfigurationError("enum mapping key 不能重复。")
        object.__setattr__(self, "value", value)
        object.__setattr__(self, "minimum", minimum)
        object.__setattr__(self, "maximum", maximum)
        object.__setattr__(self, "mappings", mappings)


@dataclass(frozen=True, slots=True)
class TransformResult:
    """A scalar result with visible clipping/warning/error state."""

    value: ScalarValue
    clipped: bool = False
    warning: str | None = None
    error: str | None = None


@dataclass(frozen=True, slots=True)
class TransformChain:
    """Apply at most eight ordered transforms without parsing display text."""

    specs: tuple[TransformSpec, ...] = ()

    def __post_init__(self) -> None:
        specs = tuple(self.specs)
        if len(specs) > MAX_TRANSFORMS_PER_CHAIN:
            raise ConfigurationError(f"transform chain 不能超过 {MAX_TRANSFORMS_PER_CHAIN} 项。")
        if any(not isinstance(spec, TransformSpec) for spec in specs):
            raise ConfigurationError("transform chain 项类型无效。")
        enum_indexes = [
            index for index, spec in enumerate(specs) if spec.kind is TransformKind.ENUM
        ]
        if enum_indexes and enum_indexes[-1] != len(specs) - 1:
            raise ConfigurationError("enum transform 必须是 chain 最后一项。")
        if len(enum_indexes) > 1:
            raise ConfigurationError("transform chain 只能包含一个 enum。")
        object.__setattr__(self, "specs", specs)

    def apply(self, value: ScalarValue) -> TransformResult:
        """Apply."""
        current = value
        clipped = False
        warning: str | None = None
        for spec in self.specs:
            if spec.kind is TransformKind.ENUM:
                key = _scalar_key(current)
                mapping = next(
                    (item for item in spec.mappings if item.key.casefold() == key.casefold()),
                    None,
                )
                if mapping is None:
                    return TransformResult(
                        value=f"Unknown({key})",
                        clipped=clipped,
                        warning=f"enum 未定义值：{key}。",
                    )
                current = mapping.label
                continue
            if not _is_number(current):
                return TransformResult(
                    value=current,
                    clipped=clipped,
                    warning=warning,
                    error="transform 输入不是有限数字。",
                )
            try:
                if spec.kind is TransformKind.SCALE:
                    current = current * spec.value  # type: ignore[operator]
                elif spec.kind is TransformKind.OFFSET:
                    current = current + spec.value  # type: ignore[operator]
                else:
                    if spec.minimum is not None and current < spec.minimum:
                        current = spec.minimum
                        clipped = True
                    if spec.maximum is not None and current > spec.maximum:
                        current = spec.maximum
                        clipped = True
                if isinstance(current, float) and not math.isfinite(current):
                    raise ValueError("transform 结果不是有限数字")
            except (TypeError, ValueError, OverflowError) as exc:
                return TransformResult(
                    value=None,
                    clipped=clipped,
                    warning=warning,
                    error=f"transform 计算失败：{type(exc).__name__}。",
                )
        return TransformResult(value=current, clipped=clipped, warning=warning)


def _finite_optional(value: object, label: str) -> float | None:
    """Finite optional."""
    if value is None:
        return None
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise ConfigurationError(f"{label} 必须是数字或 null。")
    result = float(value)
    if not math.isfinite(result):
        raise ConfigurationError(f"{label} 必须是有限数字。")
    return result


def _is_number(value: ScalarValue) -> bool:
    """Is number."""
    return (
        isinstance(value, (int, float))
        and not isinstance(value, bool)
        and (not isinstance(value, float) or math.isfinite(value))
    )


def _scalar_key(value: ScalarValue) -> str:
    """Scalar key."""
    if isinstance(value, bool):
        return "true" if value else "false"
    if isinstance(value, float) and value.is_integer():
        return str(int(value))
    if value is None:
        return "null"
    return str(value)


def format_scalar(value: ScalarValue) -> str:
    """Format a transformed scalar without exposing unbounded text."""

    if value is None:
        return "<无值>"
    if isinstance(value, bool):
        return "true" if value else "false"
    if isinstance(value, float):
        if not math.isfinite(value):
            return "<非有限值>"
        return f"{value:.6g}"
    return str(value)[:MAX_TRANSFORM_LABEL_LENGTH]


__all__ = [
    "EnumMapping",
    "TransformChain",
    "TransformKind",
    "TransformResult",
    "TransformSpec",
    "format_scalar",
]

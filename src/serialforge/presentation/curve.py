"""Pure presentation data projection for one bounded Dataset curve."""

from __future__ import annotations

import math
from collections.abc import Iterable
from dataclasses import dataclass
from typing import Final

from ..domain.datasets import DatasetSample
from ..domain.protocols import DataOrigin, ProtocolSource

MAX_CURVE_POINTS: Final = 512


@dataclass(frozen=True, slots=True)
class CurvePoint:
    """One finite numeric point in a source-relative time window."""

    elapsed: float
    value: float
    sequence: int

    def __post_init__(self) -> None:
        elapsed = _finite_number(self.elapsed)
        if elapsed is None or elapsed < 0:
            raise ValueError("curve elapsed must be a finite non-negative number")
        value = _finite_number(self.value)
        if value is None:
            raise ValueError("curve value must be a finite number")
        if (
            isinstance(self.sequence, bool)
            or not isinstance(self.sequence, int)
            or self.sequence < 1
        ):
            raise ValueError("curve sequence must be a positive integer")
        object.__setattr__(self, "elapsed", elapsed)
        object.__setattr__(self, "value", value)


@dataclass(frozen=True, slots=True)
class CurveSnapshot:
    """Immutable bounded projection consumed by the Qt painter widget."""

    field_name: str | None = None
    unit: str = ""
    source: ProtocolSource | None = None
    points: tuple[CurvePoint, ...] = ()
    sample_count: int = 0
    skipped_points: int = 0
    truncated_points: int = 0

    def __post_init__(self) -> None:
        if self.field_name is not None and not isinstance(self.field_name, str):
            raise ValueError("curve field_name must be a string or None")
        if self.field_name is not None and not self.field_name.strip():
            raise ValueError("curve field_name must be non-empty or None")
        if not isinstance(self.unit, str) or len(self.unit) > 32:
            raise ValueError("curve unit is too long")
        if self.source is not None and not isinstance(self.source, ProtocolSource):
            raise ValueError("curve source must be ProtocolSource or None")
        points = tuple(self.points)
        if len(points) > MAX_CURVE_POINTS:
            raise ValueError(f"curve points cannot exceed {MAX_CURVE_POINTS}")
        if any(not isinstance(point, CurvePoint) for point in points):
            raise ValueError("curve points must use CurvePoint")
        counters = (self.sample_count, self.skipped_points, self.truncated_points)
        if any(
            isinstance(value, bool) or not isinstance(value, int) or value < 0 for value in counters
        ):
            raise ValueError("curve counters must be non-negative integers")
        object.__setattr__(self, "field_name", self.field_name.strip() if self.field_name else None)
        object.__setattr__(self, "unit", self.unit.strip())
        object.__setattr__(self, "points", points)

    @property
    def origin(self) -> DataOrigin | None:
        """Return the source origin without exposing a second source model."""

        return self.source.origin if self.source is not None else None

    @property
    def latest_value(self) -> float | None:
        """Latest value."""
        return self.points[-1].value if self.points else None

    @property
    def elapsed(self) -> float:
        """Elapsed."""
        return self.points[-1].elapsed if self.points else 0.0


def build_curve_snapshot(
    samples: Iterable[DatasetSample],
    *,
    field_name: str | None,
    capacity: int,
    point_limit: int = MAX_CURVE_POINTS,
) -> CurveSnapshot:
    """Build one source-isolated, finite numeric curve without parsing display text."""

    if not isinstance(capacity, int) or isinstance(capacity, bool) or not 1 <= capacity <= 1_024:
        raise ValueError("curve capacity must be between 1 and 1024")
    if (
        not isinstance(point_limit, int)
        or isinstance(point_limit, bool)
        or not 1 <= point_limit <= MAX_CURVE_POINTS
    ):
        raise ValueError(f"curve point_limit must be between 1 and {MAX_CURVE_POINTS}")
    normalized_field = (
        field_name.strip() if isinstance(field_name, str) and field_name.strip() else None
    )
    if normalized_field is None:
        return CurveSnapshot()
    normalized = tuple(sample for sample in samples if isinstance(sample, DatasetSample))[
        -capacity:
    ]
    if not normalized:
        return CurveSnapshot(field_name=normalized_field)
    source = normalized[0].source
    source_samples = tuple(sample for sample in normalized if sample.source == source)
    unit = ""
    values: list[tuple[DatasetSample, object]] = []
    for sample in source_samples:
        field = next(
            (
                item
                for item in sample.values
                if item.field_name.casefold() == normalized_field.casefold()
            ),
            None,
        )
        if field is None:
            values.append((sample, None))
            continue
        unit = field.unit
        values.append((sample, field))
    first_time = source_samples[0].occurred_at
    points: list[CurvePoint] = []
    skipped = 0
    for sample, field in values:
        if field is None or field.error is not None:
            skipped += 1
            continue
        value = _finite_number(field.value)
        if value is None:
            skipped += 1
            continue
        elapsed = sample.occurred_at - first_time
        if not math.isfinite(elapsed) or elapsed < 0:
            skipped += 1
            continue
        points.append(CurvePoint(elapsed=elapsed, value=value, sequence=sample.sequence))
    truncated = max(0, len(points) - point_limit)
    if truncated:
        points = points[-point_limit:]
    return CurveSnapshot(
        field_name=normalized_field,
        unit=unit,
        source=source,
        points=tuple(points),
        sample_count=len(source_samples),
        skipped_points=skipped,
        truncated_points=truncated,
    )


def _finite_number(value: object) -> float | None:
    """Finite number."""
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        return None
    try:
        normalized = float(value)
    except OverflowError:
        return None
    return normalized if math.isfinite(normalized) else None


__all__ = ["MAX_CURVE_POINTS", "CurvePoint", "CurveSnapshot", "build_curve_snapshot"]

"""Presentation-only selectors for bounded numeric configuration values."""

from __future__ import annotations

from collections.abc import Callable
from math import isclose, isfinite

from .qt import QComboBox, Qt

TIMEOUT_OPTION_VALUES: tuple[float, ...] = (
    0.0,
    0.001,
    0.005,
    0.01,
    0.02,
    0.05,
    0.1,
    0.2,
    0.5,
    1.0,
    2.0,
    3.0,
    5.0,
    10.0,
    15.0,
    30.0,
    60.0,
)

MAX_FRAME_OPTION_VALUES: tuple[int, ...] = (
    1,
    2,
    4,
    8,
    16,
    32,
    64,
    128,
    256,
    512,
    1_024,
    2_048,
    4_096,
    8_192,
    16_384,
    32_768,
    65_536,
)

UDP_DATAGRAM_OPTION_VALUES: tuple[int, ...] = (
    256,
    512,
    1_024,
    2_048,
    4_096,
    8_192,
    16_384,
    32_768,
    65_507,
)

TCP_CLIENT_OPTION_VALUES: tuple[int, ...] = (1, 2, 4, 8, 16, 32, 64)

COMMAND_DELAY_OPTION_VALUES: tuple[int, ...] = (
    0,
    10,
    20,
    50,
    100,
    200,
    500,
    1_000,
    1_500,
    2_000,
)


def format_timeout_value(seconds: float) -> str:
    """Use a compact unit that can be understood without manual conversion."""

    if isclose(seconds, 0.0, rel_tol=0.0, abs_tol=1e-9):
        return "未设置"
    if seconds < 1.0:
        return f"{seconds * 1000:g} ms"
    return f"{seconds:g} s"


def format_integer_value(value: float, suffix: str) -> str:
    """Format an integer option with a visible unit and thousands separators."""

    return f"{int(value):,}{suffix}"


class _BoundedNumericCombo(QComboBox):
    """Share selection, range, and legacy-value behavior across numeric controls."""

    def __init__(
        self,
        minimum: float,
        maximum: float,
        *,
        values: tuple[float, ...],
        formatter: Callable[[float], str],
    ) -> None:
        super().__init__()
        self._formatter = formatter
        self._option_values = tuple(float(value) for value in values)
        self._minimum = 0.0
        self._maximum = 0.0
        self.setEditable(False)
        self.setSizeAdjustPolicy(
            QComboBox.SizeAdjustPolicy.AdjustToMinimumContentsLengthWithIcon
        )
        self.setMinimumContentsLength(7)
        self.setMinimumWidth(96)
        self.setMaximumWidth(148)
        self.setRange(minimum, maximum)

    def value(self) -> float:
        """Return the selected numeric value for a presentation binding."""

        selected = self.currentData(Qt.ItemDataRole.UserRole)
        return float(selected) if isinstance(selected, (int, float)) else self._minimum

    def setValue(self, value: float) -> None:
        """Select a bounded value, preserving exact programmatic values."""

        self._select_value(value)

    def setRange(self, minimum: float, maximum: float) -> None:
        """Rebuild visible options when a protocol-specific limit changes."""

        if minimum > maximum:
            raise ValueError("minimum must not exceed maximum")
        current = self.value() if self.count() else float(minimum)
        signals_blocked = self.blockSignals(True)
        try:
            self._minimum = float(minimum)
            self._maximum = float(maximum)
            self._rebuild_options()
            self._select_value(current)
        finally:
            self.blockSignals(signals_blocked)

    def _rebuild_options(self) -> None:
        """Rebuild options."""
        self.clear()
        for candidate in self._option_values:
            if self._minimum <= candidate <= self._maximum:
                self._append_option(candidate)
        if self.count() == 0:
            self._append_option(self._minimum)

    def _select_value(self, value: float) -> None:
        """Select value."""
        numeric = float(value)
        if not isfinite(numeric):
            raise ValueError("bounded numeric option must be finite")
        numeric = min(max(numeric, self._minimum), self._maximum)
        index = self._find_option(numeric)
        if index < 0:
            index = self._insert_option(numeric)
        self.setCurrentIndex(index)

    def _append_option(self, value: float) -> int:
        """Append option."""
        index = self.count()
        self.addItem(self._formatter(value), value)
        self._set_accessible_text(index, value)
        return index

    def _find_option(self, value: float) -> int:
        """Find option."""
        for index in range(self.count()):
            selected = self.itemData(index, Qt.ItemDataRole.UserRole)
            if isinstance(selected, (int, float)) and isclose(
                float(selected), value, rel_tol=0.0, abs_tol=1e-9
            ):
                return index
        return -1

    def _insert_option(self, value: float) -> int:
        """Insert option."""
        index = self.count()
        for candidate_index in range(self.count()):
            selected = self.itemData(candidate_index, Qt.ItemDataRole.UserRole)
            if isinstance(selected, (int, float)) and value < float(selected):
                index = candidate_index
                break
        self.insertItem(index, self._formatter(value), value)
        self._set_accessible_text(index, value)
        return index

    def _set_accessible_text(self, index: int, value: float) -> None:
        """Set accessible text."""
        self.setItemData(
            index,
            f"{self._formatter(value)}（值 {value:g}）",
            Qt.ItemDataRole.AccessibleTextRole,
        )


class BoundedFloatCombo(_BoundedNumericCombo):
    """A non-editable selector whose runtime values are seconds."""

    def __init__(
        self,
        minimum: float,
        maximum: float,
        *,
        values: tuple[float, ...] = TIMEOUT_OPTION_VALUES,
    ) -> None:
        super().__init__(
            minimum,
            maximum,
            values=values,
            formatter=format_timeout_value,
        )
        if minimum == 0.0:
            self.setToolTip("未设置表示不额外限制该项超时。")

    def value(self) -> float:
        """Value."""
        return float(super().value())

    def setValue(self, value: float) -> None:
        """Setvalue."""
        super().setValue(value)


class BoundedIntCombo(_BoundedNumericCombo):
    """A non-editable selector for bounded integer configuration values."""

    def __init__(
        self,
        minimum: int,
        maximum: int,
        *,
        values: tuple[int, ...],
        suffix: str = "",
    ) -> None:
        super().__init__(
            minimum,
            maximum,
            values=values,
            formatter=lambda value: format_integer_value(value, suffix),
        )

    def value(self) -> int:
        """Value."""
        return int(super().value())

    def setValue(self, value: int) -> None:
        """Setvalue."""
        super().setValue(int(value))

    def setRange(self, minimum: int, maximum: int) -> None:
        """Setrange."""
        super().setRange(minimum, maximum)


__all__ = [
    "COMMAND_DELAY_OPTION_VALUES",
    "MAX_FRAME_OPTION_VALUES",
    "TCP_CLIENT_OPTION_VALUES",
    "TIMEOUT_OPTION_VALUES",
    "UDP_DATAGRAM_OPTION_VALUES",
    "BoundedFloatCombo",
    "BoundedIntCombo",
    "format_integer_value",
    "format_timeout_value",
]

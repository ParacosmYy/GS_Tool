"""Shared visual primitives for connection-panel composition.

These helpers keep labels, bounded selectors, and guidance rails consistent
across transport-specific builders without importing a window or controller.
"""

from __future__ import annotations

from ..qt import QComboBox, QLabel, QSizePolicy


def configure_bounded_combo(
    combo: QComboBox,
    *,
    minimum_width: int,
    maximum_width: int,
) -> None:
    """Bound long transport option labels without changing their values."""

    combo.setSizeAdjustPolicy(QComboBox.SizeAdjustPolicy.AdjustToMinimumContentsLengthWithIcon)
    combo.setMinimumContentsLength(8)
    combo.setMinimumWidth(minimum_width)
    combo.setMaximumWidth(maximum_width)


def configure_responsive_hint(label: QLabel) -> None:
    """Keep transport guidance readable without becoming a hard grid width."""

    description = label.text()
    label.setWordWrap(True)
    label.setMinimumWidth(120)
    label.setMaximumWidth(520)
    label.setSizePolicy(QSizePolicy.Policy.Ignored, QSizePolicy.Policy.Fixed)
    label.setToolTip(description)
    label.setAccessibleDescription(description)


def field_label(text: str) -> QLabel:
    """Create a secondary connection-form label with theme-aware contrast."""

    label = QLabel(text)
    label.setProperty("role", "muted")
    return label


def section_label(text: str) -> QLabel:
    """Create a connection section heading that cannot absorb spare height."""

    label = QLabel(text)
    label.setProperty("role", "section")
    label.setSizePolicy(QSizePolicy.Policy.Preferred, QSizePolicy.Policy.Fixed)
    return label


__all__ = [
    "configure_bounded_combo",
    "configure_responsive_hint",
    "field_label",
    "section_label",
]

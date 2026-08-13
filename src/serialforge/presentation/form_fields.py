"""Small presentation helpers for readable, vertically labelled controls."""

from __future__ import annotations

from collections.abc import Iterable

from .qt import QHBoxLayout, QLabel, QSizePolicy, QVBoxLayout, QWidget


def build_labeled_field(label_text: str, control: QWidget) -> QWidget:
    """Keep a label and its control together as one tab-order unit."""

    field = QWidget()
    field.setObjectName("formField")
    field.setSizePolicy(
        QSizePolicy.Policy.Expanding,
        QSizePolicy.Policy.Preferred,
    )
    label = QLabel(label_text, field)
    label.setObjectName("formFieldLabel")
    label.setProperty("role", "muted")
    label.setAccessibleName(f"{label_text}字段标签")
    label.setAccessibleDescription(f"{label_text}配置项")

    control_policy = control.sizePolicy()
    control_policy.setHorizontalPolicy(QSizePolicy.Policy.Expanding)
    control.setSizePolicy(control_policy)

    layout = QVBoxLayout(field)
    layout.setContentsMargins(0, 0, 0, 0)
    layout.setSpacing(4)
    layout.addWidget(label)
    layout.addWidget(control)
    return field


def build_field_row(fields: Iterable[QWidget]) -> QHBoxLayout:
    """Place peer fields in a balanced row with independent stretch."""

    row = QHBoxLayout()
    row.setContentsMargins(0, 0, 0, 0)
    row.setSpacing(12)
    items = tuple(fields)
    for field in items:
        row.addWidget(field, 1)
    return row


__all__ = ["build_field_row", "build_labeled_field"]

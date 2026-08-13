"""Shared presentation-only feedback state projection."""

from __future__ import annotations

from typing import Literal

from PyQt6.QtWidgets import QWidget

FeedbackLevel = Literal["info", "working", "success", "warning", "error"]


def apply_feedback_state(widget: QWidget, level: FeedbackLevel) -> None:
    """Apply one semantic state and refresh its centralized QSS projection."""
    widget.setProperty("state", level)
    style = widget.style()
    style.unpolish(widget)
    style.polish(widget)

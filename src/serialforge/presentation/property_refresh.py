"""Shared Qt dynamic-property refresh primitive for presentation surfaces."""

from __future__ import annotations

from .qt import QWidget


def refresh_dynamic_property(widget: QWidget, name: str, value: str | bool) -> None:
    """Re-polish a widget only when a semantic QSS property changes."""

    if widget.property(name) == value:
        return
    widget.setProperty(name, value)
    style = widget.style()
    style.unpolish(widget)
    style.polish(widget)
    widget.update()


__all__ = ["refresh_dynamic_property"]

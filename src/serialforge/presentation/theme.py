"""Public theme API for the SerialForge presentation layer.

The stylesheet, palette metadata, and selectable variants live in separate
modules so theme work does not grow the application shell or hide UI policy in
one large file.
"""

from __future__ import annotations

import os

from .popup_surface import refresh_combo_popup_themes
from .qt import QWidget
from .theme_stylesheet import THEME_STYLESHEET
from .theme_stylesheet_runtime import render_theme_stylesheet
from .theme_tokens import (
    ACCENT,
    ACCENT_BLUE,
    ACCENT_PINK,
    ACCENT_PURPLE,
    ACCENT_STRONG,
    BACKGROUND,
    BORDER,
    BORDER_STRONG,
    DEFAULT_THEME_KEY,
    ERROR,
    SURFACE,
    SURFACE_INPUT,
    SURFACE_RAISED,
    TERMINAL_BACKGROUND,
    TERMINAL_TEXT,
    TEXT,
    TEXT_MUTED,
    TEXT_SUBTLE,
    THEME_OPTIONS,
    WARNING,
    ThemeSpec,
    theme_spec,
)
from .theme_variants import build_theme_override

THEME_PROPERTY = "serialforgeThemeKey"


def stylesheet_for_theme(theme_key: str = DEFAULT_THEME_KEY) -> str:
    """Return the stable stylesheet plus the selected palette overrides."""

    return render_theme_stylesheet(theme_spec(theme_key))


def apply_theme(widget: QWidget, theme_key: str = DEFAULT_THEME_KEY) -> None:
    """Apply a named presentation theme to a root widget."""

    selected = theme_spec(theme_key)
    widget.setProperty(THEME_PROPERTY, selected.key)
    widget.setStyleSheet(stylesheet_for_theme(selected.key))
    refresh_combo_popup_themes(widget, selected.key)
    for child in widget.findChildren(QWidget):
        child.update()


def theme_key_for_widget(widget: QWidget | None) -> str:
    """Resolve the nearest theme key for a custom-painted widget or dialog."""

    current = widget
    while current is not None:
        value = current.property(THEME_PROPERTY)
        if isinstance(value, str) and value:
            return theme_spec(value).key
        current = current.parentWidget()
    return DEFAULT_THEME_KEY


def theme_spec_for_widget(widget: QWidget | None) -> ThemeSpec:
    """Return the palette used by custom painters without coupling them to QSS."""

    return theme_spec(theme_key_for_widget(widget))


def reduced_motion_requested() -> bool:
    """Read the opt-in reduced-motion preference used during startup."""

    value = os.getenv("SERIALFORGE_REDUCED_MOTION", "")
    return value.strip().lower() in {"1", "true", "yes", "on"}


__all__ = [
    "ACCENT",
    "ACCENT_BLUE",
    "ACCENT_PINK",
    "ACCENT_PURPLE",
    "ACCENT_STRONG",
    "BACKGROUND",
    "BORDER",
    "BORDER_STRONG",
    "DEFAULT_THEME_KEY",
    "ERROR",
    "SURFACE",
    "SURFACE_INPUT",
    "SURFACE_RAISED",
    "TERMINAL_BACKGROUND",
    "TERMINAL_TEXT",
    "TEXT",
    "TEXT_MUTED",
    "TEXT_SUBTLE",
    "THEME_OPTIONS",
    "THEME_PROPERTY",
    "THEME_STYLESHEET",
    "WARNING",
    "ThemeSpec",
    "apply_theme",
    "build_theme_override",
    "reduced_motion_requested",
    "stylesheet_for_theme",
    "theme_key_for_widget",
    "theme_spec",
    "theme_spec_for_widget",
]

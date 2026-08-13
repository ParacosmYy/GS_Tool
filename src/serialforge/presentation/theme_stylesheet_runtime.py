"""Pure stylesheet rendering boundary for selectable SerialForge themes.

The runtime renderer owns theme composition only.  It does not know about
widgets, controllers, transport state, or application services.
"""

from __future__ import annotations

from .theme_stylesheet import THEME_STYLESHEET
from .theme_stylesheet_controls import COMBO_POPUP_STYLESHEET, FILE_DIALOG_STYLESHEET
from .theme_tokens import DEFAULT_THEME_KEY, ThemeSpec, theme_spec
from .theme_variant_controls import (
    build_combo_popup_theme_override,
    build_file_dialog_theme_override,
)
from .theme_variants import build_theme_override


def render_theme_stylesheet(theme: ThemeSpec) -> str:
    """Render a complete QSS document for one validated theme specification."""

    if theme.key == DEFAULT_THEME_KEY:
        return THEME_STYLESHEET
    return THEME_STYLESHEET + build_theme_override(theme)


def render_combo_popup_stylesheet(theme_key: str) -> str:
    """Render only the frame stylesheet needed by a top-level combo popup."""

    selected = theme_spec(theme_key)
    if selected.key == DEFAULT_THEME_KEY:
        return COMBO_POPUP_STYLESHEET
    return COMBO_POPUP_STYLESHEET + build_combo_popup_theme_override(selected)


def render_file_dialog_stylesheet(theme_key: str) -> str:
    """Render the bounded stylesheet applied to a themed Qt file dialog."""

    selected = theme_spec(theme_key)
    if selected.key == DEFAULT_THEME_KEY:
        return FILE_DIALOG_STYLESHEET
    return FILE_DIALOG_STYLESHEET + build_file_dialog_theme_override(selected)


__all__ = [
    "render_combo_popup_stylesheet",
    "render_file_dialog_stylesheet",
    "render_theme_stylesheet",
]

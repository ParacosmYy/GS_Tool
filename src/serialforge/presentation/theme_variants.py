"""Composable overrides for user-selectable presentation themes."""

from __future__ import annotations

from .theme_tokens import ThemeSpec
from .theme_variant_controls import build_controls_theme_override
from .theme_variant_shell import build_shell_theme_override


def build_theme_override(theme: ThemeSpec) -> str:
    """Compose bounded shell and native-control theme overrides."""

    return build_shell_theme_override(theme) + build_controls_theme_override(theme)


__all__ = ["build_theme_override"]

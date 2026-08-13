"""Typed wiring for the global header chrome.

The bundle contains only header-owned Qt references. Presentation lifecycle
controllers use it instead of reaching through the composition window's
temporary construction attributes.
"""

from __future__ import annotations

from dataclasses import dataclass

from .bounded_text_label import BoundedTextLabel
from .brand_mark_surface import BrandMarkSurface
from .qt import QCheckBox, QComboBox, QFrame
from .theme_palette_surface import ThemePaletteSwatch
from .widgets import SignalFieldWidget, StatusIndicator


@dataclass(frozen=True, slots=True)
class HeaderChromeBindings:
    """Immutable references for the status, motion, and theme header zones."""

    status_cluster: QFrame
    context_label: BoundedTextLabel
    source_badge: BoundedTextLabel
    state_indicator: StatusIndicator
    state_label: BoundedTextLabel
    motion_controls: QFrame
    motion_check: QCheckBox
    motion_pause_check: QCheckBox
    theme_controls: QFrame
    theme_combo: QComboBox
    theme_palette_swatch: ThemePaletteSwatch
    brand_mark: BrandMarkSurface
    signal_field: SignalFieldWidget


def header_chrome_bindings_for(window: object) -> HeaderChromeBindings | None:
    """Read the typed header bundle without importing MainWindow."""

    bindings = getattr(window, "_header_chrome_bindings", None)
    return bindings if isinstance(bindings, HeaderChromeBindings) else None


__all__ = ["HeaderChromeBindings", "header_chrome_bindings_for"]

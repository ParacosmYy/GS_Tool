"""Qt projection helpers for the shared, Qt-free font-style contract."""

from __future__ import annotations

from PyQt6.QtGui import QFont

from ..domain.models import FontStyle

_FONT_STYLES = frozenset({"regular", "semibold", "bold", "italic"})


def normalize_font_style(style: object) -> FontStyle:
    """Return a supported style without letting malformed settings reach Qt."""
    return style if isinstance(style, str) and style in _FONT_STYLES else "regular"


def font_with_style(font: QFont, style: object) -> QFont:
    """Copy a font and project one supported weight/italic combination."""
    normalized = normalize_font_style(style)
    styled = QFont(font)
    styled.setItalic(normalized == "italic")
    styled.setWeight(
        {
            "regular": QFont.Weight.Normal,
            "semibold": QFont.Weight.DemiBold,
            "bold": QFont.Weight.Bold,
            "italic": QFont.Weight.Normal,
        }[normalized]
    )
    return styled


def qss_font_weight(style: object) -> str:
    """Return the QSS weight token for one supported font style."""
    return {"regular": "400", "semibold": "600", "bold": "700", "italic": "400"}[
        normalize_font_style(style)
    ]


def qss_font_style(style: object) -> str:
    """Return the QSS slant token for one supported font style."""
    return "italic" if normalize_font_style(style) == "italic" else "normal"

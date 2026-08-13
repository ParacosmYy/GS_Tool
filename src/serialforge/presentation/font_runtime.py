"""Runtime font selection at the presentation boundary.

The shell uses system fonts instead of shipping a font asset.  Some Qt
offscreen/plugin environments expose an empty font database, so this module
may register an already-installed local Windows font as a last resort.  It
owns no business state, timers, persistence, or device access.
"""

from __future__ import annotations

import os
from pathlib import Path

from .qt import QApplication, QFont, QFontDatabase

_PREFERRED_FAMILIES = (
    "Microsoft YaHei UI",
    "Microsoft YaHei",
    "Noto Sans SC",
    "Noto Sans CJK SC",
    "Segoe UI",
)
_CJK_FAMILIES = frozenset(_PREFERRED_FAMILIES[:4])


def configure_application_font(app: QApplication) -> str | None:
    """Select a readable system family and return the chosen family name.

    The normal Windows path uses the operating system font database.  The
    local-file fallback is intentionally bounded to standard installed font
    locations and is only attempted when no CJK-capable family is visible.
    """

    families = set(QFontDatabase.families())
    if not _CJK_FAMILIES.intersection(families):
        _register_local_system_font()
        families = set(QFontDatabase.families())

    family = next((name for name in _PREFERRED_FAMILIES if name in families), None)
    if family is None and families:
        family = sorted(families)[0]
    if family is None:
        return None

    app.setFont(QFont(family, 10))
    return family


def _register_local_system_font() -> bool:
    """Register one existing local CJK font without copying or persisting it."""

    windir = os.environ.get("WINDIR")
    candidates = (
        Path(windir) / "Fonts" / "msyh.ttc" if windir else None,
        Path("C:/Windows/Fonts/msyh.ttc"),
        Path("/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc"),
        Path("/usr/share/fonts/truetype/noto/NotoSansSC-Regular.otf"),
    )
    for candidate in candidates:
        if candidate is None or not candidate.is_file():
            continue
        if QFontDatabase.addApplicationFont(str(candidate)) >= 0:
            return True
    return False


__all__ = ["configure_application_font"]

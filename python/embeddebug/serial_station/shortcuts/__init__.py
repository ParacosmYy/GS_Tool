"""键盘快捷键统一管理体系（ShortcutManager）。"""

from __future__ import annotations

from embeddebug.serial_station.shortcuts.definitions import (
    DEFAULT_SHORTCUTS,
    ShortcutCategory,
    ShortcutDef,
)
from embeddebug.serial_station.shortcuts.manager import ShortcutManager

__all__ = ["DEFAULT_SHORTCUTS", "ShortcutCategory", "ShortcutDef", "ShortcutManager"]

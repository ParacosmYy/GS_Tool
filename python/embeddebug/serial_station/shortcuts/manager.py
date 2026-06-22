"""ShortcutManager：快捷键的装配、重绑、重置与持久化。"""

from __future__ import annotations

from dataclasses import dataclass, field
from typing import Any

from PyQt6.QtCore import Qt
from PyQt6.QtGui import QKeySequence, QShortcut
from PyQt6.QtWidgets import QMainWindow

from embeddebug.serial_station.shortcuts.definitions import (
    DEFAULT_SHORTCUTS,
    ShortcutCategory,
    ShortcutDef,
)


def _effective_key(def_: ShortcutDef, overrides: dict[str, str]) -> str:
    return overrides.get(def_.id, def_.default_key_sequence)


@dataclass
class ShortcutManager:
    """键盘快捷键统一管理器。"""

    host: QMainWindow | None = None
    overrides: dict[str, str] = field(default_factory=dict)
    _shortcuts: dict[str, QShortcut] = field(default_factory=dict, repr=False)

    def register(self, host: QMainWindow) -> None:
        self.host = host
        self._teardown()
        for def_ in DEFAULT_SHORTCUTS:
            self._install(def_)

    def _teardown(self) -> None:
        for sc in self._shortcuts.values():
            try:
                sc.setEnabled(False)
                sc.deleteLater()
            except RuntimeError:
                pass
        self._shortcuts.clear()

    def _install(self, def_: ShortcutDef) -> None:
        if self.host is None:
            return
        key = _effective_key(def_, self.overrides)
        shortcut = QShortcut(QKeySequence(key), self.host)
        shortcut.setObjectName(f"serialStationShortcut_{def_.id}")
        shortcut.setContext(Qt.ShortcutContext.ApplicationShortcut)
        callback = getattr(self.host, def_.callback_name, None)
        if callable(callback):
            shortcut.activated.connect(callback)
        self._shortcuts[def_.id] = shortcut

    def list_shortcuts(self) -> list[ShortcutDef]:
        return list(DEFAULT_SHORTCUTS)

    def key_sequence_for(self, shortcut_id: str) -> str | None:
        def_ = self._find_def(shortcut_id)
        return _effective_key(def_, self.overrides) if def_ else None

    def _find_def(self, shortcut_id: str) -> ShortcutDef | None:
        for def_ in DEFAULT_SHORTCUTS:
            if def_.id == shortcut_id:
                return def_
        return None

    def rebind(self, shortcut_id: str, new_key_sequence: str) -> bool:
        def_ = self._find_def(shortcut_id)
        if def_ is None:
            return False
        self.overrides[shortcut_id] = new_key_sequence
        shortcut = self._shortcuts.get(shortcut_id)
        if shortcut is not None:
            shortcut.setKey(QKeySequence(new_key_sequence))
        return True

    def reset_to_defaults(self) -> None:
        self.overrides.clear()
        if self.host is not None:
            self._teardown()
            for def_ in DEFAULT_SHORTCUTS:
                self._install(def_)

    def to_dict(self) -> dict[str, Any]:
        return {"version": 1, "overrides": dict(self.overrides)}

    @classmethod
    def from_dict(cls, data: dict[str, Any]) -> ShortcutManager:
        raw = data.get("overrides", {}) if isinstance(data, dict) else {}
        valid_ids = {d.id for d in DEFAULT_SHORTCUTS}
        overrides = {str(k): str(v) for k, v in raw.items() if k in valid_ids and v is not None}
        return cls(overrides=overrides)

    def grouped_by_category(self) -> dict[ShortcutCategory, list[ShortcutDef]]:
        groups: dict[ShortcutCategory, list[ShortcutDef]] = {}
        for def_ in DEFAULT_SHORTCUTS:
            groups.setdefault(def_.category, []).append(def_)
        return groups

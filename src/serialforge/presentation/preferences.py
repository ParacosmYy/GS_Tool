"""Versioned presentation preferences and their single Qt persistence adapter."""

from __future__ import annotations

import logging
from dataclasses import dataclass
from typing import Protocol

from .qt import QSettings
from .theme_tokens import DEFAULT_THEME_KEY, THEME_OPTIONS

LOGGER = logging.getLogger(__name__)
PREFERENCES_SCHEMA_VERSION = 1
PREFERENCES_GROUP = "presentation"


def _is_known_theme(key: object) -> bool:
    """Is known theme."""
    return isinstance(key, str) and any(theme.key == key for theme in THEME_OPTIONS)


@dataclass(frozen=True, slots=True)
class PresentationPreferences:
    """Small, versioned UI preference DTO; it never stores a ThemeSpec copy."""

    schema_version: int = PREFERENCES_SCHEMA_VERSION
    theme_key: str = DEFAULT_THEME_KEY
    reduced_motion: bool = False
    motion_paused: bool = False

    def __post_init__(self) -> None:
        if self.schema_version != PREFERENCES_SCHEMA_VERSION:
            raise ValueError("unsupported presentation preference schema")
        if not _is_known_theme(self.theme_key):
            raise ValueError("unknown presentation theme key")
        if not isinstance(self.reduced_motion, bool) or not isinstance(self.motion_paused, bool):
            raise ValueError("motion preferences must be booleans")


class PreferenceStore(Protocol):
    """Persistence port consumed by bootstrap and lifecycle controllers."""

    def load(self) -> PresentationPreferences:
        """Load validated preferences or safe defaults."""

    def save(self, preferences: PresentationPreferences) -> None:
        """Persist preferences without changing current UI state on failure."""


def _as_schema_version(value: object) -> int | None:
    """As schema version."""
    if isinstance(value, bool):
        return None
    if isinstance(value, int):
        return value
    if isinstance(value, str) and value.strip().isdigit():
        return int(value.strip())
    return None


def _as_bool(value: object, default: bool) -> bool:
    """As bool."""
    if isinstance(value, bool):
        return value
    if isinstance(value, int) and value in {0, 1}:
        return bool(value)
    if isinstance(value, str):
        normalized = value.strip().lower()
        if normalized in {"1", "true", "yes", "on"}:
            return True
        if normalized in {"0", "false", "no", "off"}:
            return False
    return default


class QSettingsPreferenceStore:
    """Qt-only adapter; DTOs and controller code stay independent of QSettings."""

    def __init__(self, settings: QSettings | None = None) -> None:
        self._settings = settings or QSettings("SerialForge", "SerialForge")

    def load(self) -> PresentationPreferences:
        """Load one known schema and fall back field-by-field on bad values."""

        self._settings.beginGroup(PREFERENCES_GROUP)
        try:
            schema = _as_schema_version(self._settings.value("schema_version"))
            if schema != PREFERENCES_SCHEMA_VERSION:
                return PresentationPreferences()
            raw_theme = self._settings.value("theme_key")
            theme_key = raw_theme if _is_known_theme(raw_theme) else DEFAULT_THEME_KEY
            return PresentationPreferences(
                theme_key=theme_key,
                reduced_motion=_as_bool(self._settings.value("reduced_motion"), False),
                motion_paused=_as_bool(self._settings.value("motion_paused"), False),
            )
        except (TypeError, ValueError):
            return PresentationPreferences()
        finally:
            self._settings.endGroup()

    def save(self, preferences: PresentationPreferences) -> None:
        """Persist a validated snapshot; logging failure keeps the UI fail-open."""

        try:
            self._settings.beginGroup(PREFERENCES_GROUP)
            self._settings.setValue("schema_version", preferences.schema_version)
            self._settings.setValue("theme_key", preferences.theme_key)
            self._settings.setValue("reduced_motion", preferences.reduced_motion)
            self._settings.setValue("motion_paused", preferences.motion_paused)
            self._settings.endGroup()
            self._settings.sync()
            if self._settings.status() != QSettings.Status.NoError:
                LOGGER.warning("SerialForge presentation preferences could not be persisted")
        except (OSError, RuntimeError, TypeError, ValueError):
            LOGGER.warning("SerialForge presentation preference save failed", exc_info=True)


__all__ = [
    "PREFERENCES_GROUP",
    "PREFERENCES_SCHEMA_VERSION",
    "PreferenceStore",
    "PresentationPreferences",
    "QSettingsPreferenceStore",
]

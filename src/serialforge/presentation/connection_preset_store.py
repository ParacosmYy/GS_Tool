"""Persistence port and Qt adapter for safe connection preset catalogs."""

from __future__ import annotations

import logging
from typing import Protocol

from .connection_preset_codec import decode_catalog, encode_catalog
from .connection_presets import (
    BUILTIN_CONNECTION_PRESET_CATALOG,
    ConnectionPresetCatalog,
    custom_connection_presets,
    merge_connection_preset_catalog,
)
from .qt import QSettings

LOGGER = logging.getLogger(__name__)
CONNECTION_PRESET_GROUP = "connection_presets"
CONNECTION_PRESET_KEY = "catalog_json"


class ConnectionPresetCatalogStore(Protocol):
    """Stable persistence boundary for an immutable presentation catalog."""

    def load(self) -> ConnectionPresetCatalog:
        """Return a validated catalog or the built-in safe fallback."""

    def save(self, catalog: ConnectionPresetCatalog) -> bool:
        """Persist a validated catalog and report whether the adapter accepted it."""


class QSettingsConnectionPresetCatalogStore:
    """Qt-only adapter; it stores no secrets, device identity, or auto-connect action."""

    def __init__(self, settings: QSettings | None = None) -> None:
        self._settings = settings or QSettings("SerialForge", "SerialForge")

    def load(self) -> ConnectionPresetCatalog:
        """Load."""
        entered = False
        try:
            self._settings.beginGroup(CONNECTION_PRESET_GROUP)
            entered = True
            raw = self._settings.value(CONNECTION_PRESET_KEY)
            if raw is None:
                return BUILTIN_CONNECTION_PRESET_CATALOG
            try:
                loaded = decode_catalog(raw)
                return merge_connection_preset_catalog(custom_connection_presets(loaded))
            except (TypeError, ValueError, UnicodeError):
                LOGGER.warning("SerialForge connection preset catalog invalid; using built-ins")
                return BUILTIN_CONNECTION_PRESET_CATALOG
        except Exception:
            LOGGER.warning("SerialForge connection preset catalog load failed", exc_info=True)
            return BUILTIN_CONNECTION_PRESET_CATALOG
        finally:
            if entered:
                try:
                    self._settings.endGroup()
                except Exception:
                    LOGGER.warning(
                        "SerialForge connection preset catalog group close failed",
                        exc_info=True,
                    )

    def save(self, catalog: ConnectionPresetCatalog) -> bool:
        """Persist only custom entries and report whether Qt accepted the write."""

        try:
            custom = custom_connection_presets(catalog)
            storage_catalog = (
                BUILTIN_CONNECTION_PRESET_CATALOG
                if not custom
                else ConnectionPresetCatalog(custom)
            )
            encoded = encode_catalog(storage_catalog)
            entered = False
            write_succeeded = False
            try:
                self._settings.beginGroup(CONNECTION_PRESET_GROUP)
                entered = True
                self._settings.setValue(CONNECTION_PRESET_KEY, encoded)
                write_succeeded = True
            finally:
                if entered:
                    try:
                        self._settings.endGroup()
                    except Exception:
                        write_succeeded = False
                        LOGGER.warning(
                            "SerialForge connection preset catalog group close failed",
                            exc_info=True,
                        )
            if not write_succeeded:
                return False
            self._settings.sync()
            if self._settings.status() != QSettings.Status.NoError:
                LOGGER.warning("SerialForge connection preset catalog could not be persisted")
                return False
            return True
        except Exception:
            LOGGER.warning("SerialForge connection preset catalog save failed", exc_info=True)
            return False


__all__ = [
    "CONNECTION_PRESET_GROUP",
    "CONNECTION_PRESET_KEY",
    "ConnectionPresetCatalogStore",
    "QSettingsConnectionPresetCatalogStore",
]

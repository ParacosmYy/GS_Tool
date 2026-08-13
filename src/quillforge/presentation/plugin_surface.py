"""Presentation composition for plugin catalog and runtime status dialogs."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass

from PyQt6.QtWidgets import QMainWindow

from ..application.plugin_catalog import PluginCatalogSnapshot
from ..application.plugin_runtime import PluginRuntimeStatus
from ..domain.models import Locale
from .i18n import normalize_locale
from .plugin_catalog_dialog import PluginCatalogDialog
from .plugin_status_dialog import PluginStatusDialog


@dataclass(frozen=True, slots=True)
class PluginSurfaceCallbacks:
    """Semantic plugin-dialog intents routed back to MainWindow."""

    approve_requested: Callable[[object], None]
    revoke_requested: Callable[[object], None]
    enable_requested: Callable[[str], None]
    disable_requested: Callable[[str], None]


class PluginSurface:
    """Own plugin-dialog lifecycle without owning plugin governance policy."""

    def __init__(
        self,
        parent: QMainWindow,
        *,
        locale: Locale,
        callbacks: PluginSurfaceCallbacks,
    ) -> None:
        self._parent = parent
        self._locale = normalize_locale(locale)
        self._callbacks = callbacks
        self._catalog_dialog: PluginCatalogDialog | None = None
        self._status_dialog: PluginStatusDialog | None = None

    def set_locale(self, locale: Locale) -> None:
        """Refresh any open plugin dialogs and remember the next locale."""
        self._locale = normalize_locale(locale)
        if self._catalog_dialog is not None:
            self._catalog_dialog.set_locale(self._locale)
        if self._status_dialog is not None:
            self._status_dialog.set_locale(self._locale)

    def show_catalog(self, snapshot: PluginCatalogSnapshot) -> None:
        """Replace and activate the catalog projection for one scan result."""
        if self._catalog_dialog is not None:
            self._catalog_dialog.close()
        dialog = PluginCatalogDialog(snapshot, self._parent, locale=self._locale)
        dialog.approve_requested.connect(self._callbacks.approve_requested)
        dialog.revoke_requested.connect(self._callbacks.revoke_requested)
        dialog.show()
        dialog.raise_()
        dialog.activateWindow()
        self._catalog_dialog = dialog

    def show_status(self, statuses: tuple[PluginRuntimeStatus, ...]) -> None:
        """Replace and activate the registered-plugin status projection."""
        if self._status_dialog is not None:
            self._status_dialog.close()
        dialog = PluginStatusDialog(statuses, self._parent, locale=self._locale)
        dialog.enable_requested.connect(self._callbacks.enable_requested)
        dialog.disable_requested.connect(self._callbacks.disable_requested)
        dialog.show()
        dialog.raise_()
        dialog.activateWindow()
        self._status_dialog = dialog

    def set_catalog_governance_actions_enabled(self, enabled: bool) -> None:
        """Project worker ownership into the catalog mutation controls."""
        if self._catalog_dialog is not None:
            self._catalog_dialog.set_governance_actions_enabled(enabled)

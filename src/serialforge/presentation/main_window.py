"""UART-first workspace built entirely on application ports."""

from __future__ import annotations

from .connection_preset_store import ConnectionPresetCatalogStore
from .connection_presets import ConnectionPresetCatalog
from .controllers.bootstrap import initialize_window
from .controllers.lifecycle import (
    changeEvent,
    closeEvent,
    hideEvent,
    resizeEvent,
    showEvent,
)
from .preferences import PreferenceStore
from .qt import QMainWindow, QWidget
from .viewmodels import SessionViewModel


class MainWindow(QMainWindow):
    """Composition shell with only construction and Qt lifecycle overrides."""

    def __init__(
        self,
        view_model: SessionViewModel,
        preference_store: PreferenceStore,
        preset_catalog: ConnectionPresetCatalog,
        preset_store: ConnectionPresetCatalogStore,
        parent: QWidget | None = None,
    ) -> None:
        super().__init__(parent)
        initialize_window(self, view_model, preference_store, preset_catalog, preset_store)

    def closeEvent(self, *args: object, **kwargs: object) -> object:
        """Delegate Qt close lifecycle to the lifecycle controller."""

        return closeEvent(self, *args, **kwargs)

    def resizeEvent(self, *args: object, **kwargs: object) -> object:
        """Delegate resize cleanup before Qt recalculates the shell layout."""

        return resizeEvent(self, *args, **kwargs)

    def hideEvent(self, *args: object, **kwargs: object) -> object:
        """Delegate Qt hide lifecycle to the lifecycle controller."""

        return hideEvent(self, *args, **kwargs)

    def showEvent(self, *args: object, **kwargs: object) -> object:
        """Delegate Qt show lifecycle to the lifecycle controller."""

        return showEvent(self, *args, **kwargs)

    def changeEvent(self, *args: object, **kwargs: object) -> object:
        """Delegate Qt change lifecycle to the lifecycle controller."""

        return changeEvent(self, *args, **kwargs)

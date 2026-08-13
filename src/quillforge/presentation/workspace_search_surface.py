"""Presentation composition for the non-modal workspace search surface."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass
from pathlib import Path

from PyQt6.QtWidgets import QMainWindow

from ..domain.models import Locale
from .workspace_search_dialog import WorkspaceSearchDialog


@dataclass(frozen=True, slots=True)
class WorkspaceSearchSurfaceCallbacks:
    """Semantic search intents routed back to the owning shell."""

    search_requested: Callable[[str, bool], None]
    cancel_requested: Callable[[], None]
    file_requested: Callable[[object, int], None]


class WorkspaceSearchSurface:
    """Own the search dialog projection without owning search operations."""

    def __init__(
        self,
        parent: QMainWindow,
        *,
        root: Path,
        locale: Locale,
        callbacks: WorkspaceSearchSurfaceCallbacks,
    ) -> None:
        self._dialog = WorkspaceSearchDialog(root, parent, locale=locale)
        self._dialog.search_requested.connect(callbacks.search_requested)
        self._dialog.cancel_requested.connect(callbacks.cancel_requested)
        self._dialog.file_requested.connect(callbacks.file_requested)

    @property
    def root(self) -> Path:
        """Return the workspace root currently projected by the dialog."""
        return self._dialog.root

    def set_root(self, root: Path) -> None:
        """Switch the explicit search scope and clear stale result rows."""
        self._dialog.set_root(root)

    def set_locale(self, locale: Locale) -> None:
        """Refresh user-facing search text while retaining dialog state."""
        self._dialog.set_locale(locale)

    def show(self) -> None:
        """Present the non-modal search dialog as the active desktop surface."""
        self._dialog.show()
        self._dialog.raise_()
        self._dialog.activateWindow()

    def set_busy(self, busy: bool) -> None:
        """Project the active worker state into the dialog controls."""
        self._dialog.set_busy(busy)

    def set_cancel_requested(self) -> None:
        """Project cooperative cancellation feedback into the dialog."""
        self._dialog.set_cancel_requested()

    def present_result(self, result: object) -> None:
        """Project an application result without taking ownership of its policy."""
        self._dialog.present_result(result)

    def present_error(self, message: str) -> None:
        """Project a recoverable search error without changing the search scope."""
        self._dialog.present_error(message)

    def present_cancelled(self) -> None:
        """Project completion of a stale or explicitly invalidated search."""
        self._dialog.present_cancelled()

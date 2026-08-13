"""Presentation composition for the current-document find surface."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass

from PyQt6.QtWidgets import QWidget

from ..domain.models import Locale
from .feedback import FeedbackLevel
from .find_bar import FindBar


@dataclass(frozen=True, slots=True)
class FindSurfaceCallbacks:
    """Semantic find/replace intents routed back to the owning shell."""

    find_requested: Callable[[bool], None]
    replace_requested: Callable[[], None]
    replace_all_requested: Callable[[], None]
    cancel_requested: Callable[[], None]
    close_requested: Callable[[], None]
    criteria_changed: Callable[[], None]


class FindSurface:
    """Own the FindBar widget projection without owning editor policy."""

    def __init__(
        self,
        parent: QWidget,
        *,
        locale: Locale,
        callbacks: FindSurfaceCallbacks,
    ) -> None:
        self._bar = FindBar(parent, locale=locale)
        self._bar.find_requested.connect(callbacks.find_requested)
        self._bar.replace_requested.connect(callbacks.replace_requested)
        self._bar.replace_all_requested.connect(callbacks.replace_all_requested)
        self._bar.cancel_requested.connect(callbacks.cancel_requested)
        self._bar.close_requested.connect(callbacks.close_requested)
        self._bar.criteria_changed.connect(callbacks.criteria_changed)

    @property
    def widget(self) -> FindBar:
        """Return the widget for composition into the editor shell."""
        return self._bar

    def hide(self) -> None:
        """Hide the bar while leaving its query state available to the shell."""
        self._bar.hide()

    def show_find(self, *, replacement: bool = False) -> None:
        """Show the selected find mode and focus its query field."""
        self._bar.show_find(replacement=replacement)

    def query(self) -> str:
        return self._bar.query()

    def replacement(self) -> str:
        return self._bar.replacement()

    def case_sensitive(self) -> bool:
        return self._bar.case_sensitive()

    def set_status(self, message: str, *, level: FeedbackLevel = "info") -> None:
        self._bar.set_status(message, level=level)

    def set_locale(self, locale: Locale) -> None:
        self._bar.set_locale(locale)

    def set_operation_active(self, active: bool) -> None:
        self._bar.set_operation_active(active)

    def reset_session(self) -> None:
        self._bar.reset_session()

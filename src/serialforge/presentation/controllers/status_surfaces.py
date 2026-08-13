"""Shared status-label projection without exposing the MainWindow object."""

from __future__ import annotations

from dataclasses import dataclass

from ..contracts import DynamicPropertyRefresher, StatusSurfaceSource
from ..qt import QLabel


@dataclass(frozen=True, slots=True)
class _StatusSurface:
    widget: QLabel


def status_surface_source(window) -> str:
    """Return the source label shared by presentation status surfaces."""

    return "history" if window._history_source_active or window._history_file_selected else "live"


class StatusSurfaceController:
    """Own stable selectors for status labels and project source/state properties."""

    def __init__(
        self,
        *,
        source: StatusSurfaceSource,
        refresh_property: DynamicPropertyRefresher,
    ) -> None:
        self._source = source
        self._refresh_property = refresh_property
        self._surfaces: dict[str, _StatusSurface] = {}

    def register(self, key: str, widget: QLabel, *, object_name: str, state: str) -> None:
        """Register one label before its first state refresh."""

        if key in self._surfaces:
            raise ValueError(f"duplicate status surface: {key}")
        widget.setObjectName(object_name)
        widget.setProperty("source", self._source())
        widget.setProperty("state", state)
        self._surfaces[key] = _StatusSurface(widget)

    def set_state(self, key: str, state: str) -> None:
        """Update source and state through the shell's edge-triggered property refresher."""

        surface = self._surfaces[key]
        self._refresh_property(surface.widget, "source", self._source())
        self._refresh_property(surface.widget, "state", state)


__all__ = ["StatusSurfaceController", "status_surface_source"]

"""Presentation-only vertical rhythm for themed settings pages."""

from __future__ import annotations

from enum import StrEnum

from .qt import QScrollArea, Qt, QWidget


class ShortPageVerticalRhythm(StrEnum):
    """Choose the vertical rhythm for a short, non-overflowing page."""

    CENTER = "center"
    TOP = "top"


class ResponsiveScrollArea(QScrollArea):
    """Keep short-page alignment explicit without changing the scroll owner."""

    def __init__(
        self,
        page_layout,
        parent: QWidget | None = None,
        *,
        vertical_rhythm: ShortPageVerticalRhythm = ShortPageVerticalRhythm.CENTER,
    ) -> None:
        super().__init__(parent)
        self._page_layout = page_layout
        self._vertical_rhythm = vertical_rhythm
        self._alignment_mode = "top"
        self.verticalScrollBar().rangeChanged.connect(self._schedule_refresh)

    def refresh_content_alignment(self) -> None:
        """Apply the page contract while preserving top alignment on overflow."""

        viewport_height = self.viewport().height()
        content_height = max(
            self._page_layout.sizeHint().height(),
            self._page_layout.minimumSize().height(),
        )
        mode = (
            "top"
            if self._vertical_rhythm is ShortPageVerticalRhythm.TOP
            else "center" if content_height <= viewport_height else "top"
        )
        if mode == self._alignment_mode:
            return
        self._alignment_mode = mode
        vertical = (
            Qt.AlignmentFlag.AlignVCenter
            if mode == "center"
            else Qt.AlignmentFlag.AlignTop
        )
        current = self._page_layout.alignment()
        horizontal = current & Qt.AlignmentFlag.AlignHorizontal_Mask
        self._page_layout.setAlignment(horizontal | vertical)
        self._page_layout.activate()

    def resizeEvent(self, event: object) -> None:
        """Resizeevent."""
        super().resizeEvent(event)  # type: ignore[arg-type]
        self._schedule_refresh()

    def showEvent(self, event: object) -> None:
        """Showevent."""
        super().showEvent(event)  # type: ignore[arg-type]
        self._schedule_refresh()

    def _schedule_refresh(self, *_args: object) -> None:
        """Schedule refresh."""
        self.refresh_content_alignment()


__all__ = ["ResponsiveScrollArea", "ShortPageVerticalRhythm"]

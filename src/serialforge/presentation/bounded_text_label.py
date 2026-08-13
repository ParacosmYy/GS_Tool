"""Presentation label that keeps long status text readable and recoverable."""

from __future__ import annotations

from .qt import QEvent, QLabel, QSizePolicy, Qt

_PREFERRED_TEXT_WIDTH = 160


class BoundedTextLabel(QLabel):
    """Display a single-line label without letting content dictate layout width.

    The full value remains available through :meth:`full_text`, the tooltip,
    and the accessible description.  Visible text is recalculated only when
    content or geometry changes; this component owns no domain state and adds
    no timer or animation lifecycle.
    """

    def __init__(self, text: str = "", parent=None) -> None:
        super().__init__("", parent)
        self._full_text = ""
        self.setWordWrap(False)
        self.setMinimumWidth(0)
        self.setSizePolicy(
            QSizePolicy.Policy.Preferred,
            QSizePolicy.Policy.Preferred,
        )
        self.set_full_text(text)

    @property
    def full_text(self) -> str:
        """Return the unelided value represented by the label."""

        return self._full_text

    def set_full_text(self, text: str) -> None:
        """Set the value and refresh the visible, width-bounded rendering."""

        self._full_text = str(text)
        self.setToolTip(self._full_text)
        self.setAccessibleDescription(self._full_text)
        self._refresh_visible_text()

    def setText(self, text: str) -> None:
        """Keep direct QLabel updates inside the bounded-text contract."""

        self.set_full_text(text)

    def resizeEvent(self, event: object) -> None:
        """Resizeevent."""
        super().resizeEvent(event)
        self._refresh_visible_text()

    def changeEvent(self, event: object) -> None:
        """Changeevent."""
        super().changeEvent(event)
        if event.type() in {
            QEvent.Type.FontChange,
            QEvent.Type.StyleChange,
        }:
            self._refresh_visible_text()

    def sizeHint(self) -> object:
        """Offer a stable compact preference without reserving full text width."""

        hint = super().sizeHint()
        hint.setWidth(_PREFERRED_TEXT_WIDTH)
        return hint

    def minimumSizeHint(self) -> object:
        """Allow the parent status cluster to yield space at narrow widths."""

        hint = super().minimumSizeHint()
        hint.setWidth(0)
        return hint

    def _refresh_visible_text(self) -> None:
        """Refresh visible text."""
        width = max(0, self.contentsRect().width())
        visible = self._full_text
        if width > 0:
            visible = self.fontMetrics().elidedText(
                self._full_text,
                Qt.TextElideMode.ElideRight,
                width,
            )
        QLabel.setText(self, visible)


__all__ = ["BoundedTextLabel"]

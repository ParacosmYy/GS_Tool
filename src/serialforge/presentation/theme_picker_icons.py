"""Resource-free palette icons for the native theme picker."""

from __future__ import annotations

from .qt import QColor, QComboBox, QIcon, QPainter, QPen, QPixmap, QPointF, QRectF, Qt
from .theme_tokens import ThemeSpec

_ICON_SIZE = 18
_PALETTE_COLORS = ("accent", "accent_blue", "accent_purple", "accent_pink")


def refresh_theme_picker_icons(
    combo: QComboBox | None,
    themes: tuple[ThemeSpec, ...],
) -> None:
    """Attach bounded palette previews without replacing combo semantics."""

    if combo is None:
        return
    theme_by_key = {theme.key: theme for theme in themes}
    for index in range(combo.count()):
        key = combo.itemData(index, Qt.ItemDataRole.UserRole)
        theme = theme_by_key.get(key) if isinstance(key, str) else None
        if theme is not None:
            combo.setItemIcon(index, _build_icon(theme))


def _build_icon(theme: ThemeSpec) -> QIcon:
    """Build icon."""
    icon = QIcon()
    icon.addPixmap(_render_icon(theme, disabled=False), QIcon.Mode.Normal, QIcon.State.Off)
    icon.addPixmap(_render_icon(theme, disabled=False), QIcon.Mode.Selected, QIcon.State.Off)
    icon.addPixmap(_render_icon(theme, disabled=True), QIcon.Mode.Disabled, QIcon.State.Off)
    return icon


def _render_icon(theme: ThemeSpec, *, disabled: bool) -> QPixmap:
    """Render icon."""
    pixmap = QPixmap(_ICON_SIZE, _ICON_SIZE)
    pixmap.fill(Qt.GlobalColor.transparent)
    painter = QPainter(pixmap)
    painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)

    bounds = QRectF(1.0, 1.0, 16.0, 16.0)
    panel = QColor(theme.disabled_surface if disabled else theme.surface_input)
    border = QColor(theme.disabled_border if disabled else theme.border)
    panel.setAlpha(240)
    border.setAlpha(220)
    painter.setBrush(panel)
    painter.setPen(QPen(border, 1.0))
    painter.drawRoundedRect(bounds, 4.5, 4.5)

    colors = (
        theme.disabled_text,
        theme.disabled_text,
        theme.disabled_text,
        theme.disabled_text,
    ) if disabled else tuple(getattr(theme, token) for token in _PALETTE_COLORS)
    left = 4.0
    span = 10.0
    center_y = 9.0
    painter.setPen(Qt.PenStyle.NoPen)
    for index, color in enumerate(colors):
        ink = QColor(color)
        ink.setAlpha(220 if not disabled else 185)
        painter.setBrush(ink)
        painter.drawEllipse(QPointF(left + span * index / 3.0, center_y), 1.8, 1.8)

    painter.end()
    return pixmap


__all__ = ["refresh_theme_picker_icons"]

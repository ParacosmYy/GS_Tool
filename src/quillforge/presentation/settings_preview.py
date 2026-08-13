"""Presentation surface for pending settings appearance choices."""

from __future__ import annotations

from PyQt6.QtCore import Qt
from PyQt6.QtGui import QFont
from PyQt6.QtWidgets import QFrame, QHBoxLayout, QLabel, QVBoxLayout, QWidget

from ..domain.models import AccentId, FontStyle, Locale, ThemeId
from .font_style import font_with_style
from .i18n import normalize_locale, tr
from .theme import preview_stylesheet, theme_colors


class SettingsPreviewSurface:
    """Render pending appearance choices without applying application state."""

    def __init__(self, parent: QWidget) -> None:
        self._widget = QFrame(parent)
        self._widget.setObjectName("settingsPreview")
        self._title = QLabel(self._widget)
        self._title.setObjectName("settingsPreviewTitle")
        self._accent = QLabel(self._widget)
        self._accent.setObjectName("settingsPreviewAccent")
        self._sample = QLabel(self._widget)
        self._sample.setObjectName("settingsPreviewSample")
        self._sample.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self._editor_sample = QLabel(self._widget)
        self._editor_sample.setObjectName("settingsPreviewEditorSample")
        self._editor_sample.setAlignment(Qt.AlignmentFlag.AlignLeft | Qt.AlignmentFlag.AlignVCenter)
        self._editor_meta = QLabel(self._widget)
        self._editor_meta.setObjectName("settingsPreviewEditorMeta")
        self._editor_meta.setWordWrap(True)
        self._canvas = QLabel(self._widget)
        self._canvas.setObjectName("settingsPreviewCanvas")
        self._panel = QLabel(self._widget)
        self._panel.setObjectName("settingsPreviewPanel")
        self._selection = QLabel(self._widget)
        self._selection.setObjectName("settingsPreviewSelection")
        self._meta = QLabel(self._widget)
        self._meta.setObjectName("settingsPreviewMeta")
        self._meta.setWordWrap(True)

        header = QHBoxLayout()
        header.setContentsMargins(0, 0, 0, 0)
        header.addWidget(self._title)
        header.addStretch(1)
        header.addWidget(self._accent)
        swatches = QHBoxLayout()
        swatches.setContentsMargins(0, 0, 0, 0)
        swatches.setSpacing(6)
        swatches.addWidget(self._canvas)
        swatches.addWidget(self._panel)
        swatches.addWidget(self._selection)
        layout = QVBoxLayout(self._widget)
        layout.setContentsMargins(12, 10, 12, 10)
        layout.setSpacing(8)
        layout.addLayout(header)
        layout.addWidget(self._sample)
        layout.addWidget(self._editor_sample)
        layout.addWidget(self._editor_meta)
        layout.addLayout(swatches)
        layout.addWidget(self._meta)

    @property
    def widget(self) -> QFrame:
        """Return the preview widget for SettingsDialog composition."""
        return self._widget

    def project(
        self,
        *,
        locale: Locale,
        theme: ThemeId,
        accent: AccentId,
        font_family: str,
        font_size: int,
        font_style: FontStyle,
        editor_font_family: str,
        editor_font_size: int,
        editor_font_style: FontStyle,
        theme_label: str,
        accent_label: str,
        font_style_label: str,
        editor_font_style_label: str,
    ) -> None:
        """Project one pending appearance selection without side effects."""
        normalized_locale = normalize_locale(locale)
        colors = theme_colors(theme, accent)
        self._title.setText(tr("settings.preview.title", normalized_locale))
        self._accent.setText(tr("settings.preview.accent", normalized_locale))
        self._sample.setText(tr("settings.preview.sample", normalized_locale))
        self._editor_sample.setText(tr("settings.preview.editor_sample", normalized_locale))
        self._editor_meta.setText(
            tr(
                "settings.preview.editor_meta",
                normalized_locale,
                family=editor_font_family,
                size=editor_font_size,
                style=editor_font_style_label,
            )
        )
        ui_font = font_with_style(QFont(font_family), font_style)
        ui_font.setPointSize(font_size)
        editor_font = font_with_style(QFont(editor_font_family), editor_font_style)
        editor_font.setPointSize(editor_font_size)
        self._canvas.setText(tr("settings.preview.canvas", normalized_locale))
        self._panel.setText(tr("settings.preview.panel", normalized_locale))
        self._selection.setText(tr("settings.preview.selected", normalized_locale))
        self._widget.setAccessibleName(tr("settings.preview.title", normalized_locale))
        self._widget.setStyleSheet(preview_stylesheet(colors))
        self._sample.setFont(ui_font)
        self._editor_sample.setFont(editor_font)
        self._meta.setText(
            tr(
                "settings.preview.meta",
                normalized_locale,
                theme=theme_label,
                accent=accent_label,
                font=font_family,
                size=font_size,
                style=font_style_label,
            )
        )

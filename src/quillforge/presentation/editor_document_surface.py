"""Presentation adapter for per-document editor creation and signal wiring."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass

from ..domain.models import AccentId, EditorSettings, ThemeId
from .editor_widget import EditorWidget


@dataclass(frozen=True, slots=True)
class EditorDocumentCallbacks:
    """Semantic editor events routed back to the document coordinator."""

    modified_changed: Callable[[EditorWidget, bool], None]
    content_changed: Callable[[EditorWidget], None]
    caret_changed: Callable[[], None]


class EditorDocumentSurface:
    """Own editor-adapter setup without owning document or operation policy."""

    def __init__(self, *, callbacks: EditorDocumentCallbacks) -> None:
        self._callbacks = callbacks

    def create(
        self,
        *,
        language: str | None,
        text: str,
        dirty: bool,
        settings: EditorSettings,
        theme: ThemeId,
        accent: AccentId,
    ) -> EditorWidget:
        """Create one configured editor and connect its semantic callbacks."""
        editor = EditorWidget()
        editor.set_language(language)
        self.apply_settings(editor, settings=settings, theme=theme, accent=accent)
        editor.set_text(text)
        editor.set_modified(dirty)
        self._connect(editor)
        return editor

    @staticmethod
    def set_language(editor: EditorWidget, language: str | None) -> None:
        """Refresh one adapter's language hint after a path change."""
        editor.set_language(language)

    @staticmethod
    def apply_settings(
        editor: EditorWidget,
        *,
        settings: EditorSettings,
        theme: ThemeId,
        accent: AccentId,
    ) -> None:
        """Apply the current editor and appearance snapshot to one adapter."""
        editor.set_font_family(settings.font_family)
        editor.set_font_size(settings.font_size)
        editor.set_font_style(settings.font_style)
        editor.set_wrap_lines(settings.wrap_lines)
        editor.set_line_numbers(settings.show_line_numbers)
        editor.set_theme(theme, accent)

    def _connect(self, editor: EditorWidget) -> None:
        editor.modified_state_changed.connect(
            lambda dirty, editor=editor: self._callbacks.modified_changed(editor, bool(dirty))
        )
        editor.content_changed.connect(
            lambda editor=editor: self._callbacks.content_changed(editor)
        )
        editor.cursorPositionChanged.connect(lambda _line, _column: self._callbacks.caret_changed())

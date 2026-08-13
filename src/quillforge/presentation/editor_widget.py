"""QScintilla-backed editor adapter used by the first shell."""

from __future__ import annotations

import time
from collections.abc import Callable
from dataclasses import dataclass
from typing import Literal

from PyQt6.Qsci import QsciLexerPython, QsciScintilla
from PyQt6.QtCore import pyqtSignal, pyqtSlot
from PyQt6.QtGui import QFont

from ..application.editor_policy import DEFAULT_EDITOR_OPERATION_POLICY
from ..domain.models import FontStyle
from .font_style import font_with_style, normalize_font_style
from .theme import apply_editor_palette

ReplaceAllPhase = Literal[
    "counting",
    "replacing",
    "completed",
    "limit_exceeded",
    "cancelled",
    "rollback_failed",
]
TextCapturePhase = Literal["capturing", "completed", "cancelled"]


@dataclass(frozen=True, slots=True)
class ReplaceAllProgress:
    """Immutable progress returned after one UI-cooperative replace slice."""

    phase: ReplaceAllPhase
    count: int
    limit: int


@dataclass(frozen=True, slots=True)
class TextCaptureProgress:
    """Immutable progress returned after one recovery capture slice."""

    phase: TextCapturePhase
    captured_bytes: int
    total_bytes: int
    chunks: int


class EditorWidget(QsciScintilla):
    """Small adapter that keeps QScintilla details out of application code."""

    modified_state_changed = pyqtSignal(bool)
    content_changed = pyqtSignal()

    def __init__(self) -> None:
        super().__init__()
        self.setObjectName("editor")
        self.setUtf8(True)
        self.setMarginLineNumbers(0, True)
        self.setMarginWidth(0, "00000")
        self.setAutoIndent(True)
        self.setIndentationsUseTabs(False)
        self.setIndentationWidth(4)
        self._theme_id = "sakura-pop"
        self._accent_id = "rose"
        self._font_style: FontStyle = "regular"
        self.modificationChanged.connect(self._emit_modified_state)
        self.textChanged.connect(self.content_changed)

        font = QFont("Cascadia Code", 11)
        font.setStyleHint(QFont.StyleHint.Monospace)
        font = font_with_style(font, self._font_style)
        self.setFont(font)

        lexer = QsciLexerPython(self)
        lexer.setDefaultFont(font)
        self.setLexer(lexer)
        apply_editor_palette(self)

    def get_text(self) -> str:
        """Return text through the application-owned editor contract."""
        return self.text()

    def set_text(self, text: str) -> None:
        """Replace text without making callers depend on QScintilla names."""
        self.setText(text)

    def begin_text_capture(
        self,
        sink: Callable[[str], bool] | None = None,
    ) -> TextCaptureSession:
        """Create a position-safe capture session for recovery serialization."""
        return TextCaptureSession(self, sink=sink)

    def _capture_text_length(self) -> int:
        """Return the Scintilla document length in adapter-owned byte positions."""
        return self.SendScintilla(QsciScintilla.SCI_GETTEXTLENGTH)

    def _capture_position_relative(self, position: int, characters: int) -> int:
        """Advance a Scintilla position without splitting a multibyte character."""
        return self.SendScintilla(
            QsciScintilla.SCI_POSITIONRELATIVE,
            position,
            characters,
        )

    def _capture_text_range(self, start: int, end: int) -> str:
        """Read one position-aligned text range while keeping QScintilla private."""
        return self.text(start, end)

    def is_modified(self) -> bool:
        """Return the adapter's modified state."""
        return self.isModified()

    def set_modified(self, modified: bool) -> None:
        """Set the adapter's modified state."""
        self.setModified(modified)

    def set_read_only(self, read_only: bool) -> None:
        """Set the adapter's editability without exposing QScintilla names."""
        self.setReadOnly(read_only)

    def set_operation_locked(self, locked: bool) -> None:
        """Block interactive input while preserving programmatic adapter mutations."""
        self.setEnabled(not locked)

    def set_language(self, language: str | None) -> None:
        """Apply the first language hint while keeping lexer choice local."""
        if language == "python":
            lexer = QsciLexerPython(self)
            lexer.setDefaultFont(self.font())
            self.setLexer(lexer)
        else:
            self.setLexer(None)
        apply_editor_palette(self, self._theme_id, self._accent_id)

    def set_font_size(self, size: int) -> None:
        """Apply a validated font size through the editor adapter."""
        font = QFont(self.font())
        font.setPointSize(size)
        self.setFont(font)
        if self.lexer() is not None:
            self.lexer().setDefaultFont(font)

    def set_font_family(self, family: str) -> None:
        """Apply a presentation-selected editor font family."""
        font = QFont(self.font())
        font.setFamily(family)
        font.setStyleHint(QFont.StyleHint.Monospace)
        self.setFont(font)
        if self.lexer() is not None:
            self.lexer().setDefaultFont(font)

    def set_font_style(self, style: FontStyle) -> None:
        """Apply a presentation-selected editor weight or italic style."""
        self._font_style = normalize_font_style(style)
        font = font_with_style(self.font(), self._font_style)
        self.setFont(font)
        if self.lexer() is not None:
            self.lexer().setDefaultFont(font)

    def set_wrap_lines(self, enabled: bool) -> None:
        """Apply line wrapping without exposing QScintilla enum values."""
        mode = QsciScintilla.WrapMode.WrapWord if enabled else QsciScintilla.WrapMode.WrapNone
        self.setWrapMode(mode)

    def set_line_numbers(self, enabled: bool) -> None:
        """Toggle the bounded line-number margin without leaking QScintilla enums."""
        self.setMarginLineNumbers(0, enabled)

    def set_theme(self, theme: str, accent: str = "violet") -> None:
        """Apply a validated theme identifier through the adapter boundary."""
        self._theme_id = (
            theme if theme in {"ink-violet", "paper-sand", "sakura-pop"} else "sakura-pop"
        )
        self._accent_id = accent if accent in {"violet", "cyan", "rose", "amber"} else "rose"
        apply_editor_palette(self, self._theme_id, self._accent_id)

    def cursor_position(self) -> tuple[int, int]:
        """Return a stable zero-based caret position for local session state."""
        return self.getCursorPosition()

    def set_cursor_position(self, line: int, column: int) -> None:
        """Restore a clamped caret position without exposing QScintilla types."""
        if type(line) is not int or type(column) is not int or line < 0 or column < 0:
            raise ValueError("Caret position must be non-negative integers")
        target_line = min(line, max(0, self.lines() - 1))
        target_column = min(column, self.lineLength(target_line))
        self.setCursorPosition(target_line, target_column)
        self.ensureLineVisible(target_line)

    def undo(self) -> None:
        """Undo through QScintilla without leaking its API to the shell."""
        super().undo()

    def redo(self) -> None:
        """Redo through QScintilla without leaking its API to the shell."""
        super().redo()

    def cut(self) -> None:
        """Cut the current selection through the adapter boundary."""
        super().cut()

    def copy(self) -> None:
        """Copy the current selection through the adapter boundary."""
        super().copy()

    def paste(self) -> None:
        """Paste clipboard text through the adapter boundary."""
        super().paste()

    def select_all(self) -> None:
        """Select all text through the adapter boundary."""
        super().selectAll()

    def has_selection(self) -> bool:
        """Return whether QScintilla currently owns a selection."""
        return self.hasSelectedText()

    def selected_text(self) -> str:
        """Return the current selection through the adapter boundary."""
        return self.selectedText()

    def selection_bounds(self) -> tuple[int, int, int, int] | None:
        """Return the current selection coordinates for session validation."""
        if not self.hasSelectedText():
            return None
        return self.getSelection()

    def find_literal(self, query: str, *, case_sensitive: bool, forward: bool) -> bool:
        """Find a literal match while keeping QScintilla search details local."""
        if not query:
            return False
        line, index = self.getCursorPosition()
        if self.hasSelectedText():
            selection = self.getSelection()
            if forward:
                line, index = selection[2], selection[3]
            else:
                line, index = selection[0], selection[1]
        return self.findFirst(
            query,
            False,
            case_sensitive,
            False,
            True,
            forward,
            line,
            index,
        )

    def go_to_line(self, line_number: int) -> None:
        """Place the caret at a one-based line requested by a workspace result."""
        if type(line_number) is not int or line_number < 1:
            raise ValueError("line_number must be positive")
        target_line = min(line_number - 1, max(0, self.lines() - 1))
        self.setCursorPosition(target_line, 0)
        self.ensureLineVisible(target_line)
        self.setFocus()

    def replace_selected_text(self, replacement: str) -> None:
        """Replace one selected match through QScintilla."""
        super().replaceSelectedText(replacement)

    def replace_all_literal(
        self,
        query: str,
        replacement: str,
        *,
        case_sensitive: bool,
    ) -> int:
        """Keep the synchronous editor contract for non-Qt callers."""
        session = self.begin_replace_all_literal(
            query,
            replacement,
            case_sensitive=case_sensitive,
            max_matches=DEFAULT_EDITOR_OPERATION_POLICY.max_replace_matches,
        )
        progress = session.step(
            budget_ms=DEFAULT_EDITOR_OPERATION_POLICY.replace_all_slice_budget_ms,
            max_items=DEFAULT_EDITOR_OPERATION_POLICY.replace_all_items_per_slice,
        )
        while not session.is_terminal:
            progress = session.step(
                budget_ms=DEFAULT_EDITOR_OPERATION_POLICY.replace_all_slice_budget_ms,
                max_items=DEFAULT_EDITOR_OPERATION_POLICY.replace_all_items_per_slice,
            )
        if progress.phase == "limit_exceeded":
            raise ValueError(f"Replace All is limited to {progress.limit:,} matches.")
        return progress.count

    def begin_replace_all_literal(
        self,
        query: str,
        replacement: str,
        *,
        case_sensitive: bool,
        max_matches: int,
    ) -> ReplaceAllSession:
        """Create a UI-thread session that can yield between bounded slices."""
        return ReplaceAllSession(
            self,
            query,
            replacement,
            case_sensitive=case_sensitive,
            max_matches=max_matches,
        )

    @pyqtSlot(bool)
    def _emit_modified_state(self, modified: bool) -> None:
        """Translate the control signal into the adapter-owned signal."""
        self.modified_state_changed.emit(modified)


class TextCaptureSession:
    """QScintilla-owned capture that can publish into a bounded sink."""

    def __init__(
        self,
        editor: EditorWidget,
        *,
        sink: Callable[[str], bool] | None = None,
    ) -> None:
        self._editor = editor
        if sink is not None and not callable(sink):
            raise TypeError("sink must be callable")
        self._sink = sink
        self._total_bytes = editor._capture_text_length()
        self._position = 0
        self._chunks: list[str] = []
        self._accepted_chunks = 0
        self._phase: TextCapturePhase = "completed" if self._total_bytes == 0 else "capturing"

    @property
    def is_terminal(self) -> bool:
        """Return whether no further widget reads are expected."""
        return self._phase in {"completed", "cancelled"}

    def step(
        self,
        *,
        budget_ms: int,
        max_chunks: int,
        characters_per_chunk: int,
    ) -> TextCaptureProgress:
        """Read position-aligned text chunks for one bounded UI slice."""
        if type(budget_ms) is not int or budget_ms < 1:
            raise ValueError("budget_ms must be a positive integer")
        if type(max_chunks) is not int or max_chunks < 1:
            raise ValueError("max_chunks must be a positive integer")
        if type(characters_per_chunk) is not int or characters_per_chunk < 1:
            raise ValueError("characters_per_chunk must be a positive integer")
        if self.is_terminal:
            return self.progress()

        deadline_ns = time.perf_counter_ns() + budget_ms * 1_000_000
        captured = 0
        while (
            self._position < self._total_bytes
            and captured < max_chunks
            and (captured == 0 or time.perf_counter_ns() < deadline_ns)
        ):
            next_position = self._editor._capture_position_relative(
                self._position,
                characters_per_chunk,
            )
            if next_position <= self._position or next_position > self._total_bytes:
                next_position = self._total_bytes
            chunk = self._editor._capture_text_range(self._position, next_position)
            if self._sink is not None and not self._sink(chunk):
                break
            if self._sink is None:
                self._chunks.append(chunk)
            self._accepted_chunks += 1
            self._position = next_position
            captured += 1
        if self._position >= self._total_bytes:
            self._phase = "completed"
        return self.progress()

    def cancel(self) -> TextCaptureProgress:
        """Discard captured chunks without joining or serializing them."""
        if not self.is_terminal:
            self._chunks.clear()
            self._phase = "cancelled"
        return self.progress()

    def chunks(self) -> tuple[str, ...]:
        """Return chunk references for worker-side joining after completion."""
        if self._phase != "completed":
            raise RuntimeError("Text capture is not complete")
        return tuple(self._chunks)

    def progress(self) -> TextCaptureProgress:
        """Return a stable capture status for the recovery coordinator."""
        return TextCaptureProgress(
            self._phase,
            self._position,
            self._total_bytes,
            self._accepted_chunks,
        )


class ReplaceAllSession:
    """QScintilla-owned, two-phase replace operation with cooperative UI slices."""

    def __init__(
        self,
        editor: EditorWidget,
        query: str,
        replacement: str,
        *,
        case_sensitive: bool,
        max_matches: int,
    ) -> None:
        if type(max_matches) is not int or max_matches < 1:
            raise ValueError("max_matches must be a positive integer")
        self._editor = editor
        self._query = query
        self._replacement = replacement
        self._case_sensitive = case_sensitive
        self._max_matches = max_matches
        self._phase: ReplaceAllPhase = "completed" if not query else "counting"
        self._count = 0
        self._line = 0
        self._index = 0
        self._undo_open = False
        self._mutated = False
        self._rollback_succeeded = True
        self._saved_cursor = editor.getCursorPosition()
        self._saved_selection = editor.selection_bounds()

    @property
    def is_terminal(self) -> bool:
        """Return whether no further editor mutation can occur."""
        return self._phase in {
            "completed",
            "limit_exceeded",
            "cancelled",
            "rollback_failed",
        }

    @property
    def rollback_succeeded(self) -> bool:
        """Report whether a cancellation or failure returned the text to its prior state."""
        return self._rollback_succeeded

    def step(self, *, budget_ms: int, max_items: int) -> ReplaceAllProgress:
        """Advance counting or replacement and return without monopolizing the event loop."""
        if type(budget_ms) is not int or budget_ms < 1:
            raise ValueError("budget_ms must be a positive integer")
        if type(max_items) is not int or max_items < 1:
            raise ValueError("max_items must be a positive integer")
        if self.is_terminal:
            return self.progress()

        deadline_ns = time.perf_counter_ns() + budget_ms * 1_000_000
        processed = 0
        try:
            while processed < max_items and (
                processed == 0 or time.perf_counter_ns() < deadline_ns
            ):
                processed += 1
                if self._phase == "counting":
                    if not self._find_next():
                        self._restore_saved_selection()
                        self._phase = "replacing"
                        self._line = 0
                        self._index = 0
                        self._count = 0
                        self._editor.beginUndoAction()
                        self._undo_open = True
                        continue
                    self._count += 1
                    if self._count > self._max_matches:
                        self._restore_saved_selection()
                        self._phase = "limit_exceeded"
                        return self.progress()
                    self._line, self._index = self._editor.getCursorPosition()
                    continue

                if not self._find_next():
                    self._finish_undo_action()
                    self._phase = "completed"
                    return self.progress()
                self._editor.replace_selected_text(self._replacement)
                self._mutated = True
                self._count += 1
                self._line, self._index = self._editor.getCursorPosition()
        except Exception:
            try:
                self._rollback()
            except Exception as rollback_error:
                self._rollback_succeeded = False
                self._phase = "rollback_failed"
                raise RuntimeError("Replace All failed and rollback could not be completed") from (
                    rollback_error
                )
            self._restore_saved_selection()
            self._phase = "cancelled"
            raise
        return self.progress()

    def cancel(self) -> ReplaceAllProgress:
        """Stop the operation and roll back a partial replacement transaction."""
        if self.is_terminal:
            return self.progress()
        try:
            self._rollback()
        except Exception as rollback_error:
            self._rollback_succeeded = False
            self._phase = "rollback_failed"
            raise RuntimeError("Replace All cancellation could not be rolled back") from (
                rollback_error
            )
        self._restore_saved_selection()
        self._phase = "cancelled"
        return self.progress()

    def progress(self) -> ReplaceAllProgress:
        """Return a stable status suitable for presentation code."""
        return ReplaceAllProgress(self._phase, self._count, self._max_matches)

    def _find_next(self) -> bool:
        return self._editor.findFirst(
            self._query,
            False,
            self._case_sensitive,
            False,
            False,
            True,
            self._line,
            self._index,
        )

    def _finish_undo_action(self) -> None:
        if self._undo_open:
            self._editor.endUndoAction()
            self._undo_open = False

    def _rollback(self) -> None:
        self._finish_undo_action()
        if self._mutated:
            self._editor.undo()
            self._mutated = False

    def _restore_saved_selection(self) -> None:
        if self._saved_selection is None:
            self._editor.setCursorPosition(*self._saved_cursor)
        else:
            self._editor.setSelection(*self._saved_selection)

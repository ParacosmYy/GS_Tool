"""Shared, presentation-only find and replace bar."""

from PyQt6.QtCore import QSize, Qt, pyqtSignal
from PyQt6.QtGui import QPalette
from PyQt6.QtWidgets import QCheckBox, QHBoxLayout, QLabel, QLineEdit, QPushButton, QWidget

from ..domain.models import Locale
from .feedback import FeedbackLevel, apply_feedback_state
from .i18n import localize_message, normalize_locale, tr
from .icons import IconKey, themed_icon


class _FindLineEdit(QLineEdit):
    """Line edit that gives Enter direction and Esc to the owning bar."""

    find_requested = pyqtSignal(bool)
    close_requested = pyqtSignal()

    def keyPressEvent(self, event) -> None:  # noqa: N802 - Qt override name
        if event.key() in (Qt.Key.Key_Return, Qt.Key.Key_Enter):
            backward = bool(event.modifiers() & Qt.KeyboardModifier.ShiftModifier)
            self.find_requested.emit(not backward)
            event.accept()
            return
        if event.key() == Qt.Key.Key_Escape:
            self.close_requested.emit()
            event.accept()
            return
        super().keyPressEvent(event)


class FindBar(QWidget):
    """A reusable UI for current-document literal find and replace."""

    criteria_changed = pyqtSignal()
    find_requested = pyqtSignal(bool)
    replace_requested = pyqtSignal()
    replace_all_requested = pyqtSignal()
    cancel_requested = pyqtSignal()
    close_requested = pyqtSignal()

    def __init__(self, parent: QWidget | None = None, *, locale: Locale = "zh-CN") -> None:
        super().__init__(parent)
        self.setObjectName("findBar")
        self._locale = normalize_locale(locale)
        self._status_message = ""
        self._status_level: FeedbackLevel = "info"
        self._query = _FindLineEdit(self)
        self._query.setObjectName("findQuery")
        self._query.setClearButtonEnabled(True)
        self._replacement = _FindLineEdit(self)
        self._replacement.setObjectName("replaceQuery")
        self._replacement_label = QLabel(self)
        self._case_sensitive = QCheckBox(self)
        self._previous = QPushButton(self)
        self._previous.setObjectName("findPrevious")
        self._next = QPushButton(self)
        self._next.setObjectName("findNext")
        self._replace = QPushButton(self)
        self._replace_all = QPushButton(self)
        self._replace_all.setObjectName("warningAction")
        self._cancel = QPushButton(self)
        self._cancel.setObjectName("findCancel")
        self._close = QPushButton(self)
        self._close.setObjectName("findClose")
        for button in (
            self._previous,
            self._next,
            self._replace,
            self._replace_all,
            self._cancel,
            self._close,
        ):
            button.setIconSize(QSize(16, 16))
        self._status = QLabel("", self)
        self._status.setObjectName("findStatus")
        self._cancel.hide()
        self._replacement.hide()
        self._replacement_label.hide()
        self._replace.hide()
        self._replace_all.hide()

        layout = QHBoxLayout(self)
        layout.setContentsMargins(10, 8, 10, 8)
        layout.setSpacing(8)
        self._find_label = QLabel(self)
        layout.addWidget(self._find_label)
        layout.addWidget(self._query, 2)
        layout.addWidget(self._replacement_label)
        layout.addWidget(self._replacement, 2)
        layout.addWidget(self._case_sensitive)
        layout.addWidget(self._previous)
        layout.addWidget(self._next)
        layout.addWidget(self._replace)
        layout.addWidget(self._replace_all)
        layout.addWidget(self._cancel)
        layout.addWidget(self._status, 1)
        layout.addWidget(self._close)

        self._query.find_requested.connect(self.find_requested)
        self._query.close_requested.connect(self.close_requested)
        self._query.textChanged.connect(lambda _text: self.criteria_changed.emit())
        self._replacement.find_requested.connect(self.find_requested)
        self._replacement.close_requested.connect(self.close_requested)
        self._case_sensitive.stateChanged.connect(lambda _state: self.criteria_changed.emit())
        self._previous.clicked.connect(lambda: self.find_requested.emit(False))
        self._next.clicked.connect(lambda: self.find_requested.emit(True))
        self._replace.clicked.connect(self.replace_requested)
        self._replace_all.clicked.connect(self.replace_all_requested)
        self._cancel.clicked.connect(self.cancel_requested)
        self._close.clicked.connect(self.close_requested)
        self.set_locale(self._locale)

    def refresh_icons(self) -> None:
        """Retint find actions after a theme or accent projection."""
        palette = self.palette()
        foreground = palette.color(QPalette.ColorRole.ButtonText).name()
        accent = palette.color(QPalette.ColorRole.Link).name()
        if palette.color(QPalette.ColorRole.Base).lightnessF() >= 0.65:
            # Keep secondary icon strokes readable on the light pressed-row
            # surface; dark themes can retain the accent-alt detail.
            accent = foreground
        disabled_foreground = palette.color(
            QPalette.ColorGroup.Disabled,
            QPalette.ColorRole.ButtonText,
        ).name()
        for button, key in (
            (self._previous, IconKey.ARROW_UP),
            (self._next, IconKey.ARROW_DOWN),
            (self._replace, IconKey.REPLACE),
            (self._replace_all, IconKey.REPLACE),
            (self._cancel, IconKey.CLOSE),
            (self._close, IconKey.CLOSE),
        ):
            button.setIcon(
                themed_icon(
                    key,
                    foreground=foreground,
                    accent=accent,
                    disabled_foreground=disabled_foreground,
                    disabled_accent=disabled_foreground,
                )
            )

    def show_find(self, *, replacement: bool = False) -> None:
        """Show the bar and focus the query field for a keyboard-first flow."""
        self.show()
        self._query.setFocus()
        self._query.selectAll()
        self._replacement_label.setVisible(replacement)
        self._replacement.setVisible(replacement)
        self._replace.setVisible(replacement)
        self._replace_all.setVisible(replacement)
        self._set_primary_action(self._replace if replacement else self._next)

    def _set_primary_action(self, primary: QPushButton) -> None:
        """Keep exactly one current-mode action on the shared primary QSS role."""
        for button in (self._next, self._replace):
            button.setObjectName("primaryAction" if button is primary else "")
            style = button.style()
            style.unpolish(button)
            style.polish(button)

    def query(self) -> str:
        """Return the current literal query."""
        return self._query.text()

    def replacement(self) -> str:
        """Return the current replacement text, including an empty value."""
        return self._replacement.text()

    def case_sensitive(self) -> bool:
        """Return whether matching should distinguish letter case."""
        return self._case_sensitive.isChecked()

    def set_status(self, message: str, *, level: FeedbackLevel = "info") -> None:
        """Show a short operation result without a modal dialog."""
        self._status_message = message
        self._status_level = level
        self._status.setText(localize_message(message, self._locale))
        apply_feedback_state(self._status, level)

    def set_locale(self, locale: Locale) -> None:
        """Refresh the find surface labels without touching the current query."""
        self._locale = normalize_locale(locale)
        self._find_label.setText(tr("find.find", self._locale))
        self._query.setPlaceholderText(tr("find.placeholder", self._locale))
        self._replacement_label.setText(tr("find.replace", self._locale))
        self._replacement.setPlaceholderText(tr("find.replace_placeholder", self._locale))
        self._case_sensitive.setText(tr("find.case", self._locale))
        self._previous.setText(tr("find.previous", self._locale))
        self._next.setText(tr("find.next", self._locale))
        self._replace.setText(tr("find.replace_one", self._locale))
        self._replace_all.setText(tr("find.replace_all", self._locale))
        self._cancel.setText(tr("find.cancel", self._locale))
        self._close.setText(tr("find.close", self._locale))
        self._status.setText(localize_message(self._status_message, self._locale))
        apply_feedback_state(self._status, self._status_level)
        self.refresh_icons()

    def set_operation_active(self, active: bool) -> None:
        """Keep query state stable while a cooperative editor operation runs."""
        for control in (
            self._query,
            self._replacement,
            self._case_sensitive,
            self._previous,
            self._next,
            self._replace,
            self._replace_all,
        ):
            control.setEnabled(not active)
        self._cancel.setVisible(active)
        self._cancel.setEnabled(active)

    def reset_session(self) -> None:
        """Clear stale status when the active document changes."""
        self.set_status("")

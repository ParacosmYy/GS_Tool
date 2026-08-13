"""Presentation composition for the shell status rail."""

from __future__ import annotations

from PyQt6.QtCore import QTimer
from PyQt6.QtWidgets import QLabel, QSizePolicy, QStatusBar, QWidget

from ..domain.models import Locale
from .feedback import apply_feedback_state
from .i18n import localize_message, normalize_locale, tr
from .notification_contract import StatusMessageLevel
from .status_bar import StatusRail
from .status_phase_contract import StatusPhase


class StatusSurface:
    """Own the status-rail widget while exposing semantic shell projection."""

    def __init__(self, parent: QWidget, *, locale: Locale) -> None:
        self._locale = normalize_locale(locale)
        self._rail = StatusRail(parent, locale=self._locale)
        self._message = QLabel(parent)
        self._message.setObjectName("statusMessage")
        self._message.setAccessibleName(tr("accessibility.shell_notification", self._locale))
        self._message.setSizePolicy(
            QSizePolicy.Policy.Expanding,
            QSizePolicy.Policy.Preferred,
        )
        self._message.setVisible(False)
        self._message_text = ""
        self._message_level: StatusMessageLevel = "info"
        self._message_timer = QTimer(self._message)
        self._message_timer.setSingleShot(True)
        self._message_timer.timeout.connect(self._clear_message)
        self._status_bar: QStatusBar | None = None

    @property
    def widget(self) -> StatusRail:
        """Return the owned rail for the status-bar object tree."""
        return self._rail

    def set_locale(self, locale: Locale) -> None:
        """Refresh the status rail's localized labels."""
        self._locale = normalize_locale(locale)
        self._message.setAccessibleName(tr("accessibility.shell_notification", self._locale))
        self._rail.set_locale(self._locale)
        if self._message_text:
            self._message.setText(localize_message(self._message_text, self._locale))
            self._message.setToolTip(self._message.text())
            apply_feedback_state(self._message, self._message_level)

    def attach_to(self, status_bar: QStatusBar) -> None:
        """Attach the rail and notification channel to the shell status bar."""
        if self._status_bar is status_bar:
            return
        if self._status_bar is not None:
            self._status_bar.removeWidget(self._message)
            self._status_bar.removeWidget(self._rail)
        status_bar.setSizeGripEnabled(False)
        status_bar.addWidget(self._message, 1)
        status_bar.addPermanentWidget(self._rail)
        self._status_bar = status_bar

    def show_message(
        self,
        message: str,
        *,
        timeout_ms: int = 5000,
        level: StatusMessageLevel = "info",
    ) -> None:
        """Show one localized, semantically styled shell notification."""
        if self._status_bar is None:
            return
        if not message:
            self._clear_message()
            return
        self._message_text = message
        self._message_level = level
        self._message.setText(localize_message(message, self._locale))
        self._message.setToolTip(self._message.text())
        apply_feedback_state(self._message, level)
        self._message.setVisible(True)
        if timeout_ms > 0:
            self._message_timer.start(timeout_ms)
        else:
            self._message_timer.stop()

    def set_phase(self, phase: StatusPhase) -> None:
        """Project the shell lifecycle phase without exposing the rail API."""
        self._rail.set_phase(phase)

    def _clear_message(self) -> None:
        self._message_timer.stop()
        self._message_text = ""
        self._message.setText("")
        self._message.setToolTip("")
        self._message.setVisible(False)

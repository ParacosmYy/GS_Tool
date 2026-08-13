"""Small presentation-only status rail for the desktop shell."""

from __future__ import annotations

from PyQt6.QtWidgets import QHBoxLayout, QLabel, QWidget

from ..domain.models import Locale
from .i18n import normalize_locale, tr
from .status_phase_contract import StatusPhase


class StatusRail(QWidget):
    """Render explicit shell health without interpreting notification text."""

    def __init__(self, parent: QWidget | None = None, *, locale: Locale = "zh-CN") -> None:
        super().__init__(parent)
        self.setObjectName("statusRail")
        self._locale = normalize_locale(locale)
        self._context = QLabel(self)
        self._context.setObjectName("statusContext")
        self._phase = QLabel(self)
        self._phase.setObjectName("statusPhase")
        layout = QHBoxLayout(self)
        layout.setContentsMargins(8, 2, 2, 2)
        layout.setSpacing(8)
        layout.addWidget(self._context)
        layout.addWidget(self._phase)
        self.set_phase("ready")
        self.set_locale(self._locale)

    def set_phase(self, phase: StatusPhase) -> None:
        """Set an explicit phase owned by the caller's lifecycle state."""
        labels = {
            "ready": "status.ready",
            "working": "status.working",
            "attention": "status.attention",
            "error": "status.error",
        }
        label = tr(labels[phase], self._locale)
        self._phase.setText(label)
        self._phase.setAccessibleDescription(
            tr("accessibility.shell_phase_description", self._locale, phase=label)
        )
        self._phase.setProperty("state", phase)
        style = self._phase.style()
        style.unpolish(self._phase)
        style.polish(self._phase)

    def set_locale(self, locale: Locale) -> None:
        """Refresh context and the current phase label."""
        self._locale = normalize_locale(locale)
        self.setAccessibleName(tr("accessibility.shell_status", self._locale))
        self._context.setAccessibleName(tr("accessibility.workspace_context", self._locale))
        self._phase.setAccessibleName(tr("accessibility.shell_phase", self._locale))
        self._context.setText(tr("status.local", self._locale))
        phase = self._phase.property("state") or "ready"
        self.set_phase(phase)

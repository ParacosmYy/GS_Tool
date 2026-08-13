"""Presentation-only empty state for the Component telemetry surface."""

from __future__ import annotations

import math

from .action_surface import ActionRailButton
from .property_refresh import refresh_dynamic_property
from .qt import (
    QColor,
    QFrame,
    QHBoxLayout,
    QLabel,
    QPainter,
    QPen,
    QPointF,
    QSizePolicy,
    Qt,
    QVBoxLayout,
    QWidget,
    Signal,
)
from .theme import theme_spec_for_widget


class ComponentPipelineGlyph(QWidget):
    """Draw a small protocol-to-component route without image resources."""

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self._phase = 0.0
        self._animated = False
        self._available = True
        self.setFixedSize(58, 58)
        self.setFocusPolicy(Qt.FocusPolicy.NoFocus)
        self.setAttribute(Qt.WidgetAttribute.WA_TransparentForMouseEvents, True)

    def set_available(self, available: bool) -> None:
        """Set available."""
        normalized = bool(available)
        if self._available == normalized:
            return
        self._available = normalized
        self.update()

    def set_frame(self, phase: float, animated: bool) -> None:
        """Set frame."""
        self._phase = float(phase)
        self._animated = bool(animated)
        self.update()

    def stop(self) -> None:
        """Stop."""
        self._animated = False
        self.update()

    def paintEvent(self, event: object) -> None:
        """Paintevent."""
        del event
        theme = theme_spec_for_widget(self)
        bounds = self.rect().adjusted(4, 4, -4, -4)
        center = QPointF(bounds.center())
        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)

        panel = QColor(theme.surface_input)
        panel.setAlpha(215)
        border = QColor(theme.border)
        border.setAlpha(180)
        painter.setBrush(panel)
        painter.setPen(QPen(border, 1.0))
        painter.drawRoundedRect(bounds, 14.0, 14.0)

        primary = theme.accent_blue if self._available else theme.text_subtle
        secondary = theme.accent if self._available else theme.border
        rail = QColor(primary)
        rail.setAlpha(190 if self._animated else 120)
        painter.setPen(QPen(rail, 1.3))
        painter.drawLine(
            QPointF(bounds.left() + 13.0, center.y()),
            QPointF(bounds.right() - 13.0, center.y()),
        )

        for index in range(3):
            node = QColor(secondary if index == 2 else primary)
            node.setAlpha(220 if self._available else 105)
            painter.setPen(Qt.PenStyle.NoPen)
            painter.setBrush(node)
            painter.drawEllipse(
                QPointF(bounds.left() + 13.0 + index * 16.0, center.y()),
                3.0,
                3.0,
            )

        if self._available and self._animated:
            wave = (math.sin(self._phase) + 1.0) * 0.5
            pulse = QColor(theme.accent)
            pulse.setAlpha(38 + int(wave * 52))
            painter.setBrush(pulse)
            painter.drawEllipse(center, 11.0 + wave * 3.0, 11.0 + wave * 3.0)


class ComponentEmptyStateSurface(QFrame):
    """Guide the next Component action while preserving controller-owned copy."""

    MOTION_MODE = "activity"
    load_requested = Signal()

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self.setObjectName("componentEmptyState")
        self.setProperty("role", "surface")
        self.setProperty("state", "waiting")
        self.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Preferred)
        self.setMinimumHeight(94)
        self.setAccessibleName("组件帧空态")
        self._text = ""
        self._available = True

        root = QHBoxLayout(self)
        root.setContentsMargins(12, 8, 12, 8)
        root.setSpacing(12)

        self._glyph = ComponentPipelineGlyph(self)
        root.addWidget(self._glyph)

        copy = QVBoxLayout()
        copy.setContentsMargins(0, 0, 0, 0)
        copy.setSpacing(2)
        self._eyebrow = QLabel("COMPONENT / WAITING", self)
        self._eyebrow.setObjectName("componentEmptyEyebrow")
        self._eyebrow.setProperty("role", "subtle")
        copy.addWidget(self._eyebrow)
        self._title = QLabel("等待组件帧", self)
        self._title.setObjectName("componentEmptyTitle")
        copy.addWidget(self._title)
        self._hint = QLabel("接收 RX 或加载 Profile / Codec 后开始解析字段。", self)
        self._hint.setObjectName("componentEmptyHint")
        self._hint.setProperty("role", "muted")
        self._hint.setWordWrap(True)
        copy.addWidget(self._hint)
        self._action = ActionRailButton("加载 Profile / Codec", self)
        self._action.setObjectName("primaryButton")
        self._action.setAccessibleName("从空态加载组件 Profile 或 Codec")
        self._action.setAccessibleDescription("打开组件 Profile 或 Codec 选择器。")
        self._action.setToolTip("打开组件 Profile 或 Codec 选择器。")
        self._action.clicked.connect(self._emit_load_requested)
        copy.addWidget(self._action, alignment=Qt.AlignmentFlag.AlignLeft)
        root.addLayout(copy, stretch=1)

    def setText(self, text: str) -> None:
        """Keep the QLabel-like controller contract while rendering a richer card."""

        self._text = text if isinstance(text, str) else str(text)
        title, separator, hint = self._text.partition(" · ")
        self._title.setText(title or "等待组件帧")
        self._hint.setText(hint if separator else "接收 RX 或加载 Profile / Codec 后开始解析字段。")
        self._hint.setAccessibleDescription(self._hint.text())
        self.update()

    def text(self) -> str:
        """Return the controller-owned full empty-state copy."""

        return self._text

    def clear(self) -> None:
        """Clear the compatibility text before the controller hides the surface."""

        self.setText("")

    def set_action_enabled(self, enabled: bool) -> None:
        """Reflect the existing derived-source gate without recomputing it."""

        self._available = bool(enabled)
        state = "waiting" if self._available else "blocked"
        refresh_dynamic_property(self, "state", state)
        self._eyebrow.setText(
            "COMPONENT / WAITING" if self._available else "COMPONENT / BLOCKED"
        )
        self._action.setEnabled(self._available)
        self._action.setVisible(self._available)
        self._glyph.set_available(self._available)
        description = (
            "打开组件 Profile 或 Codec 选择器。"
            if self._available
            else "当前来源不支持组件解析，无法加载 Profile 或 Codec。"
        )
        self._action.setAccessibleDescription(description)
        self._action.setToolTip(description)
        self.update()

    def set_frame(self, phase: float, animated: bool) -> None:
        """Fan the shared shell frame to the decorative children."""

        self._glyph.set_frame(phase, animated)
        self._action.set_frame(phase, animated)

    def stop(self) -> None:
        """Freeze decoration during reduced motion, pause, hide, or close."""

        self._glyph.stop()
        self._action.stop()

    def _emit_load_requested(self, checked: bool = False) -> None:
        """Emit load requested."""
        del checked
        self.load_requested.emit()


__all__ = ["ComponentEmptyStateSurface", "ComponentPipelineGlyph"]

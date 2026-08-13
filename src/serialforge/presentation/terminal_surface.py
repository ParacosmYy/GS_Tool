"""Presentation-only terminal surface adornments.

The empty-state layer gives the large terminal canvas an intentional, quiet
starting point without owning transport state, timers, or business actions.
Its decorative glyph consumes the shared MotionController frame supplied by
the shell lifecycle.
"""

from __future__ import annotations

import math

from .action_surface import ActionRailButton
from .observation_viewport import ObservationViewport
from .qt import (
    QColor,
    QFrame,
    QHBoxLayout,
    QLabel,
    QPainter,
    QPen,
    QPointF,
    QRectF,
    QSizePolicy,
    Qt,
    QVBoxLayout,
    QWidget,
    Signal,
)
from .theme import theme_spec_for_widget

# Keep the previous presentation import stable while the implementation lives
# in the shared action surface module.
TerminalActionButton = ActionRailButton


class TerminalViewport(ObservationViewport):
    """Keep the established terminal type while sharing observation chrome."""

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent, scope="terminal")


class TerminalOrbitGlyph(QWidget):
    """Small resource-free signal glyph used by the terminal empty state."""

    _STATES = frozenset({"idle", "waiting", "transition", "paused", "history"})

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self._state = "idle"
        self._phase = 0.0
        self._animated = False
        self.setFixedSize(72, 72)
        self.setFocusPolicy(Qt.FocusPolicy.NoFocus)
        self.setAttribute(Qt.WidgetAttribute.WA_TransparentForMouseEvents, True)

    def set_state(self, state: str) -> None:
        """Set state."""
        normalized = state if state in self._STATES else "idle"
        if self._state == normalized:
            return
        self._state = normalized
        self.update()

    def set_frame(self, phase: float, animated: bool) -> None:
        """Set frame."""
        self._phase = phase
        self._animated = bool(animated)
        self.update()

    def stop(self) -> None:
        """Stop."""
        self._animated = False
        self.update()

    def paintEvent(self, event: object) -> None:
        """Paintevent."""
        del event
        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)
        bounds = self.rect().adjusted(5, 5, -5, -5)
        center = QPointF(bounds.center())
        theme = theme_spec_for_widget(self)
        state_colors = {
            "idle": theme.accent_blue,
            "waiting": theme.accent,
            "transition": theme.warning,
            "paused": theme.warning,
            "history": theme.accent_purple,
        }
        color = QColor(state_colors[self._state])

        panel = QColor(theme.surface_input)
        panel.setAlpha(210)
        painter.setBrush(panel)
        border = QColor(theme.border)
        border.setAlpha(180)
        painter.setPen(QPen(border, 1.0))
        painter.drawRoundedRect(bounds, 16.0, 16.0)

        ring = QColor(color)
        ring.setAlpha(120 if self._animated else 78)
        painter.setBrush(Qt.BrushStyle.NoBrush)
        painter.setPen(QPen(ring, 1.1))
        painter.drawEllipse(center, 22.0, 12.0)
        painter.drawEllipse(center, 12.0, 22.0)

        if self._animated:
            wave = (math.sin(self._phase) + 1.0) * 0.5
            halo = QColor(color)
            halo.setAlpha(26 + int(wave * 42))
            painter.setBrush(halo)
            painter.setPen(Qt.PenStyle.NoPen)
            painter.drawEllipse(center, 7.0 + wave * 3.0, 7.0 + wave * 3.0)

        painter.setPen(Qt.PenStyle.NoPen)
        painter.setBrush(color)
        painter.drawEllipse(center, 4.5, 4.5)
        painter.setBrush(QColor(theme.text))
        painter.drawEllipse(QPointF(center.x() - 1.0, center.y() - 1.2), 1.0, 1.0)

        for index in range(4):
            angle = self._phase + index * (math.pi / 2.0)
            radius_x = 22.0
            radius_y = 12.0
            node = QPointF(
                center.x() + math.cos(angle) * radius_x,
                center.y() + math.sin(angle) * radius_y,
            )
            node_color = QColor(
                theme.accent_pink if index % 2 else theme.accent_blue
            )
            node_color.setAlpha(165 if self._animated else 95)
            painter.setBrush(node_color)
            painter.drawEllipse(node, 1.8, 1.8)


class TerminalEmptyState(QFrame):
    """Quiet terminal empty state that emits navigation intent only."""

    MOTION_MODE = "activity"
    _STATES = frozenset({"idle", "waiting", "transition", "paused", "history"})
    _CTA_STATES = frozenset({"idle", "waiting", "transition"})
    connection_requested = Signal()

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self.setObjectName("terminalEmptyState")
        self.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Expanding)
        self.setFocusPolicy(Qt.FocusPolicy.NoFocus)
        self.setAccessibleName("终端空态提示")
        self._state = "idle"
        self._phase = 0.0
        self._animated = False

        root = QVBoxLayout(self)
        root.setContentsMargins(16, 8, 16, 8)
        root.addStretch(1)

        self._card = QFrame(self)
        self._card.setObjectName("terminalEmptyCard")
        self._card.setSizePolicy(QSizePolicy.Policy.Preferred, QSizePolicy.Policy.Fixed)
        self._card.setMinimumHeight(128)
        self._card.setMaximumWidth(560)
        row = QHBoxLayout(self._card)
        row.setContentsMargins(18, 10, 18, 10)
        row.setSpacing(14)

        self._glyph = TerminalOrbitGlyph(self._card)
        row.addWidget(self._glyph)
        copy = QVBoxLayout()
        copy.setSpacing(3)
        self._eyebrow = QLabel("SERIALFORGE / OBSERVE", self._card)
        self._eyebrow.setObjectName("terminalEmptyEyebrow")
        self._eyebrow.setProperty("role", "subtle")
        copy.addWidget(self._eyebrow)
        self._title = QLabel("等待链路数据", self._card)
        self._title.setObjectName("terminalEmptyTitle")
        self._title.setAccessibleName("终端空态标题")
        copy.addWidget(self._title)
        self._hint = QLabel("选择连接方式并点击“连接”；接收数据会出现在这里。", self._card)
        self._hint.setObjectName("terminalEmptyHint")
        self._hint.setProperty("role", "muted")
        self._hint.setWordWrap(True)
        self._hint.setAccessibleName("终端下一步提示")
        copy.addWidget(self._hint)
        self._connection_action = TerminalActionButton("打开链路连接", self._card)
        self._connection_action.setObjectName("primaryButton")
        self._connection_action.setAccessibleName("打开链路连接")
        self._connection_action.setAccessibleDescription(
            "跳转到链路连接页面；不会自动建立连接。"
        )
        self._connection_action.setToolTip("打开链路连接页面；不会自动连接。")
        self._connection_action.clicked.connect(self._emit_connection_requested)
        copy.addWidget(self._connection_action, alignment=Qt.AlignmentFlag.AlignLeft)
        row.addLayout(copy, stretch=1)

        root.addWidget(self._card, alignment=Qt.AlignmentFlag.AlignHCenter)
        root.addStretch(1)

    def set_context(self, state: str, title: str, hint: str) -> None:
        """Set context."""
        normalized = state if state in self._STATES else "idle"
        self._state = normalized
        self.setProperty("state", normalized)
        self._card.setProperty("state", normalized)
        self._glyph.set_state(normalized)
        self._title.setText(title)
        self._hint.setText(hint)
        self._connection_action.setVisible(normalized in self._CTA_STATES)
        self.setAccessibleDescription(f"{title}。{hint}")
        for widget in (self, self._card):
            style = widget.style()
            style.unpolish(widget)
            style.polish(widget)
            widget.update()

    def _emit_connection_requested(self, checked: bool = False) -> None:
        """Expose a presentation navigation intent without owning connection state."""

        del checked
        self.connection_requested.emit()

    def paintEvent(self, event: object) -> None:
        """Paint a quiet observation canvas behind the existing empty-state card."""

        super().paintEvent(event)
        if self.width() < 140 or self.height() < 120:
            return

        theme = theme_spec_for_widget(self)
        bounds = QRectF(self.contentsRect()).adjusted(16.0, 12.0, -16.0, -12.0)
        if bounds.width() <= 0 or bounds.height() <= 0:
            return

        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)
        painter.setClipRect(bounds)

        grid = QColor(theme.border)
        grid.setAlpha(25)
        painter.setPen(QPen(grid, 1.0))
        spacing = 32.0
        x = bounds.left()
        while x <= bounds.right():
            painter.drawLine(QPointF(x, bounds.top()), QPointF(x, bounds.bottom()))
            x += spacing
        y = bounds.top()
        while y <= bounds.bottom():
            painter.drawLine(QPointF(bounds.left(), y), QPointF(bounds.right(), y))
            y += spacing

        state_colors = {
            "idle": theme.accent_blue,
            "waiting": theme.accent,
            "transition": theme.warning,
            "paused": theme.warning,
            "history": theme.accent_purple,
        }
        signal = QColor(state_colors[self._state])
        bracket = QColor(signal)
        bracket.setAlpha(70 if self._animated else 42)
        painter.setPen(QPen(bracket, 1.2))
        corner = 14.0
        for left, top, horizontal, vertical in (
            (bounds.left(), bounds.top(), 1.0, 1.0),
            (bounds.right(), bounds.top(), -1.0, 1.0),
            (bounds.left(), bounds.bottom(), 1.0, -1.0),
            (bounds.right(), bounds.bottom(), -1.0, -1.0),
        ):
            painter.drawLine(
                QPointF(left, top), QPointF(left + horizontal * corner, top)
            )
            painter.drawLine(
                QPointF(left, top), QPointF(left, top + vertical * corner)
            )

        if self._animated:
            travel = (math.sin(self._phase) + 1.0) * 0.5
            scan_y = bounds.top() + 18.0 + (bounds.height() - 36.0) * travel
            scan = QColor(signal)
            scan.setAlpha(58)
            painter.setPen(QPen(scan, 1.2))
            painter.drawLine(QPointF(bounds.left(), scan_y), QPointF(bounds.right(), scan_y))
            painter.setPen(Qt.PenStyle.NoPen)
            painter.setBrush(scan)
            for index in range(5):
                node_x = bounds.left() + bounds.width() * index / 4.0
                painter.drawEllipse(QPointF(node_x, scan_y), 2.2, 2.2)
        else:
            static_line = QColor(signal)
            static_line.setAlpha(24)
            painter.setPen(QPen(static_line, 1.0))
            scan_y = bounds.center().y()
            painter.drawLine(QPointF(bounds.left(), scan_y), QPointF(bounds.right(), scan_y))

    def set_frame(self, phase: float, animated: bool) -> None:
        """Set frame."""
        self._phase = float(phase)
        self._animated = bool(animated)
        self.update()
        self._glyph.set_frame(phase, animated)
        self._connection_action.set_frame(phase, animated)

    def stop(self) -> None:
        """Stop."""
        self._animated = False
        self.update()
        self._glyph.stop()
        self._connection_action.stop()


__all__ = [
    "TerminalActionButton",
    "TerminalEmptyState",
    "TerminalOrbitGlyph",
    "TerminalViewport",
]

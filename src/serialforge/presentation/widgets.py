"""Small presentation-only widgets with explicit motion controls."""

from __future__ import annotations

import math
import time

from .qt import (
    QColor,
    QObject,
    QPainter,
    QPen,
    QPointF,
    QRectF,
    Qt,
    QTimer,
    QWidget,
    Signal,
)
from .theme import (
    theme_spec_for_widget,
)


class MotionController(QObject):
    """One precise presentation clock shared by visible decorative widgets.

    The timer uses one precise Qt clock with a short scheduler slot while a
    nominal frame budget calibrates the cadence to an approximately-120 Hz
    target. Qt and the desktop compositor may still
    coalesce frames, so animation phase advances from elapsed time instead of
    assuming that every timer event arrived on time; this remains a scheduler
    target, not an exact display-FPS guarantee.
    """

    frame_changed = Signal(float, bool)

    TARGET_HZ = 120
    _FRAME_PERIOD_MS = 1_000.0 / TARGET_HZ
    # Give the elapsed-time budget enough opportunities to reach the 120 Hz
    # target on Windows. The budget still emits at most one frame per callback
    # so this remains one clock without catch-up repaint bursts.
    _BASE_INTERVAL_MS = 4
    _PHASE_SPEED_RADIANS_PER_SECOND = 1.05
    _MAX_PHASE_DELTA_SECONDS = 0.05

    def __init__(self, parent: QObject | None = None) -> None:
        super().__init__(parent)
        self._timer = self._create_timer()
        self._phase = 0.0
        self._last_tick_at: float | None = None
        self._frame_budget = 0.0
        self._activity_until = 0.0
        self._motion_enabled = True
        self._paused = False
        self._suspended = False
        self._transition_active = False
        self._ambient_active = False
        self._closed = False
        self._rearm_required = True
        self._rearmed_after_show = False

    def _create_timer(self) -> QTimer:
        """Create the one timer owned by this controller."""

        timer = QTimer(self)
        timer.setSingleShot(False)
        timer.setTimerType(Qt.TimerType.PreciseTimer)
        timer.setInterval(self._BASE_INTERVAL_MS)
        timer.timeout.connect(self._tick)
        return timer

    def rearm_after_show(self) -> None:
        """Recreate the timer once after the native window becomes visible.

        Windows can retain a coarse delivery cadence when a QTimer is first
        constructed before the host window enters its visible event path.
        Recreating this controller-owned timer keeps the controller and frame
        signal stable while giving the existing PreciseTimer a post-show
        scheduling origin.  Motion state and phase remain untouched.
        """

        if self._closed or not self._rearm_required:
            return
        old_timer = self._timer
        old_timer.stop()
        try:
            old_timer.timeout.disconnect(self._tick)
        except (RuntimeError, TypeError):
            pass
        old_timer.deleteLater()
        self._timer = self._create_timer()
        self._last_tick_at = None
        self._frame_budget = 0.0
        self._rearm_required = False
        self._rearmed_after_show = True
        self._ensure_timer()

    def set_motion_enabled(self, enabled: bool) -> None:
        """Set motion enabled."""
        self._motion_enabled = bool(enabled)
        if not self._motion_enabled:
            self._stop_timer()
        else:
            self._ensure_timer()

    def set_paused(self, paused: bool) -> None:
        """Set paused."""
        self._paused = bool(paused)
        if self._paused:
            self._stop_timer()
        else:
            self._ensure_timer()

    def set_suspended(self, suspended: bool) -> None:
        """Suspend all animation while the window is hidden or minimized."""

        self._suspended = bool(suspended)
        if self._suspended:
            self._rearm_required = True
            self._stop_timer()
        else:
            self._ensure_timer()

    def set_transition_active(self, active: bool) -> None:
        """Set transition active."""
        self._transition_active = bool(active)
        self._ensure_timer()

    def set_ambient_active(self, active: bool) -> None:
        """Keep the decorative shell alive while the visible window is idle."""

        self._ambient_active = bool(active)
        if not self._ambient_active and not self._transition_active:
            self._activity_until = min(self._activity_until, time.perf_counter())
        self._ensure_timer()

    def request_activity(self, duration_ms: int = 360) -> None:
        """Merge a short visual activity window without inspecting payload data."""

        if not self._motion_enabled or self._paused or self._suspended or self._closed:
            return
        self._activity_until = max(
            self._activity_until,
            time.perf_counter() + max(0, duration_ms) / 1_000.0,
        )
        self._ensure_timer()

    def activity_active(self) -> bool:
        """Return whether a non-ambient visual activity window is live."""

        return (
            self._motion_enabled
            and not self._paused
            and not self._suspended
            and not self._closed
            and (
                self._transition_active
                or time.perf_counter() < self._activity_until
            )
        )

    def close(self) -> None:
        """Close."""
        self._closed = True
        self._stop_timer()
        self._activity_until = 0.0

    def _can_animate(self) -> bool:
        """Can animate."""
        return (
            self._motion_enabled
            and not self._paused
            and not self._suspended
            and not self._closed
            and (
                self._ambient_active
                or self._transition_active
                or time.perf_counter() < self._activity_until
            )
        )

    def _ensure_timer(self) -> None:
        """Ensure timer."""
        if self._can_animate() and not self._timer.isActive():
            self._timer.start()

    def _stop_timer(self) -> None:
        """Stop timer."""
        self._timer.stop()
        self._last_tick_at = None
        self._frame_budget = 0.0

    def _tick(self) -> None:
        """Tick."""
        if not self._can_animate():
            self._stop_timer()
            self.frame_changed.emit(self._phase, False)
            return

        now = time.perf_counter()
        elapsed = self._FRAME_PERIOD_MS / 1_000.0
        if self._last_tick_at is not None:
            elapsed = min(self._MAX_PHASE_DELTA_SECONDS, max(0.0, now - self._last_tick_at))
        self._last_tick_at = now
        self._phase = (
            self._phase + elapsed * self._PHASE_SPEED_RADIANS_PER_SECOND
        ) % (2.0 * math.pi)
        # Use the measured scheduler interval instead of assuming every
        # PreciseTimer callback arrived exactly at the nominal frame period.
        # Keep only the fractional remainder after one frame: a temporarily
        # busy event loop must not turn stale budget into a repaint burst.
        self._frame_budget += elapsed * self.TARGET_HZ
        if self._frame_budget < 1.0:
            return
        self._frame_budget %= 1.0
        self.frame_changed.emit(self._phase, True)


class StatusIndicator(QWidget):
    """A text-independent status light that never owns business state.

    The parent window supplies a lifecycle value and may request a short RX
    pulse. The light is deliberately decorative: the adjacent text label is
    the authoritative, accessible status expression.
    """

    _STATES = frozenset({"discovered", "opening", "open", "closing", "closed", "error"})
    _QT_ANGLE_UNITS = 16
    _OPENING_ARC_DEGREES = 120

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self._state = "closed"
        self._phase = 0.0
        self._animated = False
        self.setFixedSize(22, 22)
        self.setFocusPolicy(Qt.FocusPolicy.NoFocus)
        self.setAttribute(Qt.WidgetAttribute.WA_TransparentForMouseEvents, True)
        self.setAccessibleName("")
        self.setAccessibleDescription("")

    def set_state(self, state: object) -> None:
        """Reflect a lifecycle value without interpreting or changing it."""

        value = getattr(state, "value", state)
        normalized = str(value).lower()
        self._state = normalized if normalized in self._STATES else "closed"
        self.update()

    def set_frame(self, phase: float, animated: bool) -> None:
        """Consume a shared presentation frame without owning its timer."""

        self._phase = phase
        self._animated = bool(animated)
        self.update()

    def stop(self) -> None:
        """Freeze the outer glow before the owning window closes or hides."""

        self._animated = False
        self.update()

    def motion_active(self) -> bool:
        """Return whether the current lifecycle state has a moving glyph."""

        return self._state in {"opening", "open", "closing"}

    def paintEvent(self, event: object) -> None:
        """Paintevent."""
        del event
        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)
        theme = theme_spec_for_widget(self)
        colors = {
            "discovered": theme.accent_blue,
            "opening": theme.warning,
            "open": theme.accent,
            "closing": theme.warning,
            "closed": theme.text_muted,
            "error": theme.error,
        }
        color = QColor(colors[self._state])
        bounds = QRectF(self.rect()).adjusted(2.0, 2.0, -2.0, -2.0)
        center = bounds.center()
        outer = bounds.adjusted(1.0, 1.0, -1.0, -1.0)
        painter.setBrush(Qt.BrushStyle.NoBrush)
        halo = QColor(color)
        halo.setAlpha(72 if self._animated else 38)
        painter.setPen(QPen(halo, 1.0))
        painter.drawEllipse(outer)

        if self._state in {"opening", "closing"} and self._animated:
            arc_color = QColor(color)
            arc_color.setAlpha(220)
            arc_pen = QPen(arc_color, 1.5)
            arc_pen.setCapStyle(Qt.PenCapStyle.RoundCap)
            painter.setPen(arc_pen)
            start_angle = int(math.degrees(self._phase) * self._QT_ANGLE_UNITS)
            painter.drawArc(
                bounds.adjusted(1.0, 1.0, -1.0, -1.0),
                start_angle,
                self._OPENING_ARC_DEGREES * self._QT_ANGLE_UNITS,
            )
        elif self._state == "open" and self._animated:
            wave = (math.sin(self._phase) + 1.0) * 0.5
            pulse = QColor(color)
            pulse.setAlpha(int(80 + wave * 80))
            pulse_pen = QPen(pulse, 1.2)
            pulse_pen.setCapStyle(Qt.PenCapStyle.RoundCap)
            painter.setPen(pulse_pen)
            radius = 6.4 + wave * 1.4
            painter.drawEllipse(center, radius, radius)

        painter.setPen(Qt.PenStyle.NoPen)
        painter.setBrush(color)
        painter.drawEllipse(center, 3.8, 3.8)

        if self._state == "discovered":
            marker_pen = QPen(color, 1.1)
            marker_pen.setCapStyle(Qt.PenCapStyle.RoundCap)
            painter.setPen(marker_pen)
            painter.drawLine(
                QPointF(center.x() - 5.8, center.y()),
                QPointF(center.x() - 3.8, center.y()),
            )
            painter.drawLine(
                QPointF(center.x() + 3.8, center.y()),
                QPointF(center.x() + 5.8, center.y()),
            )
            painter.drawLine(
                QPointF(center.x(), center.y() - 5.8),
                QPointF(center.x(), center.y() - 3.8),
            )
            painter.drawLine(
                QPointF(center.x(), center.y() + 3.8),
                QPointF(center.x(), center.y() + 5.8),
            )
        elif self._state == "error":
            marker_pen = QPen(QColor(theme.text), 1.3)
            marker_pen.setCapStyle(Qt.PenCapStyle.RoundCap)
            painter.setPen(marker_pen)
            painter.drawLine(
                QPointF(center.x() - 3.0, center.y() - 3.0),
                QPointF(center.x() + 3.0, center.y() + 3.0),
            )
            painter.drawLine(
                QPointF(center.x() + 3.0, center.y() - 3.0),
                QPointF(center.x() - 3.0, center.y() + 3.0),
            )

        painter.setPen(Qt.PenStyle.NoPen)
        painter.setBrush(QColor(theme.text))
        painter.drawEllipse(
            QPointF(center.x() - 1.0, center.y() - 1.1),
            1.2,
            1.2,
        )


class SignalFieldWidget(QWidget):
    """A compact, resource-free geometric signal field for the shell header."""

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self._phase = 0.0
        self._animated = False
        self.setFixedSize(148, 34)
        self.setAttribute(Qt.WidgetAttribute.WA_TransparentForMouseEvents, True)
        self.setFocusPolicy(Qt.FocusPolicy.NoFocus)
        self.setAccessibleName("")
        self.setAccessibleDescription("")

    def set_frame(self, phase: float, animated: bool) -> None:
        """Consume the shared frame; static mode keeps the last frame frozen."""

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
        bounds = self.rect().adjusted(1, 1, -1, -1)

        theme = theme_spec_for_widget(self)
        panel = QColor(theme.surface)
        panel.setAlpha(235)
        painter.setBrush(panel)
        border = QColor(theme.accent_purple)
        border.setAlpha(160)
        painter.setPen(QPen(border, 1.0))
        painter.drawRoundedRect(bounds, 9.0, 9.0)

        grid = QColor(theme.border)
        grid.setAlpha(110)
        grid_pen = QPen(grid)
        grid_pen.setWidth(1)
        painter.setPen(grid_pen)
        for index in range(1, 4):
            x = bounds.left() + index * bounds.width() / 4.0
            painter.drawLine(QPointF(x, bounds.top() + 5.0), QPointF(x, bounds.bottom() - 5.0))

        painter.drawLine(
            QPointF(bounds.left() + 7.0, bounds.center().y()),
            QPointF(bounds.right() - 7.0, bounds.center().y()),
        )

        orbit = QColor(theme.accent_purple)
        orbit.setAlpha(65 if self._animated else 42)
        painter.setPen(QPen(orbit, 1.0))
        painter.setBrush(Qt.BrushStyle.NoBrush)
        painter.drawEllipse(
            QPointF(bounds.center().x(), bounds.center().y()),
            bounds.width() * 0.28,
            bounds.height() * 0.34,
        )
        painter.drawEllipse(
            QPointF(bounds.center().x(), bounds.center().y()),
            bounds.width() * 0.42,
            bounds.height() * 0.22,
        )

        sparkle_colors = (theme.accent_pink, theme.accent_blue)
        sparkle_specs = (
            (0.15, 0.28, 2.8),
            (0.84, 0.72, 2.1),
        )
        for index, ((x_ratio, y_ratio, radius), color_value) in enumerate(
            zip(sparkle_specs, sparkle_colors, strict=True)
        ):
            drift = (
                math.sin(self._phase * 0.8 + index * 2.0) * 1.2
                if self._animated
                else 0.0
            )
            sparkle = QColor(color_value)
            sparkle.setAlpha(165 if self._animated else 92)
            self._draw_sparkle(
                painter,
                QPointF(
                    bounds.left() + bounds.width() * x_ratio,
                    bounds.top() + bounds.height() * y_ratio + drift,
                ),
                radius,
                sparkle,
            )

        travel_width = max(1.0, bounds.width() - 12.0)
        for index, y_offset in enumerate((-3.0, 2.5, -1.5)):
            travel = (
                self._phase * 10.0 + index * travel_width / 3.0
            ) % travel_width
            x = bounds.left() + 6.0 + travel
            y = bounds.center().y() + y_offset + math.sin(self._phase * 0.7 + index * 1.7) * 2.8
            color = QColor(
                theme.accent
                if index == 0
                else theme.accent_pink
                if index == 1
                else theme.accent_blue
            )
            color.setAlpha(185 if self._animated else 82)
            if self._animated:
                trail = QColor(color)
                trail.setAlpha(38)
                painter.setPen(QPen(trail, 1.0))
                painter.drawLine(QPointF(x - 5.0, y), QPointF(x, y))
            painter.setPen(Qt.PenStyle.NoPen)
            painter.setBrush(color)
            painter.drawEllipse(QPointF(x, y), 1.9, 1.9)

        scan_x = (
            bounds.left()
            + 7.0
            + (
                (self._phase * 12.0) % max(1.0, bounds.width() - 14.0)
                if self._animated
                else bounds.width() * 0.62
            )
        )
        scan_color = QColor(theme.accent_pink)
        scan_color.setAlpha(110 if self._animated else 55)
        painter.setPen(QPen(scan_color, 1.0))
        painter.drawLine(
            QPointF(scan_x, bounds.top() + 4.0),
            QPointF(scan_x, bounds.bottom() - 4.0),
        )

    @staticmethod
    def _draw_sparkle(
        painter: QPainter,
        center: QPointF,
        radius: float,
        color: QColor,
    ) -> None:
        """Draw a tiny resource-free four-point sparkle for the anime rail."""

        pen = QPen(color, 1.0)
        pen.setCapStyle(Qt.PenCapStyle.RoundCap)
        painter.setPen(pen)
        painter.drawLine(
            QPointF(center.x(), center.y() - radius),
            QPointF(center.x(), center.y() + radius),
        )
        painter.drawLine(
            QPointF(center.x() - radius, center.y()),
            QPointF(center.x() + radius, center.y()),
        )
        core = QColor(color)
        core.setAlpha(min(255, color.alpha() + 35))
        painter.setPen(Qt.PenStyle.NoPen)
        painter.setBrush(core)
        painter.drawEllipse(center, radius * 0.22, radius * 0.22)


__all__ = ["MotionController", "SignalFieldWidget", "StatusIndicator"]

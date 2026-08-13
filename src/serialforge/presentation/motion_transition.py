"""One-shot property transitions driven by the shell's shared motion clock."""

from __future__ import annotations

import time

from shiboken6 import isValid

from .qt import QEasingCurve, QObject, QPropertyAnimation, Signal


class MotionDrivenAnimationGroup(QObject):
    """Advance presentation properties from ``MotionController.frame_changed``.

    Qt's default animation driver may deliver ``QPropertyAnimation`` updates
    at a lower cadence than the shell's 120 Hz presentation clock. The
    property animations here remain the interpolation/easing contract, while
    this owner applies their current values on the shared frame signal. It
    owns no timer, application state, or device work.
    """

    finished = Signal()

    def __init__(
        self,
        motion_controller: QObject,
        duration_ms: int,
        parent: QObject | None = None,
    ) -> None:
        super().__init__(parent)
        self._motion_controller = motion_controller
        self._duration_ms = max(1, int(duration_ms))
        self._tracks: list[tuple[QPropertyAnimation, QObject, str]] = []
        self._started_at: float | None = None
        self._running = False

    def add_property_animation(
        self,
        target: QObject,
        property_name: bytes | str,
        start_value: object,
        end_value: object,
        easing: QEasingCurve.Type,
    ) -> None:
        """Add one eased property track before the group starts."""

        if self._running:
            raise RuntimeError("cannot add a track to a running motion transition")
        name = (
            property_name.decode("ascii")
            if isinstance(property_name, bytes)
            else property_name
        )
        animation = QPropertyAnimation(target, property_name, self)
        animation.setDuration(self._duration_ms)
        animation.setStartValue(start_value)
        animation.setEndValue(end_value)
        animation.setEasingCurve(easing)
        self._tracks.append((animation, target, name))

    def start(self) -> None:
        """Start from the first value and request shared-clock activity."""

        self.stop()
        self._started_at = time.monotonic()
        self._running = True
        self._apply_current_time(0)
        self._motion_controller.frame_changed.connect(self._on_frame)
        request_activity = getattr(self._motion_controller, "request_activity", None)
        if callable(request_activity):
            request_activity(self._duration_ms + 120)

    def stop(self) -> None:
        """Disconnect from the shared clock without deciding the final layout."""

        if not self._running:
            return
        try:
            self._motion_controller.frame_changed.disconnect(self._on_frame)
        except (RuntimeError, TypeError):
            pass
        self._running = False
        self._started_at = None

    def _on_frame(self, _phase: float, active: bool) -> None:
        """On frame."""
        if not self._running or not active or self._started_at is None:
            return
        if any(not isValid(target) for _, target, _ in self._tracks):
            self.stop()
            self.finished.emit()
            return
        elapsed_ms = int((time.monotonic() - self._started_at) * 1_000.0)
        current_time = min(self._duration_ms, max(0, elapsed_ms))
        self._apply_current_time(current_time)
        if current_time >= self._duration_ms:
            self.stop()
            self.finished.emit()

    def _apply_current_time(self, current_time: int) -> None:
        """Apply current time."""
        for animation, target, property_name in self._tracks:
            if not isValid(target):
                continue
            animation.setCurrentTime(current_time)
            target.setProperty(property_name, animation.currentValue())


__all__ = ["MotionDrivenAnimationGroup"]

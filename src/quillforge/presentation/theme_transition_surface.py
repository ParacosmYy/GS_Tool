"""Presentation composition for the shell theme transition."""

from __future__ import annotations

from PyQt6.QtCore import QAbstractAnimation, QEasingCurve, QPropertyAnimation
from PyQt6.QtWidgets import QGraphicsOpacityEffect, QWidget


class ThemeTransitionSurface:
    """Own the short-lived opacity animation used after appearance changes."""

    _DURATION_MS = 220

    def __init__(self) -> None:
        self._animation: QPropertyAnimation | None = None
        self._target: QWidget | None = None
        self._effect: QGraphicsOpacityEffect | None = None

    def animate(self, target: QWidget | None, *, enabled: bool) -> None:
        """Start or clear the fade for ``target`` according to motion policy."""
        self._stop_current()
        if not enabled or target is None:
            return
        if target.graphicsEffect() is not None:
            return

        effect = QGraphicsOpacityEffect(target)
        target.setGraphicsEffect(effect)
        animation = QPropertyAnimation(effect, b"opacity", target)
        animation.setDuration(self._DURATION_MS)
        animation.setStartValue(0.72)
        animation.setEndValue(1.0)
        animation.setEasingCurve(QEasingCurve.Type.OutCubic)

        self._target = target
        self._effect = effect

        def clear_effect() -> None:
            if target.graphicsEffect() is effect:
                target.setGraphicsEffect(None)
            if self._animation is animation:
                self._animation = None
                self._target = None
                self._effect = None

        animation.finished.connect(clear_effect)
        self._animation = animation
        animation.start(QAbstractAnimation.DeletionPolicy.DeleteWhenStopped)

    def _stop_current(self) -> None:
        animation = self._animation
        target = self._target
        effect = self._effect
        self._animation = None
        self._target = None
        self._effect = None
        if animation is not None:
            animation.stop()
        if target is not None and target.graphicsEffect() is effect:
            target.setGraphicsEffect(None)

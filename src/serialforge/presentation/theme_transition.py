"""One-shot theme transition owned by the presentation shell.

The helper combines a short root fade with a disposable semantic-color sweep.
Both animations are owned here, remain independent of application state, and
are cleaned up together at every lifecycle boundary.
"""

from __future__ import annotations

from shiboken6 import isValid

from .motion_transition import MotionDrivenAnimationGroup
from .qt import (
    QEasingCurve,
    QGraphicsOpacityEffect,
    QRect,
    QWidget,
)
from .theme_transition_surface import ThemeTransitionSurface
from .transition_coordinator import (
    ShellTransitionKind,
    prepare_shell_transition,
    stop_shell_transitions,
)

_THEME_SWEEP_HEIGHT = 220


def start_theme_transition(
    window,
    *,
    target: QWidget | None,
    sweep_host: QWidget | None,
) -> None:
    """Fade the root and sweep only the explicit presentation chrome host."""

    if target is None or sweep_host is None:
        stop_shell_transitions(window)
        return
    if sweep_host is not target and not target.isAncestorOf(sweep_host):
        stop_shell_transitions(window)
        return
    prepare_shell_transition(window, ShellTransitionKind.THEME)
    stop_theme_transition(window)
    existing = target.graphicsEffect()
    if existing is not None and not isinstance(existing, QGraphicsOpacityEffect):
        return

    effect = QGraphicsOpacityEffect(target)
    effect.setOpacity(0.86)
    target.setGraphicsEffect(effect)
    animation = MotionDrivenAnimationGroup(
        window._motion_controller,
        180,
        window,
    )
    animation.add_property_animation(
        effect,
        b"opacity",
        0.86,
        1.0,
        QEasingCurve.Type.OutCubic,
    )

    # The root fade communicates a global palette change.  The sweep is
    # deliberately parented to the header host so it cannot cover terminal or
    # send surfaces even if the workspace grows or its layout changes.
    sweep_width = min(180, max(120, sweep_host.width() // 5))
    sweep_height = min(_THEME_SWEEP_HEIGHT, max(1, sweep_host.height()))
    sweep = ThemeTransitionSurface(sweep_host)
    sweep.setGeometry(-sweep_width, 0, sweep_width, sweep_height)
    sweep.raise_()
    sweep.show()
    animation.add_property_animation(
        sweep,
        b"geometry",
        QRect(-sweep_width, 0, sweep_width, sweep_height),
        QRect(sweep_host.width(), 0, sweep_width, sweep_height),
        QEasingCurve.Type.InOutCubic,
    )
    animation.finished.connect(
        lambda animation=animation: _finish_theme_transition(
            window,
            animation,
            effect,
            target,
            sweep,
        )
    )
    window._theme_transition = animation
    window._theme_transition_effect = effect
    window._theme_transition_overlay = sweep
    animation.start()


def stop_theme_transition(window) -> None:
    """Stop and remove the temporary effect without changing theme state."""

    animation = getattr(window, "_theme_transition", None)
    effect = getattr(window, "_theme_transition_effect", None)
    sweep = getattr(window, "_theme_transition_overlay", None)
    window._theme_transition = None
    window._theme_transition_effect = None
    window._theme_transition_overlay = None
    target = (
        effect.parent()
        if isinstance(effect, QGraphicsOpacityEffect) and isValid(effect)
        else None
    )
    if isinstance(animation, MotionDrivenAnimationGroup) and isValid(animation):
        animation.stop()
        animation.deleteLater()
    if isinstance(effect, QGraphicsOpacityEffect) and isValid(effect):
        effect.setOpacity(1.0)
        if isinstance(target, QWidget) and target.graphicsEffect() is effect:
            target.setGraphicsEffect(None)
    if isinstance(sweep, ThemeTransitionSurface) and isValid(sweep):
        sweep.hide()
        sweep.deleteLater()


def _finish_theme_transition(
    window,
    animation: MotionDrivenAnimationGroup,
    effect: QGraphicsOpacityEffect,
    target: QWidget,
    sweep: ThemeTransitionSurface,
) -> None:
    """Release only the animation that is still the active shell transition."""

    if getattr(window, "_theme_transition", None) is not animation:
        return
    window._theme_transition = None
    window._theme_transition_effect = None
    window._theme_transition_overlay = None
    if isValid(effect):
        effect.setOpacity(1.0)
        if isValid(target) and target.graphicsEffect() is effect:
            target.setGraphicsEffect(None)
    if isValid(sweep):
        sweep.hide()
        sweep.deleteLater()
    if isValid(animation):
        animation.deleteLater()


__all__ = ["start_theme_transition", "stop_theme_transition"]

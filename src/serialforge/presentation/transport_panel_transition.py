"""One-shot transition for transport-specific connection settings."""

from __future__ import annotations

from shiboken6 import isValid

from .motion_policy import decorative_motion_enabled
from .motion_transition import MotionDrivenAnimationGroup
from .qt import QEasingCurve, QGraphicsOpacityEffect, QWidget
from .transition_coordinator import (
    ShellTransitionKind,
    prepare_shell_transition,
    stop_shell_transitions,
)


def start_transport_panel_transition(window, panel: QWidget | None) -> None:
    """Fade a newly visible settings panel without owning transport state."""

    if (
        panel is None
        or window._closing
        or window.isHidden()
        or window.isMinimized()
        or not panel.isVisible()
        or not decorative_motion_enabled(window)
    ):
        stop_shell_transitions(window)
        return

    prepare_shell_transition(window, ShellTransitionKind.TRANSPORT)
    stop_transport_panel_transition(window)
    existing = panel.graphicsEffect()
    if existing is not None:
        return

    effect = QGraphicsOpacityEffect(panel)
    effect.setOpacity(0.84)
    panel.setGraphicsEffect(effect)
    animation = MotionDrivenAnimationGroup(
        window._motion_controller,
        160,
        window,
    )
    animation.add_property_animation(
        effect,
        b"opacity",
        0.84,
        1.0,
        QEasingCurve.Type.OutCubic,
    )
    animation.finished.connect(
        lambda animation=animation, effect=effect, panel=panel: _finish_transport_panel_transition(
            window,
            animation,
            effect,
            panel,
        )
    )
    window._transport_panel_transition = animation
    window._transport_panel_transition_effect = effect
    animation.start()


def stop_transport_panel_transition(window) -> None:
    """Stop and detach the temporary effect at every lifecycle boundary."""

    animation = getattr(window, "_transport_panel_transition", None)
    effect = getattr(window, "_transport_panel_transition_effect", None)
    window._transport_panel_transition = None
    window._transport_panel_transition_effect = None
    if isinstance(animation, MotionDrivenAnimationGroup) and isValid(animation):
        animation.stop()
        animation.deleteLater()
    if isinstance(effect, QGraphicsOpacityEffect) and isValid(effect):
        effect.setOpacity(1.0)
        target = effect.parent()
        if isinstance(target, QWidget) and target.graphicsEffect() is effect:
            target.setGraphicsEffect(None)


def _finish_transport_panel_transition(
    window,
    animation: MotionDrivenAnimationGroup,
    effect: QGraphicsOpacityEffect,
    panel: QWidget,
) -> None:
    """Release only the animation that is still current."""

    if getattr(window, "_transport_panel_transition", None) is not animation:
        return
    window._transport_panel_transition = None
    window._transport_panel_transition_effect = None
    if isValid(effect):
        effect.setOpacity(1.0)
        if panel.graphicsEffect() is effect:
            panel.setGraphicsEffect(None)
    if isValid(animation):
        animation.deleteLater()


__all__ = [
    "start_transport_panel_transition",
    "stop_transport_panel_transition",
]

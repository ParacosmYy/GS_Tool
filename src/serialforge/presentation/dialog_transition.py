"""One-shot entrance transition for presentation-owned custom dialogs."""

from __future__ import annotations

from shiboken6 import isValid

from .motion_policy import decorative_motion_enabled
from .motion_transition import MotionDrivenAnimationGroup
from .qt import QDialog, QEasingCurve, QGraphicsOpacityEffect

_DIALOG_START_OPACITY = 0.90
_DIALOG_TRANSITION_MS = 150


def start_dialog_transition(dialog: QDialog) -> None:
    """Fade a custom dialog in when the owning window allows decorative motion."""

    owner = dialog.parentWidget() or dialog
    if not decorative_motion_enabled(owner):
        stop_dialog_transition(dialog)
        return
    motion_controller = getattr(owner, "_motion_controller", None)
    if motion_controller is None:
        stop_dialog_transition(dialog)
        return

    stop_dialog_transition(dialog)
    if dialog.graphicsEffect() is not None:
        return

    effect = QGraphicsOpacityEffect(dialog)
    effect.setOpacity(_DIALOG_START_OPACITY)
    dialog.setGraphicsEffect(effect)
    animation = MotionDrivenAnimationGroup(
        motion_controller,
        _DIALOG_TRANSITION_MS,
        dialog,
    )
    animation.add_property_animation(
        effect,
        b"opacity",
        _DIALOG_START_OPACITY,
        1.0,
        QEasingCurve.Type.OutCubic,
    )
    animation.finished.connect(
        lambda animation=animation, effect=effect: _finish_dialog_transition(
            dialog,
            animation,
            effect,
        )
    )
    dialog._dialog_transition = animation
    dialog._dialog_transition_effect = effect
    animation.start()


def stop_dialog_transition(dialog: QDialog) -> None:
    """Stop and detach the temporary effect at every dialog hide boundary."""

    animation = getattr(dialog, "_dialog_transition", None)
    effect = getattr(dialog, "_dialog_transition_effect", None)
    dialog._dialog_transition = None
    dialog._dialog_transition_effect = None
    if isinstance(animation, MotionDrivenAnimationGroup) and isValid(animation):
        animation.stop()
        animation.deleteLater()
    if isinstance(effect, QGraphicsOpacityEffect) and isValid(effect):
        effect.setOpacity(1.0)
        if dialog.graphicsEffect() is effect:
            dialog.setGraphicsEffect(None)


def _finish_dialog_transition(
    dialog: QDialog,
    animation: MotionDrivenAnimationGroup,
    effect: QGraphicsOpacityEffect,
) -> None:
    """Release only the animation that is still current for this dialog."""

    if getattr(dialog, "_dialog_transition", None) is not animation:
        return
    dialog._dialog_transition = None
    dialog._dialog_transition_effect = None
    if isValid(effect):
        effect.setOpacity(1.0)
        if isValid(dialog) and dialog.graphicsEffect() is effect:
            dialog.setGraphicsEffect(None)
    if isValid(animation):
        animation.deleteLater()


__all__ = ["start_dialog_transition", "stop_dialog_transition"]

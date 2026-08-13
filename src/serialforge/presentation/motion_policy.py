"""Shared presentation-only motion preference policy."""

from __future__ import annotations

from .theme import reduced_motion_requested


def decorative_motion_enabled(window) -> bool:
    """Return whether one-shot and ambient decorative motion may run."""

    motion_check = getattr(window, "_motion_check", None)
    pause_check = getattr(window, "_motion_pause_check", None)
    return not (
        reduced_motion_requested()
        or (motion_check is not None and motion_check.isChecked())
        or (pause_check is not None and pause_check.isChecked())
    )


__all__ = ["decorative_motion_enabled"]

"""Mutual exclusion for one-shot shell presentation transitions."""

from __future__ import annotations

from collections.abc import Callable
from enum import StrEnum


class ShellTransitionKind(StrEnum):
    """Named transition slots that share one window-level visual budget."""

    THEME = "theme"
    WORKSPACE = "workspace"
    FOCUS = "focus"
    TRANSPORT = "transport"


def prepare_shell_transition(
    window: object,
    kind: ShellTransitionKind,
) -> None:
    """Stop every competing shell transition before one owner starts."""

    stop_shell_transitions(window, keep=kind)


def stop_shell_transitions(
    window: object,
    *,
    keep: ShellTransitionKind | None = None,
) -> None:
    """Stop shell transitions idempotently without owning their internals."""

    for kind, stop in _transition_stoppers():
        if kind is not keep:
            stop(window)


def _transition_stoppers() -> tuple[tuple[ShellTransitionKind, Callable], ...]:
    """Resolve owners lazily so transition modules do not form an import cycle."""

    from .controllers.workspace_runtime import stop_workspace_transition
    from .theme_transition import stop_theme_transition
    from .transport_panel_transition import stop_transport_panel_transition
    from .workspace_focus_transition import stop_workspace_focus_transition

    return (
        (ShellTransitionKind.THEME, stop_theme_transition),
        (ShellTransitionKind.WORKSPACE, stop_workspace_transition),
        (ShellTransitionKind.FOCUS, stop_workspace_focus_transition),
        (ShellTransitionKind.TRANSPORT, stop_transport_panel_transition),
    )


__all__ = [
    "ShellTransitionKind",
    "prepare_shell_transition",
    "stop_shell_transitions",
]

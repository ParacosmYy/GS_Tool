"""Typed presentation wiring for the workspace shell.

The bundle contains only widgets that are composed together by the workspace
owner. It is intentionally free of application state, timers, navigation
policy, and business callbacks; controllers consume it as a narrow shell
boundary instead of reaching through a collection of window attributes.
"""

from __future__ import annotations

from dataclasses import dataclass

from .qt import QFrame, QTabWidget, QToolButton
from .workspace_context_surface import WorkspaceContextLabel
from .workspace_route_surface import WorkspaceRouteSurface
from .workspace_scroll_hint import WorkspaceScrollHint
from .workspace_tab_surface import AnimatedWorkspaceTabBar


@dataclass(frozen=True, slots=True)
class WorkspaceShellBindings:
    """The complete, immutable widget bundle owned by one workspace shell."""

    shell: QFrame
    tabs: QTabWidget
    tab_bar: AnimatedWorkspaceTabBar
    route: WorkspaceRouteSurface
    context_label: WorkspaceContextLabel
    scroll_hint: WorkspaceScrollHint
    focus_button: QToolButton


def workspace_bindings_for(window: object) -> WorkspaceShellBindings | None:
    """Read the typed shell bundle without importing the MainWindow class."""

    bindings = getattr(window, "_workspace_bindings", None)
    return bindings if isinstance(bindings, WorkspaceShellBindings) else None


__all__ = ["WorkspaceShellBindings", "workspace_bindings_for"]

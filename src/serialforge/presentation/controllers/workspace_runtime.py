"""Workspace visibility and transition runtime.

This module owns the stateful part of workspace navigation: derived-panel
suspension, one-shot tab transitions, and the shared motion policy. The
workspace builder only creates widgets and wires these callbacks, while the
lifecycle controller consumes this runtime without importing the builder.
"""

from __future__ import annotations

from shiboken6 import isValid

from ...domain.models import SessionState
from ..motion_policy import decorative_motion_enabled
from ..motion_transition import MotionDrivenAnimationGroup
from ..protocol_scope import derived_source_supported
from ..qt import QEasingCurve, QGraphicsOpacityEffect, QWidget
from ..transition_coordinator import (
    ShellTransitionKind,
    prepare_shell_transition,
    stop_shell_transitions,
)
from ..workspace_bindings import workspace_bindings_for
from ..workspace_focus_transition import (
    refresh_workspace_focus_layout,
    set_workspace_focus_mode,
)

_CONNECTION_TAB_INDEX = 0
_COMPACT_CONNECTION_FOCUS_HEIGHT = 760


def set_connection_focus_override(window, enabled: bool) -> None:
    """Remember an explicit connection-page view choice for this session."""

    workspace = workspace_bindings_for(window)
    if workspace is None or workspace.tabs.currentIndex() != _CONNECTION_TAB_INDEX:
        return
    window._connection_focus_override = bool(enabled)


def _connection_focus_override(window) -> bool | None:
    """Connection focus override."""
    value = getattr(window, "_connection_focus_override", None)
    return value if isinstance(value, bool) else None


def _compact_connection_focus_required(window) -> bool:
    """Keep the connection form readable before sharing the shell vertically."""

    minimum_height = max(1, int(window.minimumHeight()))
    return int(window.height()) <= max(_COMPACT_CONNECTION_FOCUS_HEIGHT, minimum_height)


def _connection_focus_desired(window, onboarding_focus: bool) -> bool:
    """Resolve focus without overriding an explicit connection-page choice."""

    if onboarding_focus:
        return True
    override = _connection_focus_override(window)
    return override is not False and _compact_connection_focus_required(window)


def sync_compact_connection_focus(window) -> None:
    """Adapt the connection page after a resize without creating a new clock."""

    if (
        getattr(window, "_closing", False)
        or window.isHidden()
        or window.isMinimized()
        or _connection_focus_override(window) is False
        or not _compact_connection_focus_required(window)
    ):
        return
    workspace = workspace_bindings_for(window)
    if workspace is None or workspace.tabs.currentIndex() != _CONNECTION_TAB_INDEX:
        return
    if not bool(getattr(window, "_workspace_focus_mode", False)):
        set_workspace_focus_mode(window, True)


def start_connection_onboarding(window) -> None:
    """Open the initial closed-session connection route in presentation focus."""

    if getattr(window, "_closing", False):
        return
    window._connection_focus_override = None
    window._connection_onboarding = True
    workspace = workspace_bindings_for(window)
    if workspace is None or workspace.tabs.currentIndex() != _CONNECTION_TAB_INDEX:
        return
    set_workspace_focus_mode(window, True)


def leave_connection_onboarding(window) -> None:
    """Return to overview after opening, unless compact focus is still needed."""

    if not getattr(window, "_connection_onboarding", False):
        return
    window._connection_onboarding = False
    workspace = workspace_bindings_for(window)
    if workspace is None or workspace.tabs.currentIndex() != _CONNECTION_TAB_INDEX:
        return
    compact_focus = _compact_connection_focus_required(window)
    if compact_focus and _connection_focus_override(window) is not False:
        set_workspace_focus_mode(window, True)
    elif bool(getattr(window, "_workspace_focus_mode", False)):
        set_workspace_focus_mode(window, False)


def _release_workspace_effect(effect: QGraphicsOpacityEffect | None) -> None:
    """Restore and detach the temporary page effect at every exit path."""

    if not isinstance(effect, QGraphicsOpacityEffect) or not isValid(effect):
        return
    effect.setOpacity(1.0)
    target = effect.parent()
    if isinstance(target, QWidget) and target.graphicsEffect() is effect:
        target.setGraphicsEffect(None)


def stop_workspace_transition(window) -> None:
    """Restore the active page before releasing a one-shot tab animation."""

    animation = window._workspace_transition
    effect = window._workspace_transition_effect
    window._workspace_transition = None
    window._workspace_transition_effect = None
    if isinstance(animation, MotionDrivenAnimationGroup) and isValid(animation):
        animation.stop()
        animation.deleteLater()
    _release_workspace_effect(effect)


def workspace_motion_enabled(window) -> bool:
    """Share the existing motion policy with workspace transitions."""

    return decorative_motion_enabled(window)


def animate_workspace_transition(window, index: int) -> None:
    """Fade in the newly selected workspace page without touching app state."""

    if (
        window._closing
        or index < 0
        or window.isHidden()
        or window.isMinimized()
        or not workspace_motion_enabled(window)
    ):
        stop_shell_transitions(window)
        return
    focus_transition = getattr(window, "_workspace_focus_transition", None)
    if focus_transition is not None and isValid(focus_transition):
        # The focus geometry is the primary route transition when a Tab change
        # also changes overview/configuration mode. Stop any older page fade so
        # two animations do not compete for the same workspace surface.
        stop_workspace_transition(window)
        return
    workspace = workspace_bindings_for(window)
    if workspace is None:
        stop_shell_transitions(window)
        return
    page = workspace.tabs.widget(index)
    if page is None:
        stop_shell_transitions(window)
        return

    prepare_shell_transition(window, ShellTransitionKind.WORKSPACE)
    stop_workspace_transition(window)
    current_effect = page.graphicsEffect()
    if current_effect is not None and not isinstance(current_effect, QGraphicsOpacityEffect):
        return
    effect = current_effect
    if effect is None:
        effect = QGraphicsOpacityEffect(page)
        page.setGraphicsEffect(effect)
    effect.setOpacity(0.82)
    animation = MotionDrivenAnimationGroup(
        window._motion_controller,
        180,
        window,
    )
    animation.add_property_animation(
        effect,
        b"opacity",
        0.82,
        1.0,
        QEasingCurve.Type.OutCubic,
    )
    animation.finished.connect(
        lambda animation=animation, effect=effect: finish_workspace_transition(
            window, animation, effect
        )
    )
    window._workspace_transition = animation
    window._workspace_transition_effect = effect
    animation.start()


def request_workspace_activity(window, _index: int = -1) -> None:
    """Acknowledge visible user navigation through the shared motion clock."""

    if window._closing or window.isHidden() or window.isMinimized():
        return
    window._motion_controller.request_activity(320)


def finish_workspace_transition(
    window,
    animation: MotionDrivenAnimationGroup,
    effect: QGraphicsOpacityEffect,
) -> None:
    """Complete only the animation that is still current."""

    if window._workspace_transition is animation:
        window._workspace_transition = None
        window._workspace_transition_effect = None
    _release_workspace_effect(effect)
    if isValid(animation):
        animation.deleteLater()


def select_workspace_tab(window, index: int) -> None:
    """Own explicit shell navigation without changing transport or session state."""

    if window._closing:
        return
    workspace = workspace_bindings_for(window)
    if workspace is None or index < 0 or index >= workspace.tabs.count():
        return
    workspace.tabs.setCurrentIndex(index)
    workspace.tabs.setFocus()


def open_connection_setup(window) -> None:
    """Open the connection form in presentation focus from the empty-state CTA."""

    if window._closing:
        return
    workspace = workspace_bindings_for(window)
    if workspace is None:
        return
    tabs = workspace.tabs
    if not 0 <= _CONNECTION_TAB_INDEX < tabs.count():
        return
    # Set the explicit CTA intent before the tab signal runs; otherwise a
    # wide window could briefly animate into overview and immediately back.
    window._connection_focus_override = True
    tabs.setCurrentIndex(_CONNECTION_TAB_INDEX)
    if not bool(getattr(window, "_workspace_focus_mode", False)):
        set_workspace_focus_mode(window, True)
    tabs.setFocus()


def on_workspace_tab_changed(window, index: int) -> None:
    """Keep page routing and adaptive configuration focus in one owner."""

    # The lifecycle owner keeps a visible motion snapshot; route changes are
    # the explicit boundary where a hidden page may become the active page.
    from .lifecycle import invalidate_motion_surface_snapshot

    invalidate_motion_surface_snapshot(window)
    workspace = workspace_bindings_for(window)
    if workspace is not None:
        workspace.route.set_index(index)
        workspace.context_label.set_index(index)
        tabs = workspace.tabs
        scroll_hint = workspace.scroll_hint
        page = (
            tabs.widget(index)
            if tabs is not None and 0 <= index < tabs.count()
            else None
        )
        scroll_hint.bind_page(page)

        # Configuration pages need their complete first viewport at the
        # supported compact window size. A new closed session also starts on
        # the connection route in the same focus mode so the first form is not
        # clipped; compact connection focus remains adaptive until the user
        # explicitly chooses the overview.
        onboarding_focus = (
            index == _CONNECTION_TAB_INDEX
            and bool(getattr(window, "_connection_onboarding", False))
            and getattr(window._view_model, "state", None) is SessionState.CLOSED
        )
        desired_focus = (
            index != _CONNECTION_TAB_INDEX
            or _connection_focus_desired(window, onboarding_focus)
        )
        focus_mode = bool(getattr(window, "_workspace_focus_mode", False))
        if focus_mode != desired_focus:
            set_workspace_focus_mode(window, desired_focus)
        elif focus_mode:
            refresh_workspace_focus_layout(window)

    protocol_visible = (
        not window._closing
        and not window.isHidden()
        and not window.isMinimized()
        and index == getattr(window, "_protocol_tab_index", -1)
    )
    if not protocol_visible:
        window._component_render_timer.stop()
    derived_visible = protocol_visible and derived_source_supported(window)
    window._dataset_curve.set_suspended(not protocol_visible)
    if protocol_visible:
        if window._component_rows_pending is not None:
            window._flush_component_rows()
        if derived_visible:
            window._dataset_curve.set_suspended(False)
            window._dataset_curve.flush()
            window._rerender_dataset_preview()
            window._rerender_component_preview()
            window._refresh_dataset_curve(immediate=True)
        else:
            window._dataset_curve.set_suspended(False)
            window._refresh_dataset_curve(immediate=True)
            window._dataset_curve.set_suspended(True)
            window._rerender_dataset_preview()
        window._update_pipeline_summary()

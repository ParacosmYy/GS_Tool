"""Presentation-only focus mode for the dense workspace shell.

Focus mode gives configuration pages the vertical space they need on the
minimum supported window.  It hides only the lower overview surfaces; session
workers, terminal ingestion, connection state, and data recording continue
through their existing owners.
"""

from __future__ import annotations

from dataclasses import dataclass

from shiboken6 import isValid

from .motion_policy import decorative_motion_enabled
from .motion_transition import MotionDrivenAnimationGroup
from .property_refresh import refresh_dynamic_property
from .qt import (
    QEasingCurve,
    QGraphicsOpacityEffect,
    QSizePolicy,
    Qt,
    QWidget,
)
from .terminal_bindings import terminal_bindings_for
from .transition_coordinator import (
    ShellTransitionKind,
    prepare_shell_transition,
    stop_shell_transitions,
)
from .workspace_bindings import workspace_bindings_for

_MAX_HEIGHT = 16_777_215
_TABS_MAX_HEIGHT = 350
_TRANSITION_MS = 220
_REVEAL_START_OPACITY = 0.82


@dataclass(frozen=True, slots=True)
class _FocusLayoutSnapshot:
    """Temporary root/layout state that focus mode must restore exactly."""

    shell_stretch: int
    terminal_stretch: int
    shell_vertical_policy: QSizePolicy.Policy
    tabs_vertical_policy: QSizePolicy.Policy
    shell_minimum_height: int
    shell_maximum_height: int
    tabs_minimum_height: int
    tabs_maximum_height: int
    tabs_stretch: int
    route_stretch: int
    terminal_hidden: bool
    terminal_mouse_transparent: bool
    terminal_vertical_policy: QSizePolicy.Policy
    terminal_minimum_height: int
    terminal_maximum_height: int
    terminal_child_hidden: tuple[tuple[QWidget, bool], ...]


def _root_stretch(layout, widget: QWidget) -> int:
    """Root stretch."""
    if layout is None or not isinstance(widget, QWidget):
        return 0
    index = layout.indexOf(widget)
    return max(0, int(layout.stretch(index))) if index >= 0 else 0


def _set_root_stretch(layout, widget: QWidget, stretch: int) -> None:
    """Set root stretch."""
    if layout is None or not isinstance(widget, QWidget):
        return
    index = layout.indexOf(widget)
    if index >= 0:
        layout.setStretch(index, max(0, int(stretch)))


def _workspace_route_strip(window) -> QWidget | None:
    """Workspace route strip."""
    bindings = workspace_bindings_for(window)
    route = bindings.route if bindings is not None else None
    parent = route.parentWidget() if isinstance(route, QWidget) else None
    return parent if isinstance(parent, QWidget) else None


def _layout_contains(layout, widget: QWidget | None) -> bool:
    """Layout contains."""
    return layout is not None and isinstance(widget, QWidget) and layout.indexOf(widget) >= 0


def _activate_root_layout(window) -> None:
    """Recalculate the shell after a temporary height constraint changes."""

    root = getattr(window, "_app_root", None)
    if not isinstance(root, QWidget):
        return
    layout = root.layout()
    if layout is not None:
        layout.activate()


def _sync_overview_shell_height(window, bindings) -> None:
    """Let the overview shell restore its responsive floor after focus mode."""

    shell = getattr(bindings, "shell", None)
    sync = getattr(shell, "sync_overview_minimum_height", None)
    if callable(sync):
        sync()


def _lower_surfaces(window) -> tuple[QWidget, ...]:
    """Lower surfaces."""
    bindings = terminal_bindings_for(window)
    if bindings is None:
        return ()
    return tuple(
        widget
        for widget in (
            bindings.live_observation_band,
            bindings.send_control_band,
        )
        if isinstance(widget, QWidget)
    )


def _terminal_spacer(window) -> QWidget | None:
    """Terminal spacer."""
    bindings = terminal_bindings_for(window)
    if bindings is None or not isinstance(bindings.terminal_surface, QWidget):
        return None
    return bindings.terminal_surface


def _terminal_spacer_children(window) -> tuple[QWidget, ...]:
    """Terminal spacer children."""
    bindings = terminal_bindings_for(window)
    if bindings is None:
        return ()
    return tuple(
        widget
        for widget in (bindings.terminal, bindings.terminal_empty_state)
        if isinstance(widget, QWidget)
    )


def _move_focus_out_of_terminal(window) -> None:
    """Move focus out of terminal."""
    bindings = workspace_bindings_for(window)
    spacer = _terminal_spacer(window)
    focus_widget = window.focusWidget()
    if (
        bindings is None
        or spacer is None
        or not isinstance(focus_widget, QWidget)
        or not (focus_widget is spacer or spacer.isAncestorOf(focus_widget))
    ):
        return
    target = bindings.focus_button
    if not target.isEnabled() or target.focusPolicy() == Qt.FocusPolicy.NoFocus:
        target = bindings.tabs
    target.setFocus(Qt.FocusReason.OtherFocusReason)


def _capture_focus_layout(window) -> _FocusLayoutSnapshot | None:
    """Capture focus layout."""
    bindings = workspace_bindings_for(window)
    spacer = _terminal_spacer(window)
    root = getattr(window, "_app_root", None)
    root_layout = root.layout() if isinstance(root, QWidget) else None
    if bindings is None or spacer is None or root_layout is None:
        return None
    shell_policy = bindings.shell.sizePolicy()
    tabs_policy = bindings.tabs.sizePolicy()
    spacer_policy = spacer.sizePolicy()
    shell_layout = bindings.shell.layout()
    route_strip = _workspace_route_strip(window)
    if not _layout_contains(root_layout, bindings.shell) or not _layout_contains(
        root_layout, spacer
    ) or not _layout_contains(shell_layout, bindings.tabs) or not _layout_contains(
        shell_layout, route_strip
    ):
        return None
    return _FocusLayoutSnapshot(
        shell_stretch=_root_stretch(root_layout, bindings.shell),
        terminal_stretch=_root_stretch(root_layout, spacer),
        shell_vertical_policy=shell_policy.verticalPolicy(),
        tabs_vertical_policy=tabs_policy.verticalPolicy(),
        shell_minimum_height=bindings.shell.minimumHeight(),
        shell_maximum_height=bindings.shell.maximumHeight(),
        tabs_minimum_height=bindings.tabs.minimumHeight(),
        tabs_maximum_height=bindings.tabs.maximumHeight(),
        tabs_stretch=_root_stretch(shell_layout, bindings.tabs),
        route_stretch=_root_stretch(shell_layout, route_strip),
        terminal_hidden=spacer.isHidden(),
        terminal_mouse_transparent=spacer.testAttribute(
            Qt.WidgetAttribute.WA_TransparentForMouseEvents
        ),
        terminal_vertical_policy=spacer_policy.verticalPolicy(),
        terminal_minimum_height=spacer.minimumHeight(),
        terminal_maximum_height=spacer.maximumHeight(),
        terminal_child_hidden=tuple(
            (widget, widget.isHidden()) for widget in _terminal_spacer_children(window)
        ),
    )


def _set_terminal_spacer(window, enabled: bool) -> bool:
    """Hide the terminal slot while focus owns the themed workspace height."""

    spacer = _terminal_spacer(window)
    if spacer is None:
        return False
    if enabled:
        snapshot = getattr(window, "_workspace_focus_layout_snapshot", None)
        if snapshot is None:
            snapshot = _capture_focus_layout(window)
            if snapshot is None:
                return False
            window._workspace_focus_layout_snapshot = snapshot
        elif not isinstance(snapshot, _FocusLayoutSnapshot):
            return False
        _move_focus_out_of_terminal(window)
        spacer_policy = spacer.sizePolicy()
        spacer_policy.setVerticalPolicy(QSizePolicy.Policy.Ignored)
        spacer.setSizePolicy(spacer_policy)
        spacer.setMinimumHeight(0)
        spacer.setMaximumHeight(0)
        spacer.setVisible(False)
        spacer.setAttribute(Qt.WidgetAttribute.WA_TransparentForMouseEvents, True)
        for widget in _terminal_spacer_children(window):
            widget.setVisible(False)
        return True

    snapshot = getattr(window, "_workspace_focus_layout_snapshot", None)
    if not isinstance(snapshot, _FocusLayoutSnapshot):
        return False
    bindings = workspace_bindings_for(window)
    root = getattr(window, "_app_root", None)
    root_layout = root.layout() if isinstance(root, QWidget) else None
    shell_layout = bindings.shell.layout() if bindings is not None else None
    if (
        bindings is None
        or not isValid(spacer)
        or not _layout_contains(root_layout, bindings.shell)
        or not _layout_contains(root_layout, spacer)
        or not _layout_contains(shell_layout, bindings.tabs)
        or not _layout_contains(shell_layout, _workspace_route_strip(window))
        or any(not isValid(widget) for widget, _hidden in snapshot.terminal_child_hidden)
    ):
        return False
    _set_root_stretch(root_layout, bindings.shell, snapshot.shell_stretch)
    _set_root_stretch(root_layout, spacer, snapshot.terminal_stretch)
    shell_policy = bindings.shell.sizePolicy()
    shell_policy.setVerticalPolicy(snapshot.shell_vertical_policy)
    bindings.shell.setSizePolicy(shell_policy)
    tabs_policy = bindings.tabs.sizePolicy()
    tabs_policy.setVerticalPolicy(snapshot.tabs_vertical_policy)
    bindings.tabs.setSizePolicy(tabs_policy)
    bindings.shell.setMinimumHeight(snapshot.shell_minimum_height)
    bindings.shell.setMaximumHeight(snapshot.shell_maximum_height)
    bindings.tabs.setMinimumHeight(snapshot.tabs_minimum_height)
    bindings.tabs.setMaximumHeight(snapshot.tabs_maximum_height)
    _set_root_stretch(shell_layout, bindings.tabs, snapshot.tabs_stretch)
    _set_root_stretch(shell_layout, _workspace_route_strip(window), snapshot.route_stretch)
    spacer_policy = spacer.sizePolicy()
    spacer_policy.setVerticalPolicy(snapshot.terminal_vertical_policy)
    spacer.setSizePolicy(spacer_policy)
    spacer.setMinimumHeight(snapshot.terminal_minimum_height)
    spacer.setMaximumHeight(snapshot.terminal_maximum_height)
    for widget, hidden in snapshot.terminal_child_hidden:
        widget.setVisible(not hidden)
    spacer.setAttribute(
        Qt.WidgetAttribute.WA_TransparentForMouseEvents,
        snapshot.terminal_mouse_transparent,
    )
    spacer.setVisible(not snapshot.terminal_hidden)
    _activate_root_layout(window)
    window._workspace_focus_layout_snapshot = None
    _sync_overview_shell_height(window, bindings)
    _activate_root_layout(window)
    return True


def _settle_focus_shell_height(window, tabs: QWidget) -> None:
    """Fill the themed focus shell while leaving page overflow to native scrolling."""

    bindings = workspace_bindings_for(window)
    if bindings is None:
        return
    shell = bindings.shell
    tabs.setMinimumHeight(0)
    tabs.setMaximumHeight(_MAX_HEIGHT)
    shell.setMinimumHeight(0)
    shell.setMaximumHeight(_MAX_HEIGHT)
    _activate_root_layout(window)
    root = getattr(window, "_app_root", None)
    root_layout = root.layout() if isinstance(root, QWidget) else None
    margins = root_layout.contentsMargins() if root_layout is not None else None
    available_shell_height = (
        max(
            0,
            root.height()
            - (margins.bottom() if margins is not None else 0)
            - shell.y()
        )
        if isinstance(root, QWidget)
        else 0
    )
    if available_shell_height <= 0:
        return
    # The shell receives the themed root slot. Its inner tabs stretch owns the
    # remaining height above the fixed route strip; each page keeps native
    # vertical scrolling inside its existing QScrollArea.
    tabs.setMinimumHeight(0)
    tabs.setMaximumHeight(_MAX_HEIGHT)
    shell.setMinimumHeight(available_shell_height)
    shell.setMaximumHeight(available_shell_height)


def _set_focus_button(window, enabled: bool) -> None:
    """Set focus button."""
    bindings = workspace_bindings_for(window)
    if bindings is None:
        return
    button = bindings.focus_button
    button.blockSignals(True)
    button.setChecked(enabled)
    button.setText("返回总览" if enabled else "专注设置")
    description = (
        "当前为专注设置模式；实时观测、终端和发送区仍在后台工作。"
        "点击返回总览。"
        if enabled
        else "展开配置工作区，临时收起实时观测、终端和发送区；不会断开连接或停止记录。"
    )
    button.setToolTip(description)
    button.setAccessibleDescription(description)
    button.blockSignals(False)


def _set_shell_mode(window, enabled: bool) -> None:
    """Set shell mode."""
    bindings = workspace_bindings_for(window)
    if bindings is not None:
        refresh_dynamic_property(bindings.shell, "mode", "focus" if enabled else "overview")
        bindings.context_label.set_mode(enabled)


def _clear_focus_transition_effects(window) -> None:
    """Release temporary reveal effects without touching unrelated effects."""

    effects = getattr(window, "_workspace_focus_transition_effects", ())
    window._workspace_focus_transition_effects = ()
    for target, effect in effects:
        if not isinstance(effect, QGraphicsOpacityEffect) or not isValid(effect):
            continue
        effect.setOpacity(1.0)
        if isinstance(target, QWidget) and target.graphicsEffect() is effect:
            target.setGraphicsEffect(None)


def _add_reveal_animation(
    group: MotionDrivenAnimationGroup,
    target: QWidget,
) -> tuple[QWidget, QGraphicsOpacityEffect] | None:
    """Fade one newly visible surface without creating a blank first frame."""

    return _add_opacity_animation(group, target, _REVEAL_START_OPACITY, 1.0)


def _add_opacity_animation(
    group: MotionDrivenAnimationGroup,
    target: QWidget,
    start: float,
    end: float,
) -> tuple[QWidget, QGraphicsOpacityEffect] | None:
    """Add a visual-only fade without changing the target's geometry."""

    if target.graphicsEffect() is not None:
        return None
    effect = QGraphicsOpacityEffect(target)
    effect.setOpacity(start)
    target.setGraphicsEffect(effect)
    group.add_property_animation(
        effect,
        b"opacity",
        start,
        end,
        QEasingCurve.Type.OutCubic,
    )
    return target, effect


def _apply_static_layout(window) -> None:
    """Apply the current mode without starting a timer or animation."""

    _clear_focus_transition_effects(window)
    enabled = bool(getattr(window, "_workspace_focus_mode", False))
    bindings = workspace_bindings_for(window)
    tabs = bindings.tabs if bindings is not None else None
    if enabled:
        if not _set_terminal_spacer(window, True):
            return
        root = getattr(window, "_app_root", None)
        root_layout = root.layout() if isinstance(root, QWidget) else None
        spacer = _terminal_spacer(window)
        if root_layout is not None and spacer is not None:
            _set_root_stretch(root_layout, bindings.shell, 1)
            _set_root_stretch(root_layout, spacer, 0)
        shell_policy = bindings.shell.sizePolicy()
        shell_policy.setVerticalPolicy(QSizePolicy.Policy.Expanding)
        bindings.shell.setSizePolicy(shell_policy)
        tabs_policy = bindings.tabs.sizePolicy()
        tabs_policy.setVerticalPolicy(QSizePolicy.Policy.Expanding)
        bindings.tabs.setSizePolicy(tabs_policy)
        shell_layout = bindings.shell.layout()
        _set_root_stretch(shell_layout, bindings.tabs, 1)
        _set_root_stretch(shell_layout, _workspace_route_strip(window), 0)
        for surface in _lower_surfaces(window):
            surface.setMinimumHeight(0)
            surface.setMaximumHeight(0)
            surface.setVisible(False)
        if isinstance(tabs, QWidget):
            tabs.setMinimumHeight(0)
            tabs.setMaximumHeight(_MAX_HEIGHT)
            _settle_focus_shell_height(window, tabs)
    else:
        snapshot = getattr(window, "_workspace_focus_layout_snapshot", None)
        had_snapshot = isinstance(snapshot, _FocusLayoutSnapshot)
        if had_snapshot and not _set_terminal_spacer(window, False):
            return
        if not had_snapshot:
            _set_terminal_spacer(window, False)
        for surface in _lower_surfaces(window):
            surface.setMinimumHeight(0)
            surface.setMaximumHeight(_MAX_HEIGHT)
            surface.setVisible(True)
        if isinstance(tabs, QWidget) and not had_snapshot:
            bindings.shell.setMinimumHeight(0)
            bindings.shell.setMaximumHeight(_MAX_HEIGHT)
            tabs.setMinimumHeight(0)
            tabs.setMaximumHeight(_TABS_MAX_HEIGHT)
    _set_shell_mode(window, enabled)
    _set_focus_button(window, enabled)


def _stop_group(window) -> None:
    """Stop group."""
    group = getattr(window, "_workspace_focus_transition", None)
    window._workspace_focus_transition = None
    if isinstance(group, MotionDrivenAnimationGroup) and isValid(group):
        group.stop()
        group.deleteLater()
    _clear_focus_transition_effects(window)


def stop_workspace_focus_transition(window) -> None:
    """Stop focus geometry motion and settle on the selected mode."""

    _stop_group(window)
    _apply_static_layout(window)


def _start_focus_transition(window) -> None:
    """Start focus transition."""
    enabled = bool(getattr(window, "_workspace_focus_mode", False))
    bindings = workspace_bindings_for(window)
    tabs = bindings.tabs if bindings is not None else None
    if not isinstance(tabs, QWidget):
        _apply_static_layout(window)
        return

    # Settle the shell geometry before creating any visual transition. Earlier
    # versions animated maximumHeight here, which made the QTabWidget viewport
    # squeeze its active page one shared frame at a time. The layout policy is
    # now immediate and deterministic; only opacity remains animated.
    _apply_static_layout(window)
    _activate_root_layout(window)
    group = MotionDrivenAnimationGroup(
        window._motion_controller,
        _TRANSITION_MS,
        window,
    )
    reveal_effects: list[tuple[QWidget, QGraphicsOpacityEffect]] = []
    if enabled:
        # The settings page is already at its final size. A restrained fade
        # communicates the mode change without hiding or clipping controls.
        reveal = _add_opacity_animation(group, tabs, 0.86, 1.0)
        if reveal is not None:
            reveal_effects.append(reveal)
    else:
        # Overview surfaces are laid out at their final positions first, then
        # reveal independently so no sibling is temporarily compressed.
        for surface in _lower_surfaces(window):
            reveal_effect = _add_reveal_animation(group, surface)
            if reveal_effect is not None:
                reveal_effects.append(reveal_effect)

    window._workspace_focus_transition_effects = tuple(reveal_effects)
    window._workspace_focus_transition = group
    group.finished.connect(lambda group=group: _finish_focus_transition(window, group))
    group.start()


def _finish_focus_transition(window, group: MotionDrivenAnimationGroup) -> None:
    """Finish focus transition."""
    if getattr(window, "_workspace_focus_transition", None) is not group:
        return
    window._workspace_focus_transition = None
    _apply_static_layout(window)
    if isValid(group):
        group.deleteLater()


def set_workspace_focus_mode(window, enabled: bool) -> None:
    """Toggle the configuration-focused shell without touching app state."""

    if getattr(window, "_closing", False):
        return
    window._workspace_focus_mode = bool(enabled)
    _set_focus_button(window, window._workspace_focus_mode)
    _set_shell_mode(window, window._workspace_focus_mode)
    _stop_group(window)
    if (
        not decorative_motion_enabled(window)
        or window.isHidden()
        or window.isMinimized()
    ):
        stop_shell_transitions(window)
        _apply_static_layout(window)
        return
    prepare_shell_transition(window, ShellTransitionKind.FOCUS)
    _start_focus_transition(window)


def refresh_workspace_focus_layout(window) -> None:
    """Recalculate the active focus page without starting another transition."""

    if (
        not bool(getattr(window, "_workspace_focus_mode", False))
        or getattr(window, "_closing", False)
    ):
        return
    _apply_static_layout(window)
    _activate_root_layout(window)


__all__ = [
    "refresh_workspace_focus_layout",
    "set_workspace_focus_mode",
    "stop_workspace_focus_transition",
]

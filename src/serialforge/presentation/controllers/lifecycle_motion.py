"""Shared presentation-motion catalog and frame fan-out.

This module owns only the shell's motion coordination boundary.  It does not
create a clock, inspect transport payloads, or own application state; the
existing ``MotionController`` remains the sole scheduler and these functions
only project its frame into visible presentation leaves.
"""

from __future__ import annotations

from ...domain.models import SessionState
from ..chrome_bindings import header_chrome_bindings_for
from ..command_bindings import command_batch_bindings_for
from ..connection_bindings import connection_shell_bindings_for
from ..property_refresh import refresh_dynamic_property
from ..qt import QEvent, QObject
from ..terminal_bindings import terminal_bindings_for
from ..workspace_bindings import workspace_bindings_for
from .workspace_runtime import workspace_motion_enabled

_MOTION_VISIBILITY_EVENTS = frozenset(
    {
        QEvent.Type.Show,
        QEvent.Type.Hide,
        QEvent.Type.ParentChange,
    }
)


class _MotionSurfaceVisibilityFilter(QObject):
    """Invalidate the cached visible motion snapshot at Qt visibility edges."""

    def __init__(self, window: QObject) -> None:
        super().__init__(window)
        self._window = window

    def eventFilter(self, _watched: object, event: object) -> bool:
        """Eventfilter."""
        event_type = event.type() if hasattr(event, "type") else None
        if event_type in _MOTION_VISIBILITY_EVENTS:
            invalidate_motion_surface_snapshot(self._window)
        return False


def _motion_surfaces(window) -> tuple[object, ...]:
    """Return the immutable decorative catalog for one window."""

    cached = getattr(window, "_motion_surface_catalog", None)
    if isinstance(cached, tuple):
        return cached

    workspace = workspace_bindings_for(window)
    terminal = terminal_bindings_for(window)
    batch = command_batch_bindings_for(window)
    shell = connection_shell_bindings_for(window)
    chrome = header_chrome_bindings_for(window)
    shell_surfaces = (workspace.route, workspace.tab_bar) if workspace is not None else ()
    connection_surfaces = (
        (
            shell.transport_mode_surface,
            shell.status_rail,
            shell.preset_context,
            shell.save_preset_button,
            shell.delete_preset_button,
            shell.connect_button,
        )
        if shell is not None
        else ()
    )
    chrome_surfaces = (
        (
            chrome.state_indicator,
            chrome.brand_mark,
            chrome.signal_field,
            chrome.theme_palette_swatch,
        )
        if chrome is not None
        else ()
    )
    terminal_surfaces = (
        (
            terminal.terminal,
            terminal.terminal_empty_state,
            terminal.record_button,
            terminal.data_activity_label,
            terminal.send_input,
            terminal.send_state_label,
            terminal.send_context_surface,
            terminal.clear_terminal_button,
            terminal.send_button,
            terminal.save_quick_button,
            terminal.clear_history_button,
        )
        if terminal is not None
        else ()
    )
    batch_surfaces = (
        (
            batch.empty_state,
            batch.status,
            batch.stop_button,
            batch.new_button,
            batch.edit_button,
            batch.delete_button,
            batch.run_button,
        )
        if batch is not None
        else ()
    )
    surface_names = (
        "_ble_read_button",
        "_extension_station_overview",
        "_pipeline_summary",
        "_component_preview",
        "_dataset_preview",
        "_protocol_status",
        "_protocol_apply_button",
        "_protocol_reset_button",
        "_protocol_config_context",
        "_component_status",
        "_component_empty",
        "_dataset_status",
        "_dataset_curve_status",
        "_dataset_curve",
        "_replay_status",
        "_replay_start_button",
        "_replay_pause_button",
        "_replay_stop_button",
        "_load_profile_button",
        "_export_component_button",
        "_load_dataset_button",
        "_export_dataset_button",
        "_refresh_button",
        "_ble_scan_button",
        "_status_footer_surface",
        "_error_signal",
    )
    catalog = tuple(
        widget
        for widget in (
            *shell_surfaces,
            *connection_surfaces,
            *chrome_surfaces,
            *terminal_surfaces,
            *batch_surfaces,
            *(getattr(window, name, None) for name in surface_names),
        )
        if widget is not None
    )
    window._motion_surface_catalog = catalog
    window._motion_surface_snapshot = None
    window._motion_surface_snapshot_dirty = True
    visibility_filter = _MotionSurfaceVisibilityFilter(window)
    window._motion_surface_visibility_filter = visibility_filter
    for widget in catalog:
        install_event_filter = getattr(widget, "installEventFilter", None)
        if callable(install_event_filter):
            install_event_filter(visibility_filter)
    return catalog


def invalidate_motion_surface_snapshot(window) -> None:
    """Mark the visible motion snapshot stale without touching animation state."""

    if not getattr(window, "_closing", False):
        window._motion_surface_snapshot_dirty = True


def _is_activity_motion_surface(widget: object) -> bool:
    """Identify surfaces that should animate only during explicit activity."""

    return getattr(widget, "MOTION_MODE", "ambient") == "activity"


def _motion_surface_wants_frame(widget: object) -> bool:
    """Keep static-state consumers out of the per-frame repaint fan-out."""

    predicate = getattr(widget, "motion_active", None)
    return bool(predicate()) if callable(predicate) else True


def _stop_motion_surface(widget: object, *, reset_hidden: bool = False) -> None:
    """Freeze one surface and optionally clear its hidden-stop edge marker."""

    stop = getattr(widget, "stop", None)
    if callable(stop):
        stop()
    if reset_hidden:
        widget._motion_hidden_stopped = False


def _stop_motion_surfaces(window) -> None:
    """Freeze every decorative consumer without changing application state."""

    for widget in _motion_surfaces(window):
        _stop_motion_surface(widget, reset_hidden=True)
    window._motion_activity_active = None


def _activity_motion_active(window) -> bool:
    """Read the non-ambient activity state from the shared presentation clock."""

    controller = getattr(window, "_motion_controller", None)
    activity_active = getattr(controller, "activity_active", None)
    return bool(activity_active()) if callable(activity_active) else False


def _is_visible_motion_surface(window, widget: object) -> bool:
    """Use the owning window as the visibility boundary for repaint fan-out."""

    is_visible_to = getattr(widget, "isVisibleTo", None)
    if not callable(is_visible_to):
        return True
    try:
        return bool(is_visible_to(window))
    except RuntimeError:
        return False


def _motion_surface_snapshot(window) -> tuple[tuple[int, object], ...]:
    """Return visible surfaces until a presentation visibility edge invalidates them."""

    catalog = _motion_surfaces(window)
    snapshot = getattr(window, "_motion_surface_snapshot", None)
    if (
        not getattr(window, "_motion_surface_snapshot_dirty", True)
        and isinstance(snapshot, tuple)
    ):
        return snapshot

    previous = snapshot if isinstance(snapshot, tuple) else ()
    visible = tuple(
        (index, widget)
        for index, widget in enumerate(catalog)
        if _is_visible_motion_surface(window, widget)
    )
    visible_ids = {id(widget) for _index, widget in visible}
    for _index, widget in previous:
        if id(widget) not in visible_ids and _is_activity_motion_surface(widget):
            _stop_motion_surface(widget)
            widget._motion_hidden_stopped = True
    window._motion_surface_snapshot = visible
    window._motion_surface_snapshot_dirty = False
    return visible


def on_motion_frame(window, phase: float, animated: bool) -> None:
    """Fan one shared frame to visible consumers with a small phase offset."""

    if (
        window._closing
        or not window.isVisible()
        or window.isMinimized()
        or not workspace_motion_enabled(window)
    ):
        _stop_motion_surfaces(window)
        set_data_activity_motion(window, False)
        return
    surfaces = _motion_surface_snapshot(window)
    activity_active = _activity_motion_active(window)
    previous_activity = getattr(window, "_motion_activity_active", None)
    activity_edge = previous_activity is None or previous_activity != activity_active
    window._motion_activity_active = activity_active
    for index, widget in surfaces:
        if _is_activity_motion_surface(widget):
            if not activity_active:
                if activity_edge:
                    _stop_motion_surface(widget, reset_hidden=True)
                continue
            widget._motion_hidden_stopped = False
        if animated and not _motion_surface_wants_frame(widget):
            continue
        set_frame = getattr(widget, "set_frame", None)
        if callable(set_frame):
            set_frame(phase + index * 0.37, animated)
    set_data_activity_motion(window, animated)


def set_data_activity_motion(window, animated: bool) -> None:
    """Expose only the short live-RX pulse as a low-frequency style state."""

    terminal = terminal_bindings_for(window)
    if terminal is None:
        return
    label = terminal.data_activity_label
    active = bool(
        animated
        and not window._closing
        and window._view_model.state is SessionState.OPEN
        and window._last_data_activity_bytes > 0
    )
    previous_active = getattr(window, "_motion_data_activity_active", None)
    if previous_active is active:
        return
    window._motion_data_activity_active = active
    refresh_dynamic_property(label, "state", "active" if active else "idle")
    set_activity = getattr(label, "set_activity", None)
    if callable(set_activity):
        set_activity(active)
    footer = getattr(window, "_status_footer_surface", None)
    if footer is not None:
        footer.set_activity(active)
    window._refresh_live_observation_state()


__all__ = [
    "invalidate_motion_surface_snapshot",
    "on_motion_frame",
    "set_data_activity_motion",
]

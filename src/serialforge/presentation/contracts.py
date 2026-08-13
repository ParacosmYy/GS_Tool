"""Small, typed contracts shared by presentation controllers."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass
from typing import Literal, Protocol

from .connection_presets import ConnectionPreset
from .qt import QLabel, QWidget

type StatusSurfaceSource = Callable[[], Literal["live", "history"]]
type DynamicPropertyRefresher = Callable[[QWidget, str, str | bool], None]
type ConnectionPresetApplyCallback = Callable[[ConnectionPreset], None]
type ConnectionPresetMutationCallback = Callable[[], None]
type ConnectionActionCallback = Callable[..., object]


class StatusSurfaceRegistrar(Protocol):
    """Narrow registration port exposed to panel builders."""

    def register(self, key: str, widget: QLabel, *, object_name: str, state: str) -> None: ...


@dataclass(frozen=True, slots=True)
class ProtocolPanelCallbacks:
    """Qt-facing actions supplied by the shell during protocol panel construction."""

    on_protocol_preset_changed: Callable[..., None]
    on_protocol_framing_changed: Callable[..., None]
    on_apply_protocol_config: Callable[[], None]
    on_reset_protocol: Callable[[], None]
    on_mark_protocol_editor_dirty: Callable[..., None]
    on_load_component_codec: Callable[[], None]
    on_component_filter_changed: Callable[..., None]
    on_export_component_csv: Callable[[], None]
    on_load_dataset_config: Callable[[], None]
    on_export_dataset_csv: Callable[[], None]
    on_curve_series_changed: Callable[..., None]
    on_start_replay: Callable[[], None]
    on_toggle_replay_pause: Callable[[], None]
    on_stop_replay: Callable[[], None]


__all__ = [
    "ConnectionActionCallback",
    "ConnectionPresetApplyCallback",
    "ConnectionPresetMutationCallback",
    "DynamicPropertyRefresher",
    "ProtocolPanelCallbacks",
    "StatusSurfaceRegistrar",
    "StatusSurfaceSource",
]

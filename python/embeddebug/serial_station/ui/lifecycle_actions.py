"""Window lifecycle actions for the Serial Station main window."""

from __future__ import annotations

from typing import Protocol


class LifecycleActionHost(Protocol):
    """Minimal main-window surface needed by lifecycle action handlers."""


def close_window(host: LifecycleActionHost) -> None:
    host._waveform_preview.shutdown()

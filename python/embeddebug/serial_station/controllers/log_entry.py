"""UI-neutral log entry model for the Serial Station controller layer."""

from __future__ import annotations

from dataclasses import dataclass


@dataclass(frozen=True)
class SerialWorkbenchLogEntry:
    """A UI-ready serial workbench log fact without QWidget dependencies."""

    direction: str
    text: str
    raw: bytes

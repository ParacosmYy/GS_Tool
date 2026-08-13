"""Framework-neutral contract for the shell lifecycle phase."""

from __future__ import annotations

from typing import Literal

StatusPhase = Literal["ready", "working", "attention", "error"]


__all__ = ["StatusPhase"]

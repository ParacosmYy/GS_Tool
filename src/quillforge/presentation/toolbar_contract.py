"""Toolbar action contracts shared by composition and Qt projection."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass
from typing import Literal

from .icon_contract import IconKey

ToolbarActionRole = Literal["standard", "primary", "quiet", "context"]


@dataclass(frozen=True, slots=True)
class ToolbarActionSpec:
    """One command-rail action and its presentation-only metadata."""

    text_key: str
    callback: Callable[[], None]
    icon_key: IconKey | None = None
    separator_before: bool = False
    role: ToolbarActionRole = "standard"


__all__ = ["ToolbarActionRole", "ToolbarActionSpec"]

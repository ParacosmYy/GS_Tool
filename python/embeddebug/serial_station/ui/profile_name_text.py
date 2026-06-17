"""Profile name text helpers for Serial Station widgets."""

from __future__ import annotations

from typing import Protocol


class ProfileNameEdit(Protocol):
    """Minimal text-edit surface needed by profile name helpers."""

    def setText(self, text: str) -> None: ...


def apply_profile_name_text(edit: ProfileNameEdit, name: str) -> None:
    edit.setText(name)

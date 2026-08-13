"""Qt-free presentation orchestration for recovery-writer outcomes."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass


@dataclass(frozen=True, slots=True)
class RecoveryProjectionPorts[OwnerT]:
    """Typed callbacks required by recovery-writer outcome projection."""

    is_live: Callable[[OwnerT], bool]
    is_dirty: Callable[[OwnerT], bool]
    current_snapshot_id: Callable[[OwnerT], str | None]
    current_content_version: Callable[[OwnerT], int]
    schedule_delete: Callable[[str], None]
    clear_snapshot: Callable[[OwnerT], None]
    notify_newer_edits: Callable[[], None]
    notify_failure: Callable[[OwnerT, Exception], None]


class RecoveryProjectionCoordinator[OwnerT]:
    """Project recovery results after the writer lifecycle has been released."""

    def __init__(self, ports: RecoveryProjectionPorts[OwnerT]) -> None:
        self._ports = ports

    def project_saved(
        self,
        owner: OwnerT,
        content_version: int,
        snapshot_id: str,
        discarded: bool,
    ) -> None:
        """Project one completed snapshot using the existing branch order."""
        if discarded:
            self._ports.schedule_delete(snapshot_id)
            return
        if not self._ports.is_live(owner):
            self._ports.schedule_delete(snapshot_id)
            return
        still_dirty = self._ports.is_dirty(owner)
        if self._ports.current_snapshot_id(owner) != snapshot_id:
            self._ports.schedule_delete(snapshot_id)
        elif not still_dirty:
            self._ports.clear_snapshot(owner)
        elif self._ports.current_content_version(owner) != content_version:
            self._ports.notify_newer_edits()

    def project_failed(
        self,
        owner: OwnerT,
        _snapshot_id: str,
        error: Exception,
        discarded: bool,
    ) -> None:
        """Project a non-discarded failure only when the owner is still live."""
        if discarded:
            return
        if self._ports.is_live(owner):
            self._ports.notify_failure(owner, error)


__all__ = ["RecoveryProjectionCoordinator", "RecoveryProjectionPorts"]

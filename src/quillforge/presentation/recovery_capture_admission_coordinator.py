"""Qt-free admission and snapshot candidate selection for recovery capture."""

from __future__ import annotations

from collections.abc import Callable, Iterable
from dataclasses import dataclass


@dataclass(frozen=True, slots=True)
class RecoveryCaptureAdmission[TabT, StateT]:
    """Immutable capture candidate handed to the existing MainWindow pipeline."""

    tab: TabT
    document_id: str
    snapshot_id: str
    state_snapshot: StateT
    content_version: int


@dataclass(frozen=True, slots=True)
class RecoveryCaptureAdmissionPorts[TabT, StateT]:
    """Named callbacks for selecting dirty tabs without owning capture or I/O."""

    recovery_available: Callable[[], bool]
    is_busy: Callable[[], bool]
    tabs: Callable[[], Iterable[TabT]]
    document_id: Callable[[TabT], str]
    document_in_flight: Callable[[str], bool]
    is_dirty: Callable[[TabT], bool]
    snapshot_id: Callable[[TabT], str | None]
    delete_in_flight_or_pending: Callable[[str], bool]
    new_snapshot_id: Callable[[], str]
    set_snapshot_id: Callable[[TabT, str], None]
    state_snapshot: Callable[[TabT], StateT]
    state_is_dirty: Callable[[StateT], bool]
    mark_dirty: Callable[[StateT, bool], StateT]
    content_version: Callable[[TabT], int]


class RecoveryCaptureAdmissionCoordinator[TabT, StateT]:
    """Select capture candidates while leaving lifecycle ownership to callers."""

    def __init__(self, ports: RecoveryCaptureAdmissionPorts[TabT, StateT]) -> None:
        self._ports = ports

    def admit(
        self,
        on_admitted: Callable[[RecoveryCaptureAdmission[TabT, StateT]], None],
    ) -> None:
        """Admit candidates in stable order and start each before the next tab."""
        ports = self._ports
        if not ports.recovery_available() or ports.is_busy():
            return
        for tab in ports.tabs():
            document_id = ports.document_id(tab)
            if ports.document_in_flight(document_id) or not ports.is_dirty(tab):
                continue
            existing_snapshot_id = ports.snapshot_id(tab)
            if existing_snapshot_id is not None and ports.delete_in_flight_or_pending(
                existing_snapshot_id
            ):
                continue
            snapshot_id = existing_snapshot_id or ports.new_snapshot_id()
            ports.set_snapshot_id(tab, snapshot_id)
            state_snapshot = ports.state_snapshot(tab)
            if not ports.state_is_dirty(state_snapshot):
                state_snapshot = ports.mark_dirty(state_snapshot, True)
            on_admitted(
                RecoveryCaptureAdmission(
                    tab=tab,
                    document_id=document_id,
                    snapshot_id=snapshot_id,
                    state_snapshot=state_snapshot,
                    content_version=ports.content_version(tab),
                )
            )


__all__ = [
    "RecoveryCaptureAdmission",
    "RecoveryCaptureAdmissionCoordinator",
    "RecoveryCaptureAdmissionPorts",
]

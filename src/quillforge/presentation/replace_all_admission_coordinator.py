"""Qt-free admission orchestration for one Replace All operation."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass
from typing import Literal

from .replace_all_tracker import ReplaceAllJob, ReplaceAllTracker

ReplaceAllAdmissionLevel = Literal["working", "warning"]


@dataclass(frozen=True, slots=True)
class ReplaceAllAdmissionPorts[TabT, SessionT]:
    """Named application and presentation callbacks for Replace All admission."""

    is_busy: Callable[[], bool]
    active_tab: Callable[[], TabT | None]
    query: Callable[[], str]
    replacement: Callable[[], str]
    case_sensitive: Callable[[], bool]
    begin_session: Callable[[TabT, str, str, bool, int], SessionT]
    max_matches: int
    content_version: Callable[[TabT], int]
    was_dirty: Callable[[TabT], bool]
    begin_operation: Callable[[str], int]
    complete_operation: Callable[[int], bool]
    start_job: Callable[[ReplaceAllJob[TabT, SessionT]], None]
    set_status: Callable[[str, ReplaceAllAdmissionLevel], None]


class ReplaceAllAdmissionCoordinator[TabT, SessionT]:
    """Admit one Replace All job without owning Qt or editor implementation."""

    def __init__(
        self,
        tracker: ReplaceAllTracker[TabT, SessionT],
        ports: ReplaceAllAdmissionPorts[TabT, SessionT],
    ) -> None:
        self._tracker = tracker
        self._ports = ports

    def request(self) -> ReplaceAllJob[TabT, SessionT] | None:
        """Validate admission, bind a job, and hand it to the caller's starter."""
        ports = self._ports
        if self._tracker.inflight:
            return None
        if ports.is_busy():
            ports.set_status("Another editor operation is in progress", "warning")
            return None
        tab = ports.active_tab()
        query = ports.query()
        if tab is None or not query:
            ports.set_status("Enter text to find", "warning")
            return None
        try:
            session = ports.begin_session(
                tab,
                query,
                ports.replacement(),
                ports.case_sensitive(),
                ports.max_matches,
            )
        except ValueError as error:
            ports.set_status(str(error), "warning")
            return None

        operation_id = ports.begin_operation("Preparing Replace All...")
        job = self._tracker.begin(
            tab=tab,
            session=session,
            operation_id=operation_id,
            expected_content_version=ports.content_version(tab),
            was_dirty=ports.was_dirty(tab),
        )
        if job is None:
            ports.complete_operation(operation_id)
            return None
        ports.start_job(job)
        return job


__all__ = ["ReplaceAllAdmissionCoordinator", "ReplaceAllAdmissionPorts"]

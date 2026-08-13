"""Qt-free presentation orchestration for workspace-search completions."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass
from typing import Protocol

from ..application.workspace_search import WorkspaceSearchResult
from .notification_contract import NotificationSink
from .workspace_search_operation_tracker import WorkspaceSearchOperationTracker

WorkspaceSearchOperation = Callable[[], object]
WorkspaceSearchSuccess = Callable[[object, int], None]
WorkspaceSearchFailure = Callable[[Exception, int], None]
WorkspaceSearchDispatcher = Callable[
    [WorkspaceSearchOperation, int, WorkspaceSearchSuccess, WorkspaceSearchFailure],
    None,
]


class WorkspaceSearchView(Protocol):
    """Minimal result surface needed by the search callback boundary."""

    def present_result(self, result: WorkspaceSearchResult) -> None:
        """Project a validated current result."""

    def present_error(self, message: str) -> None:
        """Project a recoverable search error."""

    def present_cancelled(self) -> None:
        """Project completion of an invalidated search."""


@dataclass(frozen=True, slots=True)
class WorkspaceSearchPorts:
    """Typed callbacks required to project one workspace-search result."""

    get_surface: Callable[[], WorkspaceSearchView | None]
    summarize: Callable[[WorkspaceSearchResult], str]
    notify: NotificationSink


class WorkspaceSearchCoordinator:
    """Classify search callbacks without owning query or containment policy."""

    def __init__(
        self,
        tracker: WorkspaceSearchOperationTracker,
        ports: WorkspaceSearchPorts,
    ) -> None:
        self._tracker = tracker
        self._ports = ports

    def submit(
        self,
        *,
        operation: WorkspaceSearchOperation,
        operation_id: int,
        generation: int,
        dispatch: WorkspaceSearchDispatcher,
    ) -> None:
        """Bind search generation before invoking a worker dispatcher."""

        def on_completed(result: object, current_operation_id: int) -> None:
            self.complete(result, current_operation_id, generation)

        def on_failed(error: Exception, current_operation_id: int) -> None:
            self.fail(error, current_operation_id, generation)

        dispatch(operation, operation_id, on_completed, on_failed)

    def complete(self, result: object, operation_id: int, generation: int) -> None:
        """Project one current, invalidated, stale, or invalid result."""
        completion = self._tracker.finish(operation_id, generation)
        if completion == "stale":
            return
        surface = self._ports.get_surface()
        if completion == "invalidated":
            if surface is not None:
                surface.present_cancelled()
            return
        if surface is None:
            return
        if not isinstance(result, WorkspaceSearchResult):
            message = "The search service returned an invalid result"
            surface.present_error(message)
            self._ports.notify(message, level="error")
            return
        surface.present_result(result)
        self._ports.notify(
            self._ports.summarize(result),
            level=(
                "warning"
                if result.cancelled
                or result.truncated
                or result.limit_reason != "none"
                or result.issues
                or not result.matches
                else "success"
            ),
        )

    def fail(self, error: Exception, operation_id: int, generation: int) -> None:
        """Project one current, invalidated, or stale worker failure."""
        completion = self._tracker.finish(operation_id, generation)
        if completion == "stale":
            return
        surface = self._ports.get_surface()
        if completion == "invalidated":
            if surface is not None:
                surface.present_cancelled()
            return
        if surface is None:
            return
        surface.present_error(str(error))
        self._ports.notify(f"Workspace search failed: {error}", level="error")


__all__ = ["WorkspaceSearchCoordinator", "WorkspaceSearchPorts", "WorkspaceSearchView"]

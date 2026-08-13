"""Qt-free presentation orchestration for valid asynchronous document saves."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass
from pathlib import Path

from ..domain.models import DocumentState


@dataclass(frozen=True, slots=True)
class DocumentSaveProjectionPorts[TabT]:
    """Typed callbacks required by valid document-save projection policy."""

    apply_saved_state: Callable[[TabT, DocumentState], None]
    refresh_language: Callable[[TabT, Path | None], None]
    update_title: Callable[[TabT], None]
    clear_recovery_snapshot: Callable[[TabT], None]
    publish_saved: Callable[[DocumentState], None]
    notify_saved: Callable[[Path | None], None]
    request_session_save: Callable[[], None]


class DocumentSaveProjectionCoordinator[TabT]:
    """Project one validated live save result in the existing observable order."""

    def __init__(
        self,
        ports: DocumentSaveProjectionPorts[TabT],
    ) -> None:
        self._ports = ports

    def project(
        self,
        tab: TabT,
        result: DocumentState,
        after: Callable[[], None] | None,
    ) -> None:
        """Project a result already validated by ``DocumentSaveCoordinator``."""
        self._ports.apply_saved_state(tab, result)
        self._ports.refresh_language(tab, result.path)
        self._ports.update_title(tab)
        self._ports.clear_recovery_snapshot(tab)
        self._ports.publish_saved(result)
        self._ports.notify_saved(result.path)
        self._ports.request_session_save()
        if after is not None:
            after()


__all__ = ["DocumentSaveProjectionCoordinator", "DocumentSaveProjectionPorts"]

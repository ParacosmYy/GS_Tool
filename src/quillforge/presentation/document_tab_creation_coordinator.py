"""Qt-free orchestration for assembling and projecting a document tab."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass


@dataclass(frozen=True, slots=True)
class DocumentTabCreationPorts[OpenedT, TabT, EditorT]:
    """Typed callbacks required by document-tab assembly policy."""

    create_editor: Callable[[OpenedT], EditorT]
    create_tab: Callable[[EditorT, OpenedT, str | None], TabT]
    add_tab: Callable[[TabT, str, bool], None]
    title: Callable[[TabT], str]
    is_modified: Callable[[TabT], bool]
    update_title: Callable[[TabT], None]
    request_session_save: Callable[[], None]
    sync_status: Callable[[], None]


class DocumentTabCreationCoordinator[OpenedT, TabT, EditorT]:
    """Assemble one tab while leaving document/editor policy to adapters."""

    def __init__(
        self,
        ports: DocumentTabCreationPorts[OpenedT, TabT, EditorT],
    ) -> None:
        self._ports = ports

    def add(self, opened: OpenedT, *, recovery_snapshot_id: str | None = None) -> TabT:
        """Create, project, and finalize one tab in the existing order."""
        editor = self._ports.create_editor(opened)
        tab = self._ports.create_tab(editor, opened, recovery_snapshot_id)
        self._ports.add_tab(tab, self._ports.title(tab), self._ports.is_modified(tab))
        self._ports.update_title(tab)
        self._ports.request_session_save()
        self._ports.sync_status()
        return tab


__all__ = ["DocumentTabCreationCoordinator", "DocumentTabCreationPorts"]

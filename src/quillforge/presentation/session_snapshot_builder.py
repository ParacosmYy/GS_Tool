"""Framework-neutral assembly for the bounded local session snapshot."""

from __future__ import annotations

from collections.abc import Callable, Iterable
from pathlib import Path

from ..domain.models import SessionDocument, SessionSnapshot


def build_session_snapshot[TabT](
    *,
    workspace_root: Path | None,
    tabs: Iterable[TabT],
    active_tab: TabT | None,
    path_of: Callable[[TabT], Path | None],
    dirty_of: Callable[[TabT], bool],
    modified_of: Callable[[TabT], bool],
    cursor_of: Callable[[TabT], tuple[int, int]],
) -> SessionSnapshot:
    """Assemble clean path-backed tab metadata without owning tab objects."""
    documents: list[SessionDocument] = []
    active_path: Path | None = None
    for tab in tabs:
        path = path_of(tab)
        if path is None or dirty_of(tab) or modified_of(tab):
            continue
        try:
            line, column = cursor_of(tab)
            document = SessionDocument(path, line, column)
        except (RuntimeError, ValueError):
            continue
        documents.append(document)
        if tab is active_tab:
            active_path = path

    active_index = 0
    if active_path is not None:
        for index, document in enumerate(documents):
            if document.path == active_path:
                active_index = index
                break
    return SessionSnapshot(
        workspace_root=workspace_root,
        documents=tuple(documents),
        active_index=active_index,
    )

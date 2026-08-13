"""Framework-neutral state for validating one active-document find match."""

from __future__ import annotations

from dataclasses import dataclass

SelectionBounds = tuple[int, int, int, int]


@dataclass(frozen=True, slots=True)
class FindMatch[TabT]:
    """Stable identity of the selection produced by one find request."""

    tab: TabT
    query: str
    case_sensitive: bool
    selection: SelectionBounds
    content_version: int


@dataclass(slots=True)
class FindMatchTracker[TabT]:
    """Own only the match snapshot and its explicit invalidation semantics."""

    _match: FindMatch[TabT] | None = None

    @property
    def match(self) -> FindMatch[TabT] | None:
        """Return the last valid match snapshot, if any."""
        return self._match

    def record(
        self,
        *,
        tab: TabT,
        query: str,
        case_sensitive: bool,
        selection: SelectionBounds | None,
        content_version: int,
    ) -> None:
        """Record a match or clear the snapshot when no selection was found."""
        self._match = (
            FindMatch(tab, query, case_sensitive, selection, content_version)
            if selection is not None
            else None
        )

    def matches(
        self,
        *,
        tab: TabT,
        query: str,
        case_sensitive: bool,
        selection: SelectionBounds | None,
        content_version: int,
    ) -> bool:
        """Return whether the editor still exposes the exact recorded match."""
        if selection is None or self._match is None:
            return False
        return self._match == FindMatch(
            tab,
            query,
            case_sensitive,
            selection,
            content_version,
        )

    def clear(self) -> None:
        """Invalidate a match after criteria, document, or content changes."""
        self._match = None

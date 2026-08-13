"""Application policy for bounded, metadata-only extension discovery."""

from collections import Counter
from dataclasses import dataclass, replace
from pathlib import Path

from ..plugins.catalog import PluginCatalogEntry, invalid_catalog_entry, parse_catalog_payload
from .plugin_execution import PluginExecutionGate
from .plugin_governance import PluginApprovalService
from .ports import PluginCatalogStore


@dataclass(frozen=True, slots=True)
class PluginCatalogSnapshot:
    """Immutable result projected to presentation or diagnostics."""

    root: Path
    entries: tuple[PluginCatalogEntry, ...]
    truncated: bool = False
    scan_error: str | None = None
    approval_error: str | None = None

    def counts(self) -> dict[str, int]:
        """Return deterministic status counts without exposing mutable entries."""
        counts = Counter(entry.status for entry in self.entries)
        return {
            "valid": counts.get("valid", 0),
            "incompatible": counts.get("incompatible", 0),
            "invalid": counts.get("invalid", 0),
            "duplicate": counts.get("duplicate", 0),
        }

    def summary(self) -> str:
        """Return a user-facing diagnostic that never executes external code."""
        counts = self.counts()
        approval_counts = Counter(entry.approval_state for entry in self.entries)
        suffix = " (catalog limit reached)" if self.truncated else ""
        if self.scan_error is not None:
            suffix = f" (scan issue: {self.scan_error})"
        approval_suffix = (
            f"; approvals {approval_counts.get('approved', 0)} approved, "
            f"{approval_counts.get('stale', 0)} stale, "
            f"{approval_counts.get('not-approved', 0)} not-approved"
        )
        if self.approval_error is not None:
            approval_suffix += f" (ledger issue: {self.approval_error})"
        execution_counts = Counter(entry.execution_state for entry in self.entries)
        execution_suffix = (
            f"; external execution {execution_counts.get('denied', 0)} denied, "
            f"{execution_counts.get('authorized', 0)} authorized"
        )
        return (
            "Extension catalog: "
            f"{counts['valid']} valid, "
            f"{counts['incompatible']} incompatible, "
            f"{counts['invalid']} invalid, "
            f"{counts['duplicate']} duplicate{suffix}; "
            f"external entries remain untrusted and unloaded{approval_suffix}"
            f"{execution_suffix}"
        )


class PluginCatalogService:
    """Validate catalog candidates while leaving execution to the policy gate."""

    def __init__(
        self,
        store: PluginCatalogStore,
        approvals: PluginApprovalService | None = None,
        execution_gate: PluginExecutionGate | None = None,
    ) -> None:
        self._store = store
        self._approvals = approvals
        self._execution_gate = execution_gate or PluginExecutionGate()

    def scan(self) -> PluginCatalogSnapshot:
        """Read and validate one bounded catalog snapshot."""
        result = self._store.read_candidates()
        entries = [
            (
                invalid_catalog_entry(candidate.source_path, candidate.error)
                if candidate.error is not None
                else parse_catalog_payload(candidate.source_path, candidate.payload)
            )
            for candidate in result.candidates
        ]
        entries = _mark_duplicate_ids(entries)
        approval_error = None
        if self._approvals is not None:
            decorated, approval_error = self._approvals.decorate(tuple(entries))
            entries = list(decorated)
        entries = [_project_execution_decision(entry, self._execution_gate) for entry in entries]
        return PluginCatalogSnapshot(
            root=result.root,
            entries=tuple(entries),
            truncated=result.truncated,
            scan_error=result.error,
            approval_error=approval_error,
        )


def _mark_duplicate_ids(entries: list[PluginCatalogEntry]) -> list[PluginCatalogEntry]:
    counts = Counter(entry.plugin_id for entry in entries if entry.plugin_id is not None)
    duplicate_ids = {plugin_id for plugin_id, count in counts.items() if count > 1}
    if not duplicate_ids:
        return entries
    return [
        replace(
            entry,
            status="duplicate",
            reason=f"Duplicate plugin ID: {entry.plugin_id}",
        )
        if entry.plugin_id in duplicate_ids
        else entry
        for entry in entries
    ]


def _project_execution_decision(
    entry: PluginCatalogEntry,
    execution_gate: PluginExecutionGate,
) -> PluginCatalogEntry:
    decision = execution_gate.evaluate_catalog_entry(entry)
    return replace(
        entry,
        execution_state="authorized" if decision.allowed else "denied",
        execution_reason=decision.reason,
        execution_requirements=decision.failed_requirements,
    )

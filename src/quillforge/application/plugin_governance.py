"""Digest-bound extension approval policy with no execution authority."""

from collections.abc import Iterable
from dataclasses import replace
from time import time_ns

from ..plugins.catalog import PluginCatalogEntry
from .errors import ApplicationStateError, ApplicationValidationError
from .ports import PluginApprovalReadResult, PluginApprovalRecord, PluginApprovalStore


class PluginApprovalService:
    """Persist explicit descriptor approvals while keeping entries unloaded."""

    def __init__(self, store: PluginApprovalStore) -> None:
        self._store = store

    def decorate(
        self,
        entries: tuple[PluginCatalogEntry, ...],
    ) -> tuple[tuple[PluginCatalogEntry, ...], str | None]:
        """Apply approved/stale/not-approved state without failing open."""
        result = self._store.load()
        if result.error is not None:
            return (
                tuple(replace(entry, approval_state="not-approved") for entry in entries),
                result.error,
            )
        records = {
            (record.plugin_id, record.descriptor_sha256): record for record in result.records
        }
        approved_ids = {record.plugin_id for record in result.records}
        decorated = tuple(_with_approval_state(entry, records, approved_ids) for entry in entries)
        return decorated, None

    def approve(self, entry: PluginCatalogEntry) -> PluginApprovalRecord:
        """Record one valid descriptor approval; never enables runtime loading."""
        _ensure_approvable(entry)
        result = self._load_for_mutation()
        records = {
            (record.plugin_id, record.descriptor_sha256): record for record in result.records
        }
        record = PluginApprovalRecord(
            plugin_id=entry.plugin_id,
            descriptor_sha256=entry.descriptor_sha256,
            approved_at_ns=time_ns(),
        )
        records[(record.plugin_id, record.descriptor_sha256)] = record
        self._store.save(_sorted_records(records.values()))
        return record

    def revoke(self, entry: PluginCatalogEntry) -> None:
        """Remove all approvals for one plugin ID, including stale historical digests."""
        if entry.plugin_id is None:
            raise ApplicationValidationError("Cannot revoke an entry without a plugin ID")
        result = self._load_for_mutation()
        records = tuple(record for record in result.records if record.plugin_id != entry.plugin_id)
        if len(records) != len(result.records):
            self._store.save(records)

    def _load_for_mutation(self) -> PluginApprovalReadResult:
        result = self._store.load()
        if result.error is not None:
            raise ApplicationStateError(
                f"Approval ledger is unavailable; refusing mutation: {result.error}"
            )
        return result


def _with_approval_state(
    entry: PluginCatalogEntry,
    records: dict[tuple[str, str], PluginApprovalRecord],
    approved_ids: set[str],
) -> PluginCatalogEntry:
    if entry.status != "valid" or entry.plugin_id is None or entry.descriptor_sha256 is None:
        return entry
    key = (entry.plugin_id, entry.descriptor_sha256)
    if key in records:
        state = "approved"
    elif entry.plugin_id in approved_ids:
        state = "stale"
    else:
        state = "not-approved"
    return replace(entry, approval_state=state)


def _ensure_approvable(entry: PluginCatalogEntry) -> None:
    if entry.status != "valid":
        raise ApplicationValidationError("Only valid compatible descriptors may be approved")
    if entry.plugin_id is None or entry.descriptor_sha256 is None:
        raise ApplicationValidationError("Descriptor identity and digest are required for approval")


def _sorted_records(
    records: Iterable[PluginApprovalRecord],
) -> tuple[PluginApprovalRecord, ...]:
    return tuple(
        sorted(
            records,
            key=lambda record: (record.plugin_id, record.descriptor_sha256),
        )
    )

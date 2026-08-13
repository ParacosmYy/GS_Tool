"""Fail-closed local enablement policy for registered in-process plugins."""

from dataclasses import dataclass

from .errors import ApplicationStateError
from .ports import PluginEnablementRecord, PluginEnablementStore


@dataclass(frozen=True, slots=True)
class PluginEnablementDecision:
    """Resolved desired state and an optional policy diagnostic."""

    enabled: bool
    error: str | None = None


class PluginEnablementPolicy:
    """Resolve and persist enablement without owning plugin lifecycle."""

    def __init__(self, store: PluginEnablementStore) -> None:
        self._store = store

    def resolve(self, plugin_id: str, *, default_enabled: bool) -> PluginEnablementDecision:
        """Resolve one ID; malformed policy disables activation instead of failing open."""
        result = self._store.load()
        if result.error is not None:
            return PluginEnablementDecision(False, result.error)
        records = {record.plugin_id: record.enabled for record in result.records}
        return PluginEnablementDecision(records.get(plugin_id, default_enabled))

    def persist(self, plugin_id: str, enabled: bool) -> None:
        """Persist one explicit preference, refusing to overwrite unreadable state."""
        result = self._store.load()
        if result.error is not None:
            raise ApplicationStateError(
                f"Plugin enablement policy is unavailable; refusing mutation: {result.error}"
            )
        records = {record.plugin_id: record.enabled for record in result.records}
        records[plugin_id] = enabled
        self._store.save(
            tuple(
                PluginEnablementRecord(plugin_id=record_id, enabled=state)
                for record_id, state in sorted(records.items())
            )
        )

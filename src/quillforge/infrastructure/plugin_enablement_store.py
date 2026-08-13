"""Atomic, bounded JSON storage for local plugin enablement preferences."""

import json
import os
import re
import tempfile
from pathlib import Path

from ..application.ports import (
    PluginEnablementReadResult,
    PluginEnablementRecord,
    PluginEnablementStore,
)
from ..plugins.api import PLUGIN_ID_PATTERN, PLUGIN_MANIFEST_MAX_FIELD_LENGTH

PLUGIN_ENABLEMENT_SCHEMA_VERSION = 1
DEFAULT_PLUGIN_ENABLEMENT_MAX_RECORDS = 256
DEFAULT_PLUGIN_ENABLEMENT_MAX_BYTES = 64 * 1024
_RECORD_KEYS = frozenset({"plugin_id", "enabled"})
_PLUGIN_ID_PATTERN = re.compile(PLUGIN_ID_PATTERN.pattern)


class JsonPluginEnablementStore(PluginEnablementStore):
    """Persist only bounded ID/boolean preferences; unreadable state fails closed."""

    def __init__(
        self,
        path: Path,
        *,
        max_records: int = DEFAULT_PLUGIN_ENABLEMENT_MAX_RECORDS,
        max_bytes: int = DEFAULT_PLUGIN_ENABLEMENT_MAX_BYTES,
    ) -> None:
        if max_records < 1:
            raise ValueError("Plugin enablement record limit must be positive")
        if max_bytes < 1:
            raise ValueError("Plugin enablement byte limit must be positive")
        self._path = path.expanduser().resolve()
        self._max_records = max_records
        self._max_bytes = max_bytes

    def load(self) -> PluginEnablementReadResult:
        """Read a valid ledger or return no overrides with a diagnostic."""
        if not self._path.exists():
            return PluginEnablementReadResult(())
        try:
            with self._path.open("rb") as source:
                raw_payload = source.read(self._max_bytes + 1)
            if len(raw_payload) > self._max_bytes:
                return PluginEnablementReadResult((), self._oversize_message())
            payload = json.loads(raw_payload)
            records = _decode_records(payload, max_records=self._max_records)
        except (OSError, UnicodeError, TypeError, ValueError, json.JSONDecodeError) as error:
            return PluginEnablementReadResult(
                (),
                f"Plugin enablement policy is invalid: {error}",
            )
        return PluginEnablementReadResult(records)

    def save(self, records: tuple[PluginEnablementRecord, ...]) -> None:
        """Atomically replace the complete bounded preference ledger."""
        validated = _validate_records(records, max_records=self._max_records)
        payload = {
            "schema_version": PLUGIN_ENABLEMENT_SCHEMA_VERSION,
            "plugins": [
                {"plugin_id": record.plugin_id, "enabled": record.enabled} for record in validated
            ],
        }
        self._path.parent.mkdir(parents=True, exist_ok=True)
        temporary_path: Path | None = None
        try:
            with tempfile.NamedTemporaryFile(
                mode="w",
                encoding="utf-8",
                dir=self._path.parent,
                prefix=f".{self._path.name}.",
                suffix=".tmp",
                delete=False,
            ) as temporary:
                temporary_path = Path(temporary.name)
                json.dump(payload, temporary, ensure_ascii=False, separators=(",", ":"))
                temporary.flush()
                os.fsync(temporary.fileno())
            if temporary_path.stat().st_size > self._max_bytes:
                raise ValueError(self._oversize_message())
            os.replace(temporary_path, self._path)
        finally:
            if temporary_path is not None and temporary_path.exists():
                try:
                    temporary_path.unlink()
                except OSError:
                    pass

    def _oversize_message(self) -> str:
        return f"Plugin enablement policy exceeds {self._max_bytes} bytes"


def default_plugin_enablement_path() -> Path:
    """Return the user-local policy path without creating it."""
    local_app_data = os.environ.get("LOCALAPPDATA")
    base = Path(local_app_data) if local_app_data else Path.home() / ".local" / "share"
    return base / "QuillForge" / "plugin-enablement.json"


def _decode_records(payload: object, *, max_records: int) -> tuple[PluginEnablementRecord, ...]:
    if not isinstance(payload, dict) or set(payload) != {"schema_version", "plugins"}:
        raise ValueError("Plugin enablement policy schema is invalid")
    if payload["schema_version"] != PLUGIN_ENABLEMENT_SCHEMA_VERSION:
        raise ValueError(f"Unsupported plugin enablement schema: {payload['schema_version']!r}")
    plugins = payload["plugins"]
    if not isinstance(plugins, list):
        raise ValueError("Plugin enablement plugins must be an array")
    records = []
    for item in plugins:
        if not isinstance(item, dict) or set(item) != _RECORD_KEYS:
            raise ValueError("Plugin enablement record schema is invalid")
        plugin_id = item["plugin_id"]
        enabled = item["enabled"]
        if (
            type(plugin_id) is not str
            or not plugin_id
            or len(plugin_id) > PLUGIN_MANIFEST_MAX_FIELD_LENGTH
            or _PLUGIN_ID_PATTERN.fullmatch(plugin_id) is None
        ):
            raise ValueError("Plugin enablement record plugin ID is invalid")
        if type(enabled) is not bool:
            raise ValueError("Plugin enablement record state is invalid")
        records.append(PluginEnablementRecord(plugin_id, enabled))
    return _validate_records(tuple(records), max_records=max_records)


def _validate_records(
    records: tuple[PluginEnablementRecord, ...],
    *,
    max_records: int,
) -> tuple[PluginEnablementRecord, ...]:
    if not isinstance(records, tuple):
        raise ValueError("Plugin enablement records must be a tuple")
    if len(records) > max_records:
        raise ValueError(f"Plugin enablement policy exceeds {max_records} records")
    ids: set[str] = set()
    for record in records:
        if not isinstance(record, PluginEnablementRecord):
            raise ValueError("Plugin enablement record type is invalid")
        if (
            type(record.plugin_id) is not str
            or not record.plugin_id
            or len(record.plugin_id) > PLUGIN_MANIFEST_MAX_FIELD_LENGTH
            or _PLUGIN_ID_PATTERN.fullmatch(record.plugin_id) is None
        ):
            raise ValueError("Plugin enablement record plugin ID is invalid")
        if type(record.enabled) is not bool:
            raise ValueError("Plugin enablement record state is invalid")
        if record.plugin_id in ids:
            raise ValueError("Plugin enablement records must not contain duplicates")
        ids.add(record.plugin_id)
    return tuple(sorted(records, key=lambda record: record.plugin_id))

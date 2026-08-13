"""Versioned metadata contract for the read-only extension catalog."""

import hashlib
import json
import re
from dataclasses import dataclass
from pathlib import Path
from typing import Literal

from .api import (
    PLUGIN_API_VERSION,
    PLUGIN_MANIFEST_MAX_FIELD_LENGTH,
    PluginManifest,
    PluginPermission,
    validate_plugin_manifest,
)

PLUGIN_CATALOG_SCHEMA_VERSION = 1
CatalogEntryStatus = Literal["valid", "invalid", "incompatible", "duplicate"]
CatalogTrustState = Literal["untrusted"]
CatalogApprovalState = Literal["approved", "stale", "not-approved"]
CatalogExecutionState = Literal["not-evaluated", "denied", "authorized"]

_ENTRYPOINT_PATTERN = re.compile(r"^[A-Za-z_][A-Za-z0-9_.]*:[A-Za-z_][A-Za-z0-9_]*$")
_MANIFEST_KEYS = frozenset(
    {
        "schema_version",
        "plugin_id",
        "name",
        "version",
        "api_version",
        "entrypoint",
        "permissions",
    }
)


@dataclass(frozen=True, slots=True)
class PluginCatalogManifest:
    """Validated metadata that is safe to display but not executable."""

    plugin_id: str
    name: str
    version: str
    api_version: str
    entrypoint: str
    permissions: tuple[PluginPermission, ...]


@dataclass(frozen=True, slots=True)
class PluginCatalogEntry:
    """One catalog result with explicit trust and execution boundaries."""

    source_path: Path
    manifest: PluginCatalogManifest | None
    status: CatalogEntryStatus
    descriptor_sha256: str | None = None
    trust_state: CatalogTrustState = "untrusted"
    approval_state: CatalogApprovalState = "not-approved"
    execution_state: CatalogExecutionState = "not-evaluated"
    execution_reason: str = "not-evaluated"
    execution_requirements: tuple[str, ...] = ()
    reason: str | None = None

    @property
    def plugin_id(self) -> str | None:
        """Return the validated ID, if the source was structurally parseable."""
        return self.manifest.plugin_id if self.manifest is not None else None

    @property
    def loadable(self) -> bool:
        """External catalog entries are metadata-only in this delivery."""
        return False


def parse_catalog_payload(source_path: Path, payload: object) -> PluginCatalogEntry:
    """Validate one JSON payload without resolving its entrypoint."""
    if not isinstance(payload, dict):
        return invalid_catalog_entry(source_path, "Manifest root must be a JSON object")
    if set(payload) != _MANIFEST_KEYS:
        missing = sorted(_MANIFEST_KEYS - set(payload))
        unknown = sorted(set(payload) - _MANIFEST_KEYS)
        details: list[str] = []
        if missing:
            details.append(f"missing keys: {', '.join(missing)}")
        if unknown:
            details.append(f"unknown keys: {', '.join(unknown)}")
        return invalid_catalog_entry(source_path, "; ".join(details))

    schema_version = payload["schema_version"]
    if type(schema_version) is not int or schema_version != PLUGIN_CATALOG_SCHEMA_VERSION:
        return invalid_catalog_entry(
            source_path,
            f"Unsupported catalog schema: {schema_version!r}",
        )

    string_values: dict[str, str] = {}
    for key in ("plugin_id", "name", "version", "api_version", "entrypoint"):
        value = payload[key]
        if (
            type(value) is not str
            or not value.strip()
            or len(value) > PLUGIN_MANIFEST_MAX_FIELD_LENGTH
        ):
            return invalid_catalog_entry(source_path, f"Invalid manifest field: {key}")
        string_values[key] = value

    entrypoint = string_values["entrypoint"]
    if _ENTRYPOINT_PATTERN.fullmatch(entrypoint) is None:
        return invalid_catalog_entry(source_path, "Invalid entrypoint metadata")

    permissions = payload["permissions"]
    if not isinstance(permissions, list):
        return invalid_catalog_entry(source_path, "Permissions must be a JSON array")

    runtime_manifest = PluginManifest(
        plugin_id=string_values["plugin_id"],
        name=string_values["name"],
        version=string_values["version"],
        api_version=string_values["api_version"],
        permissions=tuple(permissions),
    )
    try:
        validate_plugin_manifest(runtime_manifest, require_supported_api=False)
    except (TypeError, ValueError) as error:
        return invalid_catalog_entry(source_path, str(error))
    manifest = PluginCatalogManifest(
        plugin_id=runtime_manifest.plugin_id,
        name=runtime_manifest.name,
        version=runtime_manifest.version,
        api_version=runtime_manifest.api_version,
        entrypoint=entrypoint,
        permissions=runtime_manifest.permissions,
    )
    descriptor_sha256 = _descriptor_sha256(payload)
    if runtime_manifest.api_version != PLUGIN_API_VERSION:
        return PluginCatalogEntry(
            source_path=source_path,
            manifest=manifest,
            status="incompatible",
            descriptor_sha256=descriptor_sha256,
            reason=(
                f"Plugin API {runtime_manifest.api_version} is unsupported; "
                f"expected {PLUGIN_API_VERSION}"
            ),
        )
    return PluginCatalogEntry(
        source_path=source_path,
        manifest=manifest,
        status="valid",
        descriptor_sha256=descriptor_sha256,
    )


def _descriptor_sha256(payload: dict[object, object]) -> str:
    """Hash one validated JSON descriptor in a stable, encoding-independent form."""
    canonical = json.dumps(
        payload,
        ensure_ascii=False,
        sort_keys=True,
        separators=(",", ":"),
    ).encode("utf-8")
    return hashlib.sha256(canonical).hexdigest()


def invalid_catalog_entry(source_path: Path, reason: str) -> PluginCatalogEntry:
    """Create a stable diagnostic entry for one unreadable or malformed source."""
    return PluginCatalogEntry(
        source_path=source_path,
        manifest=None,
        status="invalid",
        reason=reason,
    )

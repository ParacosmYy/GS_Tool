"""Versioned, deliberately small plugin API surface."""

import re
from collections.abc import Callable
from dataclasses import dataclass
from typing import Literal, Protocol, TypeVar

from ..application.commands import Command
from ..domain.models import DocumentState

PLUGIN_API_VERSION = "1.0"
PLUGIN_MANIFEST_MAX_FIELD_LENGTH = 256
PluginPermission = Literal["commands", "events", "active_document", "notifications"]
PLUGIN_PERMISSIONS = frozenset({"commands", "events", "active_document", "notifications"})
PLUGIN_ID_PATTERN = re.compile(r"^[a-z0-9]+(?:[.-][a-z0-9]+)*$")
Event = TypeVar("Event")


@dataclass(frozen=True, slots=True)
class PluginManifest:
    """Identity and compatibility metadata supplied by every plugin."""

    plugin_id: str
    name: str
    version: str
    api_version: str = PLUGIN_API_VERSION
    permissions: tuple[PluginPermission, ...] = ()


def validate_plugin_manifest(
    manifest: PluginManifest,
    *,
    require_supported_api: bool = True,
) -> None:
    """Enforce the shared identity and capability policy at every host boundary."""
    if not isinstance(manifest, PluginManifest):
        raise TypeError("Plugin manifest must be a PluginManifest")
    if (
        type(manifest.plugin_id) is not str
        or not manifest.plugin_id
        or len(manifest.plugin_id) > PLUGIN_MANIFEST_MAX_FIELD_LENGTH
        or PLUGIN_ID_PATTERN.fullmatch(manifest.plugin_id) is None
    ):
        raise ValueError(f"Invalid plugin ID: {manifest.plugin_id!r}")
    for field_name in ("name", "version", "api_version"):
        value = getattr(manifest, field_name)
        if (
            type(value) is not str
            or not value.strip()
            or len(value) > PLUGIN_MANIFEST_MAX_FIELD_LENGTH
        ):
            raise ValueError(f"Invalid plugin manifest field: {field_name}")
    if require_supported_api and manifest.api_version != PLUGIN_API_VERSION:
        raise ValueError(
            f"Unsupported plugin API {manifest.api_version}; expected {PLUGIN_API_VERSION}"
        )
    if not isinstance(manifest.permissions, tuple):
        raise ValueError("Plugin permissions must be a tuple")
    if any(type(permission) is not str for permission in manifest.permissions):
        raise ValueError("Plugin permissions must contain strings")
    if len(set(manifest.permissions)) != len(manifest.permissions):
        raise ValueError("Plugin permissions must not contain duplicates")
    unknown_permissions = set(manifest.permissions) - PLUGIN_PERMISSIONS
    if unknown_permissions:
        raise ValueError(f"Unsupported plugin permissions: {sorted(unknown_permissions)}")


@dataclass(frozen=True, slots=True)
class ActiveDocumentSnapshot:
    """Read-only active-document capability exposed to plugin commands."""

    state: DocumentState
    text: str


class PluginContext(Protocol):
    """Capabilities a plugin can request from the host."""

    def register_command(self, command: Command) -> None:
        """Register a command without accessing private UI state."""

    def subscribe(self, event_type: type[Event], handler: Callable[[Event], None]) -> None:
        """Subscribe to a UI-thread application event owned by the host."""

    def get_active_document(self) -> ActiveDocumentSnapshot | None:
        """Read a snapshot of the active document, if one exists."""

    def notify(self, message: str) -> None:
        """Show a short user-facing message; callbacks must stay quick and UI-thread-bound."""


class Plugin(Protocol):
    """Lifecycle contract for a QuillForge plugin."""

    manifest: PluginManifest

    def activate(self, context: PluginContext) -> None:
        """Activate the plugin against a host context."""

    def deactivate(self) -> None:
        """Release plugin-owned resources."""

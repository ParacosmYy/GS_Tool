"""Explicit plugin lifecycle manager with ownership and failure isolation."""

from collections.abc import Callable
from dataclasses import dataclass, replace

from ..application.commands import Command, CommandRegistry
from ..application.events import EventBus, PluginFailed
from ..application.plugin_enablement import PluginEnablementPolicy
from ..application.plugin_runtime import PluginRuntimeStatus
from .api import (
    ActiveDocumentSnapshot,
    Plugin,
    PluginContext,
    PluginManifest,
    PluginPermission,
    validate_plugin_manifest,
)


@dataclass(frozen=True, slots=True)
class PluginStatus:
    """Observable lifecycle state for diagnostics and future settings UI."""

    manifest: PluginManifest
    trusted: bool
    enabled: bool
    active: bool
    permissions: tuple[PluginPermission, ...]
    error: str | None = None


class PluginManager:
    """Register trusted plugin instances explicitly and own their resources."""

    def __init__(
        self,
        commands: CommandRegistry,
        events: EventBus,
        *,
        enablement: PluginEnablementPolicy | None = None,
    ) -> None:
        self._commands = commands
        self._events = events
        self._plugins: dict[str, Plugin] = {}
        self._contexts: dict[str, _PluginContext] = {}
        self._statuses: dict[str, PluginStatus] = {}
        self._enablement = enablement
        self._deactivating: set[str] = set()
        self._failure_recorded: set[str] = set()
        self._active_document_provider: Callable[[], ActiveDocumentSnapshot | None] | None = None
        self._notifier: Callable[[str], None] | None = None

    def set_active_document_provider(
        self,
        provider: Callable[[], ActiveDocumentSnapshot | None],
    ) -> None:
        """Attach a read-only host capability without exposing presentation objects."""
        self._active_document_provider = provider

    def set_notifier(self, notifier: Callable[[str], None]) -> None:
        """Attach a host-owned notification sink."""
        self._notifier = notifier

    def register(self, plugin: Plugin, *, trusted: bool = True) -> None:
        """Register an already-created trusted plugin; no module discovery occurs here."""
        manifest = plugin.manifest
        validate_plugin_manifest(manifest)
        if manifest.plugin_id in self._plugins:
            raise ValueError(f"Duplicate plugin ID: {manifest.plugin_id}")
        decision = (
            self._enablement.resolve(manifest.plugin_id, default_enabled=trusted)
            if self._enablement is not None
            else None
        )
        enabled = trusted and (decision.enabled if decision is not None else True)
        error = (
            "Plugin is not trusted"
            if not trusted
            else decision.error
            if decision is not None
            else None
        )
        self._plugins[manifest.plugin_id] = plugin
        self._statuses[manifest.plugin_id] = PluginStatus(
            manifest=manifest,
            trusted=trusted,
            enabled=enabled,
            active=False,
            permissions=manifest.permissions,
            error=error,
        )

    def activate_all(self) -> None:
        """Activate all registered plugins while isolating individual failures."""
        for plugin_id in self._plugins:
            self.activate(plugin_id)

    def activate(self, plugin_id: str) -> None:
        """Activate one plugin once; failed activation does not stop the host."""
        if plugin_id in self._contexts:
            return
        status = self._require_status(plugin_id)
        if not status.trusted or not status.enabled:
            return
        plugin = self._plugins[plugin_id]
        self._failure_recorded.discard(plugin_id)
        context = _PluginContext(self, plugin_id)
        try:
            plugin.activate(context)
        except Exception as error:
            context.dispose()
            self._record_failure(plugin_id, "activate", error)
            return
        self._contexts[plugin_id] = context
        self._statuses[plugin_id] = replace(status, active=True, error=None)

    def _enable_registered(self, plugin_id: str) -> None:
        """Enable one registered plugin after policy validation has completed.

        Trust is assigned at registration or by a future, separately governed
        trust boundary. Runtime enablement must never be able to promote an
        an untrusted instance or bypass a failed enablement policy.
        """
        status = self._require_status(plugin_id)
        if not status.trusted:
            raise PermissionError("Only trusted plugins may be enabled")
        self._statuses[plugin_id] = replace(
            status,
            enabled=True,
            error=None,
        )
        self.activate(plugin_id)

    def disable(self, plugin_id: str) -> None:
        """Deactivate one plugin and revoke future activation enablement."""
        self._require_status(plugin_id)
        self.deactivate(plugin_id)
        status = self._statuses[plugin_id]
        self._statuses[plugin_id] = replace(
            status,
            enabled=False,
            active=False,
            error=status.error or "Plugin disabled by enablement policy",
        )

    def set_enabled(self, plugin_id: str, enabled: bool) -> None:
        """Apply one explicit in-process lifecycle toggle."""
        status = self._require_status(plugin_id)
        if enabled:
            if not status.trusted:
                raise PermissionError("Only trusted plugins may be enabled")
        if self._enablement is not None:
            self._enablement.persist(plugin_id, enabled)
        if enabled:
            self._enable_registered(plugin_id)
        else:
            self.disable(plugin_id)

    def runtime_statuses(self) -> tuple[PluginRuntimeStatus, ...]:
        """Project internal lifecycle state through the application contract."""
        return tuple(
            PluginRuntimeStatus(
                plugin_id=status.manifest.plugin_id,
                name=status.manifest.name,
                version=status.manifest.version,
                trusted=status.trusted,
                enabled=status.enabled,
                active=status.active,
                permissions=tuple(status.permissions),
                error=status.error,
            )
            for status in self._statuses.values()
        )

    def deactivate_all(self) -> None:
        """Deactivate in registration order and clean host-owned resources."""
        for plugin_id in tuple(self._contexts):
            self.deactivate(plugin_id)

    def deactivate(self, plugin_id: str) -> None:
        """Deactivate one plugin idempotently."""
        context = self._contexts.pop(plugin_id, None)
        if context is None:
            return
        plugin = self._plugins[plugin_id]
        self._deactivating.add(plugin_id)
        try:
            plugin.deactivate()
        except Exception as error:
            self._record_failure(plugin_id, "deactivate", error)
        finally:
            self._deactivating.discard(plugin_id)
            context.dispose()
            if self._statuses[plugin_id].error is None:
                status = self._statuses[plugin_id]
                self._statuses[plugin_id] = replace(status, active=False)

    def statuses(self) -> tuple[PluginStatus, ...]:
        """Return stable lifecycle snapshots for diagnostics."""
        return tuple(self._statuses.values())

    def _active_document(self) -> ActiveDocumentSnapshot | None:
        if self._active_document_provider is None:
            return None
        return self._active_document_provider()

    def _notify(self, message: str) -> None:
        if self._notifier is not None:
            self._notifier(message)

    def _record_failure(self, plugin_id: str, phase: str, error: Exception) -> None:
        if plugin_id in self._failure_recorded:
            return
        self._failure_recorded.add(plugin_id)
        message = str(error)
        plugin = self._plugins[plugin_id]
        context = self._contexts.pop(plugin_id, None)
        if context is not None:
            context.dispose()
        if phase != "deactivate" and plugin_id not in self._deactivating:
            try:
                plugin.deactivate()
            except Exception as cleanup_error:
                message = f"{message}; plugin cleanup failed: {cleanup_error}"
        status = self._statuses[plugin_id]
        self._statuses[plugin_id] = replace(status, active=False, error=message)
        try:
            self._events.publish(PluginFailed(plugin_id, phase, message))
        except Exception:
            # Failure reporting must not turn an isolated plugin error into a host crash.
            pass

    def _permission_allowed(self, plugin_id: str, permission: PluginPermission) -> bool:
        return permission in self._statuses[plugin_id].permissions

    def _require_status(self, plugin_id: str) -> PluginStatus:
        """Return a registered status before any lifecycle or policy mutation."""
        try:
            return self._statuses[plugin_id]
        except KeyError:
            raise KeyError(f"Unknown plugin ID: {plugin_id}") from None


class _PluginContext(PluginContext):
    """Internal capability object that tracks everything owned by one plugin."""

    def __init__(self, manager: PluginManager, plugin_id: str) -> None:
        self._manager = manager
        self._plugin_id = plugin_id
        self._command_ids: list[str] = []
        self._unsubscribers: list[Callable[[], None]] = []
        self._disposed = False

    def register_command(self, command: Command) -> None:
        self._ensure_active()
        self._ensure_permission("commands")
        guarded_command = Command(
            command_id=command.command_id,
            title=command.title,
            execute=lambda: self._execute_command(command.execute),
            shortcut=command.shortcut,
            menu_id=command.menu_id,
        )
        self._manager._commands.register(guarded_command)
        self._command_ids.append(command.command_id)

    def _execute_command(self, execute: Callable[[], None]) -> None:
        try:
            execute()
        except Exception as error:
            self._manager._record_failure(self._plugin_id, "command", error)

    def subscribe(self, event_type: type[object], handler: Callable[[object], None]) -> None:
        self._ensure_active()
        self._ensure_permission("events")

        def guarded(event: object) -> None:
            if self._disposed:
                return
            try:
                handler(event)
            except Exception as error:
                self._manager._record_failure(self._plugin_id, "event", error)

        self._unsubscribers.append(self._manager._events.subscribe(event_type, guarded))

    def get_active_document(self) -> ActiveDocumentSnapshot | None:
        self._ensure_active()
        self._ensure_permission("active_document")
        return self._manager._active_document()

    def notify(self, message: str) -> None:
        self._ensure_active()
        self._ensure_permission("notifications")
        self._manager._notify(message)

    def dispose(self) -> None:
        if self._disposed:
            return
        self._disposed = True
        for unsubscribe in self._unsubscribers:
            unsubscribe()
        self._unsubscribers.clear()
        for command_id in self._command_ids:
            self._manager._commands.unregister(command_id)
        self._command_ids.clear()

    def _ensure_active(self) -> None:
        if self._disposed:
            raise RuntimeError(f"Plugin context is disposed: {self._plugin_id}")

    def _ensure_permission(self, permission: PluginPermission) -> None:
        if not self._manager._permission_allowed(self._plugin_id, permission):
            raise PermissionError(f"Plugin permission denied: {permission}")

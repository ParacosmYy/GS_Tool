"""Qt-free presentation orchestration for the extension catalog surface."""

from __future__ import annotations

from dataclasses import dataclass
from typing import Protocol

from ..application.plugin_catalog import PluginCatalogService, PluginCatalogSnapshot
from ..application.plugin_governance import PluginApprovalService
from ..plugins.catalog import PluginCatalogEntry
from .notification_contract import NotificationSink
from .plugin_operation_tracker import PluginOperationTracker
from .task_contract import TaskSubmitter


class PluginCatalogView(Protocol):
    """Presentation projection used by catalog scan/governance orchestration."""

    def show_catalog(self, snapshot: PluginCatalogSnapshot) -> None:
        """Project one immutable catalog snapshot."""

    def set_catalog_governance_actions_enabled(self, enabled: bool) -> None:
        """Enable or disable the current catalog's governance actions."""


@dataclass(frozen=True, slots=True)
class PluginCatalogPorts:
    """Typed dependencies required by catalog scan and governance orchestration."""

    catalog: PluginCatalogService | None
    approval: PluginApprovalService | None
    operations: PluginOperationTracker
    runner: TaskSubmitter
    view: PluginCatalogView
    notify: NotificationSink


class PluginCatalogCoordinator:
    """Own catalog scan/governance sequencing without owning plugin policy."""

    def __init__(self, ports: PluginCatalogPorts) -> None:
        self._ports = ports

    def show_catalog(self) -> None:
        """Scan the explicit catalog asynchronously without loading extensions."""
        if self._ports.catalog is None:
            self._ports.notify("Extension catalog is unavailable", level="error")
            return
        if self._ports.operations.in_flight("catalog-scan"):
            self._ports.notify("Extension catalog scan already in progress", level="warning")
            return
        operation_id = self._ports.operations.begin("catalog-scan")
        self._ports.notify("Scanning extension catalog...", level="info")
        self._ports.runner.submit(
            self._ports.catalog.scan,
            operation_id,
            self._complete_catalog_scan,
            self._fail_catalog_scan,
        )

    def approve_descriptor(self, entry: object) -> None:
        """Record approval in a worker without changing executable trust."""
        self._mutate_descriptor(entry, approve=True)

    def revoke_descriptor(self, entry: object) -> None:
        """Revoke descriptor approval in a worker and rescan the catalog."""
        self._mutate_descriptor(entry, approve=False)

    def _complete_catalog_scan(self, result: object, operation_id: int) -> None:
        if not self._ports.operations.complete("catalog-scan", operation_id):
            return
        if not isinstance(result, PluginCatalogSnapshot):
            self._ports.notify("Extension catalog returned an invalid result", level="error")
            return
        self._ports.notify(
            result.summary(),
            level=(
                "warning"
                if result.truncated
                or result.scan_error is not None
                or result.approval_error is not None
                else "success"
            ),
        )
        self._ports.view.show_catalog(result)

    def _mutate_descriptor(self, entry: object, *, approve: bool) -> None:
        approval_service = self._ports.approval
        if approval_service is None:
            self._ports.notify("Extension approval governance is unavailable", level="error")
            return
        if not isinstance(entry, PluginCatalogEntry):
            self._ports.notify(
                "Extension catalog returned an invalid approval target", level="error"
            )
            return
        if self._ports.operations.in_flight(
            "catalog-governance"
        ) or self._ports.operations.in_flight("catalog-scan"):
            self._ports.notify("Extension catalog operation already in progress", level="warning")
            return
        operation_id = self._ports.operations.begin("catalog-governance")
        self._ports.view.set_catalog_governance_actions_enabled(False)
        action = "approval" if approve else "revocation"
        self._ports.notify(f"Recording descriptor {action}...", level="info")

        def operation() -> object:
            if approve:
                return approval_service.approve(entry)
            approval_service.revoke(entry)
            return None

        self._ports.runner.submit(
            operation,
            operation_id,
            self._complete_descriptor_mutation,
            self._fail_descriptor_mutation,
        )

    def _complete_descriptor_mutation(self, _result: object, operation_id: int) -> None:
        if not self._ports.operations.complete("catalog-governance", operation_id):
            return
        self._ports.notify(
            "Descriptor governance updated; external code remains unloaded",
            level="success",
        )
        self.show_catalog()

    def _fail_descriptor_mutation(self, error: Exception, operation_id: int) -> None:
        if not self._ports.operations.complete("catalog-governance", operation_id):
            return
        self._ports.view.set_catalog_governance_actions_enabled(True)
        self._ports.notify(f"Descriptor governance failed: {error}", level="error")

    def _fail_catalog_scan(self, error: Exception, operation_id: int) -> None:
        if not self._ports.operations.complete("catalog-scan", operation_id):
            return
        self._ports.notify(f"Extension catalog scan failed: {error}", level="error")


__all__ = ["PluginCatalogCoordinator", "PluginCatalogPorts", "PluginCatalogView"]

"""Desktop composition root for concrete adapters and runtime lifecycle."""

from collections.abc import Sequence
from dataclasses import dataclass
from pathlib import Path

from PyQt6.QtWidgets import QApplication

from . import __version__
from .application.commands import CommandRegistry
from .application.documents import DocumentService
from .application.editor_policy import DEFAULT_EDITOR_OPERATION_POLICY
from .application.events import EventBus
from .application.plugin_catalog import PluginCatalogService
from .application.plugin_enablement import PluginEnablementPolicy
from .application.plugin_execution import PluginExecutionGate
from .application.plugin_governance import PluginApprovalService
from .application.recovery import RecoveryService
from .application.session import SessionService
from .application.settings import DEFAULT_SETTINGS, SettingsService
from .application.workspace import WorkspaceService
from .application.workspace_search import WorkspaceSearchService
from .infrastructure.file_store import FileDocumentStore
from .infrastructure.plugin_approval_store import (
    JsonPluginApprovalStore,
    default_plugin_approval_path,
)
from .infrastructure.plugin_catalog_store import (
    JsonPluginCatalogStore,
    default_plugin_catalog_directory,
)
from .infrastructure.plugin_enablement_store import (
    JsonPluginEnablementStore,
    default_plugin_enablement_path,
)
from .infrastructure.plugin_host import (
    SubprocessPluginHost,
    default_plugin_host_command,
)
from .infrastructure.recovery_channel import BoundedRecoveryChunkChannel
from .infrastructure.recovery_store import (
    JsonRecoverySnapshotStore,
    default_recovery_directory,
)
from .infrastructure.session_store import JsonSessionStore, default_session_path
from .infrastructure.settings_store import JsonSettingsStore, default_settings_path
from .infrastructure.workspace_provider import FileWorkspaceProvider
from .infrastructure.workspace_search_provider import FileWorkspaceSearchProvider
from .plugins.builtin import DocumentStatsPlugin
from .plugins.manager import PluginManager
from .presentation.main_window import MainWindow
from .presentation.theme import apply_theme


@dataclass(slots=True)
class DesktopRuntime:
    """Own the assembled desktop window and its plugin lifecycle."""

    window: MainWindow
    plugins: PluginManager
    startup_paths: tuple[Path, ...] = ()
    safe_mode: bool = False

    def prepare_startup(self) -> None:
        """Bind host capabilities and activate built-in plugins."""
        self.plugins.set_active_document_provider(self.window.active_document_snapshot)
        self.plugins.set_notifier(self.window.notify)
        self.plugins.activate_all()

    def refresh_startup_commands(self) -> None:
        """Refresh command projections after startup state has been requested."""
        self.window.refresh_command_menus()

    def preflight_editor_shell(self) -> None:
        """Construct the editor surface without restoring or showing startup UI."""
        self.window.preflight_editor_shell()

    def preflight_startup_restore(self) -> dict[str, object]:
        """Run startup restoration without showing the window or entering exec."""
        return self.window.preflight_startup_restore()

    def preflight_startup_paths(self, paths: Sequence[Path]) -> dict[str, object]:
        """Exercise normal restore and queued-path order without showing the window."""
        self.window.restore_startup_state()
        self.refresh_startup_commands()
        return self.window.preflight_open_startup_paths(paths)

    def start(self) -> None:
        """Activate built-ins and project startup state in the existing order."""
        if self.safe_mode:
            self.window.ensure_initial_document()
        else:
            self.prepare_startup()
            self.window.restore_startup_state()
        self.refresh_startup_commands()
        self.window.show()
        self.window.open_startup_paths(self.startup_paths)

    def stop(self) -> None:
        """Stop periodic UI activity before deactivating plugins."""
        self.window.stop_background_activity()
        self.plugins.deactivate_all()


def build_desktop_runtime(
    application: QApplication,
    *,
    startup_paths: Sequence[Path] = (),
    safe_mode: bool = False,
) -> DesktopRuntime:
    """Assemble concrete adapters and the presentation runtime in one place."""
    application.setApplicationName("QuillForge")
    application.setOrganizationName("QuillForge")
    application.setApplicationVersion(__version__)

    settings = SettingsService(JsonSettingsStore(default_settings_path()))
    initial_settings = DEFAULT_SETTINGS if safe_mode else settings.load()
    apply_theme(application, initial_settings.appearance)

    commands = CommandRegistry()
    events = EventBus()
    document_store = FileDocumentStore(keep_backups=True)
    documents = DocumentService(document_store)
    recovery_store = JsonRecoverySnapshotStore(default_recovery_directory())
    recovery = RecoveryService(recovery_store, document_store, chunk_store=recovery_store)
    session = SessionService(JsonSessionStore(default_session_path()))
    workspace = WorkspaceService(FileWorkspaceProvider())
    workspace_search = WorkspaceSearchService(FileWorkspaceSearchProvider())
    plugin_approvals = PluginApprovalService(
        JsonPluginApprovalStore(default_plugin_approval_path())
    )
    plugin_execution_gate = PluginExecutionGate()
    plugin_catalog = PluginCatalogService(
        JsonPluginCatalogStore(default_plugin_catalog_directory()),
        approvals=plugin_approvals,
        execution_gate=plugin_execution_gate,
    )
    plugin_enablement = PluginEnablementPolicy(
        JsonPluginEnablementStore(default_plugin_enablement_path())
    )
    plugin_host = SubprocessPluginHost(default_plugin_host_command())
    plugins = PluginManager(commands, events, enablement=plugin_enablement)
    plugins.register(DocumentStatsPlugin())

    window = MainWindow(
        documents,
        commands,
        events,
        recovery,
        session=session,
        workspace=workspace,
        workspace_search=workspace_search,
        settings=settings,
        initial_settings=initial_settings,
        plugin_catalog=plugin_catalog,
        plugin_approval=plugin_approvals,
        plugin_runtime=plugins,
        plugin_host=plugin_host,
        editor_policy=DEFAULT_EDITOR_OPERATION_POLICY,
        recovery_channel_factory=BoundedRecoveryChunkChannel,
    )
    return DesktopRuntime(
        window=window,
        plugins=plugins,
        startup_paths=tuple(startup_paths),
        safe_mode=safe_mode,
    )

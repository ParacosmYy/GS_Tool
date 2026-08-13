"""Qt shell that projects application commands and document use cases."""

import time
from collections.abc import Callable, Sequence
from dataclasses import dataclass
from functools import partial
from pathlib import Path
from stat import S_ISDIR, S_ISREG

from PyQt6.QtCore import QEventLoop, QTimer
from PyQt6.QtGui import QCloseEvent
from PyQt6.QtWidgets import (
    QApplication,
    QMainWindow,
)

from ..application.commands import CommandRegistry
from ..application.documents import DocumentService, OpenedDocument
from ..application.editor_policy import (
    DEFAULT_EDITOR_OPERATION_POLICY,
    EditorOperationPolicy,
)
from ..application.events import (
    DocumentClosed,
    DocumentOpened,
    DocumentSaved,
    EventBus,
    PluginFailed,
)
from ..application.plugin_catalog import PluginCatalogService
from ..application.plugin_governance import PluginApprovalService
from ..application.plugin_host import PluginHostClient
from ..application.plugin_runtime import PluginRuntime
from ..application.ports import (
    EditorEngine,
    RecoveryCaptureCancelled,
    RecoveryChunkChannel,
)
from ..application.recovery import RecoveryCandidate, RecoveryService
from ..application.session import DEFAULT_SESSION, SessionService
from ..application.settings import DEFAULT_SETTINGS, SettingsService
from ..application.workspace import WorkspaceService
from ..application.workspace_search import (
    WorkspaceSearchQuery,
    WorkspaceSearchResult,
    WorkspaceSearchService,
)
from ..domain.models import (
    DocumentState,
    Locale,
    RecoverySnapshot,
    SessionSnapshot,
    SettingsSnapshot,
)
from ..plugins.api import ActiveDocumentSnapshot
from .close_guard_coordinator import (
    CloseGuardCoordinator,
    CloseGuardDecision,
    CloseGuardPorts,
)
from .close_guard_feedback_coordinator import (
    CloseGuardFeedbackCoordinator,
    CloseGuardFeedbackPorts,
)
from .command_palette_surface import CommandPaletteSurface
from .command_surface import CommandSurface
from .core_command_coordinator import CoreCommandCoordinator, CoreCommandPorts
from .core_toolbar_coordinator import CoreToolbarCoordinator, CoreToolbarPorts
from .current_document_transition_coordinator import (
    CurrentDocumentTransitionCoordinator,
    CurrentDocumentTransitionPorts,
)
from .document_change_projection_coordinator import (
    DocumentChangeProjectionCoordinator,
    DocumentChangeProjectionPorts,
)
from .document_creation_admission_coordinator import (
    DocumentCreationAdmissionCoordinator,
    DocumentCreationAdmissionPorts,
)
from .document_open_admission_coordinator import (
    DocumentOpenAdmissionCoordinator,
    DocumentOpenAdmissionPorts,
)
from .document_open_coordinator import DocumentOpenCoordinator, DocumentOpenPorts
from .document_open_projection_coordinator import (
    DocumentOpenProjectionCoordinator,
    DocumentOpenProjectionPorts,
)
from .document_picker_admission_coordinator import (
    DocumentPickerAdmissionCoordinator,
    DocumentPickerAdmissionPorts,
)
from .document_save_admission_coordinator import (
    DocumentSaveAdmissionCoordinator,
    DocumentSaveAdmissionPorts,
)
from .document_save_coordinator import DocumentSaveCoordinator, DocumentSavePorts
from .document_save_picker_admission_coordinator import (
    DocumentSavePickerAdmissionCoordinator,
    DocumentSavePickerAdmissionPorts,
)
from .document_save_projection_coordinator import (
    DocumentSaveProjectionCoordinator,
    DocumentSaveProjectionPorts,
)
from .document_tab_creation_coordinator import (
    DocumentTabCreationCoordinator,
    DocumentTabCreationPorts,
)
from .document_tab_removal_coordinator import (
    DocumentTabRemovalCoordinator,
    DocumentTabRemovalPorts,
)
from .editor_action_admission_coordinator import (
    EditorActionAdmissionCoordinator,
    EditorActionAdmissionPorts,
)
from .editor_document_surface import EditorDocumentCallbacks, EditorDocumentSurface
from .editor_shell_surface import EditorShellSurface
from .editor_widget import (
    EditorWidget,
    ReplaceAllProgress,
    ReplaceAllSession,
    TextCaptureSession,
)
from .file_dialog_surface import FileDialogSurface
from .find_match_tracker import FindMatchTracker
from .find_surface import FindSurfaceCallbacks
from .i18n import localize_message, tr, workspace_search_summary
from .message_surface import MessageSurface
from .notification_contract import StatusMessageLevel
from .operation_tracker import OperationTracker
from .plugin_catalog_coordinator import PluginCatalogCoordinator, PluginCatalogPorts
from .plugin_host_coordinator import PluginHostProbeCoordinator, PluginHostProbePorts
from .plugin_operation_tracker import PluginOperationTracker
from .plugin_runtime_coordinator import PluginRuntimeCoordinator, PluginRuntimePorts
from .plugin_surface import PluginSurface, PluginSurfaceCallbacks
from .presentation_locale_coordinator import (
    PresentationLocaleCoordinator,
    PresentationLocalePorts,
)
from .recovery_capture_abort_coordinator import (
    RecoveryCaptureAbortCoordinator,
    RecoveryCaptureAbortPorts,
)
from .recovery_capture_admission_coordinator import (
    RecoveryCaptureAdmission,
    RecoveryCaptureAdmissionCoordinator,
    RecoveryCaptureAdmissionPorts,
)
from .recovery_capture_tracker import RecoveryCaptureTracker
from .recovery_delete_coordinator import RecoveryDeleteCoordinator, RecoveryDeletePorts
from .recovery_projection_coordinator import (
    RecoveryProjectionCoordinator,
    RecoveryProjectionPorts,
)
from .recovery_prompt_surface import RecoveryPromptSurface
from .recovery_scan_coordinator import RecoveryScanCoordinator, RecoveryScanPorts
from .recovery_scan_tracker import RecoveryScanTracker
from .recovery_write_coordinator import RecoveryWriteCoordinator, RecoveryWritePorts
from .replace_all_admission_coordinator import (
    ReplaceAllAdmissionCoordinator,
    ReplaceAllAdmissionPorts,
)
from .replace_all_completion_coordinator import (
    ReplaceAllCompletionCoordinator,
    ReplaceAllCompletionPorts,
)
from .replace_all_tracker import ReplaceAllJob, ReplaceAllTracker
from .session_load_coordinator import SessionLoadCoordinator, SessionLoadPorts
from .session_restore_coordinator import SessionRestoreCoordinator, SessionRestorePorts
from .session_restore_tracker import SessionRestoreTracker
from .session_save_coordinator import SessionSaveCoordinator, SessionSavePorts
from .session_save_tracker import SessionSaveTracker
from .session_snapshot_builder import build_session_snapshot
from .settings_save_admission_coordinator import (
    SettingsSaveAdmissionCoordinator,
    SettingsSaveAdmissionPorts,
)
from .settings_save_coordinator import SettingsSaveCoordinator, SettingsSavePorts
from .settings_save_projection_coordinator import (
    SettingsSaveProjectionCoordinator,
    SettingsSaveProjectionPorts,
)
from .settings_save_tracker import SettingsSaveTracker
from .settings_surface import SettingsSurface
from .status_phase_coordinator import StatusPhaseCoordinator, StatusPhaseInput
from .status_surface import StatusSurface
from .task_runner import TaskRunner
from .theme import apply_theme
from .theme_transition_surface import ThemeTransitionSurface
from .workspace_file_activation_coordinator import (
    WorkspaceFileActivationCoordinator,
    WorkspaceFileActivationPorts,
)
from .workspace_navigation_admission_coordinator import (
    WorkspaceNavigationAdmissionCoordinator,
    WorkspaceNavigationAdmissionPorts,
)
from .workspace_navigation_coordinator import (
    WorkspaceNavigationCoordinator,
    WorkspaceNavigationPorts,
)
from .workspace_navigation_projection_coordinator import (
    WorkspaceNavigationProjectionCoordinator,
    WorkspaceNavigationProjectionPorts,
)
from .workspace_operation_tracker import WorkspaceOperationTracker
from .workspace_picker_admission_coordinator import (
    WorkspacePickerAdmissionCoordinator,
    WorkspacePickerAdmissionPorts,
)
from .workspace_search_coordinator import WorkspaceSearchCoordinator, WorkspaceSearchPorts
from .workspace_search_operation_tracker import WorkspaceSearchOperationTracker
from .workspace_search_surface import (
    WorkspaceSearchSurface,
    WorkspaceSearchSurfaceCallbacks,
)
from .workspace_search_surface_admission_coordinator import (
    WorkspaceSearchSurfaceAdmissionCoordinator,
    WorkspaceSearchSurfaceAdmissionPorts,
)
from .workspace_surface import WorkspaceSurface, WorkspaceSurfaceCallbacks


@dataclass(slots=True)
class _DocumentTab:
    editor: EditorWidget
    state: DocumentState
    content_version: int = 0
    recovery_snapshot_id: str | None = None


@dataclass(slots=True)
class _RecoveryCaptureJob:
    """Presentation state for one cooperative editor-to-recovery capture."""

    tab: _DocumentTab
    session: TextCaptureSession
    snapshot_id: str
    state_snapshot: DocumentState
    content_version: int
    channel: RecoveryChunkChannel | None = None


class MainWindow(QMainWindow):
    """Multi-document shell with explicit application and plugin seams."""

    def __init__(
        self,
        documents: DocumentService,
        commands: CommandRegistry,
        events: EventBus,
        recovery: RecoveryService | None = None,
        recovery_interval_ms: int = 30_000,
        workspace: WorkspaceService | None = None,
        workspace_search: WorkspaceSearchService | None = None,
        settings: SettingsService | None = None,
        initial_settings: SettingsSnapshot | None = None,
        plugin_catalog: PluginCatalogService | None = None,
        plugin_approval: PluginApprovalService | None = None,
        plugin_runtime: PluginRuntime | None = None,
        plugin_host: PluginHostClient | None = None,
        editor_policy: EditorOperationPolicy | None = None,
        recovery_channel_factory: Callable[[int, int], RecoveryChunkChannel] | None = None,
        session: SessionService | None = None,
    ) -> None:
        super().__init__()
        self.setObjectName("mainWindow")
        self.setWindowTitle("QuillForge")
        self.setMinimumSize(980, 640)
        self.resize(1440, 900)

        self._documents = documents
        self._commands = commands
        self._events = events
        self._recovery = recovery
        self._session_service = session
        self._startup_restore_inflight = False
        self._busy = False
        self._pending_startup_paths: list[Path] = []
        self._session_restore = SessionRestoreTracker[_DocumentTab]()
        self._session_save = SessionSaveTracker()
        self._operation_tracker = OperationTracker()
        self._session_load_coordinator = SessionLoadCoordinator(
            SessionLoadPorts(
                set_last_saved=self._session_save.set_last_saved,
                set_snapshot=self._session_restore.set_snapshot,
                schedule_recovery_scan=lambda: self._schedule_recovery_scan(startup=True),
                notify=self.notify,
            )
        )
        self._session_save_timer = QTimer(self)
        self._session_save_timer.setSingleShot(True)
        self._session_save_timer.setInterval(250)
        self._workspace = workspace
        self._workspace_search = workspace_search
        self._workspace_search_surface: WorkspaceSearchSurface | None = None
        self._workspace_search_operations = WorkspaceSearchOperationTracker()
        self._workspace_search_coordinator = WorkspaceSearchCoordinator(
            self._workspace_search_operations,
            WorkspaceSearchPorts(
                get_surface=lambda: self._workspace_search_surface,
                summarize=self._workspace_search_summary,
                notify=self.notify,
            ),
        )
        self._workspace_search_surface_admission_coordinator = (
            WorkspaceSearchSurfaceAdmissionCoordinator(
                WorkspaceSearchSurfaceAdmissionPorts(
                    startup_restore_inflight=lambda: self._startup_restore_inflight,
                    search_available=lambda: (
                        self._workspace_search is not None and self._workspace is not None
                    ),
                    workspace_root=lambda: (
                        self._workspace.root if self._workspace is not None else None
                    ),
                    get_surface=lambda: self._workspace_search_surface,
                    set_surface=lambda surface: setattr(
                        self,
                        "_workspace_search_surface",
                        surface,
                    ),
                    create_surface=lambda root: WorkspaceSearchSurface(
                        self,
                        root=root,
                        locale=self._locale,
                        callbacks=WorkspaceSearchSurfaceCallbacks(
                            search_requested=self._start_workspace_search,
                            cancel_requested=self._cancel_workspace_search,
                            file_requested=self._open_workspace_search_match,
                        ),
                    ),
                    invalidate_search=self._invalidate_workspace_search,
                    notify=self.notify,
                )
            )
        )
        self._settings_service = settings
        if initial_settings is not None:
            self._settings = initial_settings
        elif settings is not None:
            self._settings = settings.load()
        else:
            self._settings = DEFAULT_SETTINGS
        self._command_surface = CommandSurface(self, self._commands, lambda: self._locale)
        self._command_palette_surface = CommandPaletteSurface(self, locale=self._locale)
        self._file_dialog_surface = FileDialogSurface(self, locale=self._locale)
        self._message_surface = MessageSurface(self, locale=self._locale)
        self._settings_surface = SettingsSurface(self)
        self._recovery_prompt_surface = RecoveryPromptSurface(self, locale=self._locale)
        self._plugin_operations = PluginOperationTracker()
        self._plugin_surface = PluginSurface(
            self,
            locale=self._locale,
            callbacks=PluginSurfaceCallbacks(
                approve_requested=lambda entry: self._plugin_catalog_coordinator.approve_descriptor(
                    entry
                ),
                revoke_requested=lambda entry: self._plugin_catalog_coordinator.revoke_descriptor(
                    entry
                ),
                enable_requested=lambda plugin_id: self._plugin_runtime_coordinator.enable_plugin(
                    plugin_id
                ),
                disable_requested=lambda plugin_id: self._plugin_runtime_coordinator.disable_plugin(
                    plugin_id
                ),
            ),
        )
        self._editor_policy = editor_policy or DEFAULT_EDITOR_OPERATION_POLICY
        self._recovery_channel_factory = recovery_channel_factory
        self._runner = TaskRunner(self)
        self._session_save_coordinator = SessionSaveCoordinator(
            self._session_save,
            SessionSavePorts(
                can_save=lambda: (
                    self._session_service is not None and not self._startup_restore_inflight
                ),
                capture_snapshot=self._build_session_snapshot,
                next_operation_id=self._operation_tracker.reserve,
                save_snapshot=self._save_session_snapshot,
                dispatch=self._runner.submit,
                notify=self.notify,
            ),
        )
        self._session_save_timer.timeout.connect(self._session_save_coordinator.request_latest)
        self._plugin_catalog_coordinator = PluginCatalogCoordinator(
            PluginCatalogPorts(
                catalog=plugin_catalog,
                approval=plugin_approval,
                operations=self._plugin_operations,
                runner=self._runner,
                view=self._plugin_surface,
                notify=self.notify,
            )
        )
        self._plugin_host_coordinator = PluginHostProbeCoordinator(
            PluginHostProbePorts(
                host=plugin_host,
                operations=self._plugin_operations,
                runner=self._runner,
                notify=self.notify,
            )
        )
        self._editor_document_surface = EditorDocumentSurface(
            callbacks=EditorDocumentCallbacks(
                modified_changed=self._on_editor_modified,
                content_changed=self._on_editor_content_changed,
                caret_changed=self._request_session_save,
            )
        )
        self._editor_shell_surface = EditorShellSurface[_DocumentTab](
            self,
            locale=self._locale,
            close_requested=self._close_tab,
            current_changed=self._on_current_tab_changed,
            find_callbacks=FindSurfaceCallbacks(
                find_requested=self._find_requested,
                replace_requested=self._replace_requested,
                replace_all_requested=self._replace_all_requested,
                cancel_requested=self._cancel_replace_all,
                close_requested=self._close_find_bar,
                criteria_changed=self._invalidate_find_match,
            ),
        )
        self._tab_surface = self._editor_shell_surface.tabs
        self._find_surface = self._editor_shell_surface.find
        self.setCentralWidget(self._editor_shell_surface.widget)
        self._current_document_transition_coordinator = CurrentDocumentTransitionCoordinator[
            _DocumentTab
        ](
            CurrentDocumentTransitionPorts(
                invalidate_find_match=self._invalidate_find_match,
                reset_find_session=self._find_surface.reset_session,
                active_tab=self._tab_surface.active_tab,
                title_of=lambda tab: _tab_title(tab.state, self._locale),
                notify_info=lambda message: self.notify(message, level="info"),
                sync_status=self._sync_status_surface,
                request_session_save=self._request_session_save,
            )
        )
        self._document_change_projection_coordinator = DocumentChangeProjectionCoordinator[
            EditorWidget, _DocumentTab
        ](
            DocumentChangeProjectionPorts(
                find_tab=self._tab_surface.find_by_editor,
                is_dirty=lambda tab: tab.state.dirty,
                mark_dirty=lambda tab, dirty: setattr(
                    tab,
                    "state",
                    self._documents.mark_dirty(tab.state, dirty),
                ),
                increment_content_version=lambda tab: setattr(
                    tab,
                    "content_version",
                    tab.content_version + 1,
                ),
                invalidate_find_match=self._invalidate_find_match,
                update_tab_title=self._update_tab_title,
                sync_status=self._sync_status_surface,
                request_session_save=self._request_session_save,
            )
        )
        self._editor_action_admission_coordinator = EditorActionAdmissionCoordinator[
            EditorEngine, _DocumentTab
        ](
            EditorActionAdmissionPorts(
                active_tab=self._tab_surface.active_tab,
                is_busy=lambda: self._busy,
                editor_of=lambda tab: tab.editor,
                focus_editor=lambda tab: tab.editor.setFocus(),
            )
        )
        self._session_restore_coordinator = SessionRestoreCoordinator[_DocumentTab](
            self._session_restore,
            SessionRestorePorts(
                is_active=lambda: self._startup_restore_inflight,
                set_current=self._tab_surface.set_current,
                tab_count=lambda: self._tab_surface.count,
                ensure_initial_document=self.ensure_initial_document,
                finish_restore=self._finish_session_restore,
                request_session_save=self._request_session_save,
                path_of=lambda tab: tab.state.path,
                find_tab=self._tab_surface.find_by_path,
                start_open=lambda path: self._start_open(path, session_restore=True),
                notify_deferred=lambda path: self.notify(
                    f"Session document deferred for recovery review: {path.name}",
                    level="warning",
                ),
            ),
        )
        self._document_tab_creation_coordinator = DocumentTabCreationCoordinator[
            OpenedDocument, _DocumentTab, EditorWidget
        ](
            DocumentTabCreationPorts(
                create_editor=lambda opened: self._editor_document_surface.create(
                    language=_language_for_path(opened.state.path),
                    text=opened.text,
                    dirty=opened.state.dirty,
                    settings=self._settings.editor,
                    theme=self._settings.appearance.theme,
                    accent=self._settings.appearance.accent,
                ),
                create_tab=lambda editor, opened, recovery_snapshot_id: _DocumentTab(
                    editor,
                    opened.state,
                    recovery_snapshot_id=recovery_snapshot_id,
                ),
                add_tab=lambda tab, title, modified: self._tab_surface.add_tab(
                    tab,
                    title,
                    modified=modified,
                ),
                title=lambda tab: _tab_title(tab.state, self._locale),
                is_modified=lambda tab: tab.state.dirty or tab.editor.is_modified(),
                update_title=self._update_tab_title,
                request_session_save=self._request_session_save,
                sync_status=self._sync_status_surface,
            ),
        )
        self._document_creation_admission_coordinator = DocumentCreationAdmissionCoordinator(
            DocumentCreationAdmissionPorts(
                is_busy=lambda: self._busy,
                startup_restore_inflight=lambda: self._startup_restore_inflight,
                new_document=self._documents.new_document,
                add_tab=self._add_tab,
                publish_opened=lambda state: self._events.publish(DocumentOpened(state)),
                notify=self.notify,
            )
        )

        self._workspace_surface: WorkspaceSurface | None = None
        self._workspace_file_activation_coordinator = WorkspaceFileActivationCoordinator[
            _DocumentTab
        ](
            WorkspaceFileActivationPorts(
                is_startup_restore_inflight=lambda: self._startup_restore_inflight,
                is_busy=lambda: self._busy,
                contains=lambda path: (
                    self._workspace is not None and self._workspace.contains(path)
                ),
                find_tab=self._tab_surface.find_by_path,
                set_current=self._tab_surface.set_current,
                start_open=self._start_open,
                notify=self.notify,
            )
        )
        self._workspace_operations = WorkspaceOperationTracker()
        self._settings_save = SettingsSaveTracker()
        self._theme_transition_surface = ThemeTransitionSurface()
        self._settings_save_projection_coordinator = SettingsSaveProjectionCoordinator(
            SettingsSaveProjectionPorts(
                apply_snapshot=self._apply_settings_snapshot,
                retranslate=self._retranslate_ui,
                apply_editor_settings=self._apply_editor_settings_to_tabs,
                animate_transition=self._animate_theme_transition,
                notify_saved=lambda: self.notify("Settings saved", level="success"),
            )
        )
        self._settings_save_coordinator = SettingsSaveCoordinator(
            self._settings_save,
            SettingsSavePorts(
                apply_settings=self._settings_save_projection_coordinator.project,
                show_invalid_result=self._show_invalid_settings_result,
                show_failure=self._show_settings_save_failed,
            ),
        )
        self._settings_save_admission_coordinator = SettingsSaveAdmissionCoordinator(
            SettingsSaveAdmissionPorts(
                get_service=lambda: self._settings_service,
                is_inflight=lambda: self._settings_save.inflight,
                edit=self._settings_surface.edit,
                reserve_operation=self._operation_tracker.reserve,
                begin_save=self._settings_save.begin,
                submit_save=self._settings_save_coordinator.submit,
                dispatch=self._runner.submit,
                notify=self.notify,
            )
        )
        if self._workspace is not None:
            self._workspace_surface = WorkspaceSurface(
                self,
                locale=self._locale,
                callbacks=WorkspaceSurfaceCallbacks(
                    folder_requested=self._choose_workspace,
                    file_picker_requested=self._open_document,
                    directory_requested=self._load_workspace_directory,
                    file_requested=self._workspace_file_activation_coordinator.activate,
                    back_requested=self._go_workspace_parent,
                    cancel_requested=self._cancel_workspace_operation,
                ),
            )
        self._workspace_navigation_projection_coordinator = (
            WorkspaceNavigationProjectionCoordinator(
                WorkspaceNavigationProjectionPorts(
                    invalidate_search=lambda: (
                        self._invalidate_workspace_search() if self._workspace is not None else None
                    ),
                    activate_workspace=lambda result: (
                        self._workspace.activate(result) if self._workspace is not None else None
                    ),
                    set_search_root=lambda root: (
                        self._workspace_search_surface.set_root(root)
                        if self._workspace_search_surface is not None
                        else None
                    ),
                    has_workspace_surface=lambda: self._workspace_surface is not None,
                    set_directory=lambda directory, root: (
                        self._workspace_surface.set_directory(directory, root)
                        if self._workspace_surface is not None
                        else None
                    ),
                    notify_opened=lambda root: self.notify(
                        f"Workspace opened: {root}", level="success"
                    ),
                    request_session_save=self._request_session_save,
                    finish_session_restore=self._finish_session_workspace_restore,
                    get_workspace_root=lambda: (
                        self._workspace.root if self._workspace is not None else None
                    ),
                )
            )
        )
        self._workspace_navigation_coordinator = WorkspaceNavigationCoordinator(
            WorkspaceNavigationPorts(
                tracker=self._workspace_operations,
                complete_operation=self._complete_operation,
                get_surface=lambda: self._workspace_surface,
                apply_opened=self._workspace_navigation_projection_coordinator.project_opened,
                apply_directory=self._workspace_navigation_projection_coordinator.project_directory,
                finish_session_restore=self._finish_session_workspace_restore,
                notify=self.notify,
            )
        )
        self._workspace_navigation_admission_coordinator = WorkspaceNavigationAdmissionCoordinator(
            WorkspaceNavigationAdmissionPorts(
                get_workspace=lambda: self._workspace,
                has_surface=lambda: self._workspace_surface is not None,
                is_busy=lambda: self._busy,
                startup_restore_inflight=lambda: self._startup_restore_inflight,
                set_loading=lambda loading: (
                    self._workspace_surface.set_loading(loading)
                    if self._workspace_surface is not None
                    else None
                ),
                begin_operation=self._begin_operation,
                begin_generation=self._workspace_operations.begin,
                submit_open=self._workspace_navigation_coordinator.submit_open,
                submit_directory=self._workspace_navigation_coordinator.submit_directory,
                dispatch=self._runner.submit,
                notify=self.notify,
            )
        )

        self._document_open_projection_coordinator = DocumentOpenProjectionCoordinator[
            _DocumentTab
        ](
            DocumentOpenProjectionPorts(
                find_existing=self._tab_surface.find_by_path,
                record_restored_tab=self._session_restore.record_restored_tab,
                show_duplicate_error=lambda: self._show_error(
                    "Open failed", "That file is already open in another tab."
                ),
                add_tab=self._add_tab,
                go_to_line=lambda tab, line_number: tab.editor.go_to_line(line_number),
                set_cursor_position=lambda tab, line, column: tab.editor.set_cursor_position(
                    line, column
                ),
                publish_opened=lambda state: self._events.publish(DocumentOpened(state)),
                notify_opened=lambda path: self.notify(
                    f"Opened {path.name if path else 'document'}",
                    level="success",
                ),
                continue_session_restore=self._continue_session_restore,
            ),
        )
        self._document_open_coordinator = DocumentOpenCoordinator(
            DocumentOpenPorts(
                complete_operation=self._complete_operation,
                is_session_restore=lambda operation_id: (
                    self._session_restore.operation_id == operation_id
                ),
                take_session_document=self._session_restore.take_open_document,
                apply_opened=self._document_open_projection_coordinator.project,
                continue_session_restore=self._continue_session_restore,
                show_error=self._show_error,
                notify=self.notify,
            )
        )
        self._document_open_admission_coordinator = DocumentOpenAdmissionCoordinator(
            DocumentOpenAdmissionPorts(
                is_busy=lambda: self._busy,
                startup_restore_inflight=lambda: self._startup_restore_inflight,
                begin_operation=self._begin_operation,
                bind_session_restore=self._session_restore.bind_open_operation,
                open_document=self._documents.open_document,
                submit_open=lambda operation, operation_id, line_number, dispatch: (
                    self._document_open_coordinator.submit(
                        operation=operation,
                        operation_id=operation_id,
                        line_number=line_number,
                        dispatch=dispatch,
                    )
                ),
                dispatch=self._runner.submit,
                notify=self.notify,
            )
        )
        self._document_save_projection_coordinator = DocumentSaveProjectionCoordinator[
            _DocumentTab
        ](
            DocumentSaveProjectionPorts(
                apply_saved_state=self._apply_saved_state,
                refresh_language=lambda tab, path: self._editor_document_surface.set_language(
                    tab.editor, _language_for_path(path)
                ),
                update_title=self._update_tab_title,
                clear_recovery_snapshot=self._clear_recovery_snapshot,
                publish_saved=lambda result: self._events.publish(DocumentSaved(result)),
                notify_saved=lambda path: self.notify(
                    f"Saved {path.name if path else 'document'}",
                    level="success",
                ),
                request_session_save=self._request_session_save,
            ),
        )
        self._document_save_coordinator = DocumentSaveCoordinator[_DocumentTab](
            DocumentSavePorts(
                complete_operation=self._complete_operation,
                contains_tab=self._tab_surface.contains,
                set_read_only=lambda tab, read_only: tab.editor.set_read_only(read_only),
                apply_saved=self._document_save_projection_coordinator.project,
                show_error=self._show_error,
            )
        )
        self._document_picker_admission_coordinator = DocumentPickerAdmissionCoordinator(
            DocumentPickerAdmissionPorts(
                is_busy=lambda: self._busy,
                startup_restore_inflight=lambda: self._startup_restore_inflight,
                choose_document=self._file_dialog_surface.choose_document,
                start_open=self._start_open,
                notify=self.notify,
            )
        )
        self._document_save_picker_admission_coordinator = DocumentSavePickerAdmissionCoordinator[
            _DocumentTab
        ](
            DocumentSavePickerAdmissionPorts(
                get_active_tab=self._tab_surface.active_tab,
                is_busy=lambda: self._busy,
                get_current_path=lambda tab: tab.state.path,
                choose_save_path=self._file_dialog_surface.choose_save_path,
                start_save=self._start_save,
            )
        )
        self._workspace_picker_admission_coordinator = WorkspacePickerAdmissionCoordinator(
            WorkspacePickerAdmissionPorts(
                has_workspace=lambda: self._workspace is not None,
                startup_restore_inflight=lambda: self._startup_restore_inflight,
                is_busy=lambda: self._busy,
                choose_workspace=self._file_dialog_surface.choose_workspace,
                start_open=self._start_workspace_open,
                notify=self.notify,
            )
        )
        self._document_save_admission_coordinator = DocumentSaveAdmissionCoordinator[_DocumentTab](
            DocumentSaveAdmissionPorts(
                is_busy=lambda: self._busy,
                startup_restore_inflight=lambda: self._startup_restore_inflight,
                find_existing=lambda target, tab: self._tab_surface.find_by_path(
                    target,
                    exclude=tab,
                ),
                show_duplicate_error=lambda: self._show_error(
                    "Save failed", "That file is already open in another tab."
                ),
                state_snapshot=lambda tab: tab.state,
                text_snapshot=lambda tab: tab.editor.get_text(),
                set_read_only=lambda tab, read_only: tab.editor.set_read_only(read_only),
                begin_operation=self._begin_operation,
                save_document=self._documents.save_document,
                submit_save=lambda operation, operation_id, tab, after, dispatch: (
                    self._document_save_coordinator.submit(
                        operation=operation,
                        operation_id=operation_id,
                        tab=tab,
                        after=after,
                        dispatch=dispatch,
                    )
                ),
                dispatch=self._runner.submit,
                notify=self.notify,
            )
        )
        self._plugin_runtime_coordinator = PluginRuntimeCoordinator(
            plugin_runtime,
            PluginRuntimePorts(
                view=self._plugin_surface,
                is_busy=lambda: self._busy,
                refresh_commands=self.refresh_command_menus,
                notify=self.notify,
            ),
        )
        self._replace_all_tracker = ReplaceAllTracker[_DocumentTab, ReplaceAllSession]()
        self._replace_all_admission_coordinator = ReplaceAllAdmissionCoordinator[
            _DocumentTab, ReplaceAllSession
        ](
            self._replace_all_tracker,
            ReplaceAllAdmissionPorts(
                is_busy=lambda: self._busy,
                active_tab=self._tab_surface.active_tab,
                query=self._find_surface.query,
                replacement=self._find_surface.replacement,
                case_sensitive=self._find_surface.case_sensitive,
                begin_session=lambda tab, query, replacement, case_sensitive, max_matches: (
                    tab.editor.begin_replace_all_literal(
                        query,
                        replacement,
                        case_sensitive=case_sensitive,
                        max_matches=max_matches,
                    )
                ),
                max_matches=self._editor_policy.max_replace_matches,
                content_version=lambda tab: tab.content_version,
                was_dirty=lambda tab: tab.state.dirty or tab.editor.is_modified(),
                begin_operation=self._begin_operation,
                complete_operation=self._complete_operation,
                start_job=self._start_replace_all_job,
                set_status=lambda message, level: self._find_surface.set_status(
                    message,
                    level=level,
                ),
            ),
        )
        self._replace_all_completion_coordinator = ReplaceAllCompletionCoordinator[
            _DocumentTab, ReplaceAllSession, ReplaceAllProgress
        ](
            self._replace_all_tracker,
            ReplaceAllCompletionPorts(
                contains_tab=self._tab_surface.contains,
                set_operation_locked=lambda tab, locked: tab.editor.set_operation_locked(locked),
                set_tab_bar_enabled=self._tab_surface.set_tab_bar_enabled,
                set_operation_active=self._find_surface.set_operation_active,
                complete_operation=self._complete_operation,
                project_outcome=self._project_replace_all_outcome,
            ),
        )
        self._recovery_state = RecoveryCaptureTracker[_RecoveryCaptureJob, _DocumentTab]()
        self._recovery_capture_admission_coordinator = RecoveryCaptureAdmissionCoordinator[
            _DocumentTab, DocumentState
        ](
            RecoveryCaptureAdmissionPorts(
                recovery_available=lambda: self._recovery is not None,
                is_busy=lambda: self._busy,
                tabs=lambda: self._tab_surface.tabs,
                document_id=lambda tab: tab.state.document_id,
                document_in_flight=self._recovery_state.document_in_flight,
                is_dirty=lambda tab: tab.state.dirty or tab.editor.is_modified(),
                snapshot_id=lambda tab: tab.recovery_snapshot_id,
                delete_in_flight_or_pending=self._recovery_state.delete_in_flight_or_pending,
                new_snapshot_id=lambda: self._recovery.new_snapshot_id(),
                set_snapshot_id=lambda tab, snapshot_id: setattr(
                    tab,
                    "recovery_snapshot_id",
                    snapshot_id,
                ),
                state_snapshot=lambda tab: tab.state,
                state_is_dirty=lambda state: state.dirty,
                mark_dirty=self._documents.mark_dirty,
                content_version=lambda tab: tab.content_version,
            )
        )
        self._recovery_projection_coordinator = RecoveryProjectionCoordinator[_DocumentTab](
            RecoveryProjectionPorts(
                is_live=self._tab_surface.contains,
                is_dirty=lambda tab: tab.state.dirty or tab.editor.is_modified(),
                current_snapshot_id=lambda tab: tab.recovery_snapshot_id,
                current_content_version=lambda tab: tab.content_version,
                schedule_delete=self._schedule_recovery_delete,
                clear_snapshot=self._clear_recovery_snapshot,
                notify_newer_edits=lambda: self.notify(
                    "Recovery snapshot kept; newer edits will be captured next cycle",
                    level="info",
                ),
                notify_failure=lambda tab, error: self.notify(
                    f"Autosave failed for "
                    f"{_tab_title(tab.state, self._locale).lstrip('*')}: {error}",
                    level="error",
                ),
            )
        )
        self._document_tab_removal_coordinator = DocumentTabRemovalCoordinator[
            _DocumentTab, _RecoveryCaptureJob
        ](
            DocumentTabRemovalPorts(
                contains=self._tab_surface.contains,
                document_id=lambda tab: tab.state.document_id,
                capture_for_document=self._recovery_state.capture_for_document,
                cancel_capture=self._cancel_recovery_capture,
                clear_recovery_snapshot=self._clear_recovery_snapshot,
                remove_tab=self._tab_surface.remove_tab,
                delete_editor=lambda tab: tab.editor.deleteLater(),
                publish_closed=lambda tab: self._events.publish(DocumentClosed(tab.state)),
                request_session_save=self._request_session_save,
                tab_count=lambda: self._tab_surface.count,
                ensure_initial_document=self._new_document,
            ),
        )
        self._recovery_write_coordinator = RecoveryWriteCoordinator[
            _RecoveryCaptureJob, _DocumentTab
        ](
            self._recovery_state,
            RecoveryWritePorts(
                document_id=lambda tab: tab.state.document_id,
                abort_capture=self._abort_recovery_write_capture,
                schedule_pending_delete=self._schedule_pending_recovery_delete,
                project_saved=self._recovery_projection_coordinator.project_saved,
                project_failed=self._recovery_projection_coordinator.project_failed,
            ),
        )
        self._recovery_capture_abort_coordinator = RecoveryCaptureAbortCoordinator[
            _RecoveryCaptureJob, _DocumentTab
        ](
            self._recovery_state,
            RecoveryCaptureAbortPorts[_RecoveryCaptureJob, _DocumentTab](
                owner_for_job=lambda job: job.tab,
                document_id=lambda tab: tab.state.document_id,
                snapshot_id=lambda job: job.snapshot_id,
                channel=lambda job: job.channel,
                cancel_session=lambda job: job.session.cancel(),
                notify_failure=self._notify_recovery_capture_failure,
                is_live=self._tab_surface.contains,
            ),
        )
        self._recovery_delete_coordinator = RecoveryDeleteCoordinator[
            _RecoveryCaptureJob, _DocumentTab
        ](
            self._recovery_state,
            RecoveryDeletePorts(
                clear_owner_snapshot=self._clear_recovery_delete_owner,
                schedule_pending_delete=self._schedule_pending_recovery_delete,
                notify=self.notify,
            ),
        )
        self._recovery_scan_tracker = RecoveryScanTracker()
        self._recovery_scan_coordinator = RecoveryScanCoordinator(
            self._recovery_scan_tracker,
            RecoveryScanPorts(
                get_session_snapshot=lambda: self._session_restore.snapshot or DEFAULT_SESSION,
                prompt_recovery=self._prompt_recovery,
                continue_session_restore=self._begin_session_restore,
                notify=self.notify,
            ),
        )
        self._recovery_timer = QTimer(self)
        self._recovery_timer.setInterval(max(1000, recovery_interval_ms))
        self._recovery_timer.timeout.connect(self._autosave_recovery)
        if self._recovery is not None:
            self._recovery_timer.start()
        self._find_match_tracker = FindMatchTracker[_DocumentTab]()
        self._close_guard_coordinator = CloseGuardCoordinator(
            CloseGuardPorts(
                is_busy=lambda: self._busy,
                workspace_search_in_flight=self._workspace_search_operations.in_flight,
                cancel_workspace_search=self._cancel_workspace_search,
                has_dirty_tabs=lambda: any(
                    tab.state.dirty or tab.editor.is_modified() for tab in self._tab_surface.tabs
                ),
                has_background_operations=lambda: (
                    self._recovery_scan_tracker.inflight
                    or self._recovery_state.has_inflight_documents
                    or self._recovery_state.has_inflight_snapshots
                    or self._recovery_state.has_delete_inflight
                    or self._settings_save.inflight
                    or self._plugin_operations.in_flight("catalog-scan")
                    or self._plugin_operations.in_flight("catalog-governance")
                    or self._plugin_operations.in_flight("host-probe")
                ),
                request_immediate_session_save=lambda: self._request_session_save(immediate=True),
                has_pending_work=self._runner.has_pending_work,
                stop_timers=self._stop_close_timers,
            )
        )
        self._close_guard_feedback_coordinator = CloseGuardFeedbackCoordinator(
            CloseGuardFeedbackPorts(
                pending_count=lambda: self._runner.pending_count,
                show_error=self._show_error,
            )
        )
        self._events.subscribe(PluginFailed, self._plugin_runtime_coordinator.on_plugin_failed)

        self._core_command_coordinator = CoreCommandCoordinator(
            self._commands,
            CoreCommandPorts(
                new_document=self._new_document,
                open_document=self._open_document,
                choose_workspace=self._choose_workspace,
                save_document=self._save_document,
                save_as_document=self._save_as_document,
                close_current_tab=self._close_current_tab,
                show_recovery_candidates=self._show_recovery_candidates,
                quit_application=self.close,
                undo=self._undo,
                redo=self._redo,
                cut=self._cut,
                copy=self._copy,
                paste=self._paste,
                select_all=self._select_all,
                show_find=self._show_find,
                show_replace=self._show_replace,
                show_workspace_search=self._show_workspace_search,
                show_command_palette=self._show_command_palette,
                show_settings=self._show_settings,
                show_extension_catalog=self._plugin_catalog_coordinator.show_catalog,
                show_plugin_status=self._plugin_runtime_coordinator.show_status,
                show_plugin_host_diagnostics=self._plugin_host_coordinator.probe,
                show_about=self._message_surface.show_about,
            ),
        )
        self._core_command_coordinator.register()
        self._create_menus()
        self._core_toolbar_coordinator = CoreToolbarCoordinator(
            CoreToolbarPorts(
                new_document=self._new_document,
                open_document=self._open_document,
                save_document=self._save_document,
                show_find=self._show_find,
                show_replace=self._show_replace,
                show_command_palette=self._show_command_palette,
                choose_workspace=self._choose_workspace if self._workspace is not None else None,
            )
        )
        self._command_surface.create_toolbar(self._core_toolbar_coordinator.actions())
        self._status_surface = StatusSurface(self, locale=self._locale)
        self._status_surface.attach_to(self.statusBar())
        self._status_phase_coordinator = StatusPhaseCoordinator()
        self._locale_coordinator = PresentationLocaleCoordinator(
            PresentationLocalePorts(
                get_locale=lambda: self._locale,
                set_window_title=lambda locale: self.setWindowTitle(tr("app.title", locale)),
                retranslate_commands=self._command_surface.retranslate,
                set_command_palette_locale=self._command_palette_surface.set_locale,
                set_editor_shell_locale=self._editor_shell_surface.set_locale,
                refresh_tab_icons=self._tab_surface.refresh_icons,
                refresh_tab_titles=self._refresh_tab_titles,
                set_file_dialog_locale=self._file_dialog_surface.set_locale,
                set_message_locale=self._message_surface.set_locale,
                set_recovery_prompt_locale=self._recovery_prompt_surface.set_locale,
                set_status_locale=self._status_surface.set_locale,
                set_workspace_search_locale=lambda locale: (
                    self._workspace_search_surface.set_locale(locale)
                    if self._workspace_search_surface is not None
                    else None
                ),
                set_plugin_locale=self._plugin_surface.set_locale,
                set_workspace_locale=(
                    self._workspace_surface.set_locale
                    if self._workspace_surface is not None
                    else None
                ),
                refresh_workspace_icons=(
                    self._workspace_surface.refresh_icons
                    if self._workspace_surface is not None
                    else None
                ),
            )
        )
        self._runner.pending_changed.connect(self._on_runner_pending_changed)
        self._retranslate_ui()

    @property
    def _locale(self):
        """Return the currently normalized UI locale from the settings snapshot."""
        return self._settings.appearance.locale

    def ensure_initial_document(self) -> None:
        """Create the first tab after plugins can subscribe to lifecycle events."""
        if self._tab_surface.count == 0:
            self._new_document(allow_during_startup=True)

    def preflight_editor_shell(self) -> None:
        """Construct one empty editor tab for a no-window startup preflight."""
        try:
            self.ensure_initial_document()
        finally:
            self._stop_close_timers()

    def preflight_startup_restore(self, *, timeout_seconds: float = 5.0) -> dict[str, object]:
        """Run production startup restore with bounded event processing and cleanup."""
        if timeout_seconds <= 0:
            raise ValueError("Startup restore preflight timeout must be positive")
        try:
            self.restore_startup_state()
            return self._wait_for_startup_preflight(timeout_seconds)
        finally:
            self._stop_close_timers()

    def preflight_open_startup_paths(
        self,
        paths: Sequence[Path],
        *,
        timeout_seconds: float = 5.0,
    ) -> dict[str, object]:
        """Drain queued startup paths after restore without showing the window."""
        if timeout_seconds <= 0:
            raise ValueError("Startup path preflight timeout must be positive")
        requested_paths = tuple(path for path in paths if isinstance(path, Path))
        try:
            self.open_startup_paths(requested_paths)
            result = self._wait_for_startup_preflight(timeout_seconds)
            result["startup_paths_total"] = len(requested_paths)
            result["startup_paths_open"] = sum(
                self._tab_surface.find_by_path(path) is not None for path in requested_paths
            )
            return result
        finally:
            self._stop_close_timers()

    def _wait_for_startup_preflight(self, timeout_seconds: float) -> dict[str, object]:
        """Process queued UI completions with a bounded, soft timeout.

        A single Qt event handler may run beyond the deadline; the check is
        therefore a cooperative timeout rather than an interrupt boundary.
        """
        application = QApplication.instance()
        if application is None:
            raise RuntimeError("Startup preflight requires a QApplication")
        deadline = time.monotonic() + timeout_seconds
        processed_batches = 0
        wake_timer = QTimer()
        wake_timer.setInterval(2)
        wake_timer.start()
        try:
            while (
                self._startup_restore_inflight
                or self._runner.has_pending_work()
                or self._pending_startup_paths
            ):
                if time.monotonic() >= deadline:
                    raise TimeoutError("Startup preflight timed out")
                application.processEvents(
                    QEventLoop.ProcessEventsFlag.AllEvents
                    | QEventLoop.ProcessEventsFlag.WaitForMoreEvents
                )
                processed_batches += 1
                if time.monotonic() >= deadline:
                    raise TimeoutError("Startup preflight timed out")
        finally:
            wake_timer.stop()
        return {
            "startup_restore_completed": not self._startup_restore_inflight,
            "pending_work": self._runner.has_pending_work(),
            "pending_startup_paths": len(self._pending_startup_paths),
            "tab_count": self._tab_surface.count,
            "processed_event_batches": processed_batches,
            "timeout_mode": "soft",
            "window_shown": False,
            "event_loop_entered": False,
        }

    def restore_startup_state(self) -> None:
        """Restore recovery first, then the bounded local session manifest."""
        if self._startup_restore_inflight:
            return
        self._startup_restore_inflight = True
        self._session_restore.clear_deferred()
        if self._session_service is None:
            self._session_save.set_last_saved(DEFAULT_SESSION)
            self._session_restore.set_snapshot(DEFAULT_SESSION)
            self._schedule_recovery_scan(startup=True)
            return
        operation_id = self._operation_tracker.reserve()
        self._session_load_coordinator.submit(
            operation=self._session_service.load,
            operation_id=operation_id,
            dispatch=self._runner.submit,
        )

    def open_startup_paths(self, paths: Sequence[Path]) -> None:
        """Queue explicit launch paths behind recovery and session restoration."""
        self._pending_startup_paths.extend(path for path in paths if isinstance(path, Path))
        if not self._startup_restore_inflight:
            self._drain_startup_paths()

    def restore_recovery_candidates(self) -> None:
        """Prompt for recovery candidates before creating the initial blank tab."""
        self._schedule_recovery_scan(startup=True)

    def refresh_command_menus(self) -> None:
        """Rebuild menu projections after a plugin registers commands."""
        self._command_surface.refresh_command_menus()

    def active_document_snapshot(self) -> ActiveDocumentSnapshot | None:
        """Provide plugins a copy-like view, never the active editor widget."""
        tab = self._tab_surface.active_tab()
        if tab is None:
            return None
        return ActiveDocumentSnapshot(tab.state, tab.editor.get_text())

    def notify(self, message: str, *, level: StatusMessageLevel = "info") -> None:
        """Host notification port used by plugins and application operations."""
        self._status_surface.show_message(message, level=level)

    def _create_menus(self) -> None:
        self._command_surface.create_menus()

    def _retranslate_ui(self) -> None:
        """Refresh user-facing shell text after a persisted locale change."""
        self._locale_coordinator.retranslate()

    def _new_document(self, *, allow_during_startup: bool = False) -> None:
        self._document_creation_admission_coordinator.admit(
            allow_during_startup=allow_during_startup,
        )

    def _open_document(self) -> None:
        self._document_picker_admission_coordinator.admit()

    def _start_open(
        self,
        path: Path,
        *,
        line_number: int | None = None,
        session_restore: bool = False,
    ) -> bool:
        """Open one explicit path through the asynchronous document boundary."""
        return self._document_open_admission_coordinator.admit(
            path,
            line_number=line_number,
            session_restore=session_restore,
        )

    def _drain_startup_paths(self) -> None:
        """Open queued launch paths one at a time through existing admissions."""
        if self._startup_restore_inflight or self._busy:
            return
        while self._pending_startup_paths:
            requested_path = self._pending_startup_paths.pop(0)
            try:
                path = requested_path.expanduser().resolve(strict=False)
                mode = path.stat().st_mode
                if S_ISDIR(mode):
                    admitted = self._start_workspace_open(path)
                elif S_ISREG(mode):
                    admitted = self._start_open(path)
                else:
                    self.notify(f"Cannot open path: {requested_path}", level="warning")
                    continue
            except (OSError, RuntimeError, ValueError) as error:
                self.notify(f"Cannot open path: {requested_path} ({error})", level="warning")
                continue
            if admitted:
                return
            if self._startup_restore_inflight or self._busy:
                return
            self.notify(f"Unable to open path: {requested_path}", level="warning")

    def _choose_workspace(self) -> None:
        self._workspace_picker_admission_coordinator.admit()

    def _show_workspace_search(self) -> None:
        """Show the asynchronous Find in Files surface for the active workspace root."""
        self._workspace_search_surface_admission_coordinator.admit()

    def _start_workspace_search(self, text: str, case_sensitive: bool) -> None:
        """Submit one immutable workspace query and retain its cooperative cancel token."""
        if self._workspace_search is None or self._workspace is None:
            self.notify("Workspace search is unavailable", level="error")
            return
        root = self._workspace.root
        surface = self._workspace_search_surface
        if root is None or surface is None:
            self.notify("Open a workspace before searching files", level="warning")
            return
        if self._workspace_search_operations.in_flight():
            surface.set_cancel_requested()
            return
        try:
            query = WorkspaceSearchQuery(root=root, needle=text, case_sensitive=case_sensitive)
        except (TypeError, ValueError) as error:
            surface.present_error(str(error))
            return
        operation_id = self._operation_tracker.reserve()
        generation, cancellation = self._workspace_search_operations.begin(operation_id)
        surface.set_busy(True)
        search_service = self._workspace_search
        self._workspace_search_coordinator.submit(
            operation=lambda: search_service.search(query, cancel_requested=cancellation.is_set),
            operation_id=operation_id,
            generation=generation,
            dispatch=self._runner.submit,
        )

    def _cancel_workspace_search(self) -> None:
        """Request cooperative cancellation; the worker remains isolated until completion."""
        surface = self._workspace_search_surface
        if not self._workspace_search_operations.cancel():
            return
        if surface is not None:
            surface.set_cancel_requested()

    def _workspace_search_summary(self, result: WorkspaceSearchResult) -> str:
        return workspace_search_summary(
            cancelled=result.cancelled,
            truncated=result.truncated,
            limit_reason=result.limit_reason,
            matches=len(result.matches),
            files_scanned=result.files_scanned,
            bytes_scanned=result.bytes_scanned,
            diagnostic_count=len(result.issues),
            diagnostics_truncated=result.issues_truncated,
            locale=self._locale,
        )

    def _open_workspace_search_match(self, path: object, line_number: int) -> None:
        """Recheck root containment before opening a result in the editor."""
        if not isinstance(path, Path) or type(line_number) is not int or line_number < 1:
            return
        if self._workspace is None or not self._workspace.contains(path):
            self.notify("That search result is outside the selected workspace", level="warning")
            return
        self._start_open(path, line_number=line_number)

    def _start_workspace_open(self, path: Path, *, session_restore: bool = False) -> bool:
        return self._workspace_navigation_admission_coordinator.admit_open(
            path,
            session_restore=session_restore,
        )

    def _load_workspace_directory(self, path: object) -> None:
        self._workspace_navigation_admission_coordinator.admit_directory(path)

    def _cancel_workspace_operation(self) -> None:
        operation_id = self._workspace_operations.active_operation_id
        if operation_id is None or not self._busy:
            return
        session_restore = self._session_restore.workspace_pending
        invalidated_operation_id = self._workspace_operations.invalidate()
        if invalidated_operation_id is None:
            return
        self._operation_tracker.cancel(invalidated_operation_id)
        self._busy = False
        self._sync_status_surface()
        if self._workspace_surface is not None:
            self._workspace_surface.set_loading(False)
            self._workspace_surface.show_error("Workspace loading cancelled; current view kept")
        self.notify("Workspace loading cancelled", level="warning")
        if session_restore:
            self._finish_session_workspace_restore()

    def _invalidate_workspace_search(self) -> None:
        """Cancel a search whose root is no longer the active workspace."""
        self._workspace_search_operations.invalidate()

    def _go_workspace_parent(self) -> None:
        if (
            self._workspace is None
            or self._workspace_surface is None
            or self._busy
            or self._startup_restore_inflight
        ):
            if self._startup_restore_inflight:
                self.notify("Restoring the previous session...", level="warning")
            return
        current = self._workspace_surface.current_path
        root = self._workspace.root
        if current is None or root is None or current == root:
            return
        self._load_workspace_directory(current.parent)

    def _finish_session_workspace_restore(self) -> None:
        """Release the startup workspace barrier on success, failure, or staleness."""
        if not self._session_restore.workspace_pending:
            return
        self._session_restore.set_workspace_pending(False)
        self._continue_session_restore()

    def _finish_session_restore(self) -> None:
        """Release the startup restore state after every document is handled."""
        self._startup_restore_inflight = False
        self._session_restore.finish()
        self._drain_startup_paths()

    def _save_document(self) -> None:
        self._document_save_picker_admission_coordinator.admit()

    def _save_as_document(self) -> None:
        self._document_save_picker_admission_coordinator.admit(force_picker=True)

    def _start_save(
        self,
        tab: _DocumentTab,
        target: Path,
        after: Callable[[], None] | None = None,
    ) -> None:
        self._document_save_admission_coordinator.admit(
            tab,
            target,
            after=after,
        )

    def _apply_saved_state(
        self,
        tab: _DocumentTab,
        result: DocumentState,
    ) -> None:
        tab.state = result
        tab.editor.set_modified(False)

    def _add_tab(
        self,
        opened: OpenedDocument,
        *,
        recovery_snapshot_id: str | None = None,
    ) -> _DocumentTab:
        return self._document_tab_creation_coordinator.add(
            opened,
            recovery_snapshot_id=recovery_snapshot_id,
        )

    def _on_editor_modified(self, editor: EditorWidget, dirty: bool) -> None:
        self._document_change_projection_coordinator.project_modified(editor, dirty)

    def _on_editor_content_changed(self, editor: EditorWidget) -> None:
        self._document_change_projection_coordinator.project_content_changed(editor)

    def _undo(self) -> None:
        self._run_editor_action(lambda editor: editor.undo())

    def _redo(self) -> None:
        self._run_editor_action(lambda editor: editor.redo())

    def _cut(self) -> None:
        self._run_editor_action(lambda editor: editor.cut())

    def _copy(self) -> None:
        self._run_editor_action(lambda editor: editor.copy())

    def _paste(self) -> None:
        self._run_editor_action(lambda editor: editor.paste())

    def _select_all(self) -> None:
        self._run_editor_action(lambda editor: editor.select_all())

    def _run_editor_action(self, action: Callable[[EditorEngine], None]) -> None:
        """Run one bounded editor-local operation against the active tab only."""
        self._editor_action_admission_coordinator.admit(action)

    def _show_find(self) -> None:
        if self._tab_surface.active_tab() is not None:
            self._find_surface.show_find()

    def _show_replace(self) -> None:
        if self._tab_surface.active_tab() is not None:
            self._find_surface.show_find(replacement=True)

    def _show_command_palette(self) -> None:
        command_id = self._command_palette_surface.choose(self._commands.all())
        if command_id is None:
            return
        command = self._commands.get(command_id)
        if command is not None:
            command.execute()
        else:
            self.notify(
                localize_message(f"Command is no longer available: {command_id}", self._locale),
                level="warning",
            )
            self.refresh_command_menus()

    def _show_settings(self) -> None:
        self._settings_save_admission_coordinator.admit(self._settings)

    def _apply_settings_snapshot(self, result: SettingsSnapshot) -> None:
        self._settings = result
        application = QApplication.instance()
        if application is not None:
            apply_theme(application, result.appearance)

    def _show_invalid_settings_result(self) -> None:
        self._show_error(
            "Settings failed",
            "The settings service returned an invalid result.",
        )

    def _show_settings_save_failed(self, error: Exception) -> None:
        self._show_error("Settings failed", error)

    def _apply_editor_settings_to_tabs(self) -> None:
        for tab in self._tab_surface.tabs:
            self._editor_document_surface.apply_settings(
                tab.editor,
                settings=self._settings.editor,
                theme=self._settings.appearance.theme,
                accent=self._settings.appearance.accent,
            )

    def _animate_theme_transition(self) -> None:
        """Fade the editor shell in after a theme/font change when motion is enabled."""
        self._theme_transition_surface.animate(
            self.centralWidget(),
            enabled=self._settings.appearance.motion_enabled,
        )

    def _close_find_bar(self) -> None:
        if self._replace_all_tracker.inflight:
            self._cancel_replace_all()
        self._invalidate_find_match()
        self._find_surface.hide()
        tab = self._tab_surface.active_tab()
        if tab is not None:
            tab.editor.setFocus()

    def _find_requested(self, forward: bool) -> None:
        if self._busy:
            self._find_surface.set_status(
                "Another editor operation is in progress",
                level="warning",
            )
            return
        tab = self._tab_surface.active_tab()
        query = self._find_surface.query()
        if tab is None or not query:
            self._find_surface.set_status("Enter text to find", level="warning")
            return
        found = tab.editor.find_literal(
            query,
            case_sensitive=self._find_surface.case_sensitive(),
            forward=forward,
        )
        selection = tab.editor.selection_bounds()
        self._find_match_tracker.record(
            tab=tab,
            query=query,
            case_sensitive=self._find_surface.case_sensitive(),
            selection=selection if found else None,
            content_version=tab.content_version,
        )
        self._find_surface.set_status(
            "Match found" if found else "No matches",
            level="success" if found else "warning",
        )

    def _replace_requested(self) -> None:
        if self._busy:
            self._find_surface.set_status(
                "Another editor operation is in progress",
                level="warning",
            )
            return
        tab = self._tab_surface.active_tab()
        query = self._find_surface.query()
        if tab is None or not query:
            self._find_surface.set_status("Enter text to find", level="warning")
            return
        selected = tab.editor.selected_text()
        case_sensitive = self._find_surface.case_sensitive()
        selection = tab.editor.selection_bounds()
        if not self._find_match_tracker.matches(
            tab=tab,
            query=query,
            case_sensitive=case_sensitive,
            selection=selection,
            content_version=tab.content_version,
        ):
            self._find_surface.set_status("Find a match first", level="warning")
            return
        same_match = (
            selected == query if case_sensitive else selected.casefold() == query.casefold()
        )
        if not same_match:
            self._find_surface.set_status("Find a match first", level="warning")
            return
        tab.editor.replace_selected_text(self._find_surface.replacement())
        self._find_surface.set_status("Replaced 1 match", level="success")

    def _replace_all_requested(self) -> None:
        self._replace_all_admission_coordinator.request()

    def _start_replace_all_job(
        self,
        job: ReplaceAllJob[_DocumentTab, ReplaceAllSession],
    ) -> None:
        """Project admission state and schedule the existing cooperative loop."""
        job.tab.editor.set_operation_locked(True)
        self._tab_surface.set_tab_bar_enabled(False)
        self._find_surface.set_operation_active(True)
        self._find_surface.set_status("Counting matches...", level="working")
        QTimer.singleShot(0, partial(self._continue_replace_all, job))

    def _continue_replace_all(
        self,
        job: ReplaceAllJob[_DocumentTab, ReplaceAllSession],
    ) -> None:
        if not self._replace_all_tracker.is_current(job):
            return
        if (
            not self._tab_surface.contains(job.tab)
            or job.tab.content_version != job.expected_content_version
        ):
            self._cancel_replace_all(
                "Replace All cancelled because the document changed",
                expected_job=job,
            )
            return
        try:
            progress = job.session.step(
                budget_ms=self._editor_policy.replace_all_slice_budget_ms,
                max_items=self._editor_policy.replace_all_items_per_slice,
            )
        except Exception as error:
            self._finish_replace_all(job, None, error)
            return
        self._replace_all_tracker.update_expected_content_version(job, job.tab.content_version)
        if progress.phase in {"completed", "limit_exceeded", "cancelled"}:
            self._finish_replace_all(job, progress, None)
            return
        self._report_replace_progress(job, progress)
        QTimer.singleShot(0, partial(self._continue_replace_all, job))

    def _report_replace_progress(
        self,
        job: ReplaceAllJob[_DocumentTab, ReplaceAllSession],
        progress: ReplaceAllProgress,
    ) -> None:
        """Throttle status updates so progress reporting cannot become the bottleneck."""
        now_ns = time.perf_counter_ns()
        if (
            job.last_report_ns == 0
            or now_ns - job.last_report_ns >= 200_000_000
            or progress.count != job.last_report_count
            and progress.count % self._editor_policy.replace_all_items_per_slice == 0
        ):
            label = "Counting" if progress.phase == "counting" else "Replacing"
            self._find_surface.set_status(
                f"{label} matches... {progress.count:,}",
                level="working",
            )
            job.last_report_ns = now_ns
            job.last_report_count = progress.count

    def _cancel_replace_all(
        self,
        message: str = "Replace All cancelled",
        *,
        expected_job: ReplaceAllJob[_DocumentTab, ReplaceAllSession] | None = None,
    ) -> None:
        job = self._replace_all_tracker.active_job
        if job is None or (
            expected_job is not None and not self._replace_all_tracker.is_current(expected_job)
        ):
            return
        try:
            progress = job.session.cancel()
        except Exception as error:
            self._finish_replace_all(job, None, error)
            return
        self._finish_replace_all(job, progress, None, message=message)

    def _finish_replace_all(
        self,
        job: ReplaceAllJob[_DocumentTab, ReplaceAllSession],
        progress: ReplaceAllProgress | None,
        error: Exception | None,
        *,
        message: str | None = None,
    ) -> None:
        self._replace_all_completion_coordinator.finish(job, progress, error, message)

    def _project_replace_all_outcome(
        self,
        job: ReplaceAllJob[_DocumentTab, ReplaceAllSession],
        progress: ReplaceAllProgress | None,
        error: Exception | None,
        message: str | None,
    ) -> None:
        if error is not None:
            restored = job.session.rollback_succeeded
            if restored and not job.was_dirty and self._tab_surface.contains(job.tab):
                self._restore_clean_state(job.tab)
            self._show_error("Replace All failed", error)
            self._find_surface.set_status(
                "Replace All failed; document was restored"
                if restored
                else "Replace All failed; document may contain partial changes",
                level="error",
            )
            return
        if progress is None:
            return
        if progress.phase == "limit_exceeded":
            self._find_surface.set_status(
                f"Stopped: more than {progress.limit:,} matches; document unchanged",
                level="warning",
            )
            return
        if progress.phase == "cancelled":
            if not job.was_dirty and self._tab_surface.contains(job.tab):
                self._restore_clean_state(job.tab)
            self._find_surface.set_status(
                message or "Replace All cancelled; document unchanged",
                level="warning",
            )
            return
        self._find_surface.set_status(
            f"Replaced {progress.count:,} matches",
            level="success",
        )

    def _restore_clean_state(self, tab: _DocumentTab) -> None:
        """Restore the original clean marker after a rolled-back operation."""
        tab.state = self._documents.mark_dirty(tab.state, False)
        tab.editor.set_modified(False)
        self._update_tab_title(tab)

    def _autosave_recovery(self) -> None:
        """Capture dirty tabs while keeping serialization and disk I/O off the UI thread."""
        self._recovery_capture_admission_coordinator.admit(self._start_recovery_capture)

    def _start_recovery_capture(
        self,
        admission: RecoveryCaptureAdmission[_DocumentTab, DocumentState],
    ) -> None:
        """Start the existing channel/editor capture pipeline for one candidate."""
        tab = admission.tab
        channel: RecoveryChunkChannel | None = None
        if self._recovery_channel_factory is not None:
            try:
                channel = self._recovery_channel_factory(
                    self._editor_policy.recovery_capture_queue_chunks,
                    self._editor_policy.recovery_capture_queue_bytes,
                )
            except Exception as error:
                name = _tab_title(tab.state, self._locale).lstrip("*")
                self.notify(
                    f"Autosave capture setup failed for {name}: {error}",
                    level="error",
                )
                return
        job = _RecoveryCaptureJob(
            tab=tab,
            session=tab.editor.begin_text_capture(sink=channel.offer if channel else None),
            snapshot_id=admission.snapshot_id,
            state_snapshot=admission.state_snapshot,
            content_version=admission.content_version,
            channel=channel,
        )
        self._recovery_state.begin_capture(admission.document_id, admission.snapshot_id, job)
        if channel is not None:
            self._recovery_state.mark_snapshot_inflight(admission.snapshot_id)
            try:
                self._submit_recovery_writer(job)
            except Exception as error:
                self._on_recovery_capture_failed(job, error, worker_started=False)
                return
        QTimer.singleShot(
            0,
            lambda document_id=admission.document_id: self._continue_recovery_capture(document_id),
        )

    def _continue_recovery_capture(self, document_id: str) -> None:
        """Read position-safe editor chunks in UI slices, then hand them to the worker."""
        job = self._recovery_state.capture_for_document(document_id)
        if job is None:
            return
        if (
            not self._tab_surface.contains(job.tab)
            or job.tab.content_version != job.content_version
            or not (job.tab.state.dirty or job.tab.editor.is_modified())
        ):
            self._cancel_recovery_capture(job)
            return
        if job.channel is not None and job.channel.state == "aborted":
            self._cancel_recovery_capture(job)
            return
        previous_progress = job.session.progress()
        try:
            progress = job.session.step(
                budget_ms=self._editor_policy.recovery_capture_slice_budget_ms,
                max_chunks=self._editor_policy.recovery_capture_chunks_per_slice,
                characters_per_chunk=self._editor_policy.recovery_capture_characters_per_chunk,
            )
        except Exception as error:
            self._on_recovery_capture_failed(job, error)
            return
        if progress.phase != "completed":
            delay_ms = 1 if progress.captured_bytes == previous_progress.captured_bytes else 0
            QTimer.singleShot(
                delay_ms,
                lambda document_id=document_id: self._continue_recovery_capture(document_id),
            )
            return
        if job.channel is not None:
            self._recovery_state.finish_capture(document_id, job)
            job.channel.finish()
            return
        try:
            chunks = job.session.chunks()
        except Exception as error:
            self._on_recovery_capture_failed(job, error)
            return
        self._recovery_state.finish_capture(document_id, job)
        self._recovery_state.mark_snapshot_inflight(job.snapshot_id)
        recovery = self._recovery
        if recovery is None:
            self._recovery_state.complete_document(document_id)
            self._recovery_state.release_snapshot(job.snapshot_id)
            return
        operation_id = self._operation_tracker.reserve()
        self._recovery_write_coordinator.submit(
            operation=lambda state=job.state_snapshot, chunks=chunks, snapshot_id=job.snapshot_id: (
                recovery.save_snapshot_chunks(state, chunks, snapshot_id=snapshot_id)
            ),
            operation_id=operation_id,
            owner=job.tab,
            content_version=job.content_version,
            snapshot_id=job.snapshot_id,
            dispatch=self._runner.submit,
        )

    def _submit_recovery_writer(self, job: _RecoveryCaptureJob) -> None:
        """Start the worker before capture so the UI can apply bounded backpressure."""
        recovery = self._recovery
        channel = job.channel
        if recovery is None or channel is None:
            raise RuntimeError("Bounded recovery writer requires recovery and a channel")
        operation_id = self._operation_tracker.reserve()

        def save_from_channel(
            state=job.state_snapshot,
            active_channel=channel,
            snapshot_id=job.snapshot_id,
        ) -> object:
            return recovery.save_snapshot_chunks(
                state,
                active_channel.consume(),
                snapshot_id=snapshot_id,
            )

        self._recovery_write_coordinator.submit(
            operation=save_from_channel,
            operation_id=operation_id,
            owner=job.tab,
            content_version=job.content_version,
            snapshot_id=job.snapshot_id,
            dispatch=self._runner.submit,
        )

    def _abort_recovery_capture(
        self,
        job: _RecoveryCaptureJob,
        reason: Exception,
        *,
        notify: bool,
        worker_started: bool = True,
    ) -> None:
        """Abort a producer and suppress its later worker callback as one unit."""
        self._recovery_capture_abort_coordinator.abort(
            job,
            reason,
            notify=notify,
            worker_started=worker_started,
        )

    def _notify_recovery_capture_failure(
        self,
        tab: _DocumentTab,
        reason: Exception,
    ) -> None:
        name = _tab_title(tab.state, self._locale).lstrip("*")
        self.notify(
            f"Autosave capture failed for {name}: {reason}",
            level="error",
        )

    def _abort_recovery_write_capture(
        self,
        job: _RecoveryCaptureJob,
        error: Exception,
    ) -> None:
        document_id = job.tab.state.document_id
        self._recovery_state.finish_capture(document_id, job)
        job.session.cancel()
        if job.channel is not None:
            job.channel.abort(error)

    def _cancel_recovery_capture(self, job: _RecoveryCaptureJob) -> None:
        """Drop a stale capture without touching the previous valid snapshot."""
        self._abort_recovery_capture(
            job,
            RecoveryCaptureCancelled("Recovery capture became stale or was cancelled"),
            notify=False,
        )

    def _on_recovery_capture_failed(
        self,
        job: _RecoveryCaptureJob,
        error: Exception,
        *,
        worker_started: bool = True,
    ) -> None:
        """Keep dirty work eligible for a later cycle after capture failure."""
        self._abort_recovery_capture(job, error, notify=True, worker_started=worker_started)

    def _schedule_recovery_scan(self, *, startup: bool) -> None:
        if self._recovery is None:
            if startup:
                self._begin_session_restore(self._session_restore.snapshot or DEFAULT_SESSION)
            else:
                self.notify("Recovery is unavailable", level="warning")
            return
        if self._recovery_scan_tracker.inflight:
            self.notify("Recovery scan already in progress", level="warning")
            return
        operation_id = self._operation_tracker.reserve()
        job = self._recovery_scan_tracker.begin(operation_id=operation_id, startup=startup)
        if job is None:
            return

        self._recovery_scan_coordinator.submit(
            operation=self._recovery.scan_candidates,
            operation_id=operation_id,
            job=job,
            dispatch=self._runner.submit,
        )

    def _begin_session_restore(self, snapshot: SessionSnapshot) -> None:
        """Start workspace and path restoration after recovery decisions finish."""
        if not self._startup_restore_inflight:
            return
        self._session_restore.begin(snapshot)
        if self._workspace is not None and snapshot.workspace_root is not None:
            self._session_restore.set_workspace_pending(True)
            self._start_workspace_open(snapshot.workspace_root, session_restore=True)
            return
        self._continue_session_restore()

    def _continue_session_restore(self) -> None:
        """Open one session path at a time and finish only after every entry is handled."""
        self._session_restore_coordinator.continue_restore()

    def _build_session_snapshot(self) -> SessionSnapshot:
        """Capture clean path-backed UI metadata without capturing document text."""
        return build_session_snapshot(
            workspace_root=self._workspace.root if self._workspace is not None else None,
            tabs=self._tab_surface.tabs,
            active_tab=self._tab_surface.active_tab(),
            path_of=lambda tab: tab.state.path,
            dirty_of=lambda tab: tab.state.dirty,
            modified_of=lambda tab: tab.editor.is_modified(),
            cursor_of=lambda tab: tab.editor.cursor_position(),
        )

    def _request_session_save(self, *, immediate: bool = False) -> None:
        """Debounce metadata changes and keep all persistence behind TaskRunner."""
        if self._session_service is None or self._startup_restore_inflight:
            return
        if immediate:
            self._session_save_timer.stop()
            self._session_save_coordinator.request_latest()
            return
        self._session_save_timer.start()

    def _save_session_snapshot(self, snapshot: SessionSnapshot) -> object:
        """Persist one coordinator-admitted snapshot through the session service."""
        session_service = self._session_service
        if session_service is None:
            raise RuntimeError("Session service is unavailable")
        return session_service.save(snapshot)

    def _show_recovery_candidates(self) -> None:
        """Schedule an explicit, non-blocking recovery inventory scan."""
        if self._startup_restore_inflight:
            self.notify("Restoring the previous session...", level="warning")
            return
        self._schedule_recovery_scan(startup=False)

    def _prompt_recovery(self, candidate: RecoveryCandidate) -> None:
        snapshot = candidate.snapshot
        if self._recovery is None:
            return
        decision = self._recovery_prompt_surface.choose(
            path=snapshot.path,
            created_at_ns=snapshot.created_at_ns,
            source_status=candidate.source_status,
        )
        if decision == "restore":
            self._restore_snapshot(snapshot)
        elif decision == "discard":
            self._discard_snapshot(snapshot)
        else:
            if snapshot.path is not None:
                self._session_restore.defer(snapshot.path)
            self.notify("Recovery snapshot kept for the next review", level="warning")

    def _restore_snapshot(self, snapshot: RecoverySnapshot) -> None:
        if self._recovery is None:
            return
        if self._tab_surface.find_by_path(snapshot.path) is not None:
            self.notify(
                "Recovery postponed because the original file is already open",
                level="warning",
            )
            return
        opened = self._recovery.restore(snapshot)
        self._add_tab(opened, recovery_snapshot_id=snapshot.snapshot_id)
        self._events.publish(DocumentOpened(opened.state))
        self.notify(
            f"Recovered {_tab_title(opened.state, self._locale).lstrip('*')}; "
            "content remains unsaved",
            level="success",
        )

    def _discard_snapshot(self, snapshot: RecoverySnapshot) -> None:
        if self._recovery is None:
            return
        self._schedule_recovery_delete(
            snapshot.snapshot_id,
            success_message="Recovery snapshot discarded; the original file was not changed",
        )

    def _clear_recovery_snapshot(self, tab: _DocumentTab) -> None:
        snapshot_id = tab.recovery_snapshot_id
        if snapshot_id is not None:
            self._schedule_recovery_delete(snapshot_id, tab=tab)

    def _clear_recovery_delete_owner(self, tab: _DocumentTab, snapshot_id: str) -> None:
        if tab.recovery_snapshot_id == snapshot_id:
            tab.recovery_snapshot_id = None

    def _schedule_pending_recovery_delete(
        self,
        snapshot_id: str,
        tab: _DocumentTab | None,
        success_message: str | None,
    ) -> None:
        self._schedule_recovery_delete(
            snapshot_id,
            tab=tab,
            success_message=success_message,
        )

    def _schedule_recovery_delete(
        self,
        snapshot_id: str,
        *,
        tab: _DocumentTab | None = None,
        success_message: str | None = None,
    ) -> None:
        if self._recovery is None:
            return
        if not self._recovery_state.request_delete(
            snapshot_id,
            tab=tab,
            success_message=success_message,
        ):
            return
        operation_id = self._operation_tracker.reserve()

        self._recovery_delete_coordinator.submit(
            operation=lambda snapshot_id=snapshot_id: self._recovery.delete_snapshot(snapshot_id),
            operation_id=operation_id,
            snapshot_id=snapshot_id,
            owner=tab,
            success_message=success_message,
            dispatch=self._runner.submit,
        )

    def _close_current_tab(self) -> None:
        if not self._busy:
            index = self._tab_surface.current_index
            if index >= 0:
                self._close_tab(index)

    def _close_tab(self, index: int) -> None:
        if (
            self._busy
            or self._startup_restore_inflight
            or index < 0
            or index >= self._tab_surface.count
        ):
            if self._startup_restore_inflight:
                self.notify("Restoring the previous session...", level="warning")
            return
        tab = self._tab_surface.tabs[index]
        if tab.state.dirty or tab.editor.is_modified():
            choice = self._message_surface.ask_save_before_close(
                _tab_title(tab.state, self._locale).lstrip("*")
            )
            if choice == "cancel":
                return
            if choice == "save":
                target = tab.state.path or self._file_dialog_surface.choose_save_path(None)
                if target is not None:
                    self._start_save(tab, target, lambda tab=tab: self._remove_tab(tab))
                return
        self._remove_tab(tab)

    def _remove_tab(self, tab: _DocumentTab) -> None:
        self._document_tab_removal_coordinator.remove(tab)

    def _begin_operation(self, message: str) -> int:
        operation_id = self._operation_tracker.begin()
        self._busy = True
        self._status_surface.set_phase("working")
        self.notify(message, level="info")
        return operation_id

    def _complete_operation(self, operation_id: int) -> bool:
        if not self._operation_tracker.complete(operation_id):
            return False
        self._busy = False
        self._sync_status_surface()
        if self._pending_startup_paths:
            QTimer.singleShot(0, self._drain_startup_paths)
        return True

    def _on_runner_pending_changed(self, _pending_count: int) -> None:
        """Project every retained worker, including queued completion delivery."""
        self._sync_status_surface()

    def _update_tab_title(self, tab: _DocumentTab) -> None:
        self._tab_surface.set_title(tab, _tab_title(tab.state, self._locale))
        self._tab_surface.set_modified(tab, tab.state.dirty or tab.editor.is_modified())

    def _refresh_tab_titles(self) -> None:
        """Reproject pathless titles after a locale change without renaming files."""
        for tab in self._tab_surface.tabs:
            self._tab_surface.set_title(tab, _tab_title(tab.state, self._locale))

    def _on_current_tab_changed(self, _index: int) -> None:
        self._current_document_transition_coordinator.project()

    def _sync_status_surface(self) -> None:
        """Keep background work ahead of dirty-document attention in the phase map."""
        tab = self._tab_surface.active_tab()
        self._status_surface.set_phase(
            self._status_phase_coordinator.project(
                StatusPhaseInput(
                    busy=self._busy,
                    pending_work=self._runner.has_pending_work(),
                    active_document_dirty=(
                        tab is not None and (tab.state.dirty or tab.editor.is_modified())
                    ),
                )
            )
        )

    def _invalidate_find_match(self) -> None:
        self._find_match_tracker.clear()

    def _show_error(self, title: str, message: str | Exception) -> None:
        self._status_surface.set_phase("error")
        self.notify(title, level="error")
        self._message_surface.show_error(title, message)
        self._sync_status_surface()

    def stop_background_activity(self) -> None:
        """Stop periodic recovery and session-save activity at lifecycle end."""
        self._stop_close_timers()

    def _stop_close_timers(self) -> None:
        self._recovery_timer.stop()
        self._session_save_timer.stop()

    def _project_close_guard_block(self, decision: CloseGuardDecision) -> None:
        self._close_guard_feedback_coordinator.project(decision)

    def closeEvent(self, event: QCloseEvent) -> None:
        decision = self._close_guard_coordinator.evaluate()
        if not decision.allowed:
            self._project_close_guard_block(decision)
            event.ignore()
            return
        event.accept()


def _tab_title(state: DocumentState, locale: Locale) -> str:
    title = state.path.name if state.path is not None else tr("document.untitled", locale)
    return f"*{title}" if state.dirty else title


def _language_for_path(path: Path | None) -> str | None:
    if path is not None and path.suffix.lower() == ".py":
        return "python"
    return None

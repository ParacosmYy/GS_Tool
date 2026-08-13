# QuillForge architecture

## Dependency direction

```text
Presentation ──> Application ──> Domain
      │               │
      └──────> Ports <┘
                    ▲
             Infrastructure

Plugins ──> Public plugin API ──> Application commands and events
```

Dependencies point inward toward stable contracts. Infrastructure and Qt-specific code must not leak into the domain model.

## Cohesion and coupling rules

Each module has one primary reason to change: domain value objects and invariants, application use cases and ports, infrastructure adapters, presentation projections, or plugin contracts/lifecycle. Cross-layer behavior crosses an explicit port or event/command contract; it does not reach through a widget, service container, or concrete adapter.

The check script enforces the highest-value import boundaries for the stable inner layers. A feature is not considered complete until its dependency direction, ownership, and extension seam are recorded in the change review.

D315 / UI-133 / ARCH-285 keeps checked-indicator readability in the existing
`presentation.theme._stylesheet()` owner. It reuses the readable-edge helper
for the native QCheckBox indicator and binds checked, checked-hover, focused,
and checked-disabled surfaces to existing theme tokens; Settings behavior
toggles inherit the same semantic rule without a widget-local adapter, signal,
state, persistence, or application boundary change.

D314 / UI-132 / ARCH-284 keeps combo-box open-state styling in the existing
`presentation.theme._stylesheet()` owner. It explicitly positions the native
`QComboBox::drop-down` subcontrol at the border's top-right and binds its `:on`
surface to the shared pressed/accent tokens, while preserving the native arrow,
popup, item data, keyboard, settings, locale, and typography boundaries. The
source audit covers the 3-theme/4-accent expanded-state contrast matrix.

D313 / UI-131 / ARCH-283 keeps typography-size stepper styling in the existing
`presentation.theme._stylesheet()` owner and scopes it to the existing
`typographyChoice` role. It uses Qt's native QSpinBox up/down subcontrols with
shared surface, border, pressed, and readable text tokens; no value range,
keyboard, persistence, settings schema, locale, editor, application, or
startup boundary changes. Native arrow painting and DPI remain verification
limits.

D312 / UI-130 / ARCH-282 keeps vertical and horizontal scrollbar styling in
the existing `presentation.theme._stylesheet()` owner. It derives normal,
hover, and pressed handles from shared border/accent tokens with readable
fallbacks on `surface_1` and `surface_2`, and changes no scrollbar model,
editor adapter, widget callback, application, locale, persistence, or startup
boundary. Native range and input semantics remain owned by Qt; native painting
and DPI remain verification limits.

D311 / UI-129 / ARCH-281 keeps shell separator styling inside the existing
`presentation.theme._stylesheet()` owner. It derives one readable normal edge
from the shared border token across `surface_0`–`surface_2`, falls back to
`text_primary` when required, and uses the existing `accent_alt` only for
hover feedback on both `QMainWindow` and `QDockWidget` separators. No widget,
layout, editor, application, locale, persistence, or startup boundary changes.
Native QSS rendering and alternate style behavior remain verification limits.

D310 / ARCH-280 keeps safe startup as an explicit boolean at the existing
`DesktopRuntime` boundary. The application dispatcher removes `--safe-mode`
before Qt argument parsing; the composition root selects immutable default
settings, skips built-in plugin activation and session/recovery restore, then
creates an initial document while preserving explicit startup paths. Normal
startup retains its existing order. No settings/session schema or second
composition root is introduced.

D309 / UI-128 / ARCH-279 keeps Tooltip styling inside the existing centralized
`_stylesheet()` presentation owner. The rule reuses `surface_3`,
`text_primary`, and existing border tokens, derives a readable `accent_alt`
edge with a `text_primary` fallback, and preserves the shared UI font family
with only a bounded relative size/weight. No widget, locale, service,
business, or persistence boundary changes. Native Tooltip painting,
accessibility, DPI, and release behavior remain explicit verification limits.

D291 / ARCH-261 keeps the normal desktop entrypoint lifecycle in one explicit
cleanup boundary. `app.main()` constructs the runtime before the boundary,
then performs `runtime.start()` and `application.exec()` in the `try` body and
always calls `runtime.stop()` in `finally`. Composition failures remain owned
by the existing early startup-failure reporter because no runtime exists yet;
startup/restore/show/path-queue failures after construction now retain the
plugin-manager cleanup path. The AST audit checks this exact `main()` control
flow without introducing a second lifecycle abstraction. Native startup and
clean-machine evidence remain open.

D292 / ARCH-262 gives that runtime cleanup boundary one explicit presentation
port. `DesktopRuntime.stop()` calls `MainWindow.stop_background_activity()`
before plugin deactivation, and the port stops only the recovery and
debounced-session timers through the existing `_stop_close_timers()` helper.
Composition does not wait for or terminate `TaskRunner` callbacks; queued work
retains its existing Qt ownership and event-loop semantics. The shutdown AST
contract guards the order and port wiring. Native worker teardown remains open.

D293 / ARCH-263 closes a narrower TaskRunner bookkeeping hole without changing
worker ownership. `TaskRunner.submit()` still retains a task before connecting
and starting it, but now rolls that retention back when either synchronous Qt
operation raises. Normal completion and submission rollback share the private
idempotent `_release_task()` boundary, so a late completion after a partially
accepted start cannot emit a duplicate pending transition. The presentation
audit guards this contract; forceful worker termination, runtime quiescing, and
native thread timing remain separate limits.

D294 / ARCH-264 keeps the startup diagnostic inside one explicit Qt application
lifetime. `_run_startup_diagnostic()` acquires one `QApplication` before its
Qt-heavy probes and releases it in `finally` only when it created that instance;
`_startup_runtime_composition()` and `_startup_restore_preflight()` reuse the
existing instance. This removes the reproduced duplicate
`Qt6111ThemeChangeObserverWindow` registration warning without changing the
normal `app.main()` entrypoint or adding a global application singleton.

D295 / ARCH-265 closes the remaining TaskRunner false-success path for
non-`Exception` worker termination. `_Task.run()` converts only the abnormal
`BaseException` branch into a private `RuntimeError` wrapper and continues
through the existing `Exception` failure callback; ordinary exceptions,
completion signaling, callback ordering, and task release remain unchanged.
The coordinator API stays narrow, while direct abnormal injection and native
thread timing remain explicit verification limits.

D296 / ARCH-266 binds frozen Qt plugin discovery to the extracted application
bundle before any `QApplication` import. `_configure_frozen_qt_plugins()` is a
single application-entry helper, a no-op for source execution, and supports the
current PyQt6 `Qt6/plugins` layout plus the legacy `Qt/plugins` fallback. It
overrides both plugin environment paths only for frozen execution, preventing a
machine-level Qt path from redirecting the portable candidate's `qwindows.dll`.
The targeted audit guards ordering and assignments; native startup and
clean-machine behavior remain explicit limits.

D297 / ARCH-267 adds a frozen-only fail-fast runtime preflight after explicit
plugin-host/diagnostic dispatch and before normal `QApplication` import. It
reuses the existing Qt platform-plugin and runtime dependency inventories and
raises one actionable `RuntimeError` when bundle files are absent, allowing the
existing `__main__` startup fallback to display and log the missing paths.
Source execution, diagnostics, plugin-host behavior, and composition remain
unchanged; native loader behavior and clean-machine startup remain explicit
limits.

D305 / UI-124 / ARCH-275 keeps localized Settings accessibility metadata in
the existing presentation boundary. `SettingsDialog._refresh_accessible_names()`
maps the nine value controls to their translated labels and the three behavior
controls to their translated text; `set_locale()` calls it after visible text
projection and before preview projection. The AST audit owns the exact 9+3
mapping and ordering contract. No accessibility model, translation service,
settings field, persistence path, or application-level dependency is added.
Native screen-reader behavior and clean-machine accessibility remain explicit
limits.

D308 / UI-127 / ARCH-278 keeps Workspace open-action discoverability in the
existing presentation boundary. `WorkspacePanel` assigns semantic
`workspaceRole` properties to the folder picker, document picker, parent
navigation, and cancel actions. One local accessibility helper refreshes
translated labels, tooltips, and accessible descriptions at `set_locale()`;
centralized QSS adds distinct folder/document edge tokens. Existing signals,
tree activation, native file picker, containment guard, asynchronous open
admission, and application ownership remain unchanged. Native screen-reader
behavior and clean-machine accessibility remain explicit limits.

D307 / UI-126 / ARCH-277 keeps Settings draft-state feedback inside the existing
presentation boundary. `SettingsDialog` captures the immutable-at-open
`SettingsSnapshot`, compares it with the current draft after preview and
behavior-control changes, and projects localized clean/changed status through
one `settingsDraftStatus` label and `draftState` property. The status reuses
the existing locale boundary, accessibility projection, and centralized QSS;
it never persists, restores, accepts, rejects, or adds a service dependency.
Native rendering, screen-reader output, and clean-machine behavior remain
explicit limits.

D306 / UI-125 / ARCH-276 keeps Settings default recovery inside the existing
presentation boundary. `SettingsDialog._restore_defaults()` consumes the
immutable application-owned `DEFAULT_SETTINGS`, blocks the 12 widget signals
for one batch projection, refreshes palette icons, and reuses `set_locale()`
and preview refresh. `RestoreDefaults` never calls Save, accept, reject, or a
persistence service; the dialog's existing Save/Cancel action boundaries stay
authoritative. No new service, port, schema field, animation policy, or
MainWindow dependency is introduced. Native button order, screen-reader
behavior, and clean-machine accessibility remain explicit limits.

D304 / UI-123 / ARCH-274 keeps the Settings language selector behind one
semantic `localeChoice` role. `SettingsDialog` assigns the presentation-only
role before language item population; `presentation.theme` owns one
normal/hover/focus/open/disabled QSS contract, and the existing pure token
resolver chooses a 3:1-safe edge across locale state surfaces. Item data,
`currentData()`, `currentIndexChanged`, `set_locale()`, labels, snapshot,
persistence, keyboard behavior, and application ownership remain unchanged.
Native polish, accessibility, clean-machine behavior, and release gates remain
explicit limits.

D303 / UI-122 / ARCH-273 keeps Settings behavior-toggle styling behind one
semantic role and two presentation tones. `SettingsDialog` assigns
`settingsRole="behaviorToggle"` plus `settingsTone="editor"` or
`"interface"` to the three existing checkboxes before their checked values are
configured. `presentation.theme` owns one normal/hover/focus/checked/disabled
QSS contract, while `presentation.theme_tokens` resolves a preferred edge
across every state surface with the 3:1 non-text floor. Existing checked
values, labels, signals, `SettingsSnapshot`, persistence, keyboard behavior,
and application ownership remain unchanged. Native polish, accessibility,
clean-machine behavior, and release gates remain explicit limits.

D302 / UI-121 / ARCH-272 keeps Settings typography styling behind one semantic
role and two presentation tones. `SettingsDialog` assigns
`settingsRole="typographyChoice"` plus `settingsTone="interface"` or
`"editor"` to the six existing font family/size/style controls before their
existing value configuration. `presentation.theme` owns one shared
normal/hover/focus/open/disabled QSS contract and two independent tone edges;
`presentation.theme_tokens.readable_edge_foreground()` resolves a preferred
accent or the existing accent-gold fallback against the pressed surface with
the 3:1 non-text floor. The audit checks declaration order, complete
tone-specific selector/value pairs, no combined cascade selector, and removal
of legacy active typography IDs. Settings values, previews, signals,
`SettingsSnapshot`, locale, persistence, keyboard behavior, and application
ownership remain unchanged. Native polish, font fallback, accessibility, and
clean-machine behavior remain explicit limits.

D301 / UI-120 / ARCH-271 keeps Settings identity-choice styling behind one
semantic presentation role. `SettingsDialog` assigns
`settingsRole="identityChoice"` to the existing `settingsTheme` and
`settingsAccent` controls before item population; `presentation.theme` owns
the normal, hover, focus, and open-menu QSS projection through one dynamic
property selector. The existing ID-based disabled rule remains deliberately
shared for compatibility. The audit now has one canonical state-table helper
plus property-order and legacy-selector checks, so D300's concrete selector
assertions no longer duplicate or drift. Item/data/icon/signal/snapshot,
locale, persistence, and application ownership remain unchanged. Qt dynamic
property and stylesheet documentation is an engineering reference; native
polishing, accessibility, and clean-machine behavior remain explicit limits.

D300 / UI-119 / ARCH-270 keeps Settings theme/accent choice hierarchy inside
the centralized `presentation.theme` QSS owner. The existing
`settingsTheme`/`settingsAccent` selectors receive a distinct `surface_3`
closed state, a three-pixel semantic accent edge, and explicit hover, focus,
and open-menu states. SettingsDialog item/data/signal/snapshot ownership,
locale, persistence, and application wiring do not move. Qt stylesheet
documentation is an engineering reference; native painting, focus metrics,
and clean-machine behavior remain explicit limits.

D299 / ARCH-269 keeps frozen Qt plugin-root selection behind one application-
entry helper. `_frozen_qt_plugin_root()` checks the Qt6 and legacy plugin roots
in order, but prefers the first root containing `platforms/qwindows.dll`; both
frozen configuration and startup diagnostics reuse it. If neither root is
complete, the existing directory fallback preserves deterministic actionable
preflight reporting. The targeted audit now uses AST function scopes to guard
ordering and reuse instead of a global source substring search. A physical
split-layout bundle, native loader behavior, and clean-machine startup remain
explicit limits.

D298 / ARCH-268 keeps the Qt DLL inventory aligned with PyInstaller's Qt6-first
and legacy Qt layout fallback. `_startup_qt_runtime_dependencies()` now tests
`PyQt6/Qt6/bin` and then `PyQt6/Qt/bin`, returning the first complete candidate
while preserving the existing diagnostic shape. The D297 preflight and startup
diagnostic therefore cannot disagree merely because a supported frozen layout
is older. A physical legacy bundle and native loader behavior remain explicit
limits.

The presentation contract audit also guards localization at the existing
notification boundary. D270 / ARCH-243 scans only static `notify(...)` string
literals and asks the existing Qt-free `localize_message` projection whether
Simplified Chinese changes them. Dynamic diagnostics remain runtime-owned;
the audit does not introduce a second catalog or mutate application state.

D273 / ARCH-244 extends the same development-time boundary to visual tokens:
it resolves the canonical theme/accent matrix and checks semantic text
foregrounds against the normal-text contrast floor. `theme_tokens.py` remains
the only owner of readable-foreground fallback decisions; the audit only
detects regressions.

D275 / ARCH-245 extends that ownership to the derived foregrounds used by the
 actual QSS state surfaces. `qss_foreground_tokens()` is framework-neutral and
 resolves accent-alt text/fill, selection, warning, success, working, and error
 foregrounds once. `theme.py` consumes those values for QSS and palette
 projection, while the audit consumes the same object for its regression gate;
no widget or audit module owns a second color decision.

D276 / ARCH-246 records the file-open intent boundary in the same Qt-free
presentation audit. `WorkspacePanel` owns only file/directory semantic signals;
`WorkspaceSurface` routes them, `WorkspaceFileActivationCoordinator` owns
containment/busy/tab admission, and `DocumentOpenAdmissionCoordinator` owns
the asynchronous open dispatch. The audit checks those seams without importing
Qt or moving filesystem/document policy into the view.

D277 / ARCH-247 records the settings/typography boundary in the same audit.
The application settings service/store own validation and persistence;
`SettingsDialog` owns input projection; `SettingsSaveProjectionCoordinator`
owns observable ordering; `theme.py` and `EditorDocumentSurface` own runtime
projection. The audit checks those owners without introducing a second settings
policy or coupling the persistence layer to Qt.

D278 / ARCH-248 records the startup-surface boundary in the same audit.
`MainWindow` owns composition order and supplies the locale provider;
`CommandSurface` owns menu/toolbar projection and resolves the provider through
one `_locale()` accessor. The audit guards that provider/accessor seam and the
four initialization anchors without constructing Qt or moving locale policy
into the presentation surface.

D279 / ARCH-249 keeps platform font discovery in `SettingsDialog`, where Qt
 runtime state is already available. The derived installed/fallback/unknown
 status is localized at the presentation boundary; `SettingsSnapshot`, the
 application settings validator, and the JSON store continue to carry only
 normalized family values. The audit checks both the projection seam and the
 raw-value persistence invariant.

D280 / ARCH-250 keeps startup settings diagnosis in the application entry-point
boundary. `_startup_settings_preflight()` reuses the infrastructure store for
one read and the application normalizer for the final schema projection; it
returns only path/presence/validity/schema metadata to the no-window report.
The diagnostic does not write settings, construct Qt, duplicate normalization
rules, or move persistence policy into presentation code.

D281 / ARCH-251 keeps user guidance outside the runtime layers. `README.md`
describes the existing entry-point contract and evidence limits but does not
add a wrapper, duplicate a diagnostic, or claim native startup. The executable
and source candidate identity therefore remains owned by D280.

D282 / ARCH-252 keeps the initial busy-state invariant in `MainWindow`, where
operation admission already owns it. The default is assigned before callback
wiring; the presentation audit inspects only `MainWindow.__init__` and allows
later runtime transitions. No coordinator, port, or infrastructure adapter
owns a duplicate busy state.

D283 / ARCH-253 keeps startup-error file lifecycle at the entry boundary. The
entry point owns the fixed app-local diagnostic path and clears only the prior
file before dispatch; `_record_startup_failure()` remains the sole writer.
Settings, recovery, session, and arbitrary user paths stay outside this
boundary.

D284 / ARCH-254 keeps complete startup composition diagnosis at the application
entry boundary while reusing the composition root. `_startup_runtime_composition`
owns only a temporary QApplication lifetime and delegates object construction
to `build_desktop_runtime`; it never starts plugins, shows the window, or
enters the event loop. The static audit guards this diagnostic seam without
duplicating MainWindow or runtime policy.

D285 / ARCH-255 keeps the pre-show startup stages in `DesktopRuntime`. The
runtime owns capability binding/plugin activation and command refresh, and
both normal startup and the no-window probe call those stages. Session and
recovery restoration remain a separate asynchronous stage, so the diagnostic
does not create queued startup work or need to own its completion policy.

D286 / ARCH-256 keeps editor-shell construction in `MainWindow` and reuses its
existing `ensure_initial_document()` path for the diagnostic-only preflight.
Timer cleanup stays with `MainWindow`, which owns the recovery and session-save
QTimers. The normal session/recovery restore path remains the sole owner of
initial-document policy during real startup.

D287 / ARCH-257 keeps user-state diagnosis at the application entry-point
boundary. `_startup_session_preflight()` reuses `SessionService.load()` so the
report observes the production session normalization contract, while
`_startup_recovery_preflight()` reuses `JsonRecoverySnapshotStore` so malformed
recovery manifests are visible without exposing snapshot text or document path
lists. The probes are read-only and diagnostic-only; asynchronous recovery and
session restore remain owned by `MainWindow` during normal startup.

D288 / ARCH-258 keeps asynchronous restore lifecycle ownership in `MainWindow`.
`preflight_startup_restore()` invokes the existing production restore chain,
waits only by processing Qt events under a bounded timeout, and stops the
recovery/session-save timers in a `finally` block. `DesktopRuntime` exposes only
the diagnostic seam; normal `start()` ordering is unchanged. The application
entry point skips this probe when valid recovery candidates would require an
explicit modal user decision, preserving the no-window boundary.

D289 / ARCH-259 keeps explicit file-open diagnosis on the existing startup-path
boundary. `DesktopRuntime.preflight_startup_paths()` preserves the normal
restore → command refresh → queued path order, while `MainWindow` reuses
`open_startup_paths()` and the shared bounded completion wait. The entry point
only validates the requested regular file and reports the result; it does not
create a second document service or file-opening worker.

D290 / ARCH-260 keeps the diagnostic completion wait in `MainWindow` while
removing avoidable CPU busy-spin. An unparented, short-lived 2ms `QTimer`
supplies a bounded wake-up and `QEventLoop.WaitForMoreEvents` lets the calling
thread sleep when the queue is empty; the existing monotonic deadline,
worker-drain conditions, timer cleanup, report fields, and no-window/no-`exec()`
boundary remain unchanged. The report labels the timeout as `soft` because a
single Qt event handler cannot be interrupted. Normal `DesktopRuntime.start()`
does not use this diagnostic helper.

## Layers

| Layer | Responsibility | Must not own |
|---|---|---|
| `domain` | document identity, state, value objects, invariants | Qt widgets, filesystem calls |
| `application` | commands, use cases, ports, orchestration | widget details, concrete database/file APIs |
| `presentation` | windows, views, actions, view models, Qt wiring | persistence policy, plugin internals |
| `infrastructure` | filesystem, settings, logging, process and OS adapters | product decisions and UI state |
| `plugins` | stable public contracts and plugin lifecycle | private widget implementation |

## Editor engine boundary

The application-facing presentation code talks to an `EditorEngine` abstraction. The first implementation wraps QScintilla, but application services must use operations such as:

- get and set document text;
- query modified state;
- apply a language hint;
- perform bounded editor-local actions such as undo, redo, clipboard operations, and select-all.

Command ownership belongs to `CommandRegistry`; Qt signals remain an adapter concern. `EditorWidget` is UI-thread-owned and is never passed to a worker or plugin.

This keeps a future custom engine or alternative control from becoming a system-wide rewrite.

## Visual system boundary

`quillforge.presentation.theme_tokens` is the framework-neutral visual-token
source. It owns immutable `ThemeColors`/`EditorColorTokens`, bounded
theme/accent resolution, and contrast helpers without importing Qt.
`quillforge.presentation.theme` remains the single generated-QSS, Qt-palette,
icon, and editor-adapter projection owner. `ThemeColors` derives readable
foregrounds for filled accent, pink, and gold endpoints; semantic selectors such as `primaryAction`,
`warningAction`, `documentTabs`, `commandBar`, and `statusMessage` consume those
tokens without owning behavior. `StatusSurface` projects transient `info`,
`success`, `warning`, and `error` levels through the same centralized QSS
source; it does not infer policy from text. `theme.py` keeps its existing
imports and private diagnostic names as a compatibility façade during this
increment. D183 scopes the only surface gradients to the main window, editor
shell, and command rail while keeping the editor canvas solid. ADR-0054 records the D29 contrast
and interaction-hierarchy refinement, and ADR-0056 records the D31 notification
hierarchy; runtime rendering, fonts, DPI, and native style metrics remain
explicit acceptance limits.

## Accessibility localization boundary

Stable shell accessibility names are projected by the same presentation locale
path as their visible counterparts. `StatusRail` owns the rail's names and
dynamic phase description; `StatusSurface` owns the notification label;
`presentation.i18n` owns the English/Simplified Chinese keys. No application
coordinator stores translated text or depends on Qt accessibility objects.
Native screen-reader behavior remains an explicit acceptance limit.

## Settings typography boundary

Font style is a versioned preference, not a widget-owned incidental value.
`domain.models.FontStyle` is the Qt-free `regular`/`semibold`/`bold`/`italic`
contract. `application.settings` owns schema-v3 normalization and legacy
fallbacks, while `infrastructure.settings_store` owns only the compatible JSON
round-trip. `SettingsDialog` and `SettingsPreviewSurface` project pending
choices, and `presentation.font_style` is the sole shared Qt mapping for
QFont/QSS weight and italic tokens. `EditorWidget` applies the validated style
through its existing adapter boundary; the settings-save coordinator still
owns the existing projection order. Native font fallback and metrics remain
explicit acceptance limits.

## Composition root

`quillforge.app` is the entry-point dispatcher and Qt event-loop owner. The
top-level `quillforge.composition` module is the desktop composition root: it
selects concrete infrastructure adapters, creates application services,
explicitly registers trusted built-in plugins, and returns a `DesktopRuntime`
that owns the `MainWindow`/plugin lifecycle. The separate
`quillforge.diagnostic_composition` module assembles Qt-free diagnostic
adapters, so diagnostic mode does not pull the desktop runtime into the
process. `DesktopRuntime.start()` preserves the existing activation →
recovery/session restore → menu refresh → show order; `stop()` deactivates
plugins after the event loop returns.

Within presentation, `quillforge.presentation.command_surface.CommandSurface`
owns the menu and command-rail widget projection. It consumes the
application-owned `CommandRegistry` and an explicit callback/locale boundary;
it does not register product behavior, access infrastructure, or expose Qt
objects to application services. `MainWindow.refresh_command_menus()` remains
the lifecycle seam used after plugin registration or lifecycle changes. ADR-0037
records the first MainWindow coordinator extraction. The adjacent
`quillforge.presentation.document_tab_surface.DocumentTabSurface` owns the
document-tab `QTabWidget`, identity-based record/index mapping, active-tab and
editor lookup, title projection, and tab-bar enablement. MainWindow keeps
document state transitions, path uniqueness, save/close confirmation,
recovery/session bookkeeping, and lifecycle callbacks; ADR-0038 records this
next bounded extraction. The
`quillforge.presentation.workspace_surface.WorkspaceSurface` similarly owns
the workspace dock/panel composition, five semantic panel-signal routes, and
dock/panel locale projection. MainWindow retains workspace service calls,
generation/operation/cancellation guards, containment checks, search/session
coordination, and error policy; ADR-0039 records the D14 boundary. The
`quillforge.presentation.workspace_search_surface.WorkspaceSearchSurface`
owns workspace-search dialog construction, its three semantic signal routes,
activation, locale, and result-state projection. MainWindow retains query
validation, `WorkspaceSearchService`/`TaskRunner` lifecycle, stale-generation
rejection, cancellation, workspace containment, document activation, and
notification policy; ADR-0040 records the D15 boundary. The workspace surface
also exposes only semantic current-path/loading/directory/error projection,
so MainWindow no longer imports or reaches into `WorkspacePanel`; ADR-0041
records the D16 closure. The
`quillforge.presentation.find_surface.FindSurface` similarly owns FindBar
composition, six semantic routes, shell placement, mode/locale/status and
operation projection; MainWindow retains active-editor lookup, literal
find/replace, Replace All sessions, rollback/cancellation, tab locking, and
error policy. ADR-0042 records the D17 boundary.

The `quillforge.presentation.settings_surface.SettingsSurface` similarly owns
`SettingsDialog` parentage and modal snapshot editing. MainWindow retains
settings-service availability, save-in-flight and TaskRunner policy, result
validation, theme/locale/editor projection, motion policy and transition
trigger timing, notifications, and errors; ADR-0043 records the D18 boundary.

The `quillforge.presentation.recovery_prompt_surface.RecoveryPromptSurface`
similarly owns recovery prompt parentage, locale-aware text, button roles, and
the typed restore/discard/later decision. MainWindow retains RecoveryService,
snapshot restore/discard, deferred session paths, document events, cleanup, and
notification policy; ADR-0044 records the D19 boundary.

The `quillforge.presentation.status_surface.StatusSurface` similarly owns
StatusRail creation, status-bar widget exposure, locale, and phase projection.
MainWindow retains busy/TaskRunner/dirty-document precedence, operation and
error policy, and close guards; ADR-0045 records the D20 boundary.
The `quillforge.presentation.file_dialog_surface.FileDialogSurface` similarly
owns localized native file, folder, and save-path selection. MainWindow retains
startup/busy guards, asynchronous document/workspace dispatch, save policy,
session/recovery behavior, and path-policy ownership; ADR-0046 records the D21
boundary. The surface makes the file-vs-folder distinction explicit without
turning the UI filter into a security or containment boundary.
The `quillforge.presentation.command_palette_surface.CommandPaletteSurface`
similarly owns command-palette modal composition, locale, parentage, and
Accepted/cancelled stable-ID projection. MainWindow re-resolves the ID through
the live registry and retains command execution, stale-ID feedback, and menu
refresh; ADR-0047 records the D22 boundary.
The `quillforge.presentation.plugin_surface.PluginSurface` similarly owns
extension-catalog and plugin-status dialog lifecycle, locale, activation, and
semantic governance/toggle routes. MainWindow retains plugin services,
TaskRunner and operation state, trust/approval/enablement policy, result
validation, notifications, and errors; ADR-0048 records the D23 boundary.
The `quillforge.presentation.message_surface.MessageSurface` similarly owns
localized save-before-close, About, and recoverable error QMessageBox
composition plus the typed Save/Discard/Cancel result. MainWindow retains
dirty-state, Save As, asynchronous save, tab removal, error-phase, and status
policy; ADR-0049 records the D24 boundary.
The `quillforge.presentation.theme_transition_surface.ThemeTransitionSurface`
similarly owns the short-lived opacity effect, easing, animation lifetime, and
owned-effect cleanup. MainWindow retains motion gating, central-widget choice,
and post-settings trigger timing; an existing external graphics effect is left
untouched; ADR-0050 records the D25 boundary.
The `quillforge.presentation.status_surface.StatusSurface` also owns
`QStatusBar` host attachment, size-grip configuration, and localized transient
notification projection. MainWindow retains phase precedence, TaskRunner and
document policy, and notification call sites; ADR-0051 records the D26
boundary. The surface now also owns the styled transient-message widget,
timeout, locale refresh, and explicit `info`/`success`/`warning`/`error` state;
MainWindow's one-argument `notify(message)` seam remains compatible and its
optional level is presentation metadata; ADR-0056 records the D31 refinement.
The `quillforge.presentation.editor_shell_surface.EditorShellSurface` owns
the central `editorShell` QWidget/layout, `DocumentTabSurface` and `FindSurface`
composition, initial FindBar hidden state, and FindBar locale routing.
MainWindow retains callbacks, document/editor state, operation policy, and
lifecycle; ADR-0052 records the D27 boundary.
The `quillforge.presentation.editor_document_surface.EditorDocumentSurface`
owns per-document `EditorWidget` creation, presentation-safe editor settings
and theme application, language-hint refresh after Save As, and modified,
content, and caret signal routing. MainWindow retains `_DocumentTab`, document
state, save/recovery/Replace All policy, and operation locking; ADR-0053
records the D28 boundary.

The `quillforge.presentation.icons` module is the single authored vector-icon
provider for shell navigation. `IconKey` is presentation metadata only;
`CommandSurface` owns command-rail icon projection and
`WorkspaceSurface`/`WorkspacePanel` own workspace-entry and navigation icon
projection. Theme refresh uses explicit visual methods and does not alter
command, document, filesystem, or locale policy; ADR-0055 records the D30
boundary.

`StatusSurface` keeps transient notification projection in the same
presentation boundary. Its `statusMessage` widget owns localization, timeout,
and explicit `info`/`success`/`warning`/`error` styling through centralized
theme tokens; MainWindow retains notification text and operation/error policy.
The optional level on `notify(message)` is source-compatible with existing
plugin callbacks; ADR-0056 records the D31 refinement.

Inline workspace feedback uses the same presentation-only state projection.
`presentation.feedback.apply_feedback_state` is the single property/repolish
seam for `info`, `working`, `success`, `warning`, and `error`; `WorkspacePanel`
and `WorkspaceSearchDialog` retain their existing localized text, result,
loading, cancellation, and error ownership while centralized QSS supplies
their visual hierarchy. FindBar status mapping remains a separate D33 slice;
ADR-0057 records this bounded D32a boundary.

FindBar status projection now reuses that seam through the existing
`FindSurface`. `FindBar` owns only raw localized status text and the current
presentation level; MainWindow supplies the level at each existing find,
replace, and Replace All outcome while retaining editor, cancellation,
rollback, operation, and error policy. ADR-0058 records the D33 boundary.

Session and Recovery outcomes use the same explicit notification contract.
MainWindow maps existing persistence, autosave, scan, restore, discard, and
cleanup results to `info`/`success`/`warning`/`error`; `SessionService`,
`RecoveryService`, `TaskRunner`, snapshot lifecycle, and close policy remain
unchanged. Invalid session persistence is `error`, and a deferred recovery
snapshot is `warning`; ADR-0059 and the D35a correction record this boundary.

Plugin and extension coordinator outcomes use the same explicit contract.
`MainWindow` maps catalog/runtime/host/governance availability, progress,
success, warning, and failure at the existing call sites while preserving
catalog trust, approval, host containment, execution-disabled policy, worker
submission, stale guards, and command refresh. No notification policy service
or second plugin state model is introduced; ADR-0060 records this bounded D35a
closure.

Workspace and long-running operation outcomes use the same explicit contract.
`_begin_operation` keeps the permanent status phase at `WORKING` and projects
its transient copy as explicit `info`; workspace/search completion,
cancellation, limit, containment, and failure outcomes carry typed
`success`/`warning`/`error` metadata at their current MainWindow call sites.
Generation, cancellation, containment, session barriers, and document open
policy remain unchanged; ADR-0061 records this bounded D36a closure.

The MainWindow notification call-site contract is now explicit end-to-end.
All 81 coordinator `notify` calls pass a legal transient level; the compatibility
default remains available to external callers. The final six calls use
success/warning/error/info according to their existing document, session,
settings, command, and tab outcomes, without introducing another policy
service or state model. Long-running progress remains a permanent `WORKING`
phase projection with an informational transient copy. ADR-0062 records this
bounded D37a closure.

The central QSS now strengthens keyboard focus visibility without introducing
another state owner. Focused command-rail/tool buttons reuse the hover surface
and primary foreground, focused document tabs add the same surface plus an
accent-alt boundary, and focused checkboxes expose both row and indicator
focus. Selected, pressed, checked, and disabled rules remain intact; ADR-0063
records this bounded D38 visual slice.

The next coordinator boundary is the framework-neutral
`presentation.operation_tracker.OperationTracker`. It owns only the
MainWindow-local monotonic operation-ID sequence and the one-current-operation
stale completion/cancellation guard. MainWindow retains the `_busy` policy,
TaskRunner submission and pending-work semantics, status phases, notifications,
workspace/session generations, service policy, and result projection. Existing
independent operation-ID callers continue through the same sequence via
`_next_operation_id()`, so the extraction does not create a second state
machine; ADR-0064 records this bounded D39 slice.

Plugin catalog scan, governance mutation, and host diagnostics now use the
Qt-free `presentation.plugin_operation_tracker.PluginOperationTracker` for
their duplicated per-domain lifecycle IDs and stale callback guards. The
three domains intentionally retain independent slots; MainWindow remains the
owner of PluginCatalog/Approval/Runtime/Host services, TaskRunner, notification
mapping, PluginSurface, command refresh, and all trust/approval/containment/
execution policy. ADR-0065 records this bounded D40 slice.

Workspace entry activation now stays inside `presentation.workspace_panel` as
one semantic item-intent helper. Existing single-click file opening and
double-click directory navigation remain intact, while Qt keyboard activation
routes files to `file_requested` and directories to `directory_requested`.
The panel still performs no filesystem or document work; MainWindow retains
containment checks, TaskRunner dispatch, document loading, workspace policy,
and notification ownership. ADR-0066 records this bounded D41/UI-27 slice.

Workspace action hierarchy remains centralized in `presentation.theme`.
`workspaceBack` and `workspaceCancel` use existing object names and theme
tokens to project explicit quiet, hover/focus, pressed, and disabled states;
the open-workspace `primaryAction` remains the primary command. No signal,
locale, loading, service, or application policy moves into the style layer.
ADR-0067 records this bounded D42/UI-28 slice.

Workspace-search lifecycle identity now lives in the Qt-free
`presentation.workspace_search_operation_tracker` boundary. It owns only
active operation ID, generation invalidation, cooperative cancellation, and
stale/current completion classification. MainWindow retains query and
service policy, TaskRunner submission, surface projection, containment,
notifications, startup/close guards, and result handling. ADR-0068 records
this bounded D43/ARCH-33 slice.

Workspace-navigation lifecycle identity now lives in the Qt-free
`presentation.workspace_operation_tracker` boundary. It owns only active
operation identity, generation allocation/invalidation, and stale/current
completion classification. MainWindow retains the generic operation tracker,
busy/status policy, TaskRunner, WorkspaceService, WorkspaceSurface,
containment, session-restore barrier, notifications, and result policy.
ADR-0069 records this bounded D44/ARCH-34 slice.

Workspace file activation remains a deliberately small semantic boundary:
WorkspacePanel routes click, double-click, and keyboard intents while
MainWindow retains busy gating, workspace containment, existing-tab identity,
and asynchronous DocumentService dispatch. ADR-0070 records this bounded
D45/ARCH-35/UI-31 correction; no new coordinator or application service was
introduced.

Workspace tree visual rhythm remains presentation-only. `WorkspacePanel`
configures standard Qt row selection/metrics, while `theme.py` projects
alternate and alternate-hover surfaces from existing tokens; semantic signals,
workspace policy, and application boundaries remain unchanged. ADR-0071 records
this bounded D46/ARCH-36/UI-32 refinement.

Session-restore value state now lives in the Qt-free
`presentation.session_restore_tracker` boundary. It owns only the immutable
snapshot reference, ordered document cursor, active path, workspace barrier,
recovery-deferred paths, and one pending document/open-operation binding.
MainWindow retains SessionService, RecoveryService, WorkspaceService,
DocumentService, TaskRunner, generic operation/busy/status state, tab
projection, notifications, startup/close guards, and result policy. ADR-0072
records this bounded D47/ARCH-37/UI-33 extraction; native callback timing,
runtime startup, and release evidence remain open.

Document-tab modified-state presentation remains behind the existing
`presentation.document_tab_surface` boundary. The authored icon provider owns
the `MODIFIED` glyph; the tab surface owns only the palette-tinted marker and
its index-aligned projection. MainWindow continues to interpret
`state.dirty or editor.is_modified()`, preserve the title asterisk, and own
save/recovery/close policy. ADR-0073 records this bounded D48/ARCH-38/UI-34
refinement; native tab rendering and release evidence remain open.

Session-save lifecycle state now lives in the Qt-free
`presentation.session_save_tracker` boundary. It owns only the saved session
baseline, latest queued snapshot, in-flight flag, positive operation binding,
and stale/invalid/valid callback classification. MainWindow retains the Qt
debounce timer, snapshot capture, startup barrier, SessionService, TaskRunner,
operation allocation, notifications, close guards, and result policy. ADR-0074
records this bounded D49/ARCH-39 extraction; native callback timing, runtime
startup, and release evidence remain open.

Recovery-capture lifecycle indexes now live in the Qt-free
`presentation.recovery_capture_tracker` boundary. It owns only opaque capture
jobs/owners, document and snapshot identity, discarded callback markers,
worker-write state, and deferred delete sequencing. MainWindow retains editor
capture slices, channel backpressure/finish/abort, RecoveryService, TaskRunner,
tab identity, notification policy, and close guards. ADR-0075 records this
bounded D50/ARCH-40 extraction; native callback timing, durability, runtime
startup, and release evidence remain open.

Settings-save callback identity now lives in the Qt-free
`presentation.settings_save_tracker` boundary. It owns only one positive
TaskRunner operation ID and stale/invalid/valid/failure classification.
MainWindow retains SettingsSurface, SettingsService, TaskRunner, persisted
snapshot application, theme/locale/font/editor projection, transition
animation, notifications, and close policy. ADR-0076 records this bounded
D51/ARCH-41 extraction; native settings interaction and release evidence
remain open.

Replace All callback identity now lives in the Qt-free
`presentation.replace_all_tracker` boundary. It owns one typed active job,
expected content-version guard, and stale finish protection; each queued
cooperative slice carries its job identity. MainWindow retains
ReplaceAllSession, QTimer scheduling, editor locking, cancellation, rollback,
dirty-marker restoration, status, notification, and operation policy.
ADR-0077 records this bounded D52/ARCH-42 extraction; native callback timing,
editor rollback, and release evidence remain open.

Find Match validation now lives in the Qt-free
`presentation.find_match_tracker` boundary. It owns only an immutable
document/query/case/selection/content-version snapshot and explicit clear or
exact-match semantics. MainWindow retains EditorWidget find/selection and
selected-text operations, busy gating, invalidation routes, replacement, and
feedback policy. ADR-0078 records this bounded D53/ARCH-43 extraction; native
selection timing and release evidence remain open.

Recovery inventory scan identity now lives in the Qt-free
`presentation.recovery_scan_tracker` boundary. It owns one immutable operation
and startup-context job and stale finish guard; MainWindow retains
RecoveryService, TaskRunner, candidate validation, recovery prompts,
session-restore continuation, notifications, and close policy. ADR-0079
records this bounded D54/ARCH-44 extraction; native callback timing and
release evidence remain open.

D210 / UI-112 / ARCH-195 keeps document name-filter policy in the existing
localized `presentation.i18n` catalog and native `FileDialogSurface` boundary.
The English and Simplified Chinese filter groups cover common text/source
extensions and retain the all-files fallback; `choose_document()`,
`choose_save_path()`, `choose_workspace()`, the async admission/open flow, and
`DocumentService` ownership do not move. Qt's public QFileDialog documentation
is the applicable first-party source; native filter rendering, platform case
sensitivity, accessibility, DPI, and release evidence remain open.

D211 / UI-113 / ARCH-196 keeps ordinary disabled workspace-row styling in the
existing centralized `presentation.theme` QSS owner. The scoped
`QTreeWidget#workspaceTree::item:disabled` selector reuses `surface_2`,
`border`, `border_strong`, and `text_muted`; the more-specific
selected-disabled selector remains authoritative. `WorkspacePanel` continues
to own disabled item admission, file/folder/Enter activation, loading,
locale, and icon projection. Qt's public Style Sheets Reference is the
applicable first-party source; native QSS/item painting, accessibility, DPI,
and release evidence remain open.

D212 / UI-114 / ARCH-197 keeps pathless document-title localization in the
existing i18n catalog and Qt-free `PresentationLocaleCoordinator` boundary.
`_tab_title` receives the active `Locale`, uses `document.untitled` only when
`DocumentState.path` is absent, and preserves saved-file basenames and dirty
markers. Existing tabs reproject through the existing
`DocumentTabSurface.set_title` seam after editor-shell locale refresh; no title
service, document mutation, or file-flow dependency is introduced. Qt's public
QTabWidget documentation is the applicable first-party source; native tab
painting, font fallback, accessibility, DPI, and release evidence remain open.

D213 / UI-115 / ARCH-198 keeps the primary editor-stage brand edge in the
existing centralized `presentation.theme` QSS owner. The scoped
`QWidget#editorShell` rule reuses the immutable `accent_pink` token and adds
only a two-pixel top boundary after the existing neutral border, preserving the
gradient, radius, editor/tab/FindBar composition, and all child-widget state
selectors. Qt's public Style Sheets Reference is the applicable first-party
source; native QSS geometry, accessibility, DPI, and release evidence remain
open.

D214 / ARCH-199 begins Phase 3 error-contract consolidation with a dependency-
free `application.errors` taxonomy. `DocumentService` and `WorkspaceService`
now classify only their application-owned validation/state failures as
`ApplicationValidationError` or `ApplicationStateError`, which retain the
existing `ValueError`/`RuntimeError` compatibility for presentation catches.
Domain `DocumentConflictError`, infrastructure adapter failures, messages,
containment, and provider behavior remain in their existing owners. Python's
public built-in exceptions documentation is the applicable first-party source;
broader service taxonomy coverage and runtime evidence remain open.

D215 / UI-116 / ARCH-200 keeps accent-endpoint foreground ownership in the
framework-neutral `presentation.theme_tokens` registry. The new immutable
`on_accent_pink` token is resolved by the existing contrast helper and consumed
only by the existing primary-action hover selector, whose background already
uses `accent_pink`. No widget-local stylesheet, action policy, settings
schema, or second color algorithm is introduced. Qt's public Style Sheets
Reference is the applicable first-party source; native rendering and release
evidence remain open.

D216 / ARCH-201 extends the existing Phase 3 application error taxonomy only
to `EditorOperationPolicy`. Its eight composition-boundary invariants now use
`ApplicationValidationError` while exact messages, defaults, `ValueError`
compatibility, dataclass shape, and presentation consumers remain unchanged.
Workspace-search, command-registry, domain-model, and presentation validation
remain separate follow-up slices. Python's public built-in exceptions
documentation is the applicable first-party source; runtime editor timing and
release evidence remain open.

D217 / UI-117 adds one non-interactive `QLabel#toolbarBrand` before the
existing command actions. `CommandSurface` owns only its toolbar projection
and locale refresh; `theme.py` remains the single QSS owner for the surface,
border, pink brand edge, and typography. No QAction, command ID, callback,
shortcut, context projection, settings state, or application policy moves.
Qt's public Style Sheets Reference is the applicable first-party source;
native rendering, accessibility, DPI, and release evidence remain open.

D218 / ARCH-202 completes the next application taxonomy boundary for
`workspace_search`. `ApplicationTypeError` preserves built-in `TypeError`
compatibility beside the existing validation/state categories, and the
workspace-search policy, value objects, request validation, and provider-result
admission now raise the matching category. The two `Path.relative_to()`
operation catches remain built-in `ValueError` catches; no search policy,
provider protocol, directory capability, result shape, or Qt dependency moves.
Python's public built-in exceptions documentation is the applicable
first-party source; runtime traversal and release evidence remain open.

D219 / ARCH-203 completes the remaining application-owned exception boundary.
`ApplicationValidationError` and `ApplicationStateError` now cover command
registration, plugin enablement/execution/governance/host contracts, session
load contracts, recovery admission, release metadata, and event-thread state.
They retain `ValueError`/`RuntimeError` compatibility and exact messages while
the dedicated recovery-channel protocol exceptions remain unchanged. Domain,
infrastructure, and presentation validators stay in their owning layers; no
Qt dependency, protocol shape, policy ordering, or runtime authority moves.
Python's public built-in exceptions documentation is the applicable
first-party source; native callbacks, runtime exchange, and release evidence
remain open.

D220 / UI-118 keeps the main-shell visual hierarchy in the existing centralized
`presentation.theme` owner. `QMainWindow#mainWindow`,
`QWidget#editorShell`, `QToolBar#commandBar`, and
`QTabBar#documentTabBar` now use a quieter surface ladder with no shell
gradients, smaller nested geometry, and a flat tab separator. No object name,
action, signal, layout, locale, font, motion, state selector, theme token
ownership, or application policy moves. Qt's public Style Sheets Reference is
the applicable first-party source; native painting, metrics, accessibility,
and release evidence remain open.

D221 / ARCH-204 extends the existing `scripts/audit_presentation_contracts.py`
AST boundary to compare literal presentation `tr()` keys with the canonical
`presentation.i18n._ENGLISH` catalog. Dynamic keys, runtime fallback, and
translation content remain outside the static gate; Python 3.12's public AST
documentation is the applicable first-party source.

D222 / ARCH-205 keeps early startup-failure handling at the executable entry
boundary in `quillforge.__main__`. A console-disabled PyInstaller candidate
now records a UTF-8 traceback under the existing user-local QuillForge data
root and attempts a native Windows message when Qt cannot yet provide a
dialog. No application, domain, infrastructure, presentation, theme,
settings, session, or plugin authority moves; actual GUI/EXE startup and
release evidence remain open. Python 3.12 standard-library APIs are the
applicable first-party reference.

D223 / ARCH-206 keeps packaged startup preflight at the existing application
dispatcher before `QApplication` construction. The `--diagnose-startup
--report <path>` contract reports source-versus-frozen execution, runtime and
Qt/QScintilla imports, composition importability, and frozen Qt/icon resource
presence with a stable JSON payload and deterministic exit status. It does not
create a window, enter the Qt event loop, or move normal GUI behavior across
layers. Python, Qt, and PyInstaller public documentation are the applicable
references; frozen runtime, native rendering, and release evidence remain open.

D225 / ARCH-207 keeps locale resolution inside the existing command-surface
presentation boundary. A single private `_locale()` accessor delegates to the
composition-owned provider so menu creation, toolbar creation, refresh, and
retranslation no longer call an undefined method. No menu/command registry,
callback, locale catalog, theme, icon, or MainWindow ownership moves; the
captured first startup exception is fixed while native runtime evidence remains
open.

D226 / ARCH-208 keeps missing private presentation-accessor detection inside
the existing `scripts/audit_presentation_contracts.py` AST boundary. Direct
`self._private()` calls in top-level presentation classes must resolve to a
class-defined method or an assigned attribute, preserving injected providers
such as `_locale_provider` without adding runtime reflection or a type-checker
dependency. This is a static typo guard, not proof of dynamic callability,
external-base behavior, native startup, or release readiness; Python 3.12's
public `ast` tooling is the applicable first-party engineering reference.

D227 / ARCH-209 keeps the same audit boundary while making its declaration
model explicit. `_class_attribute_names()` collects direct class-body
`Assign`, `AnnAssign`, and `AugAssign` names and merges them with the existing
instance-assignment discovery before private self-call validation. No broad
private-read audit, runtime callability inference, provider registry, or
presentation/application dependency is introduced; dynamic assignment,
destructuring, external-base methods, native startup, and release evidence
remain outside the proof boundary.

D228 / ARCH-210 keeps early startup failure reporting at the existing
`__main__.py` entry boundary. A fail-open helper records only fixed
executable, working-directory, frozen/runtime, and bundle-root context beside
the existing traceback, so stale-package and wrong-launch-context failures can
be distinguished without importing Qt or changing the normal GUI path. Path
privacy, native message-box rendering, clean-machine, and release evidence
remain explicit limits.

D229 / ARCH-211 keeps the same diagnostics-only ownership but closes the
remaining path-resolution failure edge. `_record_startup_failure` now treats
report-path resolution, context collection, payload construction, and writes
as fail-open stages, preserving D228's unavailable-context fallback whenever a
report can still be written. No second logger, Qt dependency, settings
contract, or application policy is introduced; native startup and release
evidence remain outside the proof boundary.

D230 / ARCH-212 keeps fallback presentation at the same `__main__.py` entry
boundary and makes its three output channels best-effort. Exception summary
stringification falls back to a stable generic message, native MessageBox
failure falls through, and stderr failure is swallowed only at this
diagnostics edge. Ordinary messages, the native title/icon request, normal
`main()` returns, and the final startup exit code remain unchanged; native
rendering and release evidence remain outside the proof boundary.

D231 / ARCH-213 closes the remaining native-fallback result edge without
changing ownership. `_show_startup_failure` returns directly only for a
nonzero `MessageBoxW` result; a zero result continues to the existing stderr
best-effort path. Ordinary message text, title/icon arguments, normal
`main()` returns, and the final startup exit code remain unchanged. Native
rendering and Win32 runtime evidence remain outside the proof boundary.

D232 / ARCH-214 keeps the same diagnostics-only ownership and moves the
`app.main` import into the existing public `main(argv)` wrapper. This places
application-module import failures inside the bottom-level startup exception
boundary while preserving package-relative/direct-source resolution, argument
dispatch, Qt construction, composition ownership, and exit behavior. No second
logger, import service, or application policy is introduced; native startup and
release evidence remain outside the proof boundary.

D233 / ARCH-215 keeps explicit desktop launch-path interpretation out of Qt and
the composition root's concrete service selection. The Qt-free
`DesktopLaunchRequest` parser retains Qt options and classifies ordered,
de-duplicated file/directory paths; `DesktopRuntime` transfers those immutable
paths through `MainWindow.open_startup_paths()`, which owns the recovery/session
barrier and reuses the existing document/workspace admission coordinators.
Missing paths are notified, multiple paths are serialized through the existing
operation completion boundary, and no second document/workspace service or OS
association owner is introduced. Native shell behavior, file associations,
clean-machine, and release evidence remain outside the proof boundary.

D235 / ARCH-216 keeps the D233 boundary and closes two local edge cases. The
Qt-free launch parser normalizes only leading-dash spelling when matching its
existing value-option set, while preserving the original Qt argument text. The
startup path drain still defers during temporary busy/session-restore states,
but emits a warning before advancing on a permanent admission rejection. No
new queue, service, registry owner, or default association policy is added;
native startup and release evidence remain outside the proof boundary.

D242 / ARCH-223 keeps file visibility policy inside the existing localized
`FileDialogSurface` boundary. The shared `dialog.text_filter` places Qt's
`All files (*)` name filter first and retains the finite text/source filter as a
secondary user choice. Open/save dialog ownership, file-vs-folder distinction,
async document admission, decoding, containment, and error policy do not move;
Qt's first-party `QFileDialog` documentation is the applicable framework
reference, while native dialog rendering and startup evidence remain outside
the proof boundary.

D264 / ARCH-242 keeps the secondary startup fallback inside the existing
Qt-free `__main__` fail-open boundary. A nested locale guard can project the
Chinese fallback without weakening the final English literal; exception,
diagnostic, MessageBox/stderr, exit-code, and normal-startup policy do not move.
Native startup and release evidence remain outside the proof boundary.

D263 / ARCH-241 keeps the workspace-search outside-root placeholder inside the
existing `WorkspaceSearchDialog` presentation projection. The relative-path
fallback uses one catalog key and is recomputed by the existing diagnostic-row
refresh after locale changes; path containment, result data, ordering,
expansion, search policy, and application ownership do not move. Native search
dialog and release evidence remain outside the proof boundary.

D262 / ARCH-240 keeps early startup fallback localization inside the Qt-free
`__main__` exception boundary. A small system-locale projection selects only
the supported `zh-CN` or `en-US` fallback labels; it does not load settings,
the presentation catalog, or Qt. Exception details, diagnostic path/log,
MessageBox/stderr fail-open behavior, exit code, and normal startup policy do
not move. Native startup and release evidence remain outside the proof
boundary.

D261 / ARCH-239 keeps the Save As default-name localization inside the existing
`FileDialogSurface` presentation boundary. When no current path exists, the
surface reuses `document.untitled` and appends `.txt`; an existing `str(current)`
path remains unchanged. Dialog title/filter, selected-path, document,
filesystem, persistence, and startup policy do not move. Native dialog and
release evidence remain outside the proof boundary.

D260 / ARCH-238 keeps Settings font-size unit localization inside the existing
presentation boundary. `SettingsDialog.set_locale` projects one catalog suffix
to both interface/editor `QSpinBox` controls; the settings snapshot remains
integer-only and the preview, range, persistence, and application policy do not
move. Native rendering evidence remains outside the proof boundary.

D259 / ARCH-237 keeps common message-dialog button localization inside the
existing presentation boundary. `MessageSurface` sets four catalog-backed
labels after `QMessageBox.setStandardButtons`, while `StandardButton` return
values, default selection, semantic roles, modal flow, and dialog ownership do
not move. No global Qt translator or application policy is introduced. Native
dialog evidence remains outside the proof boundary.

D258 / ARCH-236 keeps Replace All error localization inside the existing
presentation i18n boundary. One anchored dynamic match-limit mapping and three
exact exception mappings are projected by `localize_message`/
`localize_exception`; `EditorWidget`, `ReplaceAllSession`, rollback state,
QScintilla ownership, and error propagation do not move. Unknown diagnostics
and en-US output remain unchanged. Native startup and native dialog evidence
remain outside the proof boundary.

D257 / ARCH-235 keeps built-in plugin-status name localization inside the
existing presentation boundary. `PluginStatusDialog` resolves only the stable
`quillforge.document-stats` ID through `presentation.i18n` and retains supplied
external plugin names as fallbacks; its existing locale-refresh path remains
the projection owner. The public plugin manifest/status contract, plugin
lifecycle, and startup behavior do not move. Native startup and native dialog
evidence remain outside the proof boundary.

D256 / ARCH-234 keeps built-in Document Statistics localization inside the
existing presentation i18n boundary. The core command ID is added to the
catalog, and the built-in exact/dynamic notification forms are mapped without
changing the public plugin API, plugin implementation, counters, English
fallback, or unknown external command behavior. Native startup and native
dialog evidence remain outside the proof boundary.

D255 / ARCH-233 keeps application-error localization inside the existing
presentation i18n boundary. `i18n.py` maps a bounded set of user-visible
application messages and recursively localizes known coordinator wrappers,
while preserving paths, document names, plugin IDs, unknown details, and the
early English-locale return. Application validation, worker scheduling,
filesystem policy, and UI ownership do not move. Native startup and native
dialog evidence remain outside the proof boundary.

D254 / ARCH-232 keeps frozen runtime dependency reporting inside the existing
no-window startup diagnostic boundary. `app.py` checks a fixed tuple of Qt
Core/Gui/Widgets, QScintilla, and platform paths under the supplied frozen
root, reports missing relative paths, and leaves normal `QApplication`
construction, environment handling, and runtime composition untouched. Native
startup evidence remains outside the proof boundary.

D253 / ARCH-231 keeps workspace-entry error locale refresh inside the existing
presentation boundary. `WorkspacePanel` retains each raw `WorkspaceEntry.error`
in a private tree-item role and routes it through the shared `i18n.py` mapper
when creating and reprojecting disabled inaccessible-row tooltips. Provider
classification, entry kind/path, row disabled state, file/folder activation,
ordering, bounds, and application ownership do not move. Native startup and
native dialog evidence remain outside the proof boundary.

D252 / ARCH-230 keeps plugin diagnostic locale refresh inside the existing
presentation boundary. `PluginStatusDialog` and `PluginCatalogDialog` route
visible runtime-status errors and catalog-entry reasons through the shared
`i18n.py` mapper, while unknown provider text remains unchanged. Trust,
approval, enablement, runtime registration, external execution, catalog
scanning, and permission policy do not move. Native startup and native dialog
evidence remain outside the proof boundary.

D251 / ARCH-229 keeps workspace-search diagnostic locale refresh inside the
existing presentation boundary. `WorkspaceSearchDialog` retains the latest
immutable result only to reproject diagnostic rows, while `i18n.py` maps known
provider reasons/prefixes and preserves path/detail suffixes. List ordering,
counts, truncation, expansion state, search policy, provider behavior, and
application ownership do not move. Native startup and native dialog evidence
remain outside the proof boundary.

D250 / ARCH-228 keeps workspace-search status locale refresh inside the
existing presentation boundary. `WorkspaceSearchDialog` stores only a
catalog key, raw failure message, or immutable `WorkspaceSearchResult` source
and rebuilds the current status after locale changes. Feedback level, query,
results, diagnostics, cancellation, search services, and application policy
do not move. Native startup and native dialog evidence remain outside the
proof boundary.

D249 / ARCH-227 keeps workspace error locale refresh inside the existing
presentation boundary. `WorkspacePanel` retains only the current typed or
string error source, reprojects it through the existing i18n helpers after a
locale change, and clears it when a successful directory result or new load
supersedes it. Error state remains presentation-local; coordinator, service,
session, notification, and page-retention policy do not move. Native startup
and native dialog evidence remain outside the proof boundary.

D248 / ARCH-226 keeps workspace typed-error projection inside the existing
presentation boundary. `WorkspaceNavigationCoordinator` forwards the original
exception through the existing view/surface protocol, and `WorkspacePanel`
chooses the shared `localize_exception()` path while retaining
`localize_message()` for invalid-result strings. Navigation operations,
session restoration, service ownership, notification policy, and page
retention do not move; no workspace-specific translator is introduced.
Native startup and native dialog evidence remain outside the proof boundary.

D247 / ARCH-225 keeps typed secondary error forwarding inside the existing
`MainWindow` presentation boundary. Settings persistence and Replace All now
pass their original `Exception` objects to `_show_error()` so
`MessageSurface`/`i18n.py` can retain filesystem and codec categories without
moving policy into a translator or service. English output remains compatible;
no new callback, storage field, or cross-layer dependency is introduced, and
native startup/dialog evidence remains outside the proof boundary.

D246 / ARCH-224 keeps startup path classification inside the existing
`MainWindow._drain_startup_paths()` presentation boundary. The route performs
one `Path.stat()` and uses standard-library mode predicates to retain concrete
missing/invalid path errors while preserving directory/file/special-file
behavior and existing async admissions. No new path service, queue, worker,
application policy, or Qt ownership is introduced; native startup and shell
association evidence remain outside the proof boundary.

D245 / UI-27 keeps typed document error localization in the existing
presentation error boundary. Open/save coordinators preserve the exception
object across their existing ports, while `MessageSurface` delegates typed
filesystem and codec failures to the presentation-only `i18n.py` helper.
English remains unchanged; Chinese receives stable categories with path,
encoding, and position details retained. Unknown exceptions fall back to the
existing message mapper. No application service, file store, worker, or Qt
translator owns a second localization policy, and native file I/O/dialog
evidence remains outside the proof boundary.

D244 / UI-26 keeps explicit desktop-launch error localization in the existing
presentation message compatibility boundary. `localize_message()` adds only
the two path-error prefixes emitted by `MainWindow._drain_startup_paths()`;
English remains unchanged and the path/exception suffix remains diagnostic
data. No startup parser, document admission coordinator, application service,
or Qt translator is introduced, and native startup evidence remains outside
the proof boundary.

D243 / UI-25 keeps semantic accent resolution in the presentation projection
owner. `theme.py` exposes one private pure resolver for text that sits on
general surfaces: it preserves `accent_alt` when it meets the conservative
deepest-surface threshold and otherwise uses `text_primary`. The application
palette link color, workspace eyebrow, and generic checkbox hover selector
consume that resolver; filled accent controls continue to use the existing
`on_accent*` tokens. No widget, application service, or `ThemeColors` field
owns a second contrast policy, and runtime rendering/accessibility evidence
remains outside the proof boundary.

D241 / ARCH-222 keeps startup provenance inside the existing no-window
diagnostic boundary. The diagnostic records `quillforge.app` module/package/
origin/loader metadata and, in frozen mode, selects the first existing PyQt6
Qt6/legacy Qt plugin root before checking `platforms/qwindows.dll`. The
existing platform-plugin result reuses that selected path, so the diagnostic
does not carry a second hard-coded Qt layout policy. It reports whether
`QT_PLUGIN_PATH` is configured but never mutates the environment, constructs
`QApplication`, or changes ordinary desktop startup. PyInstaller's public
Run-time Information documentation and its installed PyQt6 runtime hook are
the applicable packaging references; native plugin loading and release
evidence remain outside the proof boundary.

D240 / ARCH-221 keeps frozen entrypoint resolution inside the existing
application entry boundary. When `__package__` is empty, direct source-file
execution retains its compatibility `sys.path` setup, while a PyInstaller
frozen process skips source-style `__file__` parent path surgery and imports
`quillforge.app` through the packaged importer. The public `main(argv)` and
startup diagnostic boundary remain unchanged; no importer abstraction or new
composition owner is introduced. PyInstaller's public Run-time Information
documentation is the applicable engineering reference for `sys.frozen` and
bundled `__file__` semantics; native startup and release evidence remain
outside the proof boundary.

D239 / ARCH-220 keeps update and rollback identity admission inside the existing
local distribution boundary. After `ShouldProcess` confirms an update or
rollback, `update.ps1` reuses `Get-QfArtifactInfo` to validate the current
target (and rollback backup) against the install-state hashes before any
`Move-Item`; rollback also rejects identical target/backup paths. The check
does not add a lock or a second transaction abstraction, and D238 recovery
guards remain responsible for changes that occur after preflight. Public
PowerShell `about_ShouldProcess`, `Get-FileHash`, and `Move-Item` documentation
are applicable first-party engineering references; updater execution, races,
permissions, and release evidence remain outside the proof boundary.

D238 / ARCH-219 keeps update and rollback recovery inside the existing local
distribution boundary. `update.ps1` treats `Write-QfState` as the commit
boundary and records each executable/backup move; failed commits restore the
prior layout only when exact expected hashes and state-owned paths still hold.
An externally changed target, occupied backup path, or uncertain hash is
preserved with a warning rather than overwritten. No transaction framework,
new state schema, update channel, or process owner is introduced. Public
PowerShell `about_ShouldProcess`, `Get-FileHash`, and `Move-Item` documentation
are applicable first-party engineering references; updater execution,
permissions, concurrency, and release evidence remain outside the proof
boundary.

D237 / ARCH-217 keeps multi-extension association ownership inside the existing
local distribution boundary. The first opt-in extension creates the shared
`QuillForge.Document` ProgId; later extensions reuse it only through the
installer's operation-owned marker and an exact command match. Extension
cleanup remains state-scoped and command-checked, while the shared ProgId
cleanup scans HKCU Classes for unknown references before deletion. Install
rollback and uninstall preserve the executable and state when association
cleanup is incomplete, preventing a dangling target. PowerShell's public
`about_ShouldProcess` and Registry provider documentation are the applicable
first-party engineering references; installer, registry, shell, and release
runtime evidence remain outside the proof boundary.

Warning-background foreground selection remains a centralized presentation
theme concern. `theme._stylesheet()` derives one local foreground from the
actual `warning_bg` token for warning message, attention phase, workspace and
Find feedback, warning action, and workspace cancel hover/focus text;
decorative gold remains an endpoint/border token and
the filled warning action continues to use `on_accent_gold`. No theme schema,
application policy, or lower-layer dependency is added. ADR-0080 records this
bounded D55/UI-35 correction; native QSS rendering and release evidence remain
open.

Complete document-tab path identity lookup now belongs to
`presentation.document_tab_surface.DocumentTabSurface`. Its typed
`find_by_path()` method reuses `domain.path_identity.path_key()` and owns
pathless/identity-exclusion semantics; MainWindow delegates ordinary registry
lookup while retaining the startup restore subset and active-tab policy. ADR-
0081 records this bounded D56/ARCH-45 extraction; native tab ordering and
release evidence remain open.

The ordered output of serial session restoration now lives at the adjacent
Qt-free `presentation.session_restore_tracker.SessionRestoreTracker[TabT]`
boundary. It records opaque existing/new tab references and selects the
canonical active-path match or first-tab fallback through a path callback;
`begin()`/`finish()` clear those projection references. MainWindow retains
session/recovery/workspace/document services, TaskRunner, tab projection,
notifications, startup/initial-document/close policy, and all restore result
handling. ADR-0082 records this bounded D57/ARCH-46 extraction; native callback
timing and release evidence remain open.

The session-load callback no longer carries the write-only `_session_load_state`
field. It consumes `SessionLoadResult.state` directly for the existing invalid
notification classification while preserving `DEFAULT_SESSION` normalization,
save baseline, recovery-first startup scheduling, and MainWindow startup/close
policy. ADR-0083 records this bounded D58/ARCH-47 simplification; no replacement
tracker or session contract was introduced.

Status phase projection has one coordinator entry point at
`MainWindow._sync_status_surface()`. The removed `_sync_active_document_phase()`
alias had no independent state or policy; editor dirty, workspace completion,
current-tab, runner, and operation callers now route directly to the same
working/attention/ready precedence. ADR-0084 records this bounded D59/ARCH-48
simplification; StatusSurface and application policy remain unchanged.

The centralized `presentation.theme._stylesheet()` now gives the document rail
and workspace dock a stronger visual hierarchy. Selected tabs use the existing
selection surface with accent/alternate-accent edges, selected hover uses the
contrast-safe `on_accent` endpoint, and the tab rail/dock title receive existing
border boundaries. ADR-0085 records this bounded D60/UI-36 refinement; tab
behavior, theme data, warning foregrounds, and application policy remain
unchanged.

Session-save metadata assembly now has a Qt-free
`presentation.session_snapshot_builder.build_session_snapshot[TabT]` boundary.
It owns only clean path-backed tab filtering, ordered `SessionDocument`
assembly, per-tab cursor validation, and active-index derivation through typed
callbacks. MainWindow remains the composition root for EditorWidget reads,
session-save debounce, SessionService, TaskRunner, startup, restore,
notifications, and close policy. ADR-0086 records this bounded D61/ARCH-49
extraction; no session service or second snapshot DTO was introduced.

The FindBar keeps its existing interaction contract while exposing a clearer
presentation hierarchy through stable navigation/cancel/close object names and
central QSS. Query and replacement inputs use existing accent endpoints,
navigation uses a compact pressed/focus state, cancel retains the warning
foreground contract, and close uses the error surface on hover. ADR-0087
records this bounded D62/UI-37 refinement; signals, locale, replace-all,
editor, and MainWindow policy remain unchanged.

The existing command-rail context label is now a compact
`QLabel#toolbarContext` chip using the current surface, border, alternate
accent, and secondary-text tokens. It remains a static localized projection;
CommandSurface, command callbacks, shortcuts, layout, and locale policy remain
unchanged. ADR-0088 records this bounded D63/UI-38 refinement.

The settings dialog now exposes presentation-only appearance/editor group
identities and token-driven control rails for theme/UI-font/UI-size versus
accent/editor-font/editor-size controls. Group title text remains on the
readable primary endpoint, including Paper/Sand; SettingsSnapshot, locale,
SettingsSurface, persistence, and MainWindow policy remain unchanged. ADR-0089
records this bounded D64/UI-39 refinement.

MainWindow no longer carries private forwarding aliases for active-tab,
editor, path, or containment lookup. Existing callers use the cohesive
`DocumentTabSurface` contract directly; path exclusion and session-restore
subset policy remain explicit at their existing owners. ADR-0090 records this
bounded D65/ARCH-50 simplification.

The plugin catalog, plugin status, workspace-search, and settings dialogs now
project primary, warning, and quiet action roles through presentation-only Qt
object names. `theme.py` keeps the `quietAction` states and shared dialog
action rail centralized, while each dialog retains its own signals,
enablement, locale, and application policy. ADR-0091 records this bounded
D66/UI-40 refinement.

MainWindow no longer forwards Save As/dirty-close path selection through
`_choose_save_path()` or About through `_show_about()`. Existing callers bind
directly to `FileDialogSurface.choose_save_path()` and
`MessageSurface.show_about()`; document, close, command, and locale policy
remain in their existing owners. ADR-0092 records this bounded D67/ARCH-51
simplification.

MessageSurface now uses equivalent instance-based QMessageBox projections so
About, errors, and unsaved-close prompts can receive stable themed identities;
RecoveryPromptSurface marks restore/discard/later as primary/warning/quiet.
The message-dialog edge, label, and button hierarchy remains centralized in
`theme.py`, while return mapping, modal behavior, locale, and recovery policy
remain unchanged. ADR-0093 records this bounded D68/UI-41 refinement.

The presentation localization boundary now also covers duplicate-open,
plugin-failure, known invalid-workspace, extension-catalog, and plugin-host
diagnostics without moving locale into application summaries. `PluginCatalogDialog`
retains its immutable summary source and re-projects it on locale refresh;
dynamic paths, identifiers, PIDs, limits, and raw error details remain
diagnostic data. ADR-0094 records this bounded D69/UI-42 closure.

`PluginCatalogDialog` now also projects its stable row and tooltip vocabulary
through `i18n.catalog_value()`, with raw-value fallback for future enum values.
Locale refresh re-renders existing in-memory rows, including the empty state;
catalog entry data, signals, governance enablement, and plugin execution policy
remain unchanged. ADR-0095 records this bounded D70/UI-43 refinement.

Plugin Status tooltip booleans now use centralized locale values rather than
Python's `True`/`False` representation; the application-owned
`PluginRuntimeStatus` contract, lifecycle controls, trust/enablement predicates,
and policy remain unchanged. ADR-0096 records this bounded D71/UI-44 closure.

Plugin Catalog and Plugin Status dialogs now receive an object-name-scoped
surface hierarchy from `theme.py`: accent-topped boundary, summary card, list
panel, focus cue, and selected-row edge/weight. Dialog composition, locale,
signals, data, governance, and plugin execution policy remain unchanged.
ADR-0097 records this bounded D72/UI-45 refinement.

Settings now projects a presentation-only live appearance preview from the
pending theme, accent, interface font, and interface-size choices. The public
`theme_colors()` resolver and `preview_stylesheet()` keep the preview on the
same canonical token/QSS boundary; `SettingsDialog` owns only refresh and
locale projection. Save/Cancel, SettingsService persistence, MainWindow theme
application, editor projection, and motion timing remain unchanged. ADR-0098
records this bounded D73/UI-46 refinement.

The D73 preview object tree now lives in `SettingsPreviewSurface`, a focused
presentation surface with one `project(...)` contract. `SettingsDialog` retains
editable controls, `SettingsSnapshot` assembly, Save/Cancel, locale flow, and
application policy; the surface retains only preview widgets, labels, and
token-bound rendering. No second settings model or application/domain
dependency is introduced. ADR-0099 records this bounded D74/UI-47 boundary.

The workspace search dialog now has a scoped visual hierarchy for root/query,
results, diagnostics, focus, selection, disabled, hover, and checked states;
the existing diagnostic toggle supplies only the semantic object name
`workspaceSearchDiagnosticsToggle`. Search services, result/diagnostic data,
TaskRunner, cancellation, locale, and file activation remain in their current
owners. ADR-0100 records this bounded D75/UI-48 refinement.

Recovery success notifications remain application-owned English diagnostic
messages and are projected only at the presentation boundary. The existing
`localize_message()` helper now recognizes the bounded
`Recovered <document>; content remains unsaved` shape, translates its static
suffix for `zh-CN`, and preserves the dynamic document name verbatim; `en-US`
continues to return the source message. No recovery, document, persistence,
or MainWindow policy moves. ADR-0101 records this bounded D76/UI-49 closure.

The centralized shell stylesheet now uses a quieter ordinary-state baseline:
the command rail, inactive document tabs, status rail, and common controls no
longer compete through simultaneous heavy fills, borders, rounding, and a
primary gradient. Selected/focus/feedback/warning/error states remain explicit
through the existing semantic selectors and contrast-aware tokens; no widget,
signal, locale, settings, document, workspace, or application policy moves.
ADR-0121 records this bounded D96/UI-50 refinement.

Extension catalog scan and descriptor approval/revocation sequencing now live
in the Qt-free `PluginCatalogCoordinator`. It depends on narrow task-submitter,
catalog-view, and notification contracts, reuses the existing
`PluginOperationTracker`, and keeps application catalog/governance services
behind their current contracts. MainWindow retains the shared plugin operation
tracker for close-event gates, host diagnostics, runtime enablement, command
refresh, locale/theme application, and all trust/security policy. ADR-0102
records this bounded D77/ARCH-52 extraction.

Diagnostic plugin-host probe sequencing now lives in the Qt-free
`PluginHostProbeCoordinator`, which reuses shared task-submitter and
notification contracts. It validates the typed probe result and maps its
existing ready/rejected/error states to notifications without owning host
trust, execution, containment, or security policy. MainWindow retains the
shared `PluginOperationTracker`, the host-probe close gate, composition and
notification ownership, and the existing application/infrastructure host
boundary. ADR-0103 records this bounded D78/ARCH-53 extraction.

Registered-plugin failure, status, and enable/disable sequencing now live in
the Qt-free `PluginRuntimeCoordinator`. It accepts the application-owned
`PluginRuntime` protocol through a typed view and notification boundary, and
receives busy/command-refresh policy as injected callbacks. MainWindow keeps
composition, the document-operation busy predicate, notification ownership,
and shared plugin close gates; `PluginRuntime` remains the sole runtime
trust/enablement/security policy boundary. ADR-0104 records this bounded
D79/ARCH-54 extraction.

Session-load result classification and recovery-first baseline projection now
live in the Qt-free `SessionLoadCoordinator`. It receives typed baseline
setters, a recovery-scan continuation callback, and the shared notification
sink; it does not own `SessionService`, `TaskRunner`, startup barriers,
workspace/tab restoration, or close policy. MainWindow retains those
application-flow and lifecycle owners, while the coordinator preserves
absent/valid/invalid/failed load semantics. ADR-0105 records this bounded
D80/ARCH-55 extraction.

Recovery inventory result classification now lives in the Qt-free
`RecoveryScanCoordinator`. It owns only `RecoveryScanTracker.finish`, typed
candidate validation, empty/failure notifications, candidate prompt dispatch,
and startup continuation through injected callbacks. MainWindow retains
RecoveryService, RecoveryPromptSurface and restore/discard/later decisions,
session/workspace/tab policy, TaskRunner, and close behavior. ADR-0106 records
this bounded D81/ARCH-56 extraction.

Session-save completion classification now lives in the Qt-free
`SessionSaveCoordinator`. It owns only matching/stale tracker completion,
invalid/failure notification projection, and latest-request draining through an
injected callback. MainWindow retains SessionService, snapshot capture,
debounce timer, TaskRunner dispatch, startup barriers, operation IDs, and close
behavior. ADR-0107 records this bounded D82/ARCH-57 extraction.

Settings-save callback classification now lives in the Qt-free
`SettingsSaveCoordinator`. It owns only `SettingsSaveTracker` completion and
failure classification plus explicit valid/invalid/failure policy callbacks.
MainWindow retains SettingsService, QApplication theme projection, locale and
editor refresh, font settings, transition timing, notifications, and close
behavior. ADR-0108 records this bounded D83/ARCH-58 extraction.

Workspace-search completion classification now lives in the Qt-free
`WorkspaceSearchCoordinator`. It owns only tracker finish classification,
typed result validation, invalidated cancellation feedback, surface projection,
and summary notification severity through explicit seams. MainWindow retains
WorkspaceSearchService, query/cancellation, root containment, result opening,
surface construction, and close behavior. ADR-0109 records this bounded
D84/ARCH-59 extraction.

Workspace-navigation completion classification now lives in the Qt-free
`WorkspaceNavigationCoordinator`. It owns only workspace tracker finish
classification, loading/error projection, invalid open/directory result
handling, and failure notification/session-restore completion through explicit
seams. MainWindow retains WorkspaceService, workspace activation,
search-root invalidation, directory-root projection, containment, document
opening, session save/restore, and close behavior. ADR-0110 records this
bounded D85/ARCH-60 extraction.

Document-open completion classification now lives in the Qt-free
`DocumentOpenCoordinator`. It owns only generic stale completion suppression,
session-restore binding consumption, `OpenedDocument` validation, ordinary
failure projection, and session-restore continuation through explicit seams.
MainWindow retains DocumentService, duplicate-tab policy, editor/tab creation,
line/cursor projection, event publication, success notifications, persistence,
and close behavior. ADR-0111 records this bounded D86/ARCH-61 extraction.

Document-save completion classification now lives in the Qt-free generic
`DocumentSaveCoordinator[TabT]`. It owns only generic stale completion,
live-tab gating, editor read-only release, `DocumentState` validation, and
invalid/failure error projection. MainWindow retains state replacement,
language/title refresh, recovery cleanup, `DocumentSaved`, success
notifications, session-save scheduling, optional continuation, and close
behavior. ADR-0112 records this bounded D87/ARCH-62 extraction.

Recovery-delete completion classification now lives in the Qt-free generic
`RecoveryDeleteCoordinator[JobT, OwnerT]`. It owns only recovery tracker
delete release, identity-guarded owner clearing, success/error notification,
and pending-delete drain. MainWindow retains delete request admission,
RecoveryService dispatch, operation IDs, capture/write lifecycle,
restore/discard decisions, persistence, and close behavior. ADR-0113 records
this bounded D88/ARCH-63 extraction.

Replace All completion cleanup now lives in the Qt-free generic
`ReplaceAllCompletionCoordinator[TabT, SessionT, ProgressT]`. It owns only
current-job release, live-tab lock cleanup, tab-bar/Find operation-state
release, and generic operation completion. MainWindow retains ReplaceAllSession
stepping/cancellation, rollback, clean-state restoration, limit/cancel/
success/error policy, editor consequences, and close behavior. ADR-0114
records this bounded D89/ARCH-64 extraction.

Recovery-write completion classification now lives in the Qt-free generic
`RecoveryWriteCoordinator[JobT, OwnerT]`. It owns only discarded-state
consumption, matching capture abort, document/write tracker release, and
saved/failed ordering. MainWindow retains tab liveness, dirty/content-version
and snapshot identity decisions, delete scheduling, notification, persistence,
and close behavior. ADR-0115 records this bounded D90/ARCH-65 extraction.

Recovery-capture failure and cancellation cleanup now lives in the Qt-free
generic `RecoveryCaptureAbortCoordinator[JobT, OwnerT]`. It owns only matching
capture identity protection, worker-started discard versus pre-worker release,
channel/session cleanup, document lifecycle release, and optional live-owner
failure notification. MainWindow retains Qt scheduling, editor capture
stepping, stale/cancel decisions, RecoveryService dispatch, retry eligibility,
and close behavior. ADR-0116 records this bounded D91/ARCH-66 extraction.

Ordered session-restore progression now lives in the Qt-free generic
`SessionRestoreCoordinator[TabT]`. It owns only startup/workspace guards,
deferred and duplicate path progression, pending-open binding, active/first-tab
completion selection, initial-document fallback, and finish/save ordering
through explicit callbacks. MainWindow retains workspace and document services,
TaskRunner/open callbacks, tab/editor projection, startup/close state, and
notification policy. ADR-0117 records this bounded D92/ARCH-67 extraction.

The existing Qt-free `RecoveryWriteCoordinator[JobT, OwnerT]` now also owns the
direct `RecoveryCaptureTracker.finish_write()` release and forwarding of any
pending-delete projection. MainWindow retains delete admission,
RecoveryService dispatch, delete callbacks, and tab/content/snapshot policy;
ADR-0118 records this bounded D93/ARCH-68 closure of the D90 boundary.

Approved document-tab removal finalization now lives in the Qt-free generic
`DocumentTabRemovalCoordinator[TabT, CaptureT]`. It owns only liveness-guarded
recovery capture/snapshot cleanup, tab projection removal, editor teardown,
`DocumentClosed` projection, session-save request, and empty-tab fallback.
MainWindow retains busy/startup/dirty/save-confirm close policy and all service
and notification decisions. ADR-0119 records this bounded D94/ARCH-69
extraction.

Document-tab assembly now lives in the Qt-free generic
`DocumentTabCreationCoordinator[OpenedT, TabT, EditorT]`. It owns only editor
and tab factory sequencing, tab-surface add, title/modified projection, session
save request, and status synchronization through callbacks. MainWindow retains
document/settings/editor policy, concrete tab/recovery identity construction,
open/recovery outcomes, and close behavior. ADR-0120 records this bounded
D95/ARCH-70 extraction.

Valid document-open projection now lives in the Qt-free generic
`DocumentOpenProjectionCoordinator[TabT]`. It owns only ordinary/session
duplicate-path projection, valid tab creation, optional line/cursor placement,
restored-tab recording, `DocumentOpened`/success projection, and session
continuation through explicit callbacks. `DocumentOpenCoordinator` continues
to own stale, invalid, and failure classification; MainWindow remains the
composition root and retains concrete editor/tab, service, event, error,
notification, and close policy. ADR-0122 records this bounded D97/ARCH-71
extraction.

Valid document-save completion projection now lives in the Qt-free generic
`DocumentSaveProjectionCoordinator[TabT]`. It owns only the ordered projection
of a validated live result: saved/clean state, language, title, recovery
cleanup, `DocumentSaved`/success feedback, session-save scheduling, and the
optional continuation. `DocumentSaveCoordinator` retains stale/invalid/failure
classification, tab liveness, and read-only release; MainWindow retains the
concrete editor, recovery, service, notification, persistence, and close policy.
ADR-0123 records this bounded D98/ARCH-72 extraction.

Valid workspace-open and directory projection now live in the Qt-free generic
`WorkspaceNavigationProjectionCoordinator`. It owns only the current-result
ordering, including search invalidation/root, workspace-open directory,
success/session-save/restore projection, and directory-root no-op guards.
`WorkspaceNavigationCoordinator` retains D85 completion, stale, invalidated,
loading, invalid, and failure classification; MainWindow retains WorkspaceService,
generation/TaskRunner, concrete surface/search, admission, and close policy.
ADR-0124 records this bounded D99/ARCH-73 extraction.

Valid settings-save projection now lives in the Qt-free generic
`SettingsSaveProjectionCoordinator`. It owns only snapshot/theme baseline,
retranslation, open-editor settings, motion transition, and success feedback
ordering. `SettingsSaveCoordinator` retains D83 tracker/stale/invalid/failure
classification; MainWindow retains QApplication/theme, locale, editor, motion,
notification, service, and close policy. ADR-0125 records this bounded
D100/ARCH-74 extraction.

The composition root passes the manager through the application-owned
`PluginRuntime` protocol. The optional external catalog is scanned only after a
user command, on the worker boundary, and never imports or executes an
entrypoint. Approval/revocation is also a worker-bound application operation;
its result triggers a fresh catalog projection. Avoid module-level singletons
so the runtime can later support multiple windows, profiles, and isolated
plugin contexts. See [the enterprise architecture migration baseline](specs/enterprise-architecture-migration.md)
and [ADR-0036](adr/0036-enterprise-composition-root.md) for the incremental
migration contract and limits.

## Extension points

1. **Commands** — stable IDs, titles, key bindings, enablement, and execution callbacks.
2. **Editor adapters** — one interface for QScintilla or a future engine.
3. **Document services** — encoding, line endings, safe save, recovery, and external-change detection.
4. **Workspace providers** — project tree, search scope, and file discovery. Navigation and content search use separate ports so a recursive search cannot silently change tree semantics.
5. **Plugins** — versioned public API, explicit lifecycle, capability registration, and failure isolation.
6. **Extension catalog** — bounded metadata discovery and trust diagnostics without dynamic loading.

## Threading rule

The UI thread owns Qt widgets, command callbacks, event publication, plugin callbacks, and short UI state transitions. File open/save use `TaskRunner` and execute only the `DocumentStore` operation in a worker. Completion is accepted only for the current operation ID; the first shell allows one document I/O operation at a time. Never move a widget across threads.

The shell maintains one tab per resolved filesystem path. Save As and asynchronous open reject a path already owned by another tab, preventing two in-memory states from competing for the same file.

`TaskRunner` disables `QRunnable` auto-deletion and retains each task until one queued completion slot has delivered success/failure and released the task. This makes the worker-to-UI payload lifetime explicit. Its read-only `pending_count`/`has_pending_work()` contract is the single lifecycle observation point for submitted work; `MainWindow.closeEvent()` rejects shutdown until that retained task set and its queued completion delivery have drained. The close guard never blocks the UI with `waitForDone()` or force-terminates a cooperative worker.

The presentation-owned `pending_changed` signal mirrors that same retained-task
boundary. `MainWindow` uses it to keep the shell status rail in `WORKING`
while any worker or queued completion remains, then projects dirty-document
`ATTENTION` or clean-idle `READY`; it does not create a second lifecycle
counter.

Current-document find and replace is a presentation/editor capability. `FindBar` owns query and replacement controls, `MainWindow` resolves the active tab on every request, and `EditorEngine` exposes only literal search/replace semantics. The QScintilla adapter owns a two-phase `ReplaceAllSession`; `MainWindow` schedules bounded slices on the Qt event loop and owns cancellation/status, while `EditorOperationPolicy` supplies product limits. File services, document metadata, plugins, and filesystem discovery do not own search state.

Recovery is a separate application use case. `RecoveryService` creates immutable snapshots through `RecoverySnapshotStore`; `JsonRecoverySnapshotStore` owns only the per-user atomic JSON files. The QScintilla adapter exposes a position-safe `TextCaptureSession`; the presentation layer advances it in bounded UI slices, rejects stale content versions, and offers chunks through the application-level `RecoveryChunkChannel` contract. The default infrastructure channel applies independent chunk-count and UTF-8-byte bounds, while `TaskRunner` owns only the worker-side consume/write operation. Chunk boundaries use Scintilla positions rather than Python string lengths, preserving multibyte characters and line-ending bytes. Autosave is skipped while another editor operation owns the UI boundary. The current default composition is bounded to 64 queued chunks and 1 MiB of queued UTF-8 bytes, plus one active candidate chunk; this is an observed handoff contract, not a product-wide native-memory ceiling. A restored document retains its source revision and remains dirty, so normal save conflict protection still applies. Recovery snapshots are never exposed through the plugin API.

Session continuity is a separate metadata use case. `SessionService` normalizes an immutable `SessionSnapshot`, while `JsonSessionStore` owns only the bounded atomic user-local manifest. The manifest contains an optional workspace root, clean path-backed tab order, adapter-level caret positions, and active-tab index; it never contains document text, recovery payloads, undo history, or plugin state. Startup first reconciles recovery candidates, then restores the workspace and opens session paths serially through `DocumentService`, skipping invalid or duplicate paths. Session writes are single-flight/latest-wins through `TaskRunner`, and the close guard includes their queued completion boundary. The first slice intentionally does not restore selection, layout, or dirty/untitled content outside Recovery.

Extension governance is deliberately narrower than runtime trust. `PluginApprovalService` maps a valid catalog entry to `approved`, `stale`, or `not-approved` using the canonical descriptor SHA-256 and delegates persistence to the `PluginApprovalStore` port. `JsonPluginApprovalStore` owns only the bounded, versioned, atomic user-local ledger and fails closed on malformed or oversized input. An approval record never sets `trust_state`, `loadable`, or any runtime loader flag; external code remains untrusted and unloaded.

The runtime control plane is a separate application contract. `PluginRuntimeStatus` is an immutable projection of explicitly registered in-process lifecycle state, and `PluginManager` is the only owner allowed to activate/deactivate plugin callbacks or clean owned commands/subscriptions. **Tools → Plugin Status** emits enable/disable intents on the UI thread and reprojects the status; it cannot see or activate catalog descriptors. This synchronous lifecycle seam is intentionally not a security boundary for untrusted code and is not reused as the future process-host protocol.

`PluginEnablementPolicy` resolves the initial enabled state at registration and
persists explicit choices through `PluginEnablementStore`. The JSON adapter is
bounded and atomic; absence uses the trusted built-in default, while malformed
or oversized state disables activation and refuses mutation. The policy is a
preference source, not a trust or signature source, and is independent from
the descriptor approval ledger.

`PluginExecutionGate` is the application-owned decision seam for any future
external plugin execution request. It consumes an immutable evidence snapshot
covering catalog validity, trust, digest-bound approval, signature, code
identity, enablement, permissions, host containment, and executor availability.
It returns a deterministic immutable decision plus all failed requirements; it
never imports modules, starts a process, or mutates policy. The composition root
uses the gate while projecting catalog entries, so Extension Catalog can show
`execution=denied` and the primary reason without gaining a loader path. The
current threshold keeps `external_execution_enabled=false`, and no executor is
wired.

`PluginHostClient` is the application port for the diagnostic external host.
`SubprocessPluginHost` owns only exact-command process creation, bounded JSONL
stdio, timeout/kill, and typed failure mapping. `quillforge.plugins.host_process`
is a Qt-free entrypoint that performs a probe-only hello/probe exchange and
`execution_enabled=false`; the decoder rejects unknown capabilities or
any true execution flag before projecting a result; it never imports a catalog
entry. The host PID and
worker completion are observable, but this is a crash/protocol boundary rather
than a complete Windows security boundary. The infrastructure-only
`ProcessContainment` seam owns a short-lived child lease and an optional
creation-time launcher. On Windows the default adapter configures an unnamed
Job Object with kill-on-close, at most two active processes for launcher/re-exec
plus host, and a 256 MiB per-process committed-memory limit; it creates the
child suspended, assigns the job, and resumes the initial thread only after
assignment. Launch/attach failure maps to a fail-closed containment error.
Non-Windows adapters report explicit unsupported fallback. The application
receives only immutable containment state/limit labels, and external execution
remains false.

Workspace navigation follows the same boundary. `WorkspaceService` owns the selected-root containment rule and entry limit; `FileWorkspaceProvider` enumerates one directory page without recursion or child symlink traversal. `WorkspacePanel` renders immutable entries and emits folder/file/cancel intents. Enumeration and file opening use `TaskRunner`; stale generations and cancelled operations cannot replace the last good tree or activate a new root.

Workspace content search is a separate application capability. `WorkspaceSearchService` validates an explicit `WorkspaceSearchQuery` and snapshots the immutable `WorkspaceSearchPolicy`; `FileWorkspaceSearchProvider` recursively walks only regular, non-symlink entries below that resolved root. It performs literal line-local matching with deterministic path order, bounded file/total bytes, file count, depth, line length, result count, and issue records. UTF-8/UTF-16 BOM text is supported; binary, excluded, oversized, inaccessible, and long-line inputs are skipped or diagnosed without crashing the search. The provider never calls `DocumentStore`, reads a Qt widget, writes files, or invokes plugins. `MainWindow` submits the provider behind `TaskRunner` with a cooperative cancellation event, operation ID, and generation; a result is projected only when current. Opening a result rechecks `WorkspaceService.contains()` and uses the editor port for one-based line navigation. This first slice intentionally has no regex, indexing, remote roots, or cross-file replacement.

Command discovery is a presentation projection over `CommandRegistry`. The palette stores only a stable command ID and resolves it again at execution time, so plugin unregisters cannot leave an executable stale object. Settings use a versioned `SettingsSnapshot` and `SettingsStore`; `SettingsService` owns defaults, schema migration, and bounded normalization, while the JSON adapter owns only atomic local persistence. `AppearanceSettings` remains an immutable domain value: locale, theme, accent, interface font/size, and motion are not Qt objects. The composition root loads the initial snapshot once, and `MainWindow` applies later persisted snapshots through the presentation boundary. The translation catalog, theme tokens, and transition animation remain presentation-only; QScintilla preferences reach the editor only through `EditorEngine` methods. Session continuity uses its own `SessionSnapshot`/`SessionStore` contract and does not expand the settings schema.

## Contract summary

| Contract | Owner | Key guarantee |
|---|---|---|
| `DocumentService` | application | immutable document state transitions and explicit save target |
| `DocumentStore` | infrastructure | encoding/line-ending preservation, optimistic revision check, atomic replacement |
| `SessionService` | application | bounded path-only continuity, safe normalization, and startup restore contract |
| `SessionStore` | infrastructure | bounded versioned session JSON with atomic replacement |
| `EventBus` | application | exact-type synchronous events on its owning UI thread |
| `CommandRegistry` | application | unique IDs and owner-controlled unregister |
| `PluginManager` | application | explicit registration, API-version validation, lifecycle cleanup, failure isolation |
| `PluginCatalogService` | application | schema/API/permission validation, deterministic duplicate diagnostics, no execution |
| `JsonPluginCatalogStore` | infrastructure | top-level bounded JSON reads from one explicit user-local directory |
| `PluginApprovalService` | application | digest-bound approval state, stale detection, fail-closed mutation policy, no execution authority |
| `JsonPluginApprovalStore` | infrastructure | bounded versioned approval records with atomic replacement and corruption diagnostics |
| `PluginRuntime` | application | immutable lifecycle status projection and explicit in-process enablement boundary |
| `PluginEnablementPolicy` | application | startup resolution and fail-closed local enablement mutations without lifecycle ownership |
| `JsonPluginEnablementStore` | infrastructure | bounded versioned local preferences with atomic replacement and corruption diagnostics |
| `PluginExecutionGate` | application | immutable, deterministic, deny-by-default external execution decisions with complete failed requirements |
| `WorkspaceSearchService` | application | validated immutable query/policy, bounded provider delegation, and result-shape checks |
| `WorkspaceSearchProvider` | application port | read-only recursive search contract with cooperative cancellation and immutable result DTOs |
| `FileWorkspaceSearchProvider` | infrastructure | deterministic local traversal, encoding/binary handling, bounds, diagnostics, and no symlink traversal |
| `PluginHostClient` | application | typed diagnostic port for a separate host process |
| `ProcessContainment` | infrastructure | short-lived child lifecycle/resource attachment and creation-before-resume seam with explicit platform state |
| `SubprocessPluginHost` | infrastructure | shell-free bounded process launch, containment lease, JSONL framing, timeout, and failure mapping |

## Recovery write projection boundary

The Qt-free `RecoveryWriteCoordinator[JobT, OwnerT]` owns discarded
classification, capture abort, document/write lifecycle release, and
pending-delete forwarding. Its saved/failed result is projected through
`RecoveryProjectionCoordinator[OwnerT]`, which owns only the valid post-write
tab-liveness, dirty/content-version, snapshot-identity, deletion, and feedback
ordering. `MainWindow` remains the composition root for concrete tab/editor,
recovery, notification, persistence, and close policy. This keeps recovery
writer lifecycle and tab presentation policy independently extensible without
introducing a service locator or Qt dependency into the coordinator.

Close readiness follows the same pattern. The Qt-free
`CloseGuardCoordinator` owns only ordered busy/search/dirty/background/pending
work classification and the existing immediate-session-save/timer-stop
sequence. `MainWindow` retains `QCloseEvent` acceptance, error-dialog and
locale projection, cooperative cancellation, TaskRunner/session/timer
callbacks, and concrete operation policy. This keeps shutdown policy explicit
without turning a presentation event into a framework-wide shutdown service.

The document rail remains a native `DocumentTabSurface`, now with the stable
`documentTabBar` semantic identity and presentation-only document-mode,
middle-elision, and non-expanding hints. Centralized QSS owns its normal,
selected, hover, focus, disabled, and close-affordance states; modified icons,
tab identity, signals, and close policy remain in the existing surface and
composition root.

The workspace resource manager remains a native `WorkspacePanel` projected
through `WorkspaceSurface`. UI-52 adds only semantic path/tree/empty identities,
localized empty/loading copy, and centralized QSS hierarchy. A private
`_sync_content_state()` keeps the empty card and tree mutually exclusive;
workspace signals, file/folder activation, keyboard routing, loading/error
ownership, WorkspaceService, containment, and session policy remain in their
existing owners.

Session persistence now has one complete Qt-free request/dispatch boundary in
`SessionSaveCoordinator`. It owns latest snapshot admission, single-flight
operation binding, completion classification, and queued-request draining via
explicit callbacks. MainWindow remains the composition root for QTimer debounce,
snapshot capture, SessionService, TaskRunner, startup restore, notifications,
and close readiness. This keeps persistence lifecycle policy reviewable without
leaking Qt or concrete services into the coordinator.

The D104/UI-53 editor canvas token hierarchy keeps shell colors and editor
syntax colors on one presentation-owned path without widening the shell token
record. `editor_color_tokens()` resolves canvas, gutter, selection, caret,
current-line, and Python syntax roles; `EditorWidget` only projects those
values to QScintilla. MainWindow, editor settings, language selection,
document operations, persistence, and close policy remain unchanged. ADR-0131
records the bounded decision; native QScintilla rendering and runtime visual
evidence remain open.

The D105/ARCH-78 recovery-write dispatch callback boundary extends the existing
Qt-free `RecoveryWriteCoordinator` with a typed generic dispatcher contract.
It owns only binding owner/content-version/snapshot identity to the existing
complete/fail callbacks; MainWindow retains operation IDs, RecoveryService,
payload/channel selection, TaskRunner, capture, notification, persistence,
and close policy. ADR-0132 records the bounded decision; native worker timing
and recovery durability remain open.

The D106/UI-54 scrollbar chrome refinement keeps both scrollbar orientations
on the centralized theme QSS path. It restores the horizontal track/handle
surface and removes the horizontal bar from the hidden subcontrol selector;
scroll mode, wrap policy, EditorWidget ownership, and application behavior
remain unchanged. ADR-0133 records the bounded visual decision; native style
engine and DPI evidence remain open.

The D107/ARCH-79 recovery-delete dispatch boundary extends the existing
Qt-free `RecoveryDeleteCoordinator` with typed operation and dispatcher
contracts. It binds snapshot identity, optional owner, and success message to
the established delete completion/failure lifecycle; MainWindow retains
delete admission, operation IDs, RecoveryService, TaskRunner, notification,
persistence, and close policy. ADR-0134 records the bounded decision; native
worker timing and filesystem durability remain open.

The D108/ARCH-80 presentation contract audit adds a stdlib AST gate for
Qt-free coordinator imports, explicit MainWindow notification levels, and the
TaskRunner pending-work observability path through MainWindow's WORKING phase.
It is a source-only regression gate: it moves no runtime behavior or error
policy and does not infer native event timing or observability completeness.
ADR-0135 records the bounded decision; runtime and release evidence remain
open.

The D109/ARCH-81 recovery-scan dispatch boundary extends the existing
Qt-free `RecoveryScanCoordinator` with typed operation/dispatcher contracts.
It binds `RecoveryScanJob` to the established result/failure lifecycle while
MainWindow retains scan admission, RecoveryService, operation IDs, TaskRunner,
startup/manual context, session continuation, and close policy. ADR-0136
records the bounded decision; native scan timing and recovery durability remain
open.

The D110/ARCH-82 document-save dispatch boundary extends the existing
Qt-free `DocumentSaveCoordinator` with typed operation/dispatcher contracts.
It binds the live tab and optional post-save continuation to the established
save lifecycle while MainWindow retains document snapshots, read-only policy,
DocumentService, operation IDs, TaskRunner, persistence, and close behavior.
ADR-0137 records the bounded decision; native save timing and filesystem
durability remain open.

The D111/ARCH-83 document-open dispatch boundary extends the existing Qt-free
`DocumentOpenCoordinator` with typed operation/success/failure/dispatcher
contracts. It binds optional line navigation to the established ordinary and
session-restore open lifecycle while MainWindow retains path selection,
restore binding, DocumentService, operation IDs, TaskRunner, persistence,
status/notification, and close behavior. ADR-0138 records the bounded
decision; native open timing and filesystem decoding remain open.

The D112/ARCH-84 settings-save dispatch boundary extends the existing Qt-free
`SettingsSaveCoordinator` with typed operation/success/failure/dispatcher
contracts. It binds the settings callback lifecycle through the existing
tracker while MainWindow retains settings editing, SettingsService,
operation IDs, TaskRunner, theme/font/locale/editor/motion projection,
notifications, persistence, and close policy. ADR-0139 records the bounded
decision; native settings rendering and worker timing remain open.

The D113/ARCH-85 session-load dispatch boundary extends the existing Qt-free
`SessionLoadCoordinator` with typed operation/success/failure/dispatcher
contracts. It binds startup session loading while preserving baseline-before-
recovery ordering and invalid-manifest retention; MainWindow retains startup
admission, SessionService, operation IDs, TaskRunner, session state,
notifications, persistence, and close policy. ADR-0140 records the bounded
decision; native startup/session timing remains open.

UI-55 keeps the settings dialog's presentation identities explicit: language,
theme, accent, interface/editor font and size, wrapping, line numbers, and
motion controls now expose stable selectors, while form labels carry the
`settingsRole=fieldLabel` semantic. Settings-scoped QSS adds readable field
labels and compact option-row hover/checked/disabled states using canonical
theme tokens. Settings values, locale flow, preview, persistence, motion, and
MainWindow ownership remain unchanged; ADR-0141 records the bounded visual
decision and native rendering evidence remains open.

The D114/ARCH-86 workspace-navigation dispatch boundary adds typed dispatcher
contracts and open/directory binding methods to the existing Qt-free
WorkspaceNavigationCoordinator. MainWindow now routes both WorkspaceService
worker submissions through that boundary while retaining operation admission,
generation allocation, TaskRunner, workspace/surface/containment policy,
session ordering, persistence, notifications, and close behavior. ADR-0142
records the bounded decision; native callback timing, filesystem behavior, and
remaining runtime/release gates remain open.

The UI-56/ARCH-87 command-rail hierarchy adds a closed presentation-only role
to ToolbarActionSpec and projects it to commandBar tool buttons. MainWindow
assigns primary, quiet, and workspace-context roles while CommandSurface
retains menu/toolbar projection and theme.py retains centralized token QSS.
Callbacks, shortcuts, locale, icons, layout, command policy, and close
behavior remain unchanged; ADR-0143 records the bounded visual decision and
native rendering evidence remains open.

The UI-57/ARCH-88 workspace-dock hierarchy scopes the existing native dock
chrome to WorkspaceDock and gives its frame, title, close, and float
subcontrols token-driven interaction states. WorkspaceSurface retains dock
placement, locale, panel signals, loading/error projection, and workspace
policy; ADR-0144 records the bounded visual decision and native docking
evidence remains open.

The D115/ARCH-89 workspace-search dispatch boundary adds typed dispatcher
contracts and generation-bound submit(...) to the existing Qt-free
WorkspaceSearchCoordinator. MainWindow now routes search through that seam
while retaining query construction, cooperative cancellation,
WorkspaceSearchService, TaskRunner, surface, containment, locale,
notifications, and close policy. ADR-0145 records the bounded decision;
native search timing and filesystem evidence remain open.

The UI-58 dialog-shell edge hierarchy adds object-scoped frame and top-accent
QSS to the existing Settings and Command Palette dialog identities through
the centralized theme owner. Existing child-control selectors, dialog
signals, locale, keyboard focus, layout, settings persistence, command
selection, plugin behavior, and workspace-search behavior remain unchanged;
ADR-0146 records the bounded decision and native dialog rendering evidence
remains open.

The D116/ARCH-90 session-save dispatch boundary adds typed operation,
success/failure, and dispatcher contracts to the existing Qt-free
SessionSaveCoordinator. Its drain path now binds the admitted save operation
to coordinator callbacks through the injected TaskRunner callable while
MainWindow retains the SessionService operation factory, snapshot capture,
debounce, notifications, startup restore, persistence, and close policy. ADR-
0147 records the bounded decision; native timing and session-store evidence
remain open.

The UI-59 control-affordance chrome adds token-driven hover/pressed/disabled
subcontrol states for existing ComboBox dropdowns and abstract SpinBox
up/down buttons in the centralized stylesheet. Native arrow semantics,
settings value ranges, signals, focus/accessibility contracts, locale,
persistence, and application policy remain unchanged; ADR-0148 records the
bounded decision and native subcontrol rendering evidence remains open.

The UI-60 dialog action-rail hierarchy wraps the existing Plugin Catalog and
Plugin Status action layouts in the shared `dialogActionRail` presentation
identity and adds a centralized token separator. Button objects, order,
enablement, signals, locale, plugin governance, and policy remain unchanged;
ADR-0149 records the bounded decision and native layout/rendering evidence
remains open.

The UI-61 accent palette swatch hierarchy adds a stateless vector swatch
renderer to `presentation/icons.py` and projects token-derived swatches onto
the existing Settings Theme and Accent combo items. SettingsDialog owns only
pending-value projection and refresh; `theme_colors` remains the single
resolved-token source, localized option text remains the semantic contract,
and settings/schema/persistence behavior remain unchanged. ADR-0150 records
the bounded decision and native combo rendering/accessibility evidence remains
open.

The UI-62 document tab close-affordance hierarchy extends only the existing
`QTabBar#documentTabBar::close-button` stylesheet contract with an 18px
minimum target and explicit focus/disabled states. `DocumentTabSurface` keeps
tab composition, close signals, tab identity, and document lifecycle policy;
ADR-0151 records the bounded decision and native subcontrol/focus evidence
remains open.

D117 / ARCH-91 removes the pure `MainWindow._next_operation_id()` forwarding
facade. The existing `OperationTracker.reserve` bound is injected directly
into `SessionSaveCoordinator`, and seven MainWindow call sites use the same
canonical tracker while `_begin_operation`/`_complete_operation` retain
busy/status policy. ADR-0152 records the bounded simplification; runtime
interleaving and release evidence remain open.

D118 / ARCH-92 adds the frozen, slotted generic `SessionRestorePorts[TabT]`
callback contract. `SessionRestoreCoordinator` now consumes one explicit
ports value instead of ten positionally ordered callbacks, while preserving
its Qt-free ordered restore, deferred/duplicate handling, pending-open,
active-tab, initial-document, finish, and save sequencing. MainWindow remains
the composition and policy owner; ADR-0153 records the bounded decision.

UI-63 gives the existing `FindBar` close button a stable 30px minimum height
and scoped token-driven pressed/disabled states while retaining danger hover
and focus feedback. FindBar signals, object identity, locale, layout, and
editor policy remain unchanged; ADR-0154 records the visual decision. Native
rendering, runtime callback timing, and release evidence remain open.

D119 / ARCH-93 adds the Qt-free generic `WorkspaceFileActivationCoordinator`
and its frozen/slotted `WorkspaceFileActivationPorts` contract. It owns only
raw file-intent validation, startup/busy admission, workspace containment,
duplicate-tab focus/notice, and routing of accepted paths to MainWindow's
existing `_start_open` boundary. WorkspacePanel click/double-click/keyboard
signals, WorkspaceService containment, DocumentTabSurface identity, and
MainWindow async/open policy remain unchanged; ADR-0155 records the bounded
decision and native tree/runtime/release evidence remain open.

D120 / ARCH-94 adds the frozen/slotted `WorkspaceNavigationPorts` contract.
`WorkspaceNavigationCoordinator` now receives named tracker, surface,
projection, restore, and notification ports instead of seven positional
callbacks. It retains completion/generation classification and remains
Qt-free; MainWindow remains the composition and application-policy owner.
ADR-0156 records the bounded decision, while native callback timing and
runtime/release evidence remain open.

UI-64 / ARCH-95 keeps the next visual refinement in the existing centralized
stylesheet. `QToolBar#commandBar` becomes a rounded elevated command rail and
`QTabBar#documentTabBar` becomes a grouped document navigator with clearer
spacing and target rhythm. Existing ThemeColors, command roles, tab identity,
focus/pressed/selected/disabled states, signals, and application policy remain
the owners; ADR-0157 records the bounded decision and native rendering/runtime/
release evidence remain open.

UI-65 / ARCH-96 keeps message feedback in the same stylesheet boundary.
Existing common/about/error/recovery `QMessageBox` objects receive semantic
top accents, readable primary/informative text, and consistent button targets;
`QToolTip` receives the same token-driven compact frame. MessageSurface,
RecoveryPromptSurface, object names, button roles, locale, and decision policy
remain unchanged; ADR-0158 records the bounded decision and native dialog/
runtime/release evidence remain open.

D121 / ARCH-97 adds the frozen/slotted `CloseGuardPorts` contract. The
Qt-free `CloseGuardCoordinator` now consumes named busy/search/dirty/background/
save/pending/timer callbacks while preserving its exact close precedence and
side-effect order. MainWindow retains QCloseEvent acceptance, messages, timers,
trackers, persistence, and application policy; ADR-0159 records the bounded
decision and native event/runtime/release evidence remain open.

D122 / ARCH-98 adds the frozen/slotted `DocumentOpenPorts` contract. The
Qt-free `DocumentOpenCoordinator` now consumes named completion, restore
identity/document, valid projection, continuation, error, and notification
callbacks while preserving ordinary/session-restore ordering. The existing
DocumentOpenProjectionCoordinator and MainWindow service/startup/close policy
remain in place; ADR-0160 records the bounded decision and native editor/
runtime/release evidence remain open.

D123 / ARCH-99 adds the frozen/slotted generic `DocumentSavePorts[TabT]`
contract. The Qt-free `DocumentSaveCoordinator` now consumes named operation,
tab-liveness, read-only, valid projection, and error callbacks while preserving
stale/liveness guards and save completion order. The existing
DocumentSaveProjectionCoordinator and MainWindow service/persistence/close
policy remain in place; ADR-0161 records the bounded decision and native
editor/runtime/release evidence remain open.

D124 / ARCH-100 adds the frozen/slotted generic
`DocumentSaveProjectionPorts[TabT]` contract. The Qt-free
`DocumentSaveProjectionCoordinator` now consumes named state, language, title,
recovery, event, notification, and session-save callbacks while preserving its
exact valid-save projection order and optional continuation. MainWindow retains
save classification, services, persistence, and application policy; ADR-0162
records the bounded decision and native editor/runtime/release evidence remain
open.

D125 / ARCH-101 adds the frozen/slotted generic
`DocumentOpenProjectionPorts[TabT]` contract. The Qt-free
`DocumentOpenProjectionCoordinator` now consumes named existing-tab, restore,
duplicate-error, tab-creation, line, cursor, event, notification, and
continuation callbacks while preserving duplicate/restored branches and
valid-open order. MainWindow retains open classification, services, persistence,
startup, close, and application policy; ADR-0163 records the bounded decision
and native editor/runtime/release evidence remain open.

D126 / ARCH-102 adds the frozen/slotted generic
`DocumentTabCreationPorts[OpenedT, TabT, EditorT]` contract. The Qt-free
`DocumentTabCreationCoordinator` now consumes named editor, tab, insertion,
title/modified, title-refresh, session-save, and status callbacks while
preserving recovery identity and assembly order. MainWindow retains concrete
editor/tab and application policy; ADR-0164 records the bounded decision and
native editor/runtime/release evidence remain open.

D127 / ARCH-103 adds the frozen/slotted generic
`DocumentTabRemovalPorts[TabT, CaptureT]` contract. The Qt-free
`DocumentTabRemovalCoordinator` now consumes named liveness, recovery capture,
snapshot, tab/editor, event, session-save, count, and empty-document callbacks
while preserving missing/live/capture/empty-tab order and bool semantics.
MainWindow retains close admission and recovery/tab policy; ADR-0165 records
the bounded decision and native tab/editor/runtime/release evidence remain
open.

UI-66 / ARCH-104 keeps central-shell visual rhythm at the existing
`EditorShellSurface` boundary. The shell now owns 10/8px content margins and
8px inter-surface spacing, while `QWidget#editorShell` projects the existing
`surface_1` token around the existing tab/editor `surface_0` canvas. Tab/Find
child order, signals, locale, fonts, motion, document policy, and MainWindow
composition remain unchanged; ADR-0166 records the bounded decision and
native rendering/runtime/release evidence remain open.

D128 / ARCH-105 adds the Qt-free `CoreCommandCoordinator` and frozen/slotted
`CoreCommandPorts` contract. It owns only deterministic construction and
registration of the 23 built-in commands; MainWindow maps its existing
callbacks by name, while CommandSurface retains QAction/menu/toolbar
projection and plugin refresh. Command IDs, order, metadata, callback
identity, registry collision behavior, and application policy remain
unchanged; ADR-0167 records the bounded decision and native menu/runtime/
release evidence remain open.

D129 / ARCH-106 adds the presentation-only `CoreToolbarCoordinator` and
frozen/slotted `CoreToolbarPorts` contract. It owns only the composition
specification for the six core toolbar actions and the optional workspace
action; MainWindow maps its existing callbacks by name, while CommandSurface
retains QToolBar/QAction projection and locale-aware presentation. Action
count, order, text keys, callback identity, icons, separators, roles, command
IDs, and application policy remain unchanged; ADR-0168 records the bounded
decision and native toolbar/runtime/release evidence remain open.

D130 / ARCH-107 extracts `IconKey`, `ToolbarActionRole`, and
`ToolbarActionSpec` into pure-Python contract modules. The icon renderer and
CommandSurface retain compatibility imports, while CoreToolbarCoordinator
now depends only on the contract layer and cannot pull PyQt6 transitively.
ADR-0169 records the dependency-direction correction; native toolbar/runtime/
release evidence remains open.

D131 / ARCH-108 adds the Qt-free `ReplaceAllAdmissionCoordinator` and frozen/
slotted `ReplaceAllAdmissionPorts` contract. It owns only busy/active-tab/
query admission, session creation, tracker/operation binding, and the starter
handoff; MainWindow retains editor, FindSurface, tab-bar, QTimer, cooperative
slice, completion, rollback, and application policy. Existing rejection
messages, operation identity, dirty/content-version capture, and runtime
evidence limits remain unchanged; ADR-0170 records the bounded decision.

D132 / ARCH-109 adds the Qt-free `RecoveryCaptureAdmissionCoordinator` and
frozen/slotted `RecoveryCaptureAdmissionPorts` contract. It owns only the
recovery-available/busy gate, dirty-tab candidate filtering, inflight/delete
pending exclusion, snapshot ID reuse/generation, dirty-state normalization,
and content-version capture; MainWindow retains channel/backpressure,
EditorWidget capture, `_RecoveryCaptureJob`, tracker registration, writer
dispatch, QTimer slices, abort/write/close policy. ADR-0171 records the
bounded decision and recovery runtime evidence remains open.

UI-67 / ARCH-110 keeps `presentation/theme.py` as the single QSS owner and
rebalances existing tokens across the main canvas, editor stage, command rail,
status rail, document tab rail/items, workspace dock, and workspace panel.
Normal surface values now provide a clear canvas-to-stage-to-interaction
ladder while existing hover, pressed, checked, selected, focus, disabled,
warning, primary, quiet, and context states remain intact. No signal,
callback, locale, font, motion, document, workspace, or application policy
changed; ADR-0172 records the bounded decision and native rendering evidence
remains open.

D133 / ARCH-111 adds the Qt-free `DocumentOpenAdmissionCoordinator` and
frozen/slotted `DocumentOpenAdmissionPorts` contract. It owns only
busy/startup-restore admission, the existing operation begin message,
session-restore operation binding, and asynchronous submission through the
existing `DocumentOpenCoordinator`/TaskRunner boundary. `FileDialogSurface`,
`WorkspacePanel`, document result classification, tab/editor projection, line
navigation, notifications, session restore, and close policy remain in their
existing owners; ADR-0173 records the bounded decision and native dialog/
runtime evidence remains open.

D134 / ARCH-112 adds the Qt-free `DocumentSaveAdmissionCoordinator` and
frozen/slotted `DocumentSaveAdmissionPorts` contract. It owns only
busy/startup-restore admission, duplicate-target rejection, state/text
snapshot capture, editor read-only protection, operation begin, and
asynchronous submission through the existing `DocumentSaveCoordinator`/
TaskRunner boundary. `FileDialogSurface`, save-as selection, document
persistence, result classification, recovery/session persistence,
notifications, and close policy remain in their existing owners; ADR-0174
records the bounded decision and native dialog/filesystem/runtime evidence
remains open.

D135 / ARCH-113 adds the Qt-free `WorkspaceNavigationAdmissionCoordinator`
and frozen/slotted `WorkspaceNavigationAdmissionPorts` contract. It owns only
workspace/surface availability and busy/startup-restore admission, loading
projection, operation begin, workspace-generation binding, and open/directory
submission through the existing `WorkspaceNavigationCoordinator`/TaskRunner
boundary. `WorkspaceService`, `WorkspaceSurface`, file/folder activation,
containment, result classification, projection, cancellation, session restore,
notifications, and close policy remain in their existing owners; ADR-0175
records the bounded decision and native workspace/runtime evidence remains
open.

D136 / ARCH-114 adds the Qt-free `SettingsSaveAdmissionCoordinator` and
frozen/slotted `SettingsSaveAdmissionPorts` contract. It owns only settings
service availability and in-flight admission, candidate editing, operation
reservation, tracker binding, and asynchronous submission through the existing
`SettingsSaveCoordinator`/TaskRunner boundary. Settings persistence and
validation, result classification, `SettingsSaveProjectionCoordinator`,
QApplication theme application, locale/font/editor refresh, motion,
notifications, and close policy remain in their existing owners; ADR-0176
records the bounded decision and native settings/runtime evidence remains
open.

D137 / ARCH-115 adds the Qt-free `DocumentCreationAdmissionCoordinator` and
frozen/slotted `DocumentCreationAdmissionPorts` contract. It owns only
busy/startup-restore admission, the explicit initial-restore exception,
document-service creation, tab projection handoff, `DocumentOpened`
publication, and success notification order. `DocumentService`,
`DocumentTabCreationCoordinator`, editor/tab projection, `EventBus`, startup
restore, notifications, and close policy remain in their existing owners;
ADR-0177 records the bounded decision and native startup/editor/tab evidence
remains open.

UI-68 / ARCH-116 keeps `presentation/theme.py` as the single QSS owner and
derives local readable success/working/error foregrounds from the actual
success, pressed, and error backgrounds. Existing status, workspace/search,
and FindBar semantic state selectors consume those endpoints; warning/gold,
accent endpoint, disabled/focus/selected, widget, signal, locale, motion, and
application policy remain unchanged. ADR-0178 records the bounded decision;
native style-engine rendering and runtime/release evidence remain open.

UI-69 / ARCH-117 keeps `presentation/theme.py` as the single QSS owner and
gives the settings dialog a bounded surface ladder: `surface_0` canvas,
distinct appearance/editor cards, `surface_3` preview, and a scoped action
rail. Existing semantic object names, form layout, buttons, focus states,
signals, settings persistence, locale/font/motion projection, and application
policy remain unchanged. ADR-0179 records the bounded decision; native
style-engine/runtime/release evidence remains open.

D138 / ARCH-118 adds the Qt-free `DocumentPickerAdmissionCoordinator` and
frozen/slotted `DocumentPickerAdmissionPorts` contract. It owns only
busy/startup-restore admission, native file-selection invocation,
cancellation, and handoff to `_start_open`; `FileDialogSurface`,
`DocumentOpenAdmissionCoordinator`, `DocumentService`, workspace directory
selection, result classification, notifications, and close policy remain in
their existing owners. ADR-0180 records the bounded decision; native
QFileDialog/filesystem/runtime/release evidence remains open.

UI-70 / ARCH-120 keeps `presentation/theme.py` as the single visual owner and
refines the existing QSS shell elevation ladder: command bar, editor shell,
document tab rail/tabs, status rail, workspace dock, and workspace empty state
now use distinct existing surface/border/accent tokens. Widget IDs, signals,
layout, locale, font, motion, semantic state, accessibility, theme/accent, and
application policy remain unchanged; ADR-0182 records the bounded decision and
native style-engine/runtime/release evidence remains open.

D140 / ARCH-121 adds the Qt-free `WorkspaceSearchSurfaceAdmissionCoordinator`,
frozen/slotted `WorkspaceSearchSurfaceAdmissionPorts`, and a narrow surface
protocol. It owns only startup-restore/service/root admission, surface
creation/reuse, root-change invalidation/reset, and showing the existing search
surface; query validation, operation tracking, cancellation, dispatch, result
classification, containment, locale, notifications, and close policy remain in
their existing owners. ADR-0183 records the bounded decision; native search
dialog/filesystem/runtime/release evidence remains open.

D141 / ARCH-122 adds the Qt-free generic
`DocumentSavePickerAdmissionCoordinator[TabT]` and frozen/slotted
`DocumentSavePickerAdmissionPorts[TabT]` contract. It owns only active-tab and
busy admission, ordinary Save versus Save As target routing, native save-path
selection, cancellation, and handoff to `_start_save`; `FileDialogSurface`,
`DocumentSaveAdmissionCoordinator`, validation/conflict/read-only/persistence,
result classification, recovery/session, notifications, and close policy remain
in their existing owners. ADR-0184 records the bounded decision; native save
dialog/filesystem/runtime/release evidence remains open.

UI-71 / ARCH-123 keeps `presentation/theme.py` as the single visual owner and
adds one `workspaceSearchQueryCard` presentation container around the existing
workspace-search query row. Scoped token QSS now distinguishes the query card,
field, hover/focus, and case checkbox while signals, keyboard behavior,
results/diagnostics/status, locale, font, motion, theme/accent, and application
policy remain unchanged. ADR-0185 records the bounded decision; native
style-engine/runtime/release evidence remains open.

D139 / ARCH-119 adds the Qt-free `WorkspacePickerAdmissionCoordinator` and
frozen/slotted `WorkspacePickerAdmissionPorts` contract. It owns only
workspace availability, startup-restore/busy admission, native directory
selection, cancellation, and handoff to `_start_workspace_open`;
`FileDialogSurface`, document file selection/opening,
`WorkspaceNavigationAdmissionCoordinator`, `WorkspaceService`, result
classification, file activation, containment, notifications, and close policy
remain in their existing owners. ADR-0181 records the bounded decision; native
QFileDialog/filesystem/runtime/release evidence remains open.

D142 / ARCH-124 closes the next static presentation contract gate without
moving runtime policy. `scripts/audit_presentation_contracts.py` now requires
top-level coordinator `*Ports` classes to use frozen/slotted dataclasses and
requires direct `self._ports.notify(...)` calls to state an explicit semantic
`level=`. The existing Qt-free dependency, MainWindow notification, and
TaskRunner pending-work checks remain active. ADR-0186 records the bounded
decision; runtime rendering, worker timing, and release evidence remain open.

D143 / ARCH-125 adds the Qt-free `PresentationLocaleCoordinator` and
frozen/slotted `PresentationLocalePorts` contract. It owns only the existing
locale snapshot and refresh ordering. MainWindow continues to own the locale
provider, translation key, concrete Qt surfaces, dynamic workspace/search
surface checks, settings, and application policy. ADR-0187 records the bounded
decision; native translation rendering, event timing, and release evidence
remain open.

D144 / ARCH-126 adds the Qt-free `CloseGuardFeedbackCoordinator` and
frozen/slotted `CloseGuardFeedbackPorts` contract. It owns only the existing
`CloseGuardDecision` reason-to-message mapping and pending-count interpolation.
MainWindow retains close classification, QCloseEvent handling, MessageSurface,
error projection, and application policy. ADR-0188 records the bounded
decision; native dialog/event timing and release evidence remain open.

UI-72 / ARCH-127 keeps `presentation/theme.py` as the single visual owner and
turns the existing `statusMessage` label into a compact token-driven rounded
state surface. The info state gains a readable surface/border and all semantic
states retain their existing token-derived backgrounds, foregrounds, and
levels; StatusSurface, timers, signals, locale, motion, and application policy
remain unchanged. ADR-0189 records the bounded decision; native QSS rendering
and release evidence remain open.

UI-73 / ARCH-128 keeps `presentation/theme.py` as the single visual owner and
removes only the redundant `QTabWidget#documentTabs::pane` outline. The
authored document-tab rail and `QsciScintilla#editor` canvas edge/focus cue
remain the visible navigation/work-area boundaries; tab/find signals, layout
owners, locale, fonts, motion, and application policy remain unchanged.
ADR-0190 records the bounded decision; native QSS specificity/rendering and
release evidence remain open.

D145 / ARCH-129 adds the Qt-free `StatusPhaseCoordinator` and canonical
`StatusPhase` contract. It owns only the deterministic working/attention/ready
priority from three application facts. MainWindow retains concrete busy,
TaskRunner, and active-tab queries plus direct error projection; StatusSurface
and StatusRail retain Qt rendering, locale, accessibility, and style refresh.
ADR-0191 records the bounded decision; native event timing and release evidence
remain open.

UI-74 / ARCH-130 keeps the status-rail object tree and phase state owner intact
and adds only a token-driven right divider and compact padding to the existing
`statusContext` selector. `statusPhase` ready/working/attention/error styles,
locale, timers, signals, and application policy remain unchanged. ADR-0192
records the bounded decision; native QSS rendering and release evidence remain
open.

D146 / ARCH-131 adds the Qt-free generic
`DocumentChangeProjectionCoordinator[EditorT, TabT]` and frozen/slotted
`DocumentChangeProjectionPorts[EditorT, TabT]` contract. It owns only the
existing modified/content callback ordering, including Find invalidation,
stale/unchanged guards, dirty/content-version mutation, tab-title/status
projection, and debounced session-save request. MainWindow retains
DocumentService, tab/editor lookup, concrete tab identity, FindSurface,
StatusSurface, session timers, Qt signal ownership, and application policy.
ADR-0193 records the bounded decision; native signal timing and release
evidence remain open.

D147 / ARCH-132 adds the Qt-free generic
`CurrentDocumentTransitionCoordinator[TabT]` and frozen/slotted
`CurrentDocumentTransitionPorts[TabT]` contract. It owns only the existing
current-tab transition order: Find invalidation, Find-session reset, active-tab
lookup, optional title/context notification, status synchronization, and
debounced session-save request. MainWindow retains concrete tab/Find/status
surfaces, notification policy, session timers, Qt signal ownership, tab
identity, and application policy. ADR-0194 records the bounded decision;
native signal timing and release evidence remain open.

UI-75 / ARCH-133 keeps `presentation/theme.py` as the single visual owner and
restores only the token-driven `border-left-color` on the specific selected
and selected-hover document-tab selectors. Existing border width, surface,
bottom accent, text, focus, disabled, close-button, tab metrics, signals,
locale, motion, and application policy remain unchanged. ADR-0195 records the
bounded decision; native QSS rendering and release evidence remain open.

D148 / ARCH-134 adds the Qt-free generic
`EditorActionAdmissionCoordinator[EditorT, TabT]` and frozen/slotted
`EditorActionAdmissionPorts[EditorT, TabT]` contract. It owns only the
existing active-tab/busy guard, editor-action invocation, and focus-restoration
order. MainWindow retains concrete tab/editor ownership, busy-state policy,
Qt focus calls, command/shortcut composition, editor behavior, and document
policy. ADR-0196 records the bounded decision; native focus/event timing and
release evidence remain open.

D149 / ARCH-135 adds the frozen/slotted Qt-free
`SettingsSaveProjectionPorts` contract around the existing
`SettingsSaveProjectionCoordinator`. It owns only the named five-step order:
apply the validated snapshot, retranslate the shell, apply editor settings,
animate the appearance transition, and notify success. MainWindow retains
settings validation/persistence, concrete editors, QApplication/theme
application, animation, notification, and policy ownership. ADR-0197 records
the bounded decision; native event timing and release evidence remain open.

D150 / ARCH-136 adds the frozen/slotted generic
`RecoveryProjectionPorts[OwnerT]` contract around the existing recovery-writer
outcome projection. It owns only the discarded/live/snapshot/content-version
saved-result branches and the discarded/live failure guard. MainWindow retains
recovery state, tab identity, deletion scheduling, clear policy, notification
text/levels, writer lifecycle, and application ownership. ADR-0198 records the
bounded decision; recovery durability/timing and release evidence remain open.

UI-76 / ARCH-137 keeps `presentation/theme.py` as the single visual owner and
changes only the inactive selected workspace/list row foreground from
`text_secondary` to `text_primary`. The existing pressed surface, strong
border, accent-left cue, disabled state, selection semantics, row geometry,
widget IDs, locale, font, motion, and application policy remain unchanged.
ADR-0199 records the bounded contrast decision; native QSS rendering and
release evidence remain open.

D151 / ARCH-138 adds the frozen/slotted `SessionLoadPorts` contract around
the existing session-load result projection. It owns only named baseline
setters, recovery-scan continuation, and notification delivery while
preserving typed-result validation, `DEFAULT_SESSION` fallback, the
`set_last_saved -> set_snapshot` baseline order, invalid-result feedback, and
recovery-first scheduling. MainWindow retains session service, TaskRunner,
startup admission, restore state, notification policy, and concrete Qt/
application ownership. ADR-0200 records the bounded decision; native startup
timing and release evidence remain open.

D152 / ARCH-139 adds the frozen/slotted `SessionSavePorts` contract around
the existing latest-wins session-save coordinator. It owns only save
admission, snapshot capture, operation identity, persistence dispatch, and
notification callbacks while preserving can-save/request, in-flight/pending,
stale, invalid, failure, and drain ordering. MainWindow retains the debounce
timer, SessionService, snapshot builder, TaskRunner, notification policy, and
startup/close ownership. ADR-0201 records the bounded decision; native timer
delivery, filesystem durability, and release evidence remain open.

D153 / ARCH-140 adds the frozen/slotted generic
`ReplaceAllCompletionPorts[TabT, SessionT, ProgressT]` contract around the
existing stale-guarded Replace All completion cleanup. It preserves
`tracker.finish -> unlock -> tab-bar enable -> operation inactive -> complete
operation -> project outcome` while MainWindow retains editor, FindSurface,
tab, operation, rollback, cancellation, and policy ownership. ADR-0202 records
the bounded decision; native event timing, editor rollback, and release
evidence remain open.

D154 / ARCH-141 adds the frozen/slotted `SettingsSavePorts` contract around
the existing settings-save result projection. It preserves stale suppression,
invalid-result feedback, valid `SettingsSnapshot` application, and matching
failure projection while MainWindow retains settings persistence, validation,
theme/locale/font/editor refresh, transition, notification, and close policy.
ADR-0203 records the bounded decision; native worker/timer timing, settings
filesystem durability, and release evidence remain open.

D155 / ARCH-142 adds the frozen/slotted `WorkspaceSearchPorts` contract around
the existing workspace-search result projection. It preserves stale
suppression, invalidation cancellation, invalid-result feedback, valid-result
summary severity, and matching failure projection while MainWindow retains
search service, query/generation cancellation, surface, containment,
notification, and policy ownership. ADR-0204 records the bounded decision;
native worker/event timing, filesystem traversal, and release evidence remain
open.

D156 / ARCH-143 adds the frozen/slotted `PluginRuntimePorts` contract around
the existing registered-plugin runtime control projection. It preserves
failure notification/command refresh, unavailable and busy guards, runtime
exception handling, and success refresh/notification/status order while
MainWindow retains runtime, trust/permission/enablement policy, command
registry, plugin surface, and close ownership. ADR-0205 records the bounded
decision; native plugin lifecycle timing and release evidence remain open.

D157 / ARCH-144 adds the frozen/slotted generic `RecoveryDeletePorts[OwnerT]`
contract around the existing recovery-delete result projection. It preserves
tracker release, live-owner cleanup, optional success notification, pending
delete scheduling, and failure notification order while MainWindow retains
recovery persistence, filesystem, capture/write state, tab identity, and close
policy ownership. ADR-0206 records the bounded decision; native delete timing,
filesystem durability, and release evidence remain open.

D158 / ARCH-145 adds the frozen/slotted `RecoveryScanPorts` contract around
the existing recovery inventory result projection. It preserves stale guard,
invalid-inventory failure, empty non-startup information, candidate prompt
order, startup continuation, and failure continuation while MainWindow retains
RecoveryService, scan worker, restore state, filesystem, notification, and
close policy ownership. ADR-0207 records the bounded decision; native scan
timing, startup scheduling, and release evidence remain open.

D159 / ARCH-146 adds the frozen/slotted generic `RecoveryWritePorts[JobT,
OwnerT]` contract around the existing recovery-write callback projection. It
preserves discarded classification, capture abort, document release,
pending-delete draining, and saved/failed projection order while MainWindow
retains recovery persistence, worker/capture concurrency, filesystem, tab,
notification, and close policy ownership. ADR-0208 records the bounded
decision; high-risk native callback timing, durability, and release evidence
remain open.

D160 / ARCH-147 adds the frozen/slotted `WorkspaceNavigationProjectionPorts`
contract around the existing workspace open/directory projection. It
preserves search invalidation, workspace activation, search-root projection,
surface guards, directory projection, opened notification, session save, and
restore continuation order while MainWindow retains WorkspaceService,
containment, document activation, TaskRunner, surface, and close policy
ownership. ADR-0209 records the bounded decision; native event timing and
release evidence remain open.

D161 / ARCH-148 adds the frozen/slotted `PluginHostProbePorts` contract around
the existing isolated plugin-host diagnostic orchestration. It preserves the
unavailable and busy guards, start-notification/dispatch order, operation-ID
stale suppression, typed result severity mapping, and failure notification
while MainWindow retains the host client, plugin-operation tracker, TaskRunner,
notification, process containment, and external-execution policy ownership.
ADR-0210 records the bounded decision; process timing, containment, and release
evidence remain open.

D162 / ARCH-149 adds the frozen/slotted `PluginCatalogPorts` contract around
the existing metadata-only catalog scan and descriptor-governance orchestration.
It preserves unavailable/busy guards, scan summary projection, typed snapshot
validation, governance target validation, action disable/reenable behavior,
stale operation suppression, governance success/rescan order, and failure
notification while MainWindow retains catalog, approval ledger, execution-gate,
TaskRunner, notification, plugin-surface, and security-policy ownership.
ADR-0211 records the bounded decision; metadata filesystem timing, plugin
security evidence, and release evidence remain open.

D163 / ARCH-150 adds the frozen/slotted generic
`RecoveryCaptureAbortPorts[JobT, OwnerT]` contract around the existing recovery
capture-abort lifecycle. It preserves matching-capture identity guards,
finish-capture, worker-started discarded/release classification, channel abort,
session cancel, document completion, and optional live-owner failure
notification while MainWindow retains RecoveryCaptureTracker, capture channel,
session, tab, worker, filesystem, and close-policy ownership. ADR-0212 records
the bounded decision; native callback interleavings, durability, and release
evidence remain open.

D164 / UI-77 adds `font-weight: 600` to the centralized
`QLabel#statusMessage` base selector. It preserves the existing info,
success, warning, and error state selectors, text/locale projection, timer,
visibility, tooltip, margins, padding, and status-rail ownership. ADR-0213
records the bounded presentation decision; native font metrics, rendering, and
release evidence remain open.

D165 / ARCH-152 makes the local support handoff packet a required release
traceability artifact. `scripts/check.ps1` and
`scripts/verify_release_handoff.ps1` check only the exact packet path and
project `support_handoff_packet_exists` into the dossier. Manifest statuses,
the ten open release gates, and the `no-go` decision remain unchanged. ADR-0214
records the bounded decision; owner acceptance and external support evidence
remain open.

D166 / UI-78 / ARCH-153 keeps font-choice presentation inside
`SettingsDialog`. The existing interface and editor font allowlists are
projected into their existing `QComboBox` items through `QFont` and
`Qt.ItemDataRole.FontRole`, so users can judge each family before saving.
`currentText()` reads, `UserRole` values, `SettingsSnapshot`, persistence,
theme/locale/font application, size ranges, and motion ownership remain
unchanged. ADR-0215 records the bounded decision; native popup rendering,
installed-font fallback metrics, and release evidence remain open.

D167 / UI-79 / ARCH-154 completes the Settings typography preview without
moving settings ownership. `SettingsPreviewSurface` now projects both the
interface and editor samples with `QFont` and explicit point sizes, while
`preview_stylesheet(colors)` retains only object-named color, border, and
layout tokens and no longer receives typography interpolation inputs. Editor
font/size controls join the existing theme/accent/locale refresh path. ADR-
0216 records the bounded decision; native rendering, fallback metrics,
accessibility, and release evidence remain open.

D168 / UI-80 / ARCH-155 completes the workspace entry visual-semantics slice
without moving workspace behavior ownership. `WorkspacePanel` keeps the
existing file/directory/inaccessible icon keys and semantic signals, but
projects a Link-role file cue on dark canvases, a primary-text fallback on
light canvases, and localized file/folder/unavailable hints. Provider error
tooltips remain authoritative, and the existing theme icon refresh path
reprojects visible entries. ADR-0217 records the bounded decision; native
icon rendering, tooltip timing, accessibility, and release evidence remain
open.

D169 / UI-81 / ARCH-156 completes the FindBar action-affordance slice without
moving find/replace behavior ownership. Existing previous/next, replace,
replace-all, cancel, and close buttons now project authored icons, including
the pure `ARROW_DOWN` contract, through the existing `themed_icon()` provider.
`FindBar.set_locale()` refreshes the icons after the established theme/accent
projection path; signals, shortcuts, query/replacement state, busy/cancel
semantics, and primary-action projection remain unchanged. ADR-0218 records
the bounded decision; native icon metrics, accessibility, and release evidence
remain open.

D170 / UI-82 / ARCH-157 completes the command-rail visual-hierarchy slice
without moving command ownership. The centralized `QToolBar#commandBar` rule
now uses a lighter card surface, restrained brand edge, more breathable
spacing, consistent button hit height/radius, quieter quiet actions, and a
compact context anchor. `CommandSurface` still owns toolbar construction,
labels, shortcuts, icons, callbacks, and locale refresh; command roles and
interaction-state selectors remain unchanged. ADR-0219 records the bounded
decision; native toolbar metrics, accessibility, and release evidence remain
open.

D171 / UI-83 / ARCH-158 completes the document-tab visual-hierarchy slice
without moving document-tab ownership. The centralized
`QTabBar#documentTabBar` rule now keeps one restrained rail, flattens inactive
tabs until hover, and reserves the raised surface/accent edge for the selected
tab while retaining focus, disabled, and close-button states.
`DocumentTabSurface` still owns tab creation/removal, title and modified-icon
projection, current-change/close signals, and identity lookup. ADR-0220 records
the bounded decision; native tab metrics, accessibility, and release evidence
remain open.

D172 / UI-84 / ARCH-159 completes the editor-stage visual-depth slice without
moving editor ownership. The centralized editor-shell rule now uses a softer
stage surface and restrained border, while `QsciScintilla#editor` remains the
primary canvas with a slightly larger radius and the existing focus/selection
token projection. `EditorWidget` and `EditorShellSurface` retain font, lexer,
syntax, caret, editing, document, and signal ownership. ADR-0221 records the
bounded decision; native QScintilla metrics, accessibility, and release
evidence remain open.

D173 / UI-85 / ARCH-160 completes the status-rail visual-hierarchy slice
without moving notification or lifecycle ownership. The centralized status
rules now use a calmer shell surface, distinct message/status capsules, and
more breathable phase pills while preserving all info/success/warning/error
messages, ready/working/attention/error phases, locale, timers, accessible
names, and state properties. ADR-0222 records the bounded decision; native
status-bar metrics, accessibility, and release evidence remain open.

D174 / UI-86 / ARCH-161 completes the Workspace-dock visual-hierarchy slice
without moving workspace navigation or activation ownership. The centralized
WorkspaceDock rules now use a normal frame border, a thinner title emphasis
rail, more breathable title spacing, and 20px close/float controls aligned
with the document-tab interaction rhythm. WorkspaceSurface retains docking,
file/folder activation, search, locale, tree selection, signals, and policy;
ADR-0223 records the bounded decision. Native docking metrics,
accessibility, and release evidence remain open.

D175 / UI-87 / ARCH-162 completes the Command Palette visual-hierarchy slice
without moving command filtering or execution ownership. The centralized
Command Palette rules now provide a visible list focus boundary, readable
hover and selected-hover states, and a compact keyboard-hint capsule.
CommandPaletteDialog retains query filtering, stable IDs, current-row
selection, item activation, return-key acceptance, locale, and modal policy;
ADR-0224 records the bounded decision. Native list metrics, accessibility,
and release evidence remain open.

D176 / UI-88 / ARCH-163 completes the FindBar visual-rhythm slice without
moving search or replacement ownership. The centralized FindBar rules now
provide a calmer 12px utility card, secondary semibold field labels, a
hover/focus surface for the case option, and a more breathable status capsule.
FindBar and FindSurface retain query/replacement values, signals, keyboard
handling, action roles, cancellation, locale, and operation-state projection;
ADR-0225 records the bounded decision. Native FindBar metrics,
accessibility, and release evidence remain open.

D177 / UI-89 / ARCH-164 completes the Settings-guidance hierarchy slice
without moving settings ownership. The centralized Settings rules now render
the apply-after-save and font-fallback notes as compact supporting capsules
with distinct accent edges. SettingsDialog and SettingsPreviewSurface retain
locale, preview, SettingsSnapshot, Save/Cancel, font fallback, persistence,
and motion policy; ADR-0226 records the bounded decision. Native dialog
metrics, accessibility, and release evidence remain open.

D178 / UI-90 / ARCH-165 completes the message-dialog action-hierarchy slice
without moving message or close policy. MessageSurface now binds Save,
Discard, Cancel, and explicit About/Error OK buttons to the existing
primary/warning/quiet visual roles after standard-button creation. The
MessageSurface return mapping, default Save, localization, recovery policy,
and application ownership remain unchanged; ADR-0227 records the bounded
decision. Native message-box metrics, accessibility, and release evidence
remain open.

D179 / UI-91 / ARCH-166 completes the plugin-catalog guidance-capsule slice
without moving plugin governance. The centralized catalog selector now gives
the existing `dialogHint` a supporting surface, border, pink accent edge,
secondary text, radius, and spacing hierarchy. PluginCatalogDialog retains
hint text, locale refresh, word-wrap, list selection, approve/revoke signals,
trust/approval policy, and application ownership; ADR-0228 records the
bounded decision. Native dialog metrics, accessibility, and release evidence
remain open.

D180 / UI-92 / ARCH-167 completes the font-style settings contract without
moving settings or editor ownership. The Qt-free `FontStyle` value contract,
schema-v3 validation, legacy fallback, localized controls, QFont preview, QSS
projection, and live editor-adapter application reuse the existing settings
seams. ADR-0229 records the bounded decision; native font fallback, metrics,
accessibility, DPI, and release evidence remain open.

D181 / UI-93 / ARCH-168 completes status accessibility localization without
moving notification or lifecycle policy. `presentation.i18n` owns the
localized status labels/templates, `StatusRail` owns its Qt object tree, and
`StatusSurface` owns notification accessibility projection. ADR-0230 records
the bounded decision; native screen-reader output, accessibility-tree
behavior, and release evidence remain open.

D182 / UI-94 / ARCH-169 completes the framework-neutral theme-token boundary.
`presentation.theme_tokens` owns immutable Qt-free palette/contrast
resolution, while `presentation.theme` remains the sole Qt palette/QSS,
authored-icon, and editor-adapter projection owner. ADR-0231 records the
compatibility façade and bounded migration; native rendering and release
evidence remain open.

D183 / UI-95 / ARCH-170 completes the restrained surface-gradient hierarchy.
Only the main window, editor shell, and command rail receive
`ThemeColors.surface_0/1/2` gradients; the editor canvas, controls,
interaction states, and application policy remain unchanged. ADR-0232 records
the bounded decision; native QSS painting, screenshots, and release evidence
remain open.

D184 / ARCH-171 completes a local distribution-script boundary without
claiming a verified installer. `packaging/QuillForge.Distribution.psm1`
centralizes path, SHA-256, state, rollback, and opt-in HKCU association
helpers; the install/update/uninstall entry points are local-only, user-local,
`ShouldProcess`-gated, and refuse broad/root or unrelated registry ownership.
ADR-0233 records that the portable release remains the source of truth, while
the scripts are an explicit, unexecuted operational contract. Installer,
updater, association, clean-machine, signing, and release-owner evidence
remain open.

D185 / UI-96 / ARCH-172 completes a focused form-control highlight slice
without moving settings or interaction ownership. The centralized stylesheet
now gives the existing QLineEdit/QSpinBox/QComboBox focus state a stronger
surface cue and gives ComboBox popup items explicit hover and selected
highlight semantics with an accent rail. Object names, values, signals,
locale, fonts, motion, layout, and application policy remain unchanged;
ADR-0234 records the bounded decision. Native QSS popup metrics, rendering,
accessibility, DPI, and release evidence remain open.

D186 / UI-97 / ARCH-173 closes the workspace file-entry discoverability gap
without creating a second document-open path. `WorkspacePanel` exposes a
localized “Open file” action, `WorkspaceSurface` forwards the semantic intent,
and `MainWindow` reuses the existing `DocumentPickerAdmissionCoordinator`,
`FileDialogSurface`, and asynchronous document-open boundary. Folder selection,
tree file activation, values, signals, locale, loading policy, and application
ownership remain intact. ADR-0235 records the bounded decision; native dialog
behavior, GUI runtime, and release evidence remain open.

D187 / UI-98 / ARCH-174 keeps the Settings action rail reachable while the
dialog content grows beyond a short viewport. `SettingsDialog` remains the
owner of form controls, preview projection, locale refresh, and snapshot
assembly. Its existing appearance group, preview, editor group, and guidance
notes now sit in one named `QWidget#settingsContent` inside a resizable
`QScrollArea#settingsScroll`; `QDialogButtonBox#dialogActions` remains in the
outer dialog layout and is therefore not part of the scrollable content. The
change adds no settings/application contract or second styling system; scoped
QSS reuses the existing token-driven scrollbar rules. ADR-0236 records the
bounded decision. Native size hints, scroll metrics, focus/accessibility, DPI,
GUI runtime, and release evidence remain open.

The D188 static revalidation closes the remaining plan-level contract-audit
checkbox without changing runtime code. `scripts/audit_presentation_contracts.py`
passes; the source inventory contains 43 frozen/slotted coordinator Ports
contracts, coordinator imports contain zero forbidden Qt/TaskRunner/MainWindow/
StatusSurface dependencies, and MainWindow notification calls have zero missing
explicit levels. This is static architecture evidence only; it does not infer
native event timing, runtime observability completeness, or release readiness.

D189 / UI-99 / ARCH-175 keeps workspace-search result-state ownership inside
`WorkspaceSearchDialog`. The existing `QListWidget` and one localized empty
state are composed by a single `QStackedLayout`; `WorkspaceSearchResult`, item
roles, activation signals, diagnostics, cancellation, and MainWindow/service
ownership do not move. Empty-state feedback reuses the existing semantic
feedback contract and scoped theme tokens, so the change adds no coordinator or
second styling system. Native stack/list rendering and runtime/release evidence
remain open.

D190 / UI-100 / ARCH-176 keeps Command Palette result-state ownership inside
`CommandPaletteDialog`. The existing command list and one localized empty
state are composed by a single `QStackedLayout`; command iteration, stable item
IDs, current-row selection, Enter/item activation, Esc close, locale handoff,
and MainWindow/CommandRegistry execution ownership do not move. The empty state
uses the centralized theme owner without adding a command coordinator or second
styling system. Native stack/list rendering and runtime/release evidence remain
open.

D191 / UI-101 / ARCH-177 keeps dynamic Find/Replace message grammar inside
`presentation.i18n`. Two strict count-bearing matchers delegate to the existing
`find.status.replaced_count` and `find.status.limit` catalog keys, while the
editor operation, worker, signals, status severity, persistence, and MainWindow
ownership remain unchanged. Removing the generic `Replaced ` prefix avoids a
partial Chinese/English message without adding a locale service or second
translation boundary. Native text metrics, accessibility, DPI, GUI runtime,
and release evidence remain open.

D192 / UI-102 / ARCH-178 keeps plugin failure phase translation inside the
existing `_localize_plugin_failure` adapter in `presentation.i18n`. A bounded
map covers the four known runtime phase values and leaves unknown phases,
plugin IDs, and diagnostic details unchanged. Plugin manager/event-bus
lifecycle, trust, containment, command refresh, notification severity, and
application ownership do not move. Native text metrics, accessibility, DPI,
GUI runtime, and release evidence remain open.

D193 / UI-103 / ARCH-179 keeps the count-bearing close-block message inside
`presentation.i18n`. The existing MessageSurface route now resolves one strict
pending-work shape through `error.wait_pending`, preserving the count and
English identity while leaving CloseGuard, TaskRunner, session-save, worker
draining, and shutdown ownership unchanged. Native text metrics, accessibility,
DPI, GUI runtime, and release evidence remain open.

D194 / UI-104 / ARCH-180 keeps native control affordance styling inside the
existing `presentation.theme` QSS projection. Explicit ComboBox and SpinBox
arrow subcontrols, plus the checked-checkbox focus boundary, consume the same
ThemeColors tokens as their parent controls; widget construction, signals,
settings values, locale, motion, application services, and domain ownership do
not move. This removes mixed Fusion-default geometry from the visual contract
without introducing a custom widget or a second asset/style pipeline. Qt's
public Style Sheets Reference is the applicable framework source; native QSS
parsing/painting, accessibility, DPI, and release evidence remain open.

D195 / ARCH-181 keeps packaging provenance and artifact staging inside the
existing `scripts/package.ps1` boundary. The source inventory and manifest
contract remain unchanged while relative paths and source-revision hashing use
APIs available to both Windows PowerShell 5.1 and PowerShell 7. No shell
compatibility helper is imported into application code, and no installer,
registry, network, or release-policy ownership moves. Microsoft API
documentation is the applicable public source; external release evidence
remains open.

D196 / ARCH-182 keeps release-dossier parsing and gate evaluation inside the
existing `scripts/verify_release_handoff.ps1` boundary. One local JSON adapter
preserves PowerShell 7's string-date behavior, Windows PowerShell 5.1's
available conversion surface, and explicit UTF-8 decoding for the existing
reports; gate predicates, mechanical-failure names, dossier fields, and no-go
semantics remain unchanged. No report refresh or runtime/release decision
ownership moves.

D197 / ARCH-183 keeps packaged-capture process and artifact policy inside the
existing `scripts/measure_packaged.ps1` boundary. Its manifest and per-run
report reads now use the same local JSON compatibility pattern and explicit
UTF-8 decoding; Qt offscreen process lifecycle, artifact binding, capture
validation, cleanup, and measurement ownership do not move. Runtime capture
remains an explicit authorized evidence gate.

D198 / UI-105 / ARCH-184 keeps menu affordance styling inside the existing
`presentation.theme` QSS projection. `QMenu::indicator` normal, hover,
checked, checked-hover, and disabled states plus the selected-and-checked item
rule consume the existing `ThemeColors` tokens; `CommandSurface`, QAction
projection, shortcuts, locale, and command execution do not move. No custom
menu, resource pipeline, or application-layer dependency is introduced. Qt's
public Style Sheets Reference is the applicable framework source; native menu
painting, accessibility, DPI, and release evidence remain open.

D199 / ARCH-185 closes the command-menu admission boundary in the application
layer. `application.commands` owns the ordered `MenuId` literal contract and
rejects unsupported values before registry admission; `presentation.command_surface`
reuses the same constant for its four localized top-level menus. This prevents
registered-but-invisible commands without introducing dynamic menus, a Tools
fallback, Qt dependencies into application code, or a plugin policy change.

D200 / ARCH-186 keeps source provenance inside `scripts/package.ps1`. Existing
canonical `relative-path=hash` lines are collected in a generic string list and
sorted with ordinal comparison before the source revision is hashed. The
source inventory, artifact binding, manifest schema, and packaging ownership
remain unchanged, and both supported PowerShell hosts now record the same
source revision. Separate PyInstaller runs may still produce different bytes;
the manifest binds each artifact independently.

D201 / UI-106 / ARCH-187 keeps disabled menu-state correction in the existing
`presentation.theme` QSS owner. The selected-disabled and checked-disabled
selectors are ordered after the generic disabled rule and explicitly project
the subdued `surface_2`/`text_muted` state with a `border_strong` left edge.
No QAction, menu registry, locale, motion, palette, or application dependency
is introduced; Qt's public Style Sheets Reference is the applicable framework
source. Native selector specificity and menu rendering remain open evidence.

D202 / ARCH-188 keeps workspace directory classification behind the existing
`application.ports.WorkspaceProvider` contract. `WorkspaceService` retains
path normalization, selected-root containment, entry limits, and user-facing
error policy, while `FileWorkspaceProvider` owns the concrete
`Path.is_dir()` predicate and directory enumeration. No search-provider,
document-path, Qt, or presentation dependency is introduced. Python's public
`pathlib` documentation is the applicable first-party source for the
concrete-path I/O boundary; live filesystem races and runtime evidence remain
open.

D203 / ARCH-189 promotes the directory predicate to the shared
`application.ports.DirectoryCapability` contract. `WorkspaceProvider` and
`WorkspaceSearchProvider` reuse the same capability; `WorkspaceService` and
`WorkspaceSearchService` retain normalization, containment, limits,
cancellation, result validation, and error policy, while their concrete file
adapters own `Path.is_dir()` and traversal. No directory service, Qt
dependency, or search-policy rewrite is introduced. Python's public
`pathlib` documentation remains the applicable first-party source; live
filesystem races and runtime evidence remain open.

D204 / UI-107 / ARCH-190 keeps the workspace-search diagnostics busy-state
projection inside the existing centralized `presentation.theme` QSS owner.
The scoped diagnostics toggle `:disabled` rule is ordered after its checked
state and reuses `surface_2`, `border`, `border_strong`, and `text_muted` so a
busy search is visibly non-actionable without changing
`WorkspaceSearchDialog.set_busy()`, signals, search policy, locale, or
application ownership. Qt's public Style Sheets Reference is the applicable
first-party framework source; native selector parsing/painting and runtime
evidence remain open.

D205 / UI-108 / ARCH-191 keeps the Find/Replace navigation busy-state
projection inside the existing centralized `presentation.theme` QSS owner.
The grouped `findPrevious`/`findNext` `:disabled` rule follows their normal
and hover/focus rules and reuses `surface_2`, `border`, and `text_muted`, so a
Replace All operation visibly owns those actions without changing
`FindBar.set_operation_active()`, editor operations, locale, icons, signals,
or application ownership. Qt's public Style Sheets Reference remains the
applicable first-party framework source; native selector parsing/painting and
runtime evidence remain open.

D207 / UI-109 / ARCH-192 keeps the Workspace refresh busy-state projection
inside the existing centralized `presentation.theme` QSS owner. The scoped
`QTreeWidget#workspaceTree:disabled` rule follows the tree's normal/focus
rules and reuses `surface_2`, `border`, and `text_muted`, so an already-visible
directory tree remains contextual but visibly non-actionable during provider
refresh without changing `WorkspacePanel.set_loading()`, row selection,
navigation, cancellation, signals, or provider ownership. Qt's public Style
Sheets Reference remains the applicable first-party framework source; native
selector parsing/painting and runtime evidence remain open.

D208 / UI-110 / ARCH-193 keeps the Replace All locked-editor projection inside
the existing centralized `presentation.theme` QSS owner. The scoped
`QsciScintilla#editor:disabled` rule follows normal/focus styling and reuses
`surface_1`, `border_strong`, and `pressed` without overriding document text,
lexer, syntax, caret, or editor adapter policy. `EditorWidget` and the existing
Replace All coordinator retain all lock/unlock ownership. Qt's public Style
Sheets Reference remains the applicable first-party framework source; native
QScintilla painting and runtime evidence remain open.

D209 / UI-111 / ARCH-194 keeps unavailable plugin action projection inside the
existing centralized `presentation.theme` QSS owner. One grouped scoped rule
targets the four existing `primaryAction`/`warningAction` button identities in
the Plugin Catalog and Plugin Status dialogs and reuses `surface_2`, `border`,
`border_strong`, and `text_muted`. Catalog/status enablement predicates,
signals, trust/approval/lifecycle policy, locale, and application ownership do
not move. Qt's public Style Sheets Reference is the applicable first-party
framework source; native selector parsing/painting, accessibility, DPI, and
release evidence remain open.

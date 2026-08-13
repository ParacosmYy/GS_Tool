# QuillForge continuation checklist

- [x] D315 / UI-133 / ARCH-285 Readable checkbox indicator states: preserve
  native checkbox interaction while making checked, hover, focus, and disabled
  indicators readable through token-derived surfaces and edge fallbacks; verify
  all 3 themes and 4 accents, rebuild the portable candidate, and record
  review/package/handoff evidence without launching EXE/Qt.

- [x] D314 / UI-132 / ARCH-284 Theme-aware combo-box expanded affordance:
  centralize explicit QComboBox drop-down geometry and popup-open state for
  language, theme, accent, font-family, and font-style choices; preserve native
  popup/keyboard semantics, verify the 3-theme/4-accent matrix and presentation
  contract, rebuild the portable candidate, and record review/package/handoff
  evidence without launching EXE/Qt.

- [x] D313 / UI-131 / ARCH-283 Theme-aware typography steppers: style the
  existing Settings font-size QSpinBox up/down subcontrols with theme tokens,
  preserve native value behavior, verify the 3-theme/4-accent matrix and
  presentation contract, rebuild the portable candidate, and record
  review/package/handoff evidence without launching EXE/Qt.

- [x] D312 / UI-130 / ARCH-282 Theme-aware scrollbars: centralize compact
  vertical/horizontal track and handle styling with readable normal,
  hover, and pressed states; preserve native scrolling behavior, verify the
  3-theme/4-accent matrix and presentation contract, rebuild the portable
  candidate, and record review/package/handoff evidence without launching
  EXE/Qt.

- [x] D311 / UI-129 / ARCH-281 Theme-aware shell separators: centralize
  readable main-window and dock-panel separator edges with existing theme
  tokens, add alternate-accent hover feedback, verify the 3-theme/4-accent
  matrix and presentation contract, rebuild the portable candidate, and
  record review/package/handoff evidence without launching EXE/Qt.

- [x] D310 / ARCH-280 Safe startup recovery path: add an explicit
  `--safe-mode` entry that uses defaults, skips session/recovery restore and
  built-in plugin activation, preserves explicit file paths, verifies the
  source contract and composition boundary, rebuilds the portable candidate,
  and records review/package/handoff evidence without launching EXE/Qt.

- [x] D309 / UI-128 / ARCH-279 Theme-aware Tooltip surface: centralize a
  token-bound QToolTip surface with readable text and edge fallback, verify the
  theme/accent matrix and static contract, rebuild the portable candidate, and
  record review/package/handoff evidence without launching EXE/Qt.

- [x] D308 / UI-127 / ARCH-278 Workspace open-action affordance: assign
  semantic folder/file/navigation/cancel roles, add localized action hints and
  accessible descriptions, preserve the existing file-open chain, verify the
  theme edge matrix and static contract, rebuild the portable candidate, and
  record review/package/handoff evidence without launching EXE/Qt.

- [x] D307 / UI-126 / ARCH-277 Settings unsaved-draft status: capture the
  opened Settings snapshot, show localized clean/changed state after preview
  and behavior edits, preserve Save/Cancel/RestoreDefaults boundaries, verify
  the theme matrix and static contract, rebuild the portable candidate, and
  record review/package/handoff evidence without launching EXE/Qt.

- [x] D306 / UI-125 / ARCH-276 Settings draft restore defaults: add a localized
  RestoreDefaults action that resets 12 current controls from DEFAULT_SETTINGS
  with signal blocking, preserves Save/Cancel persistence boundaries, verifies
  the theme matrix and static contract, rebuilds the portable candidate, and
  records review/package/handoff evidence without launching EXE/Qt.

- [x] D305 / UI-124 / ARCH-275 Localized Settings accessible names: refresh
  accessibility names for nine value controls and three behavior controls at
  the existing locale boundary, verify the AST contract, rebuild the portable
  candidate, and record review/package/handoff evidence without launching
  EXE/Qt.

- [x] D304 / UI-123 / ARCH-274 Locale choice role: consolidate the Settings
  language selector behind `settingsRole="localeChoice"`, preserve language
  item data/current values, immediate locale refresh, snapshot, signals,
  persistence, and keyboard behavior, and record package/review/handoff
  evidence without launching EXE/Qt.

- [x] D303 / UI-122 / ARCH-273 Behavior toggle role: consolidate Settings
  wrapping, line-number, and motion checkbox states behind one semantic role,
  retain editor/interface tone cues, and preserve values, signals, persistence,
  keyboard behavior, and generic indicators.

- [x] D302 / UI-121 / ARCH-272 Typography choice role: give interface/editor
  font family, size, and style controls one semantic presentation role with
  distinct interface/editor tone cues, preserving settings values, previews,
  signals, persistence, keyboard behavior, and disabled state.

- [x] D301 / UI-120 / ARCH-271 Semantic settings choice role: replace duplicated theme /
  accent ID selectors with the shared `settingsRole="identityChoice"`
  presentation contract while preserving values, icons, signals, persistence,
  and keyboard behavior; record canonical audit, source diagnostics, package,
  review, simplification, and handoff evidence without launching EXE/Qt.

- [x] D300 / UI-119 Settings choice hierarchy: visually distinguish the theme
  and accent selectors with centralized semantic QSS states while preserving
  existing values, icons, keyboard behavior, and persistence.

- [x] D299 / ARCH-269 Frozen Qt plugin root selection: prefer a frozen Qt
  plugin root containing `platforms/qwindows.dll`, reuse the selector for
  configuration and diagnostics, harden its AST-backed contract after review,
  rebuild the portable candidate, and record package/review/handoff evidence
  without launching EXE/Qt.

- [x] D298 / ARCH-268 Frozen Qt layout fallback: align frozen Qt DLL checks with
  PyInstaller's Qt6-first/Qt legacy layout policy, preserve D297 diagnostics and
  fail-fast behavior, rebuild the portable candidate, and record review/package/
  handoff evidence without launching EXE/Qt.

- [x] D297 / ARCH-267 Frozen Qt runtime preflight: fail fast before normal
  QApplication construction when a frozen Qt/QScintilla/platform bundle file
  is missing, preserve plugin-host/diagnostic/source behavior, rebuild the
  portable candidate, and record review/package/handoff evidence without
  launching EXE/Qt.

- [x] D296 / ARCH-266 Frozen Qt plugin path binding: bind the frozen PyQt6
  plugin and platform-plugin paths to the extracted application bundle before
  QApplication import, preserve source/diagnostic behavior, rebuild the
  portable candidate, and record static/package/archive/review/handoff evidence
  without launching EXE/Qt.

- [x] D295 / ARCH-265 TaskRunner abnormal termination boundary: prevent
  non-`Exception` worker termination from entering the success callback, keep
  existing Exception failure contracts, guard the conversion statically,
  rebuild the portable candidate, and record review/package/handoff evidence
  without launching EXE/Qt.

- [x] D294 / ARCH-264 startup diagnostic QApplication lifecycle: reuse one Qt
  application across runtime/restore probes, eliminate the reproduced repeated
  window-class warning, guard ownership/cleanup statically, rebuild the
  portable candidate, and record review/package/handoff evidence without
  launching EXE/Qt.

- [x] D293 / ARCH-263 TaskRunner submission rollback boundary: roll back
  retained tasks when signal connection or pool start fails, share an idempotent
  release path with queued completion, guard the contract statically, rebuild
  the portable candidate, and record review/package/handoff evidence without
  launching EXE/Qt.

- [x] D292 / ARCH-262 runtime shutdown activity boundary: stop periodic
  recovery/session-save activity through a presentation-owned lifecycle port
  before plugin deactivation, guard the order statically, rebuild the portable
  candidate, and record review/package/handoff evidence without launching
  EXE/Qt.

- [x] D291 / ARCH-261 entrypoint runtime cleanup: keep normal `runtime.start()`
  inside the existing `try/finally`, add the targeted lifecycle audit, rebuild
  the portable candidate, and record independent review/package/handoff
  evidence without launching EXE/Qt.

- [x] D290 / ARCH-260 startup preflight wait refinement: package the bounded
  Qt wake-up change, bind its identity, and record before/after event-batch
  evidence without launching EXE/Qt.
- [x] D289 / ARCH-259 file-open preflight: rebuild the portable candidate,
  bind its package identity, and record explicit regular-file opening evidence
  without launching EXE/Qt.
- [x] D288 / ARCH-258 startup restore preflight: rebuild the portable candidate,
  bind its package identity, and record bounded asynchronous restore evidence
  without launching EXE/Qt.
- [x] D287 / ARCH-257 startup state preflight: rebuild the portable candidate,
  bind its package identity, and record the read-only session/recovery startup
  evidence without launching EXE/Qt.

- [x] D286 / ARCH-256 editor-shell startup preflight: construct the first empty
  editor/QScintilla tab through the production path, stop diagnostic timers in
  cleanup, rebuild the portable candidate, and record review/package/handoff
  evidence without launching EXE/Qt.
- [x] D285 / ARCH-255 startup preparation preflight: share pre-show plugin
  preparation and command refresh between normal startup and the no-window
  diagnostic, preserve the original startup order, rebuild the portable
  candidate, and record review/package/handoff evidence without launching
  EXE/Qt.
- [x] D284 / ARCH-254 startup runtime-composition preflight: extend the
  no-window startup report through the existing DesktopRuntime/MainWindow
  constructor graph with a temporary QApplication, preserve the no-show/no-
  event-loop boundary, rebuild the portable candidate, and record review,
  package, and handoff evidence without launching EXE/Qt.
- [x] D283 / ARCH-253 startup-error log lifecycle: clear only the prior
  QuillForge startup-error log before each entry attempt, preserve fail-open
  current-failure recording, add a Qt-free lifecycle contract, rebuild the
  portable candidate, and record review/package/handoff evidence without
  launching EXE/Qt.
- [x] D282 / ARCH-252 MainWindow startup-state initialization: establish the
  existing `_busy=False` default before coordinator callback wiring, add a
  Qt-free construction-order contract, rebuild the portable candidate, and
  record review/package/handoff evidence without launching EXE/Qt.
- [x] D281 / ARCH-251 startup diagnostic user guidance: document the existing
  no-window command, settings preflight fields, exit codes, startup-error log,
  privacy boundary, and native-startup limitation while keeping the D280
  artifact unchanged and recording review/handoff evidence.
- [x] D280 / ARCH-250 startup settings preflight: extend the Qt-free startup
  diagnostic with settings path/presence/decode/normalized-schema evidence,
  preserve the existing safe fallback and no-preference-value boundary,
  rebuild the portable candidate, and record review/package/handoff evidence
  without launching EXE/Qt.
- [x] D279 / ARCH-249 font availability status projection: show localized
  installed/fallback/unknown status for interface/editor font choices while
  preserving raw family persistence, rebuild the portable candidate, and
  record static/source-diagnostic/archive/review/handoff evidence without
  launching EXE/Qt.
- [x] D278 / ARCH-248 command-surface startup contract: preserve the existing
  locale-provider/accessor startup fix, guard the MainWindow ordering that
  first consumes it, rebuild the portable candidate, and record static/source
  diagnostic/archive/review/handoff evidence without launching EXE/Qt.
- [x] D277 / ARCH-247 settings typography contract audit: guard the existing
  language, theme/accent, UI/editor font family, size, style, persistence,
  live application, and motion-transition routes with a Qt-free contract;
  rebuild the portable candidate and record review/package/handoff evidence
  without launching EXE/Qt.
- [x] D276 / ARCH-246 workspace file-open contract audit: enforce the
  existing file-vs-folder intent split from WorkspacePanel through
  WorkspaceSurface, WorkspaceFileActivationCoordinator, and asynchronous
  document opening, preserve containment/tab/busy/session policy, rebuild the
  portable candidate, and record review/package/handoff evidence without
  launching EXE/Qt.
- [x] D275 / ARCH-245 QSS semantic contrast contract: centralize the existing
  derived foregrounds for accent-alt, selection, warning, success, working,
  and error surfaces in the framework-neutral token layer; reuse the same
  resolver from runtime QSS/palette projection and the expanded 3-theme ×
  4-accent audit; rebuild the portable candidate and record review/package/
  handoff evidence without launching EXE/Qt.
- [x] D273 / ARCH-244 theme contrast regression audit: enforce the existing
  readable foreground contract across 3 themes × 4 accents and 144 semantic
  text pairs, preserve runtime token/QSS behavior, rebuild the portable
  candidate, and record static/package/review/handoff evidence without
  launching EXE/Qt.
- [x] D270 / ARCH-243 static notification localization audit: extend the
  existing presentation AST gate to catch new static ASCII `notify(...)`
  literals that are unchanged by the zh-CN localizer, preserve dynamic
  diagnostics and runtime behavior, rebuild the portable candidate, and record
  static/package/review/handoff evidence without launching EXE/Qt.
- [x] D264 / ARCH-242 secondary startup fallback localization: preserve the
  final English fail-open fallback while projecting Chinese labels when the
  outer startup-error construction fails, rebuild both candidates, and record
  source/PE/package/review/handoff evidence without launching EXE/Qt.
- [x] D263 / ARCH-241 workspace-search outside-workspace placeholder
  localization: project the outside-root diagnostic path fallback through the
  existing search catalog and locale-refresh path, preserve containment and
  result state, rebuild both candidates, and record static/package/review/
  handoff evidence without launching EXE/Qt.
- [x] D262 / ARCH-240 early startup fallback localization: keep the pre-Qt
  startup failure path Qt-free while projecting Chinese fallback labels from
  system locale hints, preserve English/diagnostic/exit-code behavior, rebuild
  both candidates, and record static/package/review/handoff evidence without
  launching EXE/Qt.
- [x] D261 / ARCH-239 file-dialog default-name localization: reuse the
  localized `document.untitled` fallback for Save As when no current path
  exists, preserve existing paths and `.txt`, rebuild both candidates, and
  record static/package/review/handoff evidence without launching EXE/Qt.

- [x] D260 / ARCH-238 font-size unit locale refresh: route the Settings interface/editor font-size suffix through the locale catalog and existing refresh path, preserve integer values and ranges, rebuild both candidates, and record static/package/review/handoff evidence without launching EXE/Qt.
- [x] D259 / ARCH-237 message-dialog standard button localization: explicitly project Save/Discard/Cancel/OK through the selected locale after QMessageBox button creation, preserve StandardButton semantics and roles, rebuild both candidates, and record static/package/review/handoff evidence without launching EXE/Qt.
- [x] D258 / ARCH-236 Replace All error localization: route bounded match-limit, text-capture, and rollback-failure messages through the existing presentation localizer, preserve editor transaction behavior and unknown diagnostics, rebuild both candidates, and record static/package/review/handoff evidence without launching EXE/Qt.
- [x] D257 / ARCH-235 built-in plugin status-name localization: route the built-in Document Statistics status name through the existing stable-ID presentation resolver, preserve en-US and external plugin metadata, rebuild both candidates, and record static/package/review/handoff evidence without launching EXE/Qt.

- [x] D256 / ARCH-234 built-in document-statistics localization: route the built-in command title and exact/dynamic notifications through the existing presentation i18n boundary, preserve en-US and external plugin fallback behavior, rebuild both candidates, and record static/package/review/handoff evidence without launching EXE/Qt.
- [x] D255 / ARCH-233 application-error localization boundary: localize bounded user-visible application and coordinator-wrapper details through the existing presentation i18n boundary, preserve dynamic diagnostics and en-US behavior, rebuild both candidates, and record static/package/review/handoff evidence without launching EXE/Qt.
- [x] D252 / ARCH-230 plugin diagnostic locale refresh: route runtime-status errors and catalog-entry reasons through the shared presentation localizer, preserve unknown provider text and plugin policy, rebuild both candidates, and record static/package/review/handoff evidence without launching EXE/Qt.
- [x] D253 / ARCH-231 workspace-entry error locale refresh: retain inaccessible-entry error sources in the workspace item projection, localize known provider reasons and refresh disabled-row tooltips on locale changes without changing file/folder activation or provider policy, rebuild both candidates, and record static/package/review/handoff evidence without launching EXE/Qt.
- [x] D254 / ARCH-232 frozen-startup runtime dependency diagnostic: report explicit Qt/QScintilla/platform dependency presence and missing relative paths in the existing no-window frozen diagnostic without changing normal startup, rebuild both candidates, and record static/package/review/handoff evidence without launching EXE/Qt.
- [x] D251 / ARCH-229 workspace-search diagnostic locale refresh: retain the latest immutable result for source-backed diagnostic re-projection, localize known provider reasons/prefixes while preserving path/detail suffixes and expansion state, rebuild both candidates, and record static/package/review/handoff evidence without launching EXE/Qt.
- [x] D250 / ARCH-228 workspace search status locale refresh: retain catalog/error/summary source state and reproject current status after locale changes, rebuild both candidates, and record static/package/review/handoff evidence without launching EXE/Qt.
- [x] D249 / ARCH-227 workspace error locale refresh: retain the current typed/string workspace error source, reproject it after locale changes, clear it on successful directory/new-load state, rebuild both candidates, and record static/package/review/handoff evidence without launching EXE/Qt.
- [x] D248 / ARCH-226 workspace typed-error projection: preserve workspace navigation exceptions through the existing panel surface so zh-CN can classify permission/path failures, rebuild both candidates, and record static/package/review/handoff evidence without launching EXE/Qt.
- [x] D247 / ARCH-225 typed secondary error localization: preserve Exception objects through the settings-save and Replace All presentation handlers so zh-CN can retain filesystem/codec context, rebuild both candidates, and record static/package/review/handoff evidence without launching EXE/Qt.
- [x] D246 / ARCH-224 startup path stat routing: replace swallowing file/dir predicates with one stat-based classifier, preserve async folder/file/special-file routing, retain concrete path errors, rebuild both candidates, and record static/package/review/handoff evidence without launching EXE/Qt.
- [x] D245 / UI-27 typed file-error localization: preserve typed open/save exceptions through the existing error surface, localize common filesystem/codec failures in zh-CN while preserving en-US and diagnostics, rebuild both candidates, and record static/package/review/handoff evidence without launching EXE/Qt.
- [x] D244 / UI-26 startup-path error localization: localize the two missing explicit-file-launch failure prefixes while preserving English output, path/detail text, package identity, archive evidence, and no-launch boundaries.
- [x] D243 / UI-25 readable accent text endpoints: route surface text that previously used raw `accent_alt` through one conservative foreground resolver, rebuild both candidates, and record contrast, archive, package, review, and handoff evidence without launching EXE/Qt.
- [x] D242 / ARCH-223 file dialog all-files default: make the existing localized open/save filter default to `All files (*)` while retaining the text/source filter, rebuild both candidates, and record static/package/handoff evidence without launching EXE/Qt or native dialogs.
- [x] D241 / ARCH-222 startup provenance diagnostic: extend only the no-window startup diagnostic with frozen import provenance and Qt plugin-path evidence, reuse the selected platform-plugin path, rebuild both candidates, and record static/package/review/handoff evidence without launching EXE/Qt.
- [x] D240 / ARCH-221 frozen entrypoint path hardening: restrict direct-source `sys.path` setup to non-frozen execution, preserve package/frozen imports and diagnostics, rebuild both candidates, and record static/package/review/handoff evidence without launching EXE/Qt.
- [x] D239 / ARCH-220 target identity preflight: validate update target and rollback target/backup SHA-256 identity after confirmation and before any file move, reject identical paths, rebuild both package candidates, and record static/package/handoff evidence without executing updater/rollback or launching EXE/Qt.
- [x] D238 / ARCH-219 update transaction safety: track local updater/rollback file exchanges and state commit boundaries, restore prior target/backup layout only under exact hash/path guards, preserve uncertain external changes, rebuild both package candidates, and record static/package/handoff evidence without executing updater/rollback or launching EXE/Qt.
- [x] D237 / ARCH-217 multi-association safety: allow the opt-in installer to register multiple extensions through one operation-owned ProgId, guard shared-key deletion against unknown references, preserve the executable on incomplete rollback/uninstall cleanup, rebuild both package candidates, and record static/package/handoff evidence without launching EXE/Qt or touching registry state.
- [x] D235 / ARCH-216 startup-path edge routing: preserve one/two-dash Qt value options, warn on non-temporary startup admission rejection, rebuild the portable EXE, and record source/package/review/handoff evidence without launching EXE/Qt or touching registry state.
- [x] D233 / ARCH-215 desktop file-launch routing: separate Qt options from explicit file/directory paths, queue them behind recovery/session restoration, reuse existing async document/workspace admission, rebuild the portable EXE, and record source/package/review/handoff evidence without launching EXE/Qt.
- [x] D232 / ARCH-214 guarded entry-point application import: resolve `app.main` inside the existing startup exception boundary, preserve the public `main(argv)` contract and direct/bundled paths, and record source, package, review, and handoff evidence without launching EXE/Qt.
- [x] D231 / ARCH-213 MessageBox zero-return fallback: continue stderr when Win32 reports a zero return, preserve nonzero behavior and exit code, and record source probes, review, package refresh, and handoff evidence without launching EXE/Qt.
- [x] D230 / ARCH-212 startup fallback display fail-open: protect exception stringification, native MessageBox fallback, and stderr output without changing ordinary failure text or exit code; record source probes, review, package refresh, and handoff evidence without launching EXE/Qt.
- [x] D229 / ARCH-211 startup diagnostic path fail-open: protect path resolution, context fallback, payload construction, and report writes so early diagnostics cannot mask the original startup exception; record source probes, review, package refresh, and handoff evidence without launching EXE/Qt.
- [x] D228 / ARCH-210 startup failure context record: add fail-open executable/working-directory/frozen/runtime/bundle metadata to the existing early startup log, rebuild the candidate, and record source/package/handoff evidence without launching EXE/Qt.
- [x] D227 / ARCH-209 private attribute declaration-aware audit: teach the existing presentation AST gate to recognize direct class-level assignments and annotated fields, preserve the narrow private-call scope, rebuild the candidate, and record static/package/handoff evidence.
- [x] D226 / ARCH-208 private self-call contract audit: extend the existing presentation AST gate to catch missing direct private accessors before shell startup, preserve assigned callable providers, rebuild the candidate, and record static/package/handoff evidence.
- [x] D220 / UI-118 main-shell low-noise visual hierarchy: flatten four existing shell selectors, validate 3-theme × 4-accent state/contrast projection, record reviews/simplification, package identity, and expected release NO-GO.
- [x] D222 / ARCH-205 windowed startup-failure boundary: record early startup tracebacks and surface a native Windows fallback message without changing application behavior; verify source, archive, package identity, and expected release NO-GO.
- [x] D223 / ARCH-206 no-window packaged startup diagnostic: add `--diagnose-startup --report <path>` before `QApplication`, report runtime/Qt/QScintilla/composition/frozen-resource checks, and record source probe, dual-shell package, archive, identity, and expected release NO-GO evidence.
- [x] D225 / ARCH-207 command-surface locale accessor startup fix: resolve the captured missing `_locale` AttributeError through the existing provider, rebuild both package candidates, and record root-cause, archive, identity, check, and handoff evidence.
- [x] D221 / ARCH-204 i18n literal-key static gate: compare literal presentation `tr()` keys with the canonical English catalog and record the static/package evidence.
- [x] D219 / ARCH-203 remaining application error taxonomy: migrate nine Qt-free application modules to stable built-in-compatible validation/state categories, record reviews/simplification, package identity, and expected release NO-GO.

- [x] UI-13 FindBar action hierarchy and bright-accent contrast fix.
- [x] Update roadmap/register/acceptance/handoff evidence for UI-13.
- [x] Rebuild and verify the UI-13 package identity.
- [x] Deploy and validate `skills/quillforge-ui-visual-quality` and `skills/quillforge-enterprise-architecture`.
- [x] UI-14 command-palette hierarchy and mature surface rhythm.
- [x] Update roadmap/register/acceptance/handoff evidence for UI-14.
- [x] Rebuild and verify the UI-14 package identity.
- [x] Record public-source enterprise architecture baseline and ADR-0036.
- [x] Extract the desktop composition root into `src/quillforge/composition.py`.
- [x] Isolate the Qt-free workspace-search diagnostic adapter in `src/quillforge/diagnostic_composition.py`.
- [x] Extract the menu/command-rail presentation projection into `src/quillforge/presentation/command_surface.py`.
- [x] Extract the document-tab widget projection and identity/index coordination into `src/quillforge/presentation/document_tab_surface.py`.
- [x] Extract workspace dock/panel composition and semantic signal projection into `src/quillforge/presentation/workspace_surface.py`.
- [x] Extract workspace-search dialog composition and semantic signal projection into `src/quillforge/presentation/workspace_search_surface.py`.
- [x] Close the workspace result-projection boundary so MainWindow no longer imports or directly reaches into `WorkspacePanel`.
- [x] Extract FindBar composition and semantic projection into `src/quillforge/presentation/find_surface.py`.
- [x] Extract SettingsDialog composition and modal snapshot projection into `src/quillforge/presentation/settings_surface.py`.
- [x] Extract recovery prompt composition and typed decision projection into `src/quillforge/presentation/recovery_prompt_surface.py`.
- [x] Extract StatusRail composition and semantic phase projection into `src/quillforge/presentation/status_surface.py`.
- [x] Extract native file/folder/save dialog selection into `src/quillforge/presentation/file_dialog_surface.py`, including the startup-restore Open guard.
- [x] Extract command-palette modal composition into `src/quillforge/presentation/command_palette_surface.py` and localize stale-command feedback.
- [x] Extract plugin catalog/status dialog composition into `src/quillforge/presentation/plugin_surface.py` while keeping trust/approval/enablement policy in MainWindow.
- [x] Extract common save-before-close, About, and recoverable error message composition into `src/quillforge/presentation/message_surface.py` while keeping close/save/error/status policy in MainWindow.
- [x] Extract theme-transition effect and animation lifecycle into `src/quillforge/presentation/theme_transition_surface.py` while keeping motion policy and trigger timing in MainWindow.
- [x] Move QStatusBar attachment, size-grip configuration, and localized transient notification projection behind `src/quillforge/presentation/status_surface.py` while keeping phase/operation policy in MainWindow.
- [x] Move central `editorShell` QWidget/layout, `DocumentTabSurface`/`FindSurface` composition, initial FindBar hidden state, and FindBar locale routing behind `src/quillforge/presentation/editor_shell_surface.py` while keeping document/editor/operation policy in MainWindow.
- [x] Move per-document EditorWidget creation, presentation settings/theme application, Save As language refresh, and semantic signal wiring behind `src/quillforge/presentation/editor_document_surface.py` while keeping document/operation policy in MainWindow.
- [x] UI-15 visual endpoint contrast and shell rhythm: derive a dedicated readable gold foreground and refine centralized command-rail/document-tab hierarchy.
- [x] UI-16 authored vector iconography: replace platform standard icons in the command rail/workspace tree with a theme-tinted semantic provider and explicit refresh route.
- [x] UI-17 semantic transient notification hierarchy: give StatusSurface a localized statusMessage widget with explicit info/success/warning/error levels, preserve notify compatibility, and verify all supported message contrast pairs.
- [x] UI-18/D32a inline feedback hierarchy: distinguish loading, success, warning, and error states in workspace and Find in Files through centralized presentation tokens.
- [x] UI-19/D33 FindBar feedback projection: distinguish editor find/replace progress, success, warning, and error statuses through the existing FindSurface seam.
- [x] UI-20/D34a Session/Recovery notification severity closure: explicitly highlight recovery/session success, warning, info, and error outcomes through the shared status notification contract.
- [x] D34a correction: invalid/failed session persistence is error and deferred recovery is warning.
- [x] UI-21/D35a plugin/extension notification severity closure: distinguish catalog, governance, runtime, and host diagnostic in-progress info, success, warning, and failure outcomes through the shared status contract while retaining the permanent WORKING phase.
- [x] UI-22/D36a workspace/operation notification severity closure: distinguish workspace/search in-progress info, success, warning, containment, cancellation, and failure outcomes through the shared status contract while retaining the permanent WORKING phase.
- [x] UI-23/D37a MainWindow notification contract closure: make all 81 current coordinator notifications explicit info/success/warning/error while preserving the permanent WORKING phase and external compatibility.
- [x] UI-24/D38 focus-state visibility: strengthen centralized keyboard focus cues for command/tool buttons, document tabs, and settings checkboxes without changing behavior ownership.
- [x] D39/ARCH-29/UI-25 operation-tracker boundary: centralize MainWindow's monotonic operation IDs and stale active-operation guard without moving busy/status/TaskRunner policy.
- [x] D40/ARCH-30/UI-26 plugin operation-state boundary: centralize independent plugin operation lifecycle IDs and stale guards without moving trust, approval, enablement, host, notification, or TaskRunner policy.
- [x] D41 workspace-entry activation: preserve mouse behavior and add Enter/Return keyboard file/folder activation through one presentation-only semantic route.
- [x] D42/UI-28 workspace action hierarchy: give existing Back and Cancel actions explicit quiet/hover/focus/pressed/disabled visual states through centralized QSS without changing behavior.
- [x] D43/ARCH-33/UI-29 workspace-search lifecycle boundary: centralize search ID/generation/cancellation/stale guards without moving service, TaskRunner, surface, containment, notification, startup, or close policy.
- [x] D44/ARCH-34/UI-30 workspace-navigation lifecycle boundary: centralize workspace open/list ID/generation/stale guards while retaining generic busy/status and all workspace/session policy in MainWindow.
- [x] D45/ARCH-35/UI-31 workspace file activation consistency: support click/double-click/Enter file activation, retain folder navigation, and reuse an already-open tab while keeping busy/containment/async policy in MainWindow.
- [x] D46/ARCH-36/UI-32 workspace tree visual rhythm: add standard row hierarchy and readable long-name elision through WorkspacePanel/theme only, preserving semantic signals and application policy.
- [x] D47/ARCH-37/UI-33 session-restore state boundary: isolate typed ordered restore values and the pending document/open-operation binding while preserving MainWindow services, TaskRunner, tab projection, notifications, startup/close guards, and result policy.
- [x] D48/ARCH-38/UI-34 document-tab state clarity: project a theme-tinted authored modified marker through DocumentTabSurface while preserving title, close, current-tab, and document policy.
- [x] D49/ARCH-39 session-save state boundary: isolate latest-wins debounce/single-flight state and matching operation callbacks while preserving MainWindow timer, SessionService, TaskRunner, close, and notification policy.
- [x] D50/ARCH-40 recovery-capture lifecycle boundary: isolate cooperative capture jobs, channel ownership, cancellation/discard markers, and cleanup coordination while preserving MainWindow editor, RecoveryService, TaskRunner, notification, and close policy.
- [x] D51/ARCH-41 settings-save callback boundary: isolate settings-save operation identity and stale/invalid/failure classification while preserving MainWindow settings, theme, locale, editor, transition, notification, and close policy.
- [x] D52/ARCH-42 Replace All lifecycle boundary: isolate the active job identity, expected content version, and stale finish guard while preserving MainWindow editor, timer, cancellation, rollback, status, and operation policy.
- [x] D53/ARCH-43 Find Match snapshot boundary: isolate document/query/selection/content-version matching and invalidation while preserving MainWindow editor, busy, replacement, and feedback policy.
- [x] D54/ARCH-44 Recovery Scan lifecycle boundary: isolate typed scan identity, startup context, and stale completion protection while preserving MainWindow RecoveryService, TaskRunner, candidate validation, restore, notification, and close policy.
- [x] D55/UI-35 warning-background foreground contract: derive readable warning text from warning_bg while preserving the existing gold endpoint and interaction policy.
- [x] D56/ARCH-45 document-tab identity lookup boundary: keep complete tab/path registry lookup in DocumentTabSurface while preserving MainWindow session-restore subset policy.
- [x] D57/ARCH-46 session-restore tab projection boundary: centralize ordered restored-tab recording and active-tab selection input while preserving MainWindow restore policy, services, async flow, and close guards.
- [x] D58/ARCH-47 session-load coordinator state simplification: remove the write-only load-state field while preserving result normalization, restore/recovery sequencing, notifications, and save baseline policy.
- [x] D59/ARCH-48 status-phase forwarding simplification: remove the coordinator-only `_sync_active_document_phase()` alias while preserving every dirty-document, workspace, and tab-change status projection call.
- [x] UI-36 document-tab selection hierarchy: strengthen selected/hover/focus tab contrast and dock-title boundary through centralized theme QSS without changing tab behavior or theme contracts.
- [x] D61/ARCH-49 session-snapshot capture boundary: isolate clean path-backed tab metadata and active-index assembly behind a Qt-free contract while preserving MainWindow editor reads, session policy, save debounce, and close behavior.
- [x] D62/UI-37 find-bar action hierarchy: add presentation-only action identities and strengthen find/replace input/action QSS while preserving all find behavior, locale, warning, and application policy.
- [x] D63/UI-38 toolbar context chip: strengthen the existing localized local/safe command-rail context label through centralized QSS without changing CommandSurface, i18n, layout, or command behavior.
- [x] D64/UI-39 settings control hierarchy: visually separate appearance/editor groups and theme/accent/font/size controls through presentation metadata and centralized QSS without changing settings behavior or persistence.
- [x] D65/ARCH-50 document-tab forwarding simplification: remove MainWindow's pure active/editor/path/contains forwarding aliases while preserving DocumentTabSurface, session restore, save/open, recovery, Replace All, Find, and close policy.
- [x] D66/UI-40 dialog action hierarchy: distinguish primary, warning, and quiet actions across plugin catalog/status, workspace search, and settings through presentation metadata and centralized QSS without changing signals, enablement, locale, persistence, or application policy.
- [x] D67/ARCH-51 MainWindow file-dialog/About forwarding simplification: remove `_choose_save_path` and `_show_about` aliases while preserving FileDialogSurface, MessageSurface, Save As, dirty-close, command, locale, and application policy.
- [x] D68/UI-41 message-dialog visual hierarchy: theme About/error/unsaved-close/recovery QMessageBox projections and distinguish recovery restore/discard/later actions without changing localized text, button semantics, return mapping, or policy.
- [x] D69/UI-42 notification localization closure: localize duplicate-open, plugin failure, structured plugin summaries, and known invalid-workspace notifications at the presentation boundary while preserving diagnostic details and locale-free application summaries.
- [x] D70/UI-43 plugin catalog locale projection: localize catalog row/tool-tip field and enum labels with safe raw-value fallback while preserving dynamic metadata, entry state, signals, and governance policy.
- [x] D71/UI-44 plugin status boolean locale projection: localize enabled/active tooltip booleans while preserving typed runtime status, lifecycle controls, and policy.
- [x] D72/UI-45 plugin-dialog visual hierarchy: add centralized accent/boundary, summary-card, list-panel, focus, and selected-row QSS while preserving dialog behavior and plugin policy.
- [x] D73/UI-46 settings appearance preview: project pending theme, accent, interface font, and interface size choices through a token-driven presentation preview without changing Save/Cancel, persistence, global application timing, or editor policy.
- [x] D74/UI-47 settings preview surface boundary: extract the preview object tree into a focused presentation surface with a one-way projection contract while preserving SettingsDialog controls, snapshot assembly, Save/Cancel, locale flow, and application policy.
- [x] D75/UI-48 workspace-search visual hierarchy: distinguish scope, query, result, diagnostic, focus, hover, selected, checked, and disabled states through centralized QSS plus one semantic toggle identity without changing search behavior or async policy.
- [x] D76/UI-49 recovery notification dynamic localization: keep recovered document names and diagnostics intact while projecting the complete success message through the existing presentation locale boundary.
- [x] D77/ARCH-52 plugin catalog coordinator boundary: move catalog scan and descriptor governance sequencing behind a Qt-free typed presentation coordinator while preserving MainWindow close gates and plugin security policy.
- [x] D78/ARCH-53 plugin-host probe coordinator boundary: move diagnostic host probe sequencing behind a Qt-free typed presentation coordinator while preserving MainWindow close gates and host execution/security policy.
- [x] D79/ARCH-54 plugin runtime coordinator boundary: move runtime failure/status/enablement sequencing behind a Qt-free typed presentation coordinator while preserving MainWindow busy/close policy and application runtime security.
- [x] D80/ARCH-55 session-load coordinator boundary: move SessionLoadResult classification and recovery-first baseline projection behind a Qt-free typed coordinator while preserving startup, persistence, workspace, and close policy.
- [x] D81/ARCH-56 recovery-scan coordinator boundary: move inventory classification, stale finish, notifications, and startup continuation behind a Qt-free typed coordinator while preserving recovery decisions and close policy.
- [x] D82/ARCH-57 session-save coordinator boundary: move session-save completion classification, stale suppression, notifications, and latest-request draining behind a Qt-free typed coordinator while preserving persistence, startup, and close policy.
- [x] D83/ARCH-58 settings-save coordinator boundary: move SettingsSaveTracker completion/failure classification behind a Qt-free typed coordinator while preserving settings application, theme/locale/font/editor refresh, transition, and close policy.
- [x] D84/ARCH-59 workspace-search coordinator boundary: move workspace-search callback classification, typed result validation, invalidated cancellation, surface projection, and summary severity behind a Qt-free typed coordinator while preserving service, query, cancel, containment, result-open, and close policy.
- [x] D85/ARCH-60 workspace-navigation coordinator boundary: move workspace open/directory completion classification, loading/error projection, invalid-result handling, and failure/session-restore lifecycle behind a Qt-free typed coordinator while preserving WorkspaceService, activation, root/containment, document, persistence, and close policy.
- [x] D86/ARCH-61 document-open coordinator boundary: move ordinary/session-restore document-open completion classification, stale suppression, result validation, failure projection, and restore continuation behind a Qt-free typed coordinator while preserving DocumentService, duplicate-tab/editor/line/cursor/event, persistence, and close policy.
- [x] D87/ARCH-62 document-save coordinator boundary: move document-save stale/liveness classification, read-only release, DocumentState validation, and invalid/failure projection behind a Qt-free generic coordinator while preserving state/language/title/recovery/event/notification/session-save/continuation and close policy.
- [x] D88/ARCH-63 recovery-delete coordinator boundary: move recovery delete tracker release, owner identity clearing, success/error notification, and pending-delete drain behind a Qt-free generic coordinator while preserving request admission, RecoveryService dispatch, capture/write/recovery/persistence/close policy.
- [x] D89/ARCH-64 Replace All completion coordinator boundary: move Replace All current-job release, live-tab editor unlock, tab-bar/Find operation-state release, and generic operation completion behind a Qt-free generic coordinator while preserving editor slicing/cancel/rollback/clean-state/limit/result and close policy.
- [x] D90/ARCH-65 recovery-write coordinator boundary: move recovery writer discarded/capture/document/write lifecycle release and saved/failed ordering behind a Qt-free generic coordinator while preserving tab/content/snapshot/delete/notification/persistence and close policy.
- [x] D91/ARCH-66 recovery-capture abort coordinator boundary: move recovery capture identity protection, worker-started discard versus pre-worker release, channel/session cleanup, document lifecycle release, and optional live-owner failure notification behind a Qt-free generic coordinator while preserving Qt scheduling, capture stepping, retry/stale decisions, RecoveryService, notification, persistence, and close policy.
- [x] D92/ARCH-67 session-restore coordinator boundary: move ordered session restore startup/workspace guards, deferred and duplicate path progression, pending-open binding, active/first-tab completion selection, initial-document fallback, and finish/save ordering behind a Qt-free generic coordinator while preserving workspace/document services, TaskRunner/open callbacks, tab/editor projection, startup/close state, and notification policy.
- [x] D93/ARCH-68 recovery-write finish boundary: move RecoveryCaptureTracker.finish_write() and pending-delete projection into the existing Qt-free RecoveryWriteCoordinator while preserving delete admission, RecoveryService dispatch, delete callbacks, tab/content/snapshot policy, and close behavior.
- [x] D94/ARCH-69 document-tab removal coordinator boundary: move approved tab removal recovery cleanup, tab/editor/event/session finalization, and empty-tab fallback behind a Qt-free generic coordinator while preserving busy/startup/dirty/save-confirm close policy and service/notification decisions.
- [x] D95/ARCH-70 document-tab creation coordinator boundary: move editor/tab assembly, surface add, title/modified projection, session-save request, and status synchronization behind a Qt-free generic coordinator while preserving document/settings/editor policy, concrete tab/recovery identity construction, open/recovery outcomes, and close behavior.
- [x] D96/UI-50 shell visual rhythm: quiet ordinary toolbar/tab/status/control surfaces, retain explicit selected/focus/feedback states, and preserve theme tokens, signals, locale, accessibility cues, and application policy.
- [x] D97/ARCH-71 document-open projection coordinator boundary: move valid-open duplicate handling, tab/editor projection, line/cursor restoration, restored-tab recording, event/notification projection, and session-restore continuation behind a Qt-free generic coordinator while preserving D86 callback classification and MainWindow service/close policy.
- [x] D98/ARCH-72 document-save projection coordinator boundary: move valid-save state/language/title/recovery/event/notification/session-save/continuation ordering behind a Qt-free generic coordinator while preserving D87 classification, concrete editor policy, service ownership, and close behavior.
- [x] D99/ARCH-73 workspace-navigation projection coordinator boundary: move valid workspace-open/directory activation, search-root/directory projection, success/session-save/restore ordering behind a Qt-free generic coordinator while preserving D85 classification, admission, service ownership, and close behavior.
- [x] D100/ARCH-74 settings-save projection coordinator boundary: move valid settings state/theme/locale/editor/motion/success projection ordering behind a Qt-free generic coordinator while preserving D83 classification, QApplication policy, settings service ownership, and close behavior.
- [x] D101/ARCH-75 recovery-write projection coordinator boundary: move valid recovery saved/failed tab, snapshot identity, dirty/content-version, delete, and feedback projection behind a Qt-free generic coordinator while preserving D90 lifecycle classification and close behavior.
- [x] D102/ARCH-76 close-readiness coordinator boundary: move close guard precedence and ordered session-save/timer-stop decisions behind a Qt-free coordinator while preserving MainWindow error-dialog projection, event acceptance, TaskRunner, and close behavior.
- [x] UI-51 document-tab rail state clarity: make selected, modified, truncated, keyboard-focus, and close affordance states visually explicit through the existing semantic tab surface and centralized QSS without changing tab behavior.
- [x] UI-52 workspace resource-manager hierarchy: make path, empty, loading, tree, and navigation-control states visually explicit through semantic presentation identities and centralized QSS without changing workspace signals or file/folder activation behavior.
- [x] D103/ARCH-77 session-save request/dispatch boundary: move latest snapshot request, single-flight admission, operation binding, TaskRunner submission callback, and completion drain behind the existing Qt-free SessionSaveCoordinator while preserving MainWindow timer, SessionService, snapshot capture, startup, notification, and close policy.
- [x] D104/UI-53 editor canvas token hierarchy: unify editor canvas, gutter, selection, caret, current-line, and Python syntax colors through the canonical theme token resolver without changing editor behavior or settings contracts.
- [x] D105/ARCH-78 recovery-write dispatch callback boundary: consolidate duplicate TaskRunner callback binding behind the existing Qt-free RecoveryWriteCoordinator without changing recovery payload, lifecycle, or close semantics.
- [x] D106/UI-54 scrollbar chrome: restore and visually align horizontal scrolling with the existing vertical token states without changing scroll behavior or editor settings.
- [x] D107/ARCH-79 recovery-delete dispatch callback boundary: bind recovery-delete success/failure callbacks inside the existing Qt-free coordinator while preserving request admission, RecoveryService, TaskRunner, pending-delete drain, notification, persistence, and close policy.
- [x] D108/ARCH-80 cross-module contract/error/observability audit: add only the necessary AST static gate for Qt-free coordinator imports, explicit notification levels, and TaskRunner pending-work observability wiring.
- [x] D109/ARCH-81 recovery-scan dispatch callback boundary: bind recovery-scan success/failure callbacks inside the existing Qt-free coordinator while preserving startup continuation, manual scan feedback, RecoveryService, TaskRunner, and close policy.
- [x] D110/ARCH-82 document-save dispatch callback boundary: bind document-save success/failure callbacks inside the existing Qt-free coordinator while preserving document snapshots, read-only policy, projection continuation, TaskRunner, and close behavior.
- [x] D111/ARCH-83 document-open dispatch callback boundary: bind ordinary/session-restore open callbacks inside the existing Qt-free coordinator while preserving line navigation, stale guards, restore continuation, DocumentService, TaskRunner, and close policy.
- [x] D112/ARCH-84 settings-save dispatch callback boundary: bind settings-save success/failure callbacks inside the existing Qt-free coordinator while preserving tracker admission, settings projection order, theme/font/locale/motion application, TaskRunner, and close policy.
- [x] D113/ARCH-85 session-load dispatch callback boundary: bind startup session-load success/failure callbacks inside the existing Qt-free coordinator while preserving recovery-first continuation, invalid-manifest retention, SessionService, TaskRunner, and close policy.
- [x] UI-55 settings field hierarchy: give language/theme/accent/font/size and editor-option controls stable semantic identities so centralized QSS can create readable groups and highlighted states without changing settings behavior.
- [x] D114/ARCH-86 workspace-navigation dispatch callback boundary: bind workspace-open and directory-load callbacks inside the existing Qt-free coordinator while preserving generation invalidation, loading/error projection, session restore, WorkspaceService, TaskRunner, and close policy.
- [x] UI-56/ARCH-87 command-rail visual role hierarchy: give toolbar actions explicit standard/primary/quiet/workspace-context roles through presentation metadata and centralized QSS without changing callbacks, shortcuts, locale, icons, layout, or command policy.
- [x] UI-57/ARCH-88 workspace-dock chrome hierarchy: style the existing WorkspaceDock frame/title/close/float states through scoped token QSS without changing docking, workspace signals, navigation, locale, or close policy.
- [x] D115/ARCH-89 workspace-search dispatch callback boundary: bind search success/failure callbacks inside the existing Qt-free coordinator while preserving query/cancellation, generation invalidation, WorkspaceSearchService, TaskRunner, surface, locale, and close policy.
- [x] UI-58 dialog-shell edge hierarchy: give the existing Settings and Command Palette dialogs token-driven outer frame/top-accent states without changing child controls, signals, locale, keyboard focus, layout, persistence, or command policy.
- [x] D116/ARCH-90 session-save dispatch callback boundary: bind latest-wins session-save callbacks inside the existing Qt-free coordinator while preserving snapshot capture/debounce, SessionService, persistence, notifications, startup restore, and close policy.
- [x] UI-59 control-affordance chrome: give existing ComboBox and SpinBox native subcontrols readable token-driven hover/pressed/disabled states without changing settings values, signals, focus, locale, or persistence.
- [x] UI-60 dialog action-rail hierarchy: give plugin catalog/status action rows a shared semantic rail and token separator without changing button objects, enablement, signals, locale, or plugin policy.
- [x] UI-61 accent palette swatch hierarchy: give Settings Theme and Accent choices token-derived vector swatches without changing localized labels, ThemeId/AccentId values, preview, persistence, or application policy.
- [x] UI-62 document tab close-affordance hierarchy: give the existing document-tab close subcontrol an 18px target plus focus/disabled token states without changing tab signals, lifecycle, or document policy.
- [x] D117/ARCH-91 operation-reserve facade simplification: remove the pure MainWindow operation-ID forwarding facade while preserving the canonical tracker, SessionSaveCoordinator callable shape, busy/status policy, and async lifecycle.
- [x] D118/ARCH-92 session-restore ports contract: replace the long positional callback list with a frozen/slotted typed ports contract while preserving ordered restore, tracker state, services, startup/close, and persistence policy.
- [x] UI-63 Find close-affordance stability: give the existing Find close control a stable target height plus pressed/disabled token states without changing FindBar signals or editor policy.
- [x] D119/ARCH-93 workspace file activation boundary: isolate workspace file-intent admission and duplicate/open routing behind a Qt-free typed coordinator while preserving WorkspacePanel signals, WorkspaceService containment, tab identity, and `_start_open`.
- [x] D120/ARCH-94 workspace-navigation ports contract: replace positional navigation callbacks with a frozen/slotted named ports contract while preserving generation, stale/invalidation, projection, restore, notification, and MainWindow policy ownership.
- [x] UI-64/ARCH-95 shell elevation and visual rhythm: refine the existing centralized QSS for the command rail and document-tab rail using existing tokens while preserving states, signals, layout owners, and application policy.
- [x] UI-65/ARCH-96 semantic message and tooltip chrome: refine existing QMessageBox/QToolTip selectors with token-driven semantic accents and readable text while preserving dialog roles, locale, decisions, and application policy.
- [x] UI-66/ARCH-104 editor-stage visual rhythm: give the existing EditorShellSurface explicit stage margins/spacing and a token-driven shell/canvas surface hierarchy without changing tab/Find order, signals, locale, fonts, motion, or application policy.
- [x] D128/ARCH-105 core command registration boundary: extract the 23-command built-in catalog into a Qt-free named ports coordinator while preserving IDs, order, metadata, callback identity, CommandRegistry semantics, CommandSurface projection, plugin refresh, and MainWindow policy.
- [x] D129/ARCH-106 core toolbar composition boundary: extract six core toolbar action specifications plus the optional workspace action into a presentation-only named ports coordinator while preserving count, order, text keys, callback identity, icons, separators, roles, CommandSurface projection, locale, and MainWindow policy.
- [x] D130/ARCH-107 pure toolbar contract extraction: move IconKey, ToolbarActionRole, and ToolbarActionSpec into pure-Python contract modules while preserving compatibility imports, toolbar behavior, Qt projection, locale, and MainWindow policy.
- [x] D131/ARCH-108 Replace All admission ports boundary: extract busy/active-tab/query admission, session creation, operation/tracker binding, and starter handoff into a Qt-free named ports coordinator while preserving precedence, messages, session arguments, job identity, dirty/content-version capture, QTimer slicing, completion, rollback, and MainWindow policy.
- [x] D132/ARCH-109 recovery capture admission ports boundary: extract recovery/busy gating, dirty-tab candidate filtering, inflight/delete-pending exclusion, snapshot identity, dirty-state normalization, and content-version capture into a Qt-free named ports coordinator while preserving channel/backpressure, editor capture, tracker/writer/QTimer lifecycle, abort/write, close policy, and MainWindow policy.
- [x] D121/ARCH-97 close-guard ports contract: replace positional close-readiness callbacks with a frozen/slotted named ports contract while preserving precedence, cancellation, immediate-save, pending-work, timer-stop, QCloseEvent, and MainWindow policy.
- [x] D122/ARCH-98 document-open ports contract: replace positional ordinary/session-restore callbacks with a frozen/slotted named ports contract while preserving completion, invalid/failure, projection, line navigation, notification, and continuation policy.
- [x] D123/ARCH-99 document-save ports contract: replace positional save callbacks with a frozen/slotted named ports contract while preserving stale/liveness classification, read-only release, result validation, error projection, and MainWindow policy.
- [x] D124/ARCH-100 document-save projection ports contract: replace positional valid-save projection callbacks with a frozen/slotted generic named ports contract while preserving state/language/title/recovery/event/notification/session-save/continuation order and MainWindow policy.
- [x] D125/ARCH-101 document-open projection ports contract: replace positional valid-open projection callbacks with a frozen/slotted generic named ports contract while preserving duplicate/restored branches, line/cursor projection, event/notification, continuation, and MainWindow policy.
- [x] D126/ARCH-102 document-tab creation ports contract: replace positional tab-assembly callbacks with a frozen/slotted generic named ports contract while preserving editor/tab construction, surface add, title/modified projection, session-save request, status sync, and MainWindow policy.
- [x] D127/ARCH-103 document-tab removal ports contract: replace positional removal callbacks with a frozen/slotted generic named ports contract while preserving liveness, recovery-capture cancellation, snapshot cleanup, tab/editor/event/session finalization, empty-tab fallback, and MainWindow close policy.
- [x] UI-67/ARCH-110 shell surface depth and primary-work-area hierarchy: separate the command rail, editor stage, document canvas, workspace dock, and focus/hover states through centralized semantic tokens without changing command callbacks, tab/find order, workspace signals, locale, font, motion, or application policy.
- [x] D133/ARCH-111 document-open admission boundary: extract busy/startup-restore admission, operation binding, session-restore binding, and asynchronous open submission into a Qt-free named ports coordinator while preserving file/folder activation, result classification, line navigation, notifications, and close policy.
- [x] D134/ARCH-112 document-save admission boundary: extract busy/startup-restore admission, duplicate-target rejection, snapshot/read-only protection, operation binding, and asynchronous save submission into a Qt-free named ports coordinator while preserving save-as selection, conflict policy, continuation, result classification, and close policy.
- [x] D135/ARCH-113 workspace-navigation admission boundary: extract workspace-open/directory-load availability gates, startup-restore admission, loading projection, operation/generation binding, and asynchronous submission into a Qt-free named ports coordinator while preserving workspace service, result classification, session restore, notification, containment, and close policy.
- [x] D136/ARCH-114 settings-save admission boundary: extract settings-service availability, in-flight admission, candidate editing, operation/tracker binding, and asynchronous save submission into a Qt-free named ports coordinator while preserving theme/locale/font/motion application, persistence, result classification, notifications, and close policy.
- [x] D137/ARCH-115 document-creation admission boundary: extract busy/startup-restore admission, new-document creation, tab projection handoff, lifecycle event publication, and success notification into a Qt-free named ports coordinator while preserving initial-document restore behavior and MainWindow policy.
- [x] UI-68/ARCH-116 semantic state contrast closure: derive readable success/working/error foregrounds from actual backgrounds across all themes and accents while preserving warning/gold, widget, signal, locale, motion, and application policy.
- [x] UI-69/ARCH-117 settings-dialog card hierarchy: refine existing dialog canvas, appearance/editor cards, preview card, and action rail through centralized QSS without changing controls, layout, signals, settings behavior, locale, font, motion, or application policy.
- [x] D138/ARCH-118 document-picker admission boundary: extract busy/startup-restore admission, native file selection, cancellation, and handoff to the existing asynchronous open boundary while preserving file-dialog, document-service, result, and MainWindow policy ownership.
- [x] D139/ARCH-119 workspace-picker admission boundary: extract workspace availability, busy/startup-restore admission, native directory selection, cancellation, and handoff to the existing workspace navigation boundary while preserving folder/file separation and MainWindow policy ownership.
- [x] UI-70/ARCH-120 modern shell elevation: refine the existing centralized QSS surface ladder and hover/empty-state highlights without changing widget IDs, signals, layout, locale, fonts, motion, semantic states, theme/accent behavior, or application policy.
- [x] D140/ARCH-121 workspace-search surface admission boundary: extract startup-restore/service/root admission, surface creation/reuse, root-change invalidation/reset, and presentation while preserving query validation, operation tracking, cancellation, dispatch, results, containment, locale, and MainWindow policy ownership.
- [x] D141/ARCH-122 document-save picker admission boundary: extract active-tab/busy admission, ordinary Save versus Save As routing, native save-path selection, cancellation, and handoff while preserving save validation, conflict/read-only/persistence, results, recovery/session, notifications, and MainWindow policy ownership.
- [x] UI-71/ARCH-123 workspace-search query card: group existing query controls in a token-driven presentation card with readable hover/focus states without changing search signals, keyboard behavior, results, diagnostics, locale, fonts, motion, theme/accent behavior, or application policy.
- [x] D142/ARCH-124 presentation contract audit closure: enforce frozen/slotted coordinator Ports contracts and explicit coordinator notification levels through the existing AST audit without changing runtime policy.
- [x] D143/ARCH-125 presentation locale coordinator: move only the established locale refresh order behind a frozen/slotted Qt-free callback contract while preserving MainWindow surface ownership and optional-surface behavior.
- [x] D144/ARCH-126 close-guard feedback coordinator: move only the existing close-block reason/message projection behind a frozen/slotted Qt-free callback contract while preserving close classification, QCloseEvent, MessageSurface, and application policy.
- [x] UI-72/ARCH-127 status notification pill: refine the existing centralized statusMessage QSS into a token-driven rounded state hierarchy without changing notifications, signals, locale, motion, or application policy.
- [x] D146/ARCH-131 editor-change projection: move the existing modified/content callback ordering behind a frozen/slotted Qt-free coordinator while preserving tab/editor lookup, DocumentService dirty mutation, Find invalidation, title/status/session-save ordering, and MainWindow policy ownership.
- [x] D147/ARCH-132 current-document transition projection: preserve the existing tab-change order while centralizing Find reset, active-document context notification, status refresh, and debounced session-save request behind a Qt-free typed contract.
- [x] UI-75/ARCH-133 active document tab emphasis: restore the existing accent-left selected-tab cue in the specific document-tab selector without changing tab signals, metrics, close affordance, or application policy.
- [x] D148/ARCH-134 editor-action admission: preserve no-active-tab/busy rejection, editor action execution, and focus restoration behind a Qt-free typed contract without moving editor or policy ownership.
- [x] D149/ARCH-135 settings-save projection Ports contract: replace positional projection callbacks with a frozen/slotted Qt-free named contract while preserving apply/retranslate/editor/animation/notification order and MainWindow ownership.
- [x] D150/ARCH-136 recovery projection Ports contract: replace positional recovery outcome callbacks with a frozen/slotted Qt-free named contract while preserving discarded/live/snapshot/content-version and failure branch order.
- [x] UI-76/ARCH-137 inactive-selection contrast closure: restore readable text on inactive selected workspace/list rows without changing selection semantics, layout, tokens, or policy.
- [x] D151/ARCH-138 session-load Ports contract: preserve typed result classification, DEFAULT_SESSION fallback, baseline projection, notification, and recovery-first scheduling through a frozen/slotted Qt-free named contract.
- [x] D152/ARCH-139 session-save Ports contract: preserve latest-wins admission, stale suppression, invalid/failure notification, operation identity, and drain ordering through a frozen/slotted Qt-free named contract.
- [x] D153/ARCH-140 Replace All completion Ports contract: preserve finish/stale suppression, tab unlock, tab-bar enablement, operation completion, and outcome projection order through a frozen/slotted Qt-free named contract.
- [x] D154/ARCH-141 settings-save Ports contract: preserve stale/invalid/valid/failure result classification and projection through a frozen/slotted Qt-free named contract.
- [x] D155/ARCH-142 workspace-search Ports contract: preserve stale/invalidated/invalid/valid/failure result classification and projection through a frozen/slotted Qt-free named contract.
- [x] D156/ARCH-143 plugin-runtime Ports contract: preserve failure, unavailable, busy, exception, success, and status projection through a frozen/slotted Qt-free named contract.
- [x] D157/ARCH-144 recovery-delete Ports contract: preserve completion, owner cleanup, pending-delete drain, success notification, failure release, and error projection through a frozen/slotted Qt-free named contract.
- [x] D158/ARCH-145 recovery-scan Ports contract: preserve stale, invalid-inventory, empty, candidate, startup-continuation, and failure-continuation projection through a frozen/slotted Qt-free named contract.
- [x] D159/ARCH-146 recovery-write Ports contract: preserve discarded classification, capture abort, document release, pending-delete drain, saved projection, and failed projection through a frozen/slotted generic named contract.
- [x] D160/ARCH-147 workspace-navigation projection Ports contract: preserve opened/directory projection order, missing-surface guards, search invalidation, session save, and restore continuation through a frozen/slotted named contract.
- [x] D161/ARCH-148 plugin-host probe Ports contract: preserve unavailable/busy guards, start/dispatch order, stale operation suppression, typed-result severity mapping, and failure notification through a frozen/slotted named contract.
- [x] D162/ARCH-149 plugin-catalog Ports contract: preserve scan/governance guards, result classification, action enablement, governance success/rescan order, stale suppression, and failure notification through a frozen/slotted named contract.
- [x] D163/ARCH-150 recovery-capture abort Ports contract: preserve matching-capture guard, discarded/released snapshot branch, channel/session cancellation, document completion, live-owner notification, and duplicate/stale suppression through a frozen/slotted generic named contract.
- [x] UI-73/ARCH-128 document-stage edge: remove the duplicated document tab-pane outline through centralized QSS while preserving tab/editor focus and application policy.
- [x] D145/ARCH-129 status-phase policy: move only the busy/pending/dirty-to-working/attention/ready decision behind a Qt-free typed coordinator while preserving StatusSurface rendering, error projection, and MainWindow policy ownership.
- [x] UI-74/ARCH-130 status-rail context divider: add a token-driven separator and compact spacing between the local context label and phase pill without changing status semantics or application policy.
- [x] Complete the static architecture contract/error/observability audit gates. D188 revalidation passes: existing AST audit, 43 Ports, zero forbidden coordinator imports, and zero missing MainWindow notification levels.
- [x] D189/UI-99/ARCH-175 workspace-search empty-state boundary: make initial and no-match result states visibly explicit through the existing surface, locale catalog, and centralized theme without changing search policy. Static/package evidence is recorded; native/runtime and release gates remain open.
- [x] D190/UI-100/ARCH-176 command-palette empty-state boundary: make empty and filtered-no-result states explicit through the existing palette surface without changing command registry or execution policy. Static/package evidence is recorded; native/runtime and release gates remain open.
- [x] D191/UI-101/ARCH-177 dynamic find-status localization closure: remove English plural/limit suffix leakage from Chinese Find/Replace feedback through the existing presentation localization boundary; static/package evidence is recorded, while native/runtime and release gates remain open.
- [x] D192/UI-102/ARCH-178 plugin-failure phase localization closure: translate stable plugin lifecycle/event phase labels inside the existing presentation localization boundary without changing plugin runtime policy; static/package evidence is recorded, while native/runtime and release gates remain open.
- [x] D193/UI-103/ARCH-179 close-guard pending feedback localization: translate the dynamic pending background-work count through the existing presentation localization boundary without changing close or TaskRunner policy; static/package evidence is recorded, while native/runtime and release gates remain open.
- [x] D194/UI-104/ARCH-180 native control affordance cohesion: make combo-box, spin-box, and checkbox small affordances use the centralized theme contract instead of mixed Fusion defaults; preserve all widget behavior and record static/package evidence while native/runtime and release gates remain open. PowerShell 7 package evidence is recorded; Windows PowerShell 5.1 compatibility remains a follow-up gap.
- [x] D195/ARCH-181 Windows PowerShell packaging compatibility: replace .NET Core-only relative-path and hash helpers with Windows PowerShell 5.1/7-compatible APIs while preserving source inventory, manifest identity, atomic root-copy behavior, and release no-go boundaries.
- [x] D196/ARCH-182 release verifier PowerShell compatibility: preserve UTF-8 JSON parsing, gate predicates, mechanical failures, dossier writing, and expected no-go behavior under Windows PowerShell 5.1 and PowerShell 7.
- [x] D197/ARCH-183 packaged measurement PowerShell compatibility: preserve artifact binding, capture validation, process lifecycle, cleanup, and runtime evidence limits while making manifest/report JSON reads compatible with Windows PowerShell 5.1 and PowerShell 7; measurement remains unrun.
- [ ] Complete authorized runtime, clean-machine, and release gates.
- [ ] Preserve the open D7.3, D7.4, D8 legal/clean-machine, and release gates.
- [x] Continue with the next distinct user-visible gap after UI-16; UI-17 through UI-76 and the static 12-theme/accent contrast matrix are recorded, while native visual runtime evidence remains open.
- [x] D187/UI-98/ARCH-174 settings responsive scroll boundary: keep the
  existing settings content scrollable at short heights while pinning
  Save/Cancel outside the viewport; preserve all settings contracts.

## Completed bounded slice — support handoff packet check (D165 / ARCH-152)

- Outcome: require the local `docs/support/HANDOFF.md` packet in the release static checks and dossier evidence.
- Scope: `scripts/check.ps1` and `scripts/verify_release_handoff.ps1` only; no gate/status/no-go/runtime behavior changed.
- Recorded evidence: `D165-POWERSHELL-PARSE-PROBE=PASS`, `D165-SCOPE-INVARIANT-PROBE=PASS`, `D165-SUPPORT-PACKET-CHECK-PROBE=PASS`, `D165-SIMPLIFICATION-ASSESSMENT=PASS`; architect `PASS`, independent bounded review `NO_CONCLUSION`.
- Limits: file presence is not owner acceptance; clean-machine, cross-machine, legal, signing, installer, updater, runtime, support-channel, and release-owner gates remain open.

## Completed bounded slice — status-message visual weight (D164 / UI-77)

- Outcome: make transient status feedback easier to distinguish through one centralized `QLabel#statusMessage` semibold declaration.
- Scope: `src/quillforge/presentation/theme.py` only; no state, lifecycle, locale, layout, notification, or application-policy change.
- Recorded evidence: `D164-AST-PROBE=PASS`, `D164-STATUS-MESSAGE-QSS-PROBE=PASS`, `D164-SINGLE-SCOPE-PROBE=PASS`, `D164-INDEPENDENT-REVIEW=PASS`, `D164-SIMPLIFICATION-ASSESSMENT=PASS`; architect and independent source reviews are `PASS`.
- Limits: native font fallback/metrics, Qt/EXE rendering, screenshots, clean-machine, cross-machine, legal, support, signing, installer, update, and release-owner gates remain open.

## Completed bounded slice — font-choice preview (D166 / UI-78 / ARCH-153)

- Outcome: make existing interface/editor font choices visibly previewable in Settings without changing selected values or application policy.
- Scope: `src/quillforge/presentation/settings_dialog.py` only; `QFont` is projected through `Qt.ItemDataRole.FontRole` for the two existing font combos.
- Recorded evidence: `D166-AST-PROBE=PASS`, `D166-FONT-ROLE-SHAPE-PROBE=PASS`, `D166-SETTINGS-CONTRACT-PROBE=PASS`, `D166-SIMPLIFICATION-ASSESSMENT=PASS`; architect `PASS` with bounded conditions, independent review `NO_CONCLUSION`.
- Limits: native popup rendering, installed-font fallback/metrics, DPI, accessibility, Qt/EXE runtime, clean-machine, cross-machine, legal, support, signing, installer, updater, and release-owner gates remain open.

## Completed bounded slice — Settings typography preview (D167 / UI-79 / ARCH-154)

- Outcome: preview both interface and editor font/size selections in Settings before save.
- Scope: four presentation files only; both samples use QFont/point size, editor controls refresh the existing preview path, and preview QSS contains no font interpolation.
- Recorded evidence: `D167-AST-PROBE=PASS`, `D167-EDITOR-FONT-SIZE-PROJECTION-PROBE=PASS`, `D167-REFRESH-WIRING-PROBE=PASS`, `D167-SCOPED-QSS-PROBE=PASS`, `D167-I18N-KEY-PROBE=PASS`, `D167-I18N-PLACEHOLDER-PROBE=PASS`, `D167-QFONT-BOTH-SAMPLES-PROBE=PASS`, `D167-SAFE-PREVIEW-QSS-PROBE=PASS`; architect and final independent review `PASS`, parent and simplification `PASS`.
- Limits: native preview rendering, installed-font fallback/metrics, DPI, accessibility, Qt/EXE runtime, clean-machine, cross-machine, legal, support, signing, installer, updater, and release-owner gates remain open.

## Completed bounded slice — workspace entry visual semantics (D168 / UI-80 / ARCH-155)

- Outcome: make file, folder, and unavailable workspace entries easier to scan
  through palette-aware authored icon emphasis and localized semantic hints.
- Scope: `workspace_panel.py` and `i18n.py` only; provider diagnostics,
  item roles, file/directory signals, click/double-click/Enter behavior, and
  application policy remain unchanged.
- Recorded evidence: `D168-AST-PROBE=PASS`,
  `D168-I18N-SEMANTIC-HINT-PROBE=PASS`,
  `D168-TOOLTIP-PRECEDENCE-PROBE=PASS`, `D168-ICON-REFRESH-PROBE=PASS`,
  `D168-SIGNAL-PRESERVATION-PROBE=PASS`, `D168-CONTRAST-PROBE=PASS`,
  `D168-SIMPLIFICATION-ASSESSMENT=PASS`; architect and independent windows
  are explicitly `NO_CONCLUSION`, parent review `PASS`.
- Limits: native icon/tooltip rendering, accessibility, DPI, Qt/EXE runtime,
  clean-machine, cross-machine, legal, support, signing, installer, updater,
  and release-owner gates remain open.

## Completed bounded slice — Plugin disabled action hierarchy (D209 / UI-111 / ARCH-194)

- Outcome: the Plugin Catalog and Plugin Status unavailable actions now use a
  scoped subdued surface, neutral left boundary, and readable muted text;
  enablement predicates, signals, plugin policy, locale, and ownership remain
  unchanged.
- Scope: centralized `presentation.theme` QSS only, plus synchronized
  delivery records; no plugin state service or second styling system.
- Evidence: `D209-PLUGIN-DISABLED-SOURCE-PROBE=PASS`,
  `D209-QSS-DISABLED-CONTRAST-PROBE=PASS combinations=12 min=5.14`,
  `D209-COMPILEALL=PASS`, `D209-RUFF=PASS`, `D209-FORMAT=PASS`,
  `D209-PRESENTATION-AUDIT=PASS`, `D209-PACKAGE-BUILD-PS51=PASS`,
  `D209-PACKAGE-BUILD-PS7=PASS`, `D209-PACKAGE-IDENTITY-PROBE=PASS`.
- Review: parent `PASS`; simplification `PASS`; `Averroes the 6th / Luna
  max` architecture and `Goodall the 6th / Luna max` independent windows
  returned `NO_CONCLUSION` after two bounded waits.
- Limits: native Qt rendering, accessibility, DPI, GUI/EXE runtime,
  clean-machine, cross-machine, legal, support, signing, installer, updater,
  and release-owner gates remain open; no embedded C/C++, MCU, RTOS, or
  manufacturer requirement applies.

## Completed bounded slice — Default document filter coverage (D210 / UI-112 / ARCH-195)

- Outcome: localized native open/save dialogs now list 18 common text/source
  extensions by default and retain the All files fallback; open/save/folder
  boundaries, async document flow, encoding, persistence, and policy remain
  unchanged.
- Scope: existing `presentation.i18n` filter catalog only, plus synchronized
  delivery records; no new file-open path or MIME service.
- Evidence: `D210-DIALOG-FILTER-PROBE=PASS locales=2 extensions=18
  all-files-fallback=present`, `D210-COMPILEALL=PASS`, `D210-RUFF=PASS`,
  `D210-FORMAT=PASS`, `D210-PRESENTATION-AUDIT=PASS`,
  `D210-PACKAGE-BUILD-PS51=PASS`, `D210-PACKAGE-BUILD-PS7=PASS`,
  `D210-PACKAGE-IDENTITY-PROBE=PASS`.
- Review: parent `PASS`; simplification `PASS`; `Beauvoir the 6th / Luna
  max` architecture and `Noether the 6th / Luna max` independent windows
  returned `NO_CONCLUSION` after two bounded waits.
- Limits: native QFileDialog rendering, accessibility, DPI, GUI/EXE runtime,
  clean-machine, cross-machine, legal, support, signing, installer, updater,
  and release-owner gates remain open; no embedded C/C++, MCU, RTOS, or
  manufacturer requirement applies.

## Completed bounded slice — Workspace-search application error taxonomy (D218 / ARCH-202)

- Outcome: workspace-search application and provider-result validation now
  uses `ApplicationValidationError` for invariant failures and
  `ApplicationTypeError` for type failures, both preserving their built-in
  catch compatibility.
- Scope: `src/quillforge/application/errors.py` and
  `src/quillforge/application/workspace_search.py`; exact messages, validation
  order, search limits, provider protocol, directory capability, result shape,
  and Qt-free direction remain unchanged.
- Recorded evidence: `D218-WORKSPACE-SEARCH-TAXONOMY-PROBE=PASS branches=39`,
  `D218-COMPILEALL=PASS`, `D218-RUFF=PASS`, `D218-FORMAT=PASS`,
  `D218-PRESENTATION-AUDIT=PASS`, `D218-PACKAGE-BUILD-PS51=PASS`,
  `D218-PACKAGE-BUILD-PS7=PASS`, `D218-PACKAGE-IDENTITY-PROBE=PASS`,
  `D218-SIMPLIFICATION-ASSESSMENT=PASS`.
- Review: parent `PASS`; `Pascal the 6th / Luna max` architecture and
  `James the 6th / Luna max` independent windows returned `NO_CONCLUSION`
  after two bounded waits each; no child PASS is claimed.
- Limits: runtime filesystem traversal, provider timing, cancellation,
  GUI/EXE startup, accessibility, font/DPI, clean-machine, cross-machine,
  signing, installer, updater, legal, support, and release-owner gates remain
  open. No embedded C/C++, MCU, RTOS, or manufacturer requirement applies.

## Completed bounded slice — Toolbar brand anchor (D217 / UI-117)

- Outcome: the top command rail now begins with a compact non-interactive
  `✦ QuillForge` identity chip, with locale-aware title metadata and one
  centralized QSS projection.
- Scope: `src/quillforge/presentation/command_surface.py` and the existing
  toolbar section in `src/quillforge/presentation/theme.py`; QAction order,
  callbacks, shortcuts, context, settings, motion, and application ownership
  remain unchanged.
- Recorded evidence: `D217-BRAND-ANCHOR-SOURCE-PROBE=PASS`,
  `D217-BRAND-CONTRAST-PROBE=PASS combos=12 min=11.16`,
  `D217-COMPILEALL=PASS`, `D217-RUFF=PASS`, `D217-FORMAT=PASS`,
  `D217-PRESENTATION-AUDIT=PASS`, `D217-PACKAGE-BUILD-PS51=PASS`,
  `D217-PACKAGE-BUILD-PS7=PASS`, `D217-PACKAGE-IDENTITY-PROBE=PASS`,
  `D217-SIMPLIFICATION-ASSESSMENT=PASS`.
- Review: parent `PASS`; `Godel the 6th / Luna max` architecture and
  `Darwin the 6th / Luna max` independent windows returned `NO_CONCLUSION`
  after two bounded waits each; no child PASS is claimed.
- Limits: native QSS rendering, GUI/EXE startup, accessibility, font/DPI,
  clean-machine, cross-machine, signing, installer, updater, legal, support,
  and release-owner gates remain open. No embedded C/C++, MCU, RTOS, or
  manufacturer requirement applies.

## Completed bounded slice — Editor policy error taxonomy (D216 / ARCH-201)

- Outcome: all eight `EditorOperationPolicy` limit invariants now use the
  existing Qt-free `ApplicationValidationError` category while preserving
  exact messages, defaults, dataclass shape, `ValueError` compatibility, and
  presentation consumers.
- Scope: `src/quillforge/application/editor_policy.py` only, reusing
  `src/quillforge/application/errors.py`; workspace-search, command,
  domain, and presentation validation remain out of scope.
- Recorded evidence: `D216-EDITOR-POLICY-TAXONOMY-PROBE=PASS fields=8`,
  `D216-COMPILEALL=PASS`, `D216-RUFF=PASS`, `D216-FORMAT=PASS`,
  `D216-PRESENTATION-AUDIT=PASS`, `D216-PACKAGE-BUILD-PS51=PASS`,
  `D216-PACKAGE-BUILD-PS7=PASS`, `D216-PACKAGE-IDENTITY-PROBE=PASS`,
  `D216-SIMPLIFICATION-ASSESSMENT=PASS`.
- Review: parent `PASS`; `Leibniz the 6th / Luna max` architecture and
  `Dewey the 6th / Luna max` independent windows returned `NO_CONCLUSION`
  after two bounded waits each; no child PASS is claimed.
- Limits: native/runtime editor behavior, GUI/EXE startup, accessibility,
  font/DPI, clean-machine, cross-machine, signing, installer, updater, legal,
  support, and release-owner gates remain open. No embedded C/C++, MCU, RTOS,
  or manufacturer requirement applies.

## Completed bounded slice — Semantic accent endpoint foregrounds (D215 / UI-116 / ARCH-200)

- Outcome: the primary-action pink hover state now uses a dedicated
  `ThemeColors.on_accent_pink` foreground resolved through the existing
  contrast helper across all 12 theme/accent combinations.
- Scope: `src/quillforge/presentation/theme_tokens.py` and the existing
  `QPushButton#primaryAction:hover` selector in `theme.py`; no new stylesheet,
  action policy, settings schema, locale, motion, or behavior owner.
- Recorded evidence: `D215-PINK-ENDPOINT-CONTRAST-PROBE=PASS combos=12`,
  `D215-QSS-PINK-WIRING-PROBE=PASS combos=12`,
  `D215-THEME-IMPORT-PROBE=PASS`, `D215-COMPILEALL=PASS`,
  `D215-RUFF=PASS`, `D215-FORMAT=PASS`, `D215-PRESENTATION-AUDIT=PASS`,
  `D215-PACKAGE-BUILD-PS51=PASS`, `D215-PACKAGE-BUILD-PS7=PASS`,
  `D215-PACKAGE-IDENTITY-PROBE=PASS`,
  `D215-SIMPLIFICATION-ASSESSMENT=PASS`.
- Review: parent `PASS`; `Newton the 6th / Luna max` architecture and
  `Galileo the 6th / Luna max` independent windows returned `NO_CONCLUSION`
  after two bounded waits each; no child PASS is claimed.
- Limits: native QSS rendering, GUI/EXE startup, accessibility, font/DPI,
  clean-machine, cross-machine, signing, installer, updater, legal, support,
  and release-owner gates remain open. No embedded C/C++, MCU, RTOS, or
  manufacturer requirement applies.

## Completed bounded slice — Application error taxonomy (D214 / ARCH-199)

- Outcome: document/workspace application use cases now classify owned
  validation/state failures through a dependency-free taxonomy while preserving
  existing built-in catches, messages, domain conflict ownership, provider
  behavior, containment, and UI projection.
- Scope: `src/quillforge/application/errors.py`, focused raises in
  `documents.py` and `workspace.py`; no adapter wrapping, result-union redesign,
  domain migration, or behavior policy change.
- Recorded evidence: `D214-APPLICATION-ERROR-TAXONOMY-SOURCE-PROBE=PASS`,
  `D214-EXCEPTION-COMPATIBILITY-PROBE=PASS`, `D214-COMPILEALL=PASS`,
  `D214-RUFF=PASS`, `D214-FORMAT=PASS`, `D214-PRESENTATION-AUDIT=PASS`,
  `D214-PACKAGE-BUILD-PS51=PASS`, `D214-PACKAGE-BUILD-PS7=PASS`,
  `D214-PACKAGE-IDENTITY-PROBE=PASS`,
  `D214-SIMPLIFICATION-ASSESSMENT=PASS`.
- Review: parent `PASS`; `Arendt the 6th / Luna max` architecture returned
  `NO_CONCLUSION`; `Fermat the 6th` completed a bounded independent static
  review with `PASS`; a separate `Aristotle the 6th / Luna max` window
  returned `NO_CONCLUSION` after two bounded waits.
- Limits: broader application taxonomy, runtime filesystem/provider behavior,
  GUI/EXE startup, clean-machine, cross-machine, signing, installer, updater,
  legal, support, and release-owner gates remain open. No embedded C/C++, MCU,
  RTOS, or manufacturer requirement applies.

## Completed bounded slice — Editor-shell brand edge (D213 / UI-115 / ARCH-198)

- Outcome: the central editor stage now uses one token-driven two-pixel pink
  top edge aligned with the command rail and workspace hierarchy; child widgets,
  editor behavior, locale, motion, and document policy remain unchanged.
- Scope: one `QWidget#editorShell` `border-top` declaration in
  `src/quillforge/presentation/theme.py`; no wrapper widget, palette mutation,
  state service, or behavior change.
- Recorded evidence: `D213-EDITOR-SHELL-ACCENT-SOURCE-PROBE=PASS`,
  `D213-ACCENT-PINK-TOKEN-PROBE=PASS themes=3`, `D213-COMPILEALL=PASS`,
  `D213-RUFF=PASS`, `D213-FORMAT=PASS`, `D213-PRESENTATION-AUDIT=PASS`,
  `D213-PACKAGE-BUILD-PS51=PASS`, `D213-PACKAGE-BUILD-PS7=PASS`,
  `D213-PACKAGE-IDENTITY-PROBE=PASS`,
  `D213-SIMPLIFICATION-ASSESSMENT=PASS`.
- Review: parent `PASS`; `Mencius the 6th / Luna max` architecture and
  `Tesla the 6th / Luna max` independent review windows returned
  `NO_CONCLUSION` after two bounded waits each; no child PASS is claimed.
- Limits: native QSS geometry, accessibility, DPI, GUI/EXE startup,
  clean-machine, cross-machine, signing, installer, updater, legal, support,
  and release-owner gates remain open. No embedded C/C++, MCU, RTOS, or
  manufacturer requirement applies.

## Completed bounded slice — Localized untitled tab title (D212 / UI-114 / ARCH-197)

- Outcome: pathless document tabs now use the active English or Simplified
  Chinese catalog label, and existing tabs reproject after locale changes;
  saved basenames and dirty markers remain unchanged.
- Scope: `document.untitled` in `i18n.py`, locale-aware `_tab_title`, and one
  `refresh_tab_titles` callback in the existing Qt-free locale coordinator;
  no title service, document mutation, file rename, or open/save policy change.
- Recorded evidence: `D212-LOCALIZED-TAB-TITLE-SOURCE-PROBE=PASS`,
  `D212-LOCALE-REFRESH-ORDER-PROBE=PASS`,
  `D212-FIXED-UNTITLED-REGRESSION-PROBE=PASS`,
  `D212-I18N-KEY-PROBE=PASS locales=2`, `D212-COMPILEALL=PASS`,
  `D212-RUFF=PASS`, `D212-FORMAT=PASS`, `D212-PRESENTATION-AUDIT=PASS`,
  `D212-PACKAGE-BUILD-PS51=PASS`, `D212-PACKAGE-BUILD-PS7=PASS`,
  `D212-PACKAGE-IDENTITY-PROBE=PASS`,
  `D212-SIMPLIFICATION-ASSESSMENT=PASS`.
- Review: parent `PASS`; `Faraday the 6th / Luna max` architecture and
  `Kepler the 6th / Luna max` independent review windows returned
  `NO_CONCLUSION` after two bounded waits each; no child PASS is claimed.
- Limits: native QTabWidget painting, font fallback, accessibility, DPI,
  GUI/EXE startup, clean-machine, cross-machine, signing, installer, updater,
  legal, support, and release-owner gates remain open. No embedded C/C++, MCU,
  RTOS, or manufacturer requirement applies.

## Completed bounded slice — Workspace disabled-item hierarchy (D211 / UI-113 / ARCH-196)

- Outcome: inaccessible workspace rows and the disabled truncation marker now
  use a subdued surface, neutral left boundary, and readable muted text while
  selected-disabled specificity and all workspace behavior remain unchanged.
- Scope: one centralized `QTreeWidget#workspaceTree::item:disabled` rule in
  `src/quillforge/presentation/theme.py`; no custom delegate, workspace
  service, provider policy, item data, signal, activation, locale, or icon
  behavior change.
- Recorded evidence: `D211-WORKSPACE-DISABLED-SOURCE-PROBE=PASS`,
  `D211-QSS-SPECIFICITY-SOURCE-PROBE=PASS`,
  `D211-DISABLED-CONTRAST-PROBE=PASS combinations=12 min=5.14`,
  `D211-COMPILEALL=PASS`, `D211-RUFF=PASS`, `D211-FORMAT=PASS`,
  `D211-PRESENTATION-AUDIT=PASS`, `D211-PACKAGE-BUILD-PS51=PASS`,
  `D211-PACKAGE-BUILD-PS7=PASS`, `D211-PACKAGE-IDENTITY-PROBE=PASS`,
  `D211-SIMPLIFICATION-ASSESSMENT=PASS`.
- Review: parent `PASS`; `Maxwell the 6th / Luna max` architecture and
  `Kuhn the 6th / Luna max` independent review windows returned
  `NO_CONCLUSION` after two bounded waits each; no child PASS is claimed.
- Limits: native QSS/item painting, accessibility, DPI, GUI/EXE startup,
  clean-machine, cross-machine, signing, installer, updater, legal, support,
  and release-owner gates remain open. No embedded C/C++, MCU, RTOS, or
  manufacturer requirement applies.

## Completed bounded slice — Menu affordance cohesion (D198 / UI-105 / ARCH-184)

- Outcome: centralized menu QSS now gives checkable actions explicit
  normal/hover/checked/disabled indicator states and a selected-and-checked
  hierarchy across all 12 theme/accent projections.
- Scope: `presentation/theme.py` only for source behavior; command projection,
  QAction state, shortcuts, locale, motion, and application ownership remain
  unchanged.
- Recorded evidence: `D198-MENU-AFFORDANCE-QSS-PROBE=PASS`,
  `D198-CONTRAST-PROBE=PASS`, `D198-COMPILE-RUFF-FORMAT=PASS`,
  `D198-PRESENTATION-AUDIT=PASS`, `D198-PACKAGE-BUILD-PS51=PASS`,
  `D198-PACKAGE-BUILD-PS7=PASS`, `D198-PACKAGE-IDENTITY-PROBE=PASS`;
  architect and independent windows are explicitly `NO_CONCLUSION`, parent
  review `PASS`.
- Safety boundary: no GUI/QApplication, EXE launch, screenshot, native menu
  rendering, or test-only asset was run or created.
- Public-source applicability: Qt Style Sheets Reference applies to the QSS
  subcontrols; embedded C/C++, MCU, RTOS, and manufacturer requirements do
  not apply. No private ByteDance standard or compliance claim is made.
- Limits: native menu metrics/accessibility, DPI, runtime, clean-machine,
  cross-machine, legal, support, signing, installer, updater, and release
  owner gates remain open.

## Completed bounded slice — Command menu contract closure (D199 / ARCH-185)

- Outcome: the application command registry now rejects unsupported menu IDs
  instead of admitting commands that presentation would silently omit.
- Scope: `application.commands` owns the four-value `MenuId` contract;
  `presentation.command_surface` reuses it; menu labels, Qt objects, command
  execution, locale, shortcuts, and plugin policy remain in their existing
  owners.
- Evidence: `D199-MENU-CONTRACT-PROBE=PASS valid=4 invalid=reject`,
  `D199-BOUNDARY-SCOPE-PROBE=PASS`, `D199-STATIC-AUDIT=PASS`,
  `D199-COMPILE-RUFF-FORMAT=PASS`, `D199-PACKAGE-BUILD-PS51=PASS`,
  `D199-PACKAGE-BUILD-PS7=PASS`.
- Review: parent `PASS`; simplification `PASS`; Carson/Dirac delegated
  windows are recorded as `NO_CONCLUSION`.
- Safety boundary: no GUI/QApplication, EXE launch, screenshot, or test-only
  asset was created or run. Embedded requirements do not apply.
- Acceptance: `S251`; runtime plugin integration and release gates remain
  open.

## Completed bounded slice — Package source-revision determinism (D200 / ARCH-186)

- Outcome: source-revision ordering in `scripts/package.ps1` is now ordinal
  and identical under Windows PowerShell 5.1 and PowerShell 7.
- Scope: packaging source-line collection and provenance evidence only;
  application and presentation code are untouched.
- Evidence: `D200-STABLE-SORT-PROTOTYPE=PASS shells_identical`,
  `D200-PS51-PARSE-PROBE=PASS`, `D200-PS7-PARSE-PROBE=PASS`,
  `D200-DETERMINISTIC-SORT-SOURCE-PROBE=PASS`,
  `D200-COMPILE-RUFF-FORMAT=PASS`, `D200-PACKAGE-BUILD-PS51=PASS`,
  `D200-PACKAGE-BUILD-PS7=PASS`, and matching source revision
  `tree-sha256:79cca7bc5704fca205ef41385a588c49d11c9cef77f7ec9a90d574acafad7646`.
- Review: parent `PASS`; simplification `PASS`; Carver architecture is
  `NO_CONCLUSION`, and Hubble independent review is a bounded `PASS`.
- Safety boundary: no EXE launch, runtime capture, deployment, or test-only
  asset was created or run. Acceptance `S252`; external release gates remain
  open.

## Completed bounded slice — Disabled menu state hierarchy (D201 / UI-106 / ARCH-187)

- Outcome: selected-disabled and checked-disabled menu items now use a quiet
  `surface_2` background, `border_strong` left boundary, and readable muted
  foreground instead of inheriting the active accent background.
- Scope: one centralized `theme.py` QSS block; QAction state, command
  projection, locale, motion, and application ownership remain unchanged.
- Evidence: `D201-MENU-DISABLED-QSS-PROBE=PASS combinations=12 selectors=2`,
  `D201-DISABLED-CONTRAST-PROBE=PASS minimum=text_muted/surface_2>=4.5`,
  `D201-COMPILE-RUFF-FORMAT=PASS`, `D201-PACKAGE-BUILD-PS51=PASS`,
  `D201-PACKAGE-BUILD-PS7=PASS`, `D201-PACKAGE-IDENTITY-PROBE=PASS`.
- Review: parent `PASS`; simplification `PASS`; architecture and independent
  windows are explicitly `NO_CONCLUSION`.
- Safety boundary: no GUI/QApplication, EXE launch, screenshot, native QSS
  rendering, accessibility, or test-only asset was created or run.
- Acceptance: `S253`; runtime and enterprise release gates remain open.

## Completed bounded slice — Workspace provider directory capability (D202 / ARCH-188)

- Outcome: `WorkspaceService` now delegates directory classification through
  the existing `WorkspaceProvider` Port, leaving the concrete filesystem
  predicate in `FileWorkspaceProvider` while preserving normalization,
  containment, limits, error text, and workspace/file UI behavior.
- Scope: `application/ports.py`, `application/workspace.py`, and
  `infrastructure/workspace_provider.py`; no search provider, document path
  policy, UI signal, or asynchronous behavior changed.
- Evidence: `D202-WORKSPACE-PREDICATE-PROBE=PASS port=1 adapter=1 app_direct_is_dir=0`,
  `D202-COMPILEALL=PASS`, `D202-RUFF=PASS`, `D202-FORMAT=PASS`,
  `D202-PRESENTATION-AUDIT=PASS`, `D202-PACKAGE-BUILD-PS51=PASS`,
  `D202-PACKAGE-BUILD-PS7=PASS`, `D202-PACKAGE-IDENTITY-PROBE=PASS`.
- Review: parent `PASS`; simplification `PASS`; architecture and independent
  windows are explicitly `NO_CONCLUSION`.
- Safety boundary: no GUI/QApplication, EXE launch, screenshot, native file
  dialog, live filesystem-race check, or test-only asset was created or run.
- Acceptance: `S254`; runtime and enterprise release gates remain open.

## Completed bounded slice — Shared directory capability for workspace search (D203 / ARCH-189)

- Outcome: workspace navigation and workspace search now reuse one
  `DirectoryCapability` Port; concrete `Path.is_dir()` calls remain inside
  their filesystem adapters while search limits, cancellation, containment,
  error text, and UI behavior remain unchanged.
- Scope: `application/ports.py`, `application/workspace_search.py`, and
  `infrastructure/workspace_search_provider.py`; no new directory service or
  search-policy rewrite was introduced.
- Evidence: `D203-DIRECTORY-CAPABILITY-PROBE=PASS shared=1 workspace=1 search=1 application_is_dir=0`,
  `D203-COMPILEALL=PASS`, `D203-RUFF=PASS`, `D203-FORMAT=PASS`,
  `D203-PRESENTATION-AUDIT=PASS`, `D203-PACKAGE-BUILD-PS51=PASS`,
  `D203-PACKAGE-BUILD-PS7=PASS`, `D203-PACKAGE-IDENTITY-PROBE=PASS`.
- Review: parent `PASS`; simplification `PASS`; architecture and independent
  windows are explicitly `NO_CONCLUSION`.
- Safety boundary: no GUI/QApplication, EXE launch, screenshot, native file
  dialog, live filesystem-race check, or test-only asset was created or run.
- Acceptance: `S255`; runtime and enterprise release gates remain open.

## Completed bounded slice — Workspace-search diagnostics disabled state (D204 / UI-107 / ARCH-190)

- Outcome: the Find in Files diagnostics disclosure control now uses a
  subdued centralized QSS projection while disabled during a search,
  including checked-and-disabled ordering; search behavior and enablement
  policy remain unchanged.
- Scope: `src/quillforge/presentation/theme.py` plus synchronized delivery
  records; no widget subclass, search-policy rewrite, or new token was added.
- Evidence: `D204-DISABLED-STATE-PROBE=PASS`, `D204-COMPILEALL=PASS`,
  `D204-RUFF=PASS`, `D204-FORMAT=PASS`, `D204-PRESENTATION-AUDIT=PASS`,
  `D204-PACKAGE-BUILD-PS51=PASS`, `D204-PACKAGE-BUILD-PS7=PASS`,
  `D204-PACKAGE-IDENTITY-PROBE=PASS`.
- Review: parent `PASS`; simplification `PASS`; architecture and independent
  windows are explicitly `NO_CONCLUSION`.
- Safety boundary: no GUI/EXE startup, native QSS rendering, screenshot,
  live search interleaving, or test-only asset was run or created.
- Acceptance: `S256`; native rendering, runtime, accessibility, and enterprise
  release gates remain open.

## Completed bounded slice — Find/Replace navigation disabled state (D205 / UI-108 / ARCH-191)

- Outcome: Find/Replace previous and next navigation now use a subdued
  centralized QSS projection while disabled during an active operation; their
  available-state hierarchy and operation behavior remain unchanged.
- Scope: `src/quillforge/presentation/theme.py` plus synchronized delivery
  records; no widget subclass, editor-policy rewrite, or new token was added.
- Evidence: `D205-DISABLED-STATE-SOURCE-PROBE=PASS`,
  `D205-FINDBAR-BEHAVIOR-PROBE=PASS`,
  `D205-QSS-DISABLED-CONTRAST-PROBE=PASS combinations=12 min=5.14`,
  `D205-COMPILEALL=PASS`, `D205-RUFF=PASS`, `D205-FORMAT=PASS`,
  `D205-PRESENTATION-AUDIT=PASS`, `D205-PACKAGE-BUILD-PS51=PASS`,
  `D205-PACKAGE-BUILD-PS7=PASS`, `D205-PACKAGE-IDENTITY-PROBE=PASS`.
- Review: parent `PASS`; simplification `PASS`; architecture and independent
  windows are explicitly `NO_CONCLUSION`.
- Safety boundary: no GUI/EXE startup, native QSS rendering, screenshot,
  live editor operation, or test-only asset was run or created.
- Acceptance: `S257`; native rendering, runtime, accessibility, and enterprise
  release gates remain open.

## Completed bounded slice — Workspace tree disabled state (D207 / UI-109 / ARCH-192)

- Outcome: the visible populated Workspace tree now uses a subdued
  centralized QSS projection while disabled during provider refresh; row
  states, tree data, and navigation behavior remain unchanged.
- Scope: `src/quillforge/presentation/theme.py` plus synchronized delivery
  records; no widget subclass, provider-policy rewrite, or new token was
  added.
- Evidence: `D207-DISABLED-STATE-SOURCE-PROBE=PASS`,
  `D207-QSS-DISABLED-CONTRAST-PROBE=PASS combinations=12 min=5.14`,
  `D207-COMPILEALL=PASS`, `D207-RUFF=PASS`, `D207-FORMAT=PASS`,
  `D207-PRESENTATION-AUDIT=PASS`, `D207-PACKAGE-BUILD-PS51=PASS`,
  `D207-PACKAGE-BUILD-PS7=PASS`, `D207-PACKAGE-IDENTITY-PROBE=PASS`.
- Review: parent `PASS`; simplification `PASS`; architecture and independent
  windows are explicitly `NO_CONCLUSION`.
- Safety boundary: no GUI/EXE startup, native QSS rendering, screenshot,
  live provider refresh, or test-only asset was run or created.
- Acceptance: `S258`; native rendering, runtime, accessibility, and enterprise
  release gates remain open.

## Completed bounded slice — Editor locked disabled state (D208 / UI-110 / ARCH-193)

- Outcome: the Replace All locked QScintilla editor now uses a subdued
  centralized QSS canvas/boundary/selection projection without muting document
  text or syntax colors; editor lock and operation behavior remain unchanged.
- Scope: `src/quillforge/presentation/theme.py` plus synchronized delivery
  records; no lexer mutation, editor state service, or new token was added.
- Evidence: `D208-DISABLED-STATE-SOURCE-PROBE=PASS`,
  `D208-QSS-CANVAS-CONTRAST-PROBE=PASS combinations=12 min=12.87`,
  `D208-COMPILEALL=PASS`, `D208-RUFF=PASS`, `D208-FORMAT=PASS`,
  `D208-PRESENTATION-AUDIT=PASS`, `D208-PACKAGE-BUILD-PS51=PASS`,
  `D208-PACKAGE-BUILD-PS7=PASS`, `D208-PACKAGE-IDENTITY-PROBE=PASS`.
- Review: parent `PASS`; simplification `PASS`; architecture and independent
  windows are explicitly `NO_CONCLUSION`.
- Safety boundary: no GUI/EXE startup, native QScintilla rendering,
  screenshot, live Replace All operation, or test-only asset was run or
  created.
- Acceptance: `S259`; native rendering, runtime, accessibility, and
  enterprise release gates remain open.

## Completed bounded slice — Command-rail visual hierarchy (D170 / UI-82 / ARCH-157)

- Outcome: make the top command rail lighter, more breathable, and easier to
  scan while preserving all command behavior.
- Scope: `theme.py` command-bar/context QSS only; command construction, roles,
  labels, shortcuts, callbacks, icons, and application policy remain
  unchanged.
- Recorded evidence: `D170-QSS-CONTRACT-PROBE=PASS`,
  `D170-CONTRAST-PROBE=PASS`, `D170-STATE-CONTRAST-PROBE=PASS`,
  `D170-SIMPLIFICATION-ASSESSMENT=PASS`; architect and independent windows are
  explicitly `NO_CONCLUSION`, parent review `PASS`.
- Limits: native toolbar metrics, accessibility, DPI, Qt/EXE runtime,
  clean-machine, cross-machine, legal, support, signing, installer, updater,
  and release-owner gates remain open.

## Completed bounded slice — Document-tab visual hierarchy (D171 / UI-83 / ARCH-158)

- Outcome: make the document tab rail lighter and easier to scan while
  preserving all tab and document behavior.
- Scope: `theme.py` document-tab and close-button QSS only; tab creation,
  titles, modified markers, icons, signals, and application policy remain
  unchanged.
- Recorded evidence: `D171-QSS-CONTRACT-PROBE=PASS`,
  `D171-CONTRAST-PROBE=PASS`, `D171-SIMPLIFICATION-ASSESSMENT=PASS`;
  architect and independent windows are explicitly `NO_CONCLUSION`, parent
  review `PASS`.
- Limits: native tab painting/layout, metrics, accessibility, DPI, Qt/EXE
  runtime, clean-machine, cross-machine, legal, support, signing, installer,
  updater, and release-owner gates remain open.

## Completed bounded slice — Editor-stage visual depth (D172 / UI-84 / ARCH-159)

- Outcome: make the central editor feel like one primary canvas inside a
  softer stage while preserving all editing behavior.
- Scope: `theme.py` editor-shell/editor QSS only; fonts, lexer, syntax, caret,
  selection, line numbers, wrapping, signals, and application policy remain
  unchanged.
- Recorded evidence: `D172-QSS-CONTRACT-PROBE=PASS`,
  `D172-CONTRAST-PROBE=PASS`, `D172-SIMPLIFICATION-ASSESSMENT=PASS`;
  architect and independent windows are explicitly `NO_CONCLUSION`, parent
  review `PASS`.
- Limits: native QScintilla painting/layout, metrics, accessibility, DPI,
  Qt/EXE runtime, clean-machine, cross-machine, legal, support, signing,
  installer, updater, and release-owner gates remain open.

## Completed bounded slice — Status-rail visual hierarchy (D173 / UI-85 / ARCH-160)

- Outcome: make the bottom status area calmer and easier to scan while
  preserving notification and lifecycle semantics.
- Scope: `theme.py` status-bar/message/rail/context/phase QSS only; text,
  severity, locale, tooltip, timer, phase state, accessible names, and policy
  remain unchanged.
- Recorded evidence: `D173-QSS-CONTRACT-PROBE=PASS`,
  `D173-CONTRAST-PROBE=PASS`, `D173-SIMPLIFICATION-ASSESSMENT=PASS`;
  architect and independent windows are explicitly `NO_CONCLUSION`, parent
  review `PASS`.
- Limits: native status-bar painting/layout, metrics, accessibility, DPI,
  Qt/EXE runtime, clean-machine, cross-machine, legal, support, signing,
  installer, updater, and release-owner gates remain open.

## Completed bounded slice — Workspace-dock visual hierarchy (D174 / UI-86 / ARCH-161)

- Outcome: make the left Workspace dock lighter and easier to scan while
  preserving workspace navigation and file behavior.
- Scope: `theme.py` WorkspaceDock frame/title/close/float QSS only; docking,
  tree data, file/folder activation, search, locale, signals, and policy remain
  unchanged.
- Recorded evidence: `D174-QSS-CONTRACT-PROBE=PASS`,
  `D174-WORKSPACE-CONTRAST-PROBE=PASS`, `D174-SIMPLIFICATION-ASSESSMENT=PASS`;
  architect and independent windows are explicitly `NO_CONCLUSION`, parent
  review `PASS`.
- Limits: native Qt docking/title-button painting, metrics, accessibility,
  DPI, Qt/EXE runtime, clean-machine, cross-machine, legal, support, signing,
  installer, updater, and release-owner gates remain open.

## Completed bounded slice — Command Palette visual hierarchy (D175 / UI-87 / ARCH-162)

- Outcome: make the keyboard-first Command Palette easier to scan while
  preserving command filtering and activation behavior.
- Scope: `theme.py` Command Palette list/hint QSS only; filtering, ordering,
  stable IDs, selection, activation, return-key, locale, and modal policy
  remain unchanged.
- Recorded evidence: `D175-QSS-CONTRACT-PROBE=PASS`,
  `D175-COMMAND-BEHAVIOR-SOURCE-PROBE=PASS`,
  `D175-COMMAND-PALETTE-CONTRAST-PROBE=PASS`,
  `D175-SIMPLIFICATION-ASSESSMENT=PASS`; architect and independent windows
  are explicitly `NO_CONCLUSION`, parent review `PASS`.
- Limits: native Qt list/focus painting and metrics, accessibility, DPI,
  Qt/EXE runtime, clean-machine, cross-machine, legal, support, signing,
  installer, updater, and release-owner gates remain open.

## Completed bounded slice — FindBar visual rhythm (D176 / UI-88 / ARCH-163)

- Outcome: make the editor Find/Replace bar calmer and easier to scan while
  preserving search and replacement behavior.
- Scope: `theme.py` FindBar container/label/checkbox/status QSS only; query,
  replacement, signals, keyboard handling, action roles, locale, and operation
  state remain unchanged.
- Recorded evidence: `D176-FINDBAR-QSS-CONTRACT-PROBE=PASS`,
  `D176-FINDBAR-BEHAVIOR-SOURCE-PROBE=PASS`,
  `D176-FINDBAR-CONTRAST-PROBE=PASS`,
  `D176-SIMPLIFICATION-ASSESSMENT=PASS`; architect and independent windows
  are explicitly `NO_CONCLUSION`, parent review `PASS`.
- Limits: native Qt FindBar painting/layout and metrics, accessibility, DPI,
  Qt/EXE runtime, clean-machine, cross-machine, legal, support, signing,
  installer, updater, and release-owner gates remain open.

## Completed bounded slice — Settings guidance hierarchy (D177 / UI-89 / ARCH-164)

- Outcome: make Settings guidance easier to scan while preserving settings
  values, preview, save, locale, font fallback, persistence, and motion.
- Scope: `theme.py` `settingsNote`/`settingsFontNote` QSS only; no dialog or
  SettingsService behavior changed.
- Recorded evidence: `D177-SETTINGS-NOTE-QSS-CONTRACT-PROBE=PASS`,
  `D177-SETTINGS-BEHAVIOR-SOURCE-PROBE=PASS`,
  `D177-SETTINGS-NOTE-CONTRAST-PROBE=PASS`,
  `D177-SIMPLIFICATION-ASSESSMENT=PASS`; architect and independent windows
  are explicitly `NO_CONCLUSION`, parent review `PASS`.
- Limits: native Qt dialog painting/layout and metrics, accessibility, DPI,
  Qt/EXE runtime, clean-machine, cross-machine, legal, support, signing,
  installer, updater, and release-owner gates remain open.

## Completed bounded slice — Message dialog action hierarchy (D178 / UI-90 / ARCH-165)

- Outcome: make common message-box actions scannable as primary, warning, and
  quiet choices while preserving message decisions and localization.
- Scope: `message_surface.py` standard-button role projection; existing theme
  role selectors remain the visual owner.
- Recorded evidence: `D178-MESSAGE-ACTION-SOURCE-PROBE=PASS`,
  `D178-MESSAGE-THEME-ROLE-CONTRACT-PROBE=PASS`,
  `D178-MESSAGE-BEHAVIOR-SOURCE-PROBE=PASS`,
  `D178-SIMPLIFICATION-ASSESSMENT=PASS`; architect and independent windows
  are explicitly `NO_CONCLUSION`, parent review `PASS`.
- Limits: native Qt message-box painting/layout and metrics, accessibility,
  DPI, Qt/EXE runtime, clean-machine, cross-machine, legal, support, signing,
  installer, updater, and release-owner gates remain open.

## Completed bounded slice — Plugin catalog guidance capsule (D179 / UI-91 / ARCH-166)

- Outcome: make the plugin catalog governance hint easier to scan while
  preserving catalog content, locale, selection, governance signals, and
  trust/approval policy.
- Scope: `theme.py` scoped `dialogHint` QSS only; no PluginCatalogDialog
  behavior changed.
- Recorded evidence: `D179-PLUGIN-HINT-QSS-CONTRACT-PROBE=PASS`,
  `D179-PLUGIN-HINT-BEHAVIOR-SOURCE-PROBE=PASS`,
  `D179-PLUGIN-HINT-CONTRAST-PROBE=PASS`,
  `D179-SIMPLIFICATION-ASSESSMENT=PASS`; architect and independent windows
  are explicitly `NO_CONCLUSION`, parent review `PASS`.
- Limits: native Qt dialog painting/layout and metrics, accessibility, DPI,
  Qt/EXE runtime, clean-machine, cross-machine, legal, support, signing,
  installer, updater, and release-owner gates remain open.

## Completed bounded slice — Font-style settings contract (D180 / UI-92 / ARCH-167)

- Outcome: deliver actual interface and editor font-style switching with
  localized controls, pending preview, persistence, schema compatibility, and
  application to the current editor tabs.
- Scope: `domain.models`, `application.settings`,
  `infrastructure.settings_store`, and the existing presentation settings,
  theme, preview, and editor-adapter seams; no new settings system or editor
  operation policy.
- Supported values: `regular`, `semibold`, `bold`, and `italic` for both UI and
  editor fonts.
- Recorded evidence: `D180-SETTINGS-FONT-STYLE-CONTRACT-PROBE=PASS`,
  `D180-I18N-FONT-STYLE-PROBE=PASS`, `D180-QSS-FONT-STYLE-PROBE=PASS`,
  `D180-QFONT-PROJECTION-PROBE=PASS`, `D180-SOURCE-WIRING-PROBE=PASS`,
  `D180-SIMPLIFICATION-ASSESSMENT=PASS`; architect and independent Terra
  windows returned `NO_CONCLUSION`, parent review `PASS`.
- Limits: native Qt font fallback/rendering, metrics, accessibility, DPI,
  GUI/EXE runtime, clean-machine, cross-machine, legal, support, signing,
  installer, updater, and release-owner gates remain open.

## Completed bounded slice — Status accessibility localization (D181 / UI-93 / ARCH-168)

- Outcome: complete the English/Simplified Chinese locale projection for the
  status rail's accessibility names and dynamic phase description.
- Scope: `presentation.i18n`, `presentation.status_bar`, and
  `presentation.status_surface`; visible notification payloads, severity,
  timers, lifecycle policy, QSS, and application ownership remain unchanged.
- Recorded evidence: `D181-ACCESSIBILITY-LOCALE-PROBE=PASS`,
  `D181-SOURCE-WIRING-PROBE=PASS`, `D181-SIMPLIFICATION-ASSESSMENT=PASS`;
  architect and independent windows are explicitly `NO_CONCLUSION`, parent
  review `PASS`.
- Limits: native screen-reader output, accessibility-tree behavior, DPI,
  Qt/EXE runtime, clean-machine, cross-machine, legal, support, signing,
  installer, updater, and release-owner gates remain open.

## Completed bounded slice — Framework-neutral theme token boundary (D182 / UI-94 / ARCH-169)

- Outcome: separate pure palette/token/contrast resolution from Qt-specific
  palette and QSS projection to improve extensibility and module cohesion.
- Scope: new `presentation/theme_tokens.py` plus the compatibility import edge
  in `presentation/theme.py`; theme IDs, accent IDs, gold readability, editor
  tokens, QSS selectors, settings, locale, motion, and application ownership
  remain unchanged.
- Recorded evidence: `D182-TOKEN-BOUNDARY-PROBE=PASS`,
  `D182-TOKEN-COMPATIBILITY-PROBE=PASS`,
  `D182-CONTRAST-ENDPOINT-PROBE=PASS`, `D182-QSS-WIRING-PROBE=PASS`,
  `D182-SIMPLIFICATION-ASSESSMENT=PASS`; architect and independent windows
  are explicitly `NO_CONCLUSION`, parent review `PASS`.
- Limits: native Qt rendering, font fallback, accessibility, DPI, GUI/EXE
  runtime, clean-machine, cross-machine, legal, support, signing, installer,
  updater, and release-owner gates remain open.

## Completed bounded slice — Restrained surface-gradient hierarchy (D183 / UI-95 / ARCH-170)

- Outcome: add a modern depth cue to the main window, editor shell, and
  command rail through three restrained ThemeColors-driven gradients.
- Scope: three QSS selectors in `presentation/theme.py`; editor canvas,
  controls, state selectors, settings, locale, motion, signals, and
  application policy remain unchanged.
- Recorded evidence: `D183-GRADIENT-CONTRACT-PROBE=PASS`,
  `D183-SURFACE-ENDPOINT-CONTRAST-PROBE=PASS`,
  `D183-SIMPLIFICATION-ASSESSMENT=PASS`; architect and independent windows
  are explicitly `NO_CONCLUSION`, parent review `PASS`.
- Limits: native Qt QSS parsing/painting, DPI, screenshot/runtime visual
  review, GUI/EXE runtime, clean-machine, cross-machine, legal, support,
  signing, installer, updater, and release-owner gates remain open.

## Completed bounded slice — Local hash-gated distribution scripts (D184 / ARCH-171)

- Outcome: add local install/update/uninstall entry points around the portable
  executable with SHA-256 verification, user-local default placement, bounded
  rollback, and explicit opt-in HKCU file associations.
- Scope: `packaging/QuillForge.Distribution.psm1`, `packaging/install.ps1`,
  `packaging/update.ps1`, and `packaging/uninstall.ps1`; no Python/UI/runtime
  behavior changed.
- Recorded evidence: `D184-STATIC-DISTRIBUTION-PROBE=PASS`,
  `D184-POWERSHELL-PARSE=PASS`, `D184-SIMPLIFICATION-ASSESSMENT=PASS`;
  architecture window returned `NO_CONCLUSION`; independent review is recorded
  separately; parent review is the only claimed PASS conclusion.
- Safety boundary: no script was executed, no registry was touched, no network
  request or EXE was started, and no test-only asset was created.
- Limits: installer/update/association runtime, clean-machine, signing,
  cross-machine, support, and release-owner gates remain open.

## Completed bounded slice — Form-control highlight closure (D185 / UI-96 / ARCH-172)

- Outcome: strengthen existing input focus surfaces and ComboBox popup hover/
  selected states so settings choices have a visible highlight hierarchy.
- Scope: centralized QSS in `src/quillforge/presentation/theme.py` only;
  settings values, signals, layout, locale, fonts, motion, and application
  policy remain unchanged.
- Recorded evidence: `D185-QSS-HIGHLIGHT-CONTRACT-PROBE=PASS combos=12`,
  `D185-COMPILE-RUFF-FORMAT=PASS`, `D185-SIMPLIFICATION-ASSESSMENT=PASS`;
  architect and independent windows returned `NO_CONCLUSION`, parent review
  is the only claimed PASS conclusion.
- Safety boundary: no GUI/EXE startup, screenshot, native popup rendering, or
  unit-test asset was created or run.
- Limits: native popup metrics/accessibility, DPI, runtime, clean-machine,
  cross-machine, legal, support, signing, installer, updater, and release
  owner gates remain open.

## Completed bounded slice — Workspace file-entry closure (D186 / UI-97 / ARCH-173)

- Outcome: expose an explicit localized Open file action in the workspace dock
  for the already-supported file picker/document-open flow.
- Scope: `workspace_panel.py`, `workspace_surface.py`, `main_window.py`,
  `file_dialog_surface.py` boundary reuse, `i18n.py`, and scoped workspace QSS;
  no second open service or folder/tree activation policy.
- Recorded evidence: `D186-WORKSPACE-FILE-ACTION-PROBE=PASS`,
  `D186-SURFACE-CALLBACK-PROBE=PASS`,
  `D186-REUSE-OPEN-BOUNDARY-PROBE=PASS`, `D186-I18N-PROBE=PASS en+zh-CN`,
  `D186-FILE-DIALOG-CHAIN-PROBE=PASS`,
  `D186-FILE-ACTION-QSS-PROBE=PASS`, `D186-COMPILE-RUFF-FORMAT=PASS`;
  architecture and independent windows are recorded separately.
- Safety boundary: no GUI/EXE startup, native file dialog, screenshot, or
  test-only asset was run or created.
- Limits: native dialog behavior, accessibility, DPI, runtime, clean-machine,
  cross-machine, signing, installer, updater, support, and release-owner gates
  remain open.

## Completed bounded slice — FindBar authored action icons (D169 / UI-81 / ARCH-156)

- Outcome: add consistent authored icons to Find/Replace navigation, replace,
  cancel, and close actions while preserving all existing behavior.
- Scope: `find_bar.py`, `icon_contract.py`, and `icons.py` only; existing
  localized labels, signals, shortcuts, state projection, and application
  policy remain unchanged.
- Recorded evidence: `D169-CONTRACT-AST-PROBE=PASS`,
  `D169-ICONS-AST-PROBE=PASS`, `D169-FIND-AST-PROBE=PASS`,
  `D169-ICON-MAPPING-PROBE=PASS`, `D169-SIGNAL-PRESERVATION-PROBE=PASS`,
  `D169-LOCALE-REFRESH-PROBE=PASS`, `D169-CONTRAST-PROBE=PASS`,
  `D169-SIMPLIFICATION-ASSESSMENT=PASS`; architect and independent windows
  are explicitly `NO_CONCLUSION`, parent review `PASS`.
- Limits: native icon/tooltip rendering, accessibility, DPI, Qt/EXE runtime,
  clean-machine, cross-machine, legal, support, signing, installer, updater,
  and release-owner gates remain open.

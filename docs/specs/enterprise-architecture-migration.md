# QuillForge enterprise architecture migration baseline

| Field | Value |
|---|---|
| Status | Phase 1 baseline and first slice in progress |
| Owner | Architect |
| Scope | Python 3.12 / PyQt6 desktop application |
| Compatibility | Preserve current commands, settings, plugin deny-by-default behavior, file flows, and no-launch policy |

## Source applicability

This project does not have access to ByteDance's private internal engineering
standards. The following are public sources and are used only for the stated,
transferable observations:

- [CloudWeGo About](https://www.cloudwego.io/about/) — ByteDance's public
  CloudWeGo ecosystem describes high performance, scalability, reliability,
  independently usable components, and a community feedback loop.
- [CloudWeGo open-source announcement](https://www.cloudwego.io/blog/2021/09/13/cloudwego-open-source-announcement/)
  — documents the public practice of decoupling the open core from internal
  infrastructure, keeping internal/external code aligned where possible, and
  releasing verified capabilities incrementally.
- [Kitex framework extension](https://www.cloudwego.io/docs/kitex/tutorials/framework-exten/)
  — describes interface-oriented extension through middleware, options, and
  composable suites.
- [Hertz public repository](https://github.com/cloudwego/hertz) — documents
  extension points, error handling, logging, monitoring, and tracing as public
  framework concerns.
- [Python 3.12 typing documentation](https://docs.python.org/3.12/library/typing.html)
  — documents `Protocol` as the structural contract mechanism used by this
  Python application.
- [Qt 6 QObject documentation](https://doc.qt.io/qt-6/qobject.html) and
  [QCoreApplication event-loop documentation](https://doc.qt.io/qt-6/qcoreapplication.html)
  — define QObject ownership/signals and the event-loop boundary that the
  presentation layer must respect.
- [PyInstaller runtime information](https://pyinstaller.org/en/stable/runtime-information.html)
  — defines frozen-process identity and bundled-resource path behavior used by
  the release/diagnostic boundary.

These sources are engineering references, not manufacturer requirements or
claims about ByteDance's private implementation.

## User outcome

QuillForge should be extensible by adding a bounded service, adapter, or
presentation projection rather than editing a giant cross-layer function. A
future maintainer should be able to locate ownership, contract, failure path,
and verification evidence for each material feature.

## Target architecture

```text
Entry points / diagnostics
          |
Composition root (wiring only)
     /         |          \
Presentation  Application  Infrastructure adapters
     |           |                 |
     +------> typed ports <--------+
                  |
                Domain

Plugins -> public plugin API -> application commands/events
```

### Invariants

1. Domain models and policies do not import Qt, filesystem, subprocess, or
   concrete widget types.
2. Application services own use-case sequencing and typed failure semantics;
   they depend on ports, not concrete infrastructure.
3. Infrastructure owns OS/filesystem/process/persistence details behind ports.
4. Presentation owns Qt object trees, signals, QSS, locale projection, and
   view state; it never becomes a persistence or plugin-loader layer.
5. The composition root is the only place allowed to select concrete adapters,
   built-in plugins, default paths, and lifecycle wiring.
6. Every extension seam has one canonical contract and an explicit version or
   compatibility rule. Trust, enablement, capability, and execution remain
   separate decisions.
7. Async work reports lifecycle through one observable contract, and workers
   never mutate widgets directly.
8. A material change ships as a small vertical slice with an ADR/review,
   static evidence, package provenance, explicit unrun checks, and one handoff.

## Migration phases

| Phase | Outcome | Status |
|---|---|---|
| 1 | Extract and name the composition root; keep runtime behavior byte-for-byte equivalent at the call boundary | accepted-with-limits |
| 2 | Split `MainWindow` orchestration into application-facing coordinators and presentation projections without moving business policy | current slices: command, document-tab, workspace, and workspace-search surfaces |
| 3 | Consolidate typed application contracts and error taxonomy; remove concrete-adapter leakage | in progress; D214 first slice accepted-with-limits; broader coverage pending |
| 4 | Make plugin/command/document/workspace extension contracts versioned and observable | partly present; audit pending |
| 5 | Add CI/static architecture and package gates plus release evidence automation | partly present; external gates open |
| 6 | Perform bounded performance and runtime visual validation with authorization | open |

## Phase 1 acceptance

- `app.py` remains the entry-point/diagnostic dispatcher and event-loop owner;
  diagnostic adapter selection is delegated to the Qt-free diagnostic
  composition boundary.
- A separate composition module owns concrete adapter construction and desktop
  runtime lifecycle wiring.
- `MainWindow` receives the same application services and ports as before.
- Plugin activation/deactivation ordering, startup restore, menu refresh, and
  close behavior are unchanged.
- Application/domain modules do not gain infrastructure or Qt imports.
- Static compile/lint/format, source boundary, JSON/handoff, package identity,
  and release no-go checks are recorded. Qt startup remains unrun.

## Phase 2 first-slice acceptance

- `CommandSurface` owns only menu/toolbar Qt projection and locale refresh.
- `MainWindow` retains core command registration, callback ownership, and the
  stable `refresh_command_menus()` lifecycle seam.
- Command ordering, shortcuts, toolbar order/separator/icons, plugin refresh,
  and locale retranslation remain source-equivalent.
- The new presentation coordinator does not import infrastructure or move
  application/domain policy into Qt code.

## Phase 2 document-tab surface acceptance

- `DocumentTabSurface` owns the `QTabWidget`, identity-based record collection,
  current-index/active-record lookup, editor lookup, title synchronization, and
  tab-bar enablement.
- `MainWindow` retains document state transitions, path uniqueness,
  save-before-close confirmation, recovery/session bookkeeping, lifecycle event
  publication, and the close/current callbacks supplied to the surface.
- The surface's structural `DocumentTabLike` contract requires only an
  `EditorWidget`; it does not import application services or infrastructure.
- Add/remove ordering keeps the record collection and Qt index projection
  consistent when current-change callbacks are delivered.
- Static boundary, compile/lint/format, handoff, package identity, and release
  no-go evidence are recorded; Qt runtime startup and visual interaction remain
  unrun under the no-launch policy.

## Phase 2 workspace surface acceptance

- `WorkspaceSurface` owns creation and lifetime parenting of the workspace
  `QDockWidget` and `WorkspacePanel`, the five existing semantic signal routes,
  and dock/panel locale projection.
- `MainWindow` retains `WorkspaceService`/`TaskRunner` operation state,
  generation and cancellation guards, root containment, workspace-search and
  session-restore coordination, notifications, and error policy.
- `WorkspaceSurfaceCallbacks` carries semantic intents only and does not expose
  application services, mutable containers, or a widget to the callback owner.
- Existing signal payloads and callback targets remain source-equivalent; the
  panel's visible directory page remains owned by `WorkspacePanel`.
- Static boundary, compile/lint/format, handoff, package identity, and release
  no-go evidence are recorded; Qt startup, dock parenting, and interactive file
  activation remain unrun under the no-launch policy.

## Phase 2 workspace file activation acceptance

- The existing WorkspacePanel semantic route supports file click,
  double-click, and Enter/Return activation while directory double-click and
  keyboard navigation remain intact.
- MainWindow retains busy gating, workspace containment, existing-tab
  identity, asynchronous document dispatch, and notification policy.
- No new coordinator, service, filesystem policy, or Qt signal is introduced;
  static boundary, compile/lint/format, handoff, package identity, and release
  no-go evidence are recorded while native interaction remains unrun.

## Phase 2 workspace tree visual-rhythm acceptance

- `WorkspacePanel` may configure standard Qt view presentation only: alternate
  rows, full-row single selection, uniform row heights, and display elision.
- `theme.py` remains the sole QSS token projection; alternate and alternate-
  hover states reuse existing surfaces and do not override selected/disabled
  intent.
- Semantic signals, item data, MainWindow policy, application services, and
  filesystem behavior remain unchanged; static/package/release no-go evidence
  is recorded while native rendering remains unrun.

## Phase 2 workspace-search surface acceptance

- `WorkspaceSearchSurface` owns creation and parent lifetime of the
  `WorkspaceSearchDialog`, its three semantic signal routes, non-modal
  activation, locale projection, busy/cancellation feedback, and result/error
  projection.
- `MainWindow` retains `WorkspaceSearchService` query validation,
  `TaskRunner` submission, operation/generation/cancellation state, stale
  result rejection, root containment, document activation, notifications, and
  session/workspace coordination.
- `WorkspaceSearchSurfaceCallbacks` preserves the existing `(str, bool)`,
  `()`, and `(object, int)` payload contracts and does not expose application
  services, mutable containers, or infrastructure.
- Switching workspace roots invalidates an active search before the surface
  clears stale rows, preserving the previous cancellation/generation order.
- Static boundary, compile/lint/format, handoff, package identity, and release
  no-go evidence are recorded; Qt startup, dialog activation, visual search,
  and interactive result opening remain unrun under the no-launch policy.

## Phase 2 workspace projection-closure acceptance

- `WorkspaceSurface` owns semantic current-path, loading, directory-result, and
  recoverable-error projection in addition to the dock/panel object tree and
  navigation-intent routes.
- `MainWindow` imports neither `WorkspacePanel` nor a panel compatibility
  property; it projects workspace results only through `WorkspaceSurface`.
- The surface methods delegate one-to-one to the existing panel and do not
  create a second directory model, move application policy, or change the
  `WorkspaceService`/`TaskRunner` lifecycle.
- Existing operation completion, generation/cancellation, session-restore,
  containment, notification, and error branches remain source-equivalent.
- Static boundary, compile/lint/format, handoff, package identity, and release
  no-go evidence are recorded; Qt startup and interactive workspace behavior
  remain unrun under the no-launch policy.

## Phase 2 Find surface acceptance

- `FindSurface` owns `FindBar` construction/lifetime, six existing semantic
  signal routes, shell widget placement, hide/show-find mode, locale, query/
  replacement/case access, status, operation-active, and reset projection.
- `MainWindow` retains active-tab/editor lookup, literal find/replace calls,
  `_find_match`, Replace All session/rollback/cancellation, content-version
  checks, tab locking, notifications, and error policy.
- Signal payloads and keyboard behavior remain source-equivalent; the surface
  introduces no application/infrastructure dependency or second editor state.
- Static boundary, compile/lint/format, handoff, package identity, and release
  no-go evidence are recorded; Qt startup, focus/keyboard, and interactive
  Replace All behavior remain unrun under the no-launch policy.

## Phase 2 settings surface acceptance

- `SettingsSurface` owns `SettingsDialog` parentage and modal execution, and
  returns `SettingsSnapshot | None` only after the existing Save/cancel path.
- `MainWindow` retains settings-service availability, save-in-flight guards,
  TaskRunner submission, operation IDs, result validation, theme/locale/editor
  projection, motion policy and transition trigger timing, notifications, and
  error policy.
- The surface imports only Qt, the domain snapshot, and the existing
  presentation dialog; it introduces no persistence, worker, or second state
  model.
- Static boundary, compile/lint/format, handoff, package identity, and release
  no-go evidence are recorded; Qt startup and interactive settings behavior
  remain unrun under the no-launch policy.

## Phase 2 recovery prompt surface acceptance

- `RecoveryPromptSurface` owns prompt parentage, locale-aware path/time/source
  text, button roles, and a typed `restore | discard | later` decision.
- `MainWindow` retains RecoveryService calls, snapshot restore/discard,
  deferred session paths, document events, cleanup scheduling, and
  notifications.
- The surface imports no RecoveryService, application policy, or infrastructure
  adapter and introduces no second recovery state model.
- Static boundary, compile/lint/format, handoff, package identity, and release
  no-go evidence are recorded; Qt startup and interactive recovery behavior
  remain unrun under the no-launch policy.

## Phase 2 status surface acceptance

- `StatusSurface` owns StatusRail creation/parentage, status-bar widget
  exposure, locale projection, and semantic phase projection.
- `MainWindow` retains working-over-attention precedence, explicit error
  projection, operation/document state, TaskRunner lifecycle, and close guards.
- The surface imports only Qt, domain Locale, and the existing presentation
  rail; it introduces no second phase model or application policy.
- Static boundary, compile/lint/format, handoff, package identity, and release
  no-go evidence are recorded; Qt startup and status rendering remain unrun
  under the no-launch policy.

## Phase 2 file-dialog surface acceptance

- `FileDialogSurface` owns localized native `getOpenFileName`,
  `getExistingDirectory`, and `getSaveFileName` composition and returns
  `Path | None` with the existing cancellation/default-path semantics.
- `MainWindow` retains startup/busy guards, asynchronous document/workspace
  dispatch, Save As policy, session/recovery paths, notifications, and all
  containment decisions.
- Direct Open and Save As remain explicit local-path flows; workspace-tree and
  search-result containment are unchanged. The surface imports no application
  or infrastructure code and introduces no second path-policy model.
- Static boundary, compile/lint/format, handoff, package identity, and release
  no-go evidence are recorded; Qt startup and native dialog interaction remain
  unrun under the no-launch policy.

## Phase 2 command-palette surface acceptance

- `CommandPaletteSurface` owns command-palette modal parentage, locale, and
  Accepted/cancelled stable-ID projection; it never executes commands or owns
  the registry.
- `MainWindow` re-resolves the selected ID through the live `CommandRegistry`,
  retains execution/stale-ID/menu-refresh policy, and localizes stale-command
  feedback through the existing message adapter.
- The existing `CommandPaletteDialog` accepts the real `QWidget | None` parent
  contract; no command search or plugin behavior changes.
- Static boundary, compile/lint/format, handoff, package identity, and release
  no-go evidence are recorded; Qt startup and palette interaction remain unrun
  under the no-launch policy.

## Phase 2 plugin surface acceptance

- `PluginSurface` owns extension-catalog and plugin-status dialog construction,
  replacement, activation, locale, four semantic callback routes, and catalog
  governance-button projection.
- `MainWindow` retains PluginCatalog/Approval/Runtime services, TaskRunner and
  operation state, trust/approval/enablement policy, result validation,
  notifications, and error behavior.
- The surface creates no plugin lifecycle state, does not execute plugin code,
  and does not broaden the existing D6 trust/approval/external-execution
  contracts. The independent review window's no-conclusion status is explicit.
- Static boundary, compile/lint/format, handoff, package identity, and release
  no-go evidence are recorded; Qt startup and plugin-dialog interaction remain
  unrun under the no-launch policy.

## Phase 2 message surface acceptance

- `MessageSurface` owns localized save-before-close, About, and recoverable
  error QMessageBox composition and returns a typed `save | discard | cancel`
  decision for close confirmation.
- `MainWindow` retains dirty-state checks, Save As selection, asynchronous save
  dispatch, tab removal, error phase projection, and status synchronization.
- The surface imports no application service, document state, persistence
  adapter, or TaskRunner and introduces no second close/save/error state model.
- Static boundary, compile/lint/format, handoff, package identity, and release
  no-go evidence are recorded; Qt startup and message-dialog interaction
  remain unrun under the no-launch policy. The independent review window's
  no-conclusion status is explicit.

## Phase 2 theme transition surface acceptance

- `ThemeTransitionSurface` owns the short-lived opacity effect, easing,
  animation lifetime, interrupted-animation replacement, and cleanup of only
  the effect it created; it leaves an existing external graphics effect
  untouched.
- `MainWindow` retains the `motion_enabled` decision, central-widget selection,
  and post-settings-save trigger order; disabled motion always clears an active
  transition through the same surface seam.
- The surface imports no settings, theme, editor, application-service,
  persistence, or TaskRunner code and introduces no general animation
  framework.
- Static boundary, compile/lint/format, handoff, package identity, and release
  no-go evidence are recorded; Qt startup and animation rendering remain
  unrun under the no-launch policy. The independent review window's
  no-conclusion status is explicit.

## Phase 2 status-surface host acceptance

- `StatusSurface` owns `QStatusBar` attachment, size-grip configuration,
  permanent `StatusRail` placement, locale state, and localized transient
  notification projection with the existing 5000 ms default.
- `MainWindow` retains phase precedence, TaskRunner/document policy,
  notification call sites, and the single status-bar host selection during
  initialization.
- Repeated attachment to the same host is idempotent; a different host first
  removes the owned rail. The surface imports no application service,
  document state, persistence, TaskRunner, or operation policy.
- Static boundary, compile/lint/format, handoff, package identity, and release
  no-go evidence are recorded; Qt startup and status-bar interaction remain
  unrun under the no-launch policy. The independent review window's
  no-conclusion status is explicit.

## Phase 2 central editor shell acceptance

- `EditorShellSurface` owns the central `editorShell` QWidget/layout,
  `DocumentTabSurface` and `FindSurface` child composition, initial FindBar
  hidden state, and the FindBar locale route.
- `MainWindow` retains the semantic callbacks, document/editor state,
  Find/Replace and close/save policy, operation lifecycle, and session/recovery
  decisions; the surface owns no application or document policy.
- The surface imports no application service, document model, persistence
  adapter, or TaskRunner and introduces no general shell/layout framework.
- Static boundary, compile/lint/format, handoff, package identity, and release
  no-go evidence are recorded; Qt startup, tab/FindBar interaction, and visual
  acceptance remain unrun under the no-launch policy. The independent review
  window's no-conclusion status is explicit.

## Phase 2 editor-document adapter acceptance

- `EditorDocumentSurface` owns `EditorWidget` creation, initial language and
  settings application, text/dirty initialization order, Save As language-hint
  refresh, and modified/content/caret signal routing.
- `MainWindow` retains `_DocumentTab`, document service results, dirty-state and
  content-version policy, save/recovery/Replace All semantics, session writes,
  and operation locking; callbacks remain semantic and supplied by the shell.
- The surface imports no application service, document record, persistence
  adapter, TaskRunner, or operation policy and introduces no second document
  state model.
- Static boundary, compile/lint/format, handoff, package identity, and release
  no-go evidence are recorded; Qt startup, editor interaction, visual output,
  and accessibility remain unrun under the no-launch policy. The independent
  review result or explicit no-conclusion status is recorded.

## Phase 1 visual token contrast and hierarchy acceptance

- `ThemeColors` derives a deterministic readable foreground for each filled
  accent endpoint; the warning action's amber/砂金 hover state uses its own
  `on_accent_gold` token rather than borrowing an unrelated endpoint choice.
- Existing theme IDs, accent IDs, settings, localization, action roles,
  keyboard behavior, and editor syntax roles remain unchanged.
- The command rail and document-tab surface receive only centralized QSS
  spacing/border/radius hierarchy refinements; no widget-local stylesheet or
  second theme source is introduced.
- A static contrast probe covers every theme/accent endpoint and the warning
  attention pair at >= 4.5:1. Compile/lint/format, handoff, package identity,
  and release no-go evidence are recorded; Qt rendering, fonts, DPI,
  accessibility, and cross-machine appearance remain unrun. The independent
  review or explicit no-conclusion status is recorded.

## Phase 1 authored iconography acceptance

- `presentation/icons.py` is the sole authored vector-icon provider for shell
  navigation and exposes semantic `IconKey` values without application,
  document, filesystem, persistence, or command-registry dependencies.
- `CommandSurface` owns toolbar icon projection and explicit palette-based
  retinting; `WorkspaceSurface`/`WorkspacePanel` own workspace-entry and
  navigation icon projection. Theme refresh does not depend on locale refresh
  side effects.
- Toolbar labels, callbacks, shortcuts, command IDs, locale strings, and
  workspace file-first-click/folder-double-click behavior remain unchanged;
  `QStyle.StandardPixmap` is not used by the presentation layer.
- Compile/lint/format, source-boundary, handoff, package identity, and release
  no-go evidence are recorded. Qt painting, high-DPI/native metrics,
  screenshots, accessibility, and cross-machine appearance remain unrun; the
  independent review or explicit no-conclusion status is recorded.

## Phase 1 semantic transient notification acceptance

- `StatusSurface` remains the sole presentation owner of the status-bar host,
  permanent lifecycle rail, and transient notification widget.
- `StatusSurface.show_message()` accepts an optional presentation-only
  `info`/`success`/`warning`/`error` level, defaults to `info`, preserves the
  existing 5000 ms default and message replacement order, and reprojects the
  current raw message when locale changes.
- `MainWindow` retains notification text, operation/error policy, phase
  precedence, and close guards; `notify(message)` remains valid for existing
  plugin callbacks and does not infer level from arbitrary strings.
- Centralized `statusMessage` QSS uses existing theme tokens. A static probe
  covers all four levels across every supported theme/accent combination at
  >= 4.5:1 for the message foreground/background pair.
- Compile/lint/format, handoff, package identity, and release no-go evidence
  are recorded; Qt startup, status-bar rendering, timeout timing, screen
  reader behavior, DPI, and cross-machine appearance remain unrun. The
  independent review result or explicit no-conclusion status is recorded.

## Phase 1 inline workspace feedback acceptance

- `presentation.feedback` is the single presentation-only contract for
  `info`/`working`/`success`/`warning`/`error` state projection and dynamic QSS
  refresh; it owns no application, filesystem, persistence, or worker policy.
- `WorkspacePanel` owns current-path/loading/directory-result/error text and
  `WorkspaceSearchDialog` owns query/result/cancellation/diagnostic text;
  both reuse the shared projection without changing signals, callbacks,
  operation IDs, cancellation, root containment, or localized message text.
- Centralized workspace/search status selectors distinguish working, success,
  warning, and error through surface/border/foreground hierarchy. A static
  probe covers 60 foreground/background pairs across all supported themes,
  accents, and five states at >= 4.5:1.
- FindBar status mapping is explicitly deferred to D33 because its existing
  MainWindow call sites require a separate outcome mapping review. Compile,
  lint, format, handoff, package identity, and release no-go evidence are
  recorded; Qt startup, native QSS specificity, visual rendering, screen
  reader behavior, font metrics, DPI, and cross-machine appearance remain
  unrun. The independent review window's no-conclusion status is recorded.

## Phase 1 FindBar semantic feedback acceptance

- `FindBar.set_status(message)` remains valid as a one-argument compatibility
  seam; an optional `FeedbackLevel` keyword is forwarded through `FindSurface`
  and reapplied after locale changes and dynamic QSS refresh.
- `MainWindow` maps existing Find/Replace/Replace All outcomes explicitly:
  successful matches/replacements are `success`, input/conflict/stale-match/
  limit/cancellation outcomes are `warning`, cooperative progress is
  `working`, and Replace All failures are `error`.
- `FindBar` owns only localized status text and presentation state. MainWindow
  retains active-tab lookup, editor operations, operation IDs, tab locking,
  cancellation, rollback, notifications, and error policy; keyboard signals
  and payloads remain unchanged.
- Centralized `findStatus` selectors cover five states. A static probe covers
  60 foreground/background pairs across all supported themes and accents at
  >= 4.5:1. Compile/lint/format, handoff, package identity, and release no-go
  evidence are recorded; Qt startup, interactive Find/Replace, native QSS
  specificity, screen-reader output, font metrics, DPI, and cross-machine
appearance remain unrun. The independent review result or explicit
no-conclusion status is recorded.

## Phase 1 Session/Recovery notification severity acceptance

- The existing `MainWindow.notify(message, *, level=...)` contract is reused
  for nineteen Session/Recovery call sites. Failure and invalid-persistence
  outcomes are `error`, user-attention fallback/deferred/postponed outcomes
  are `warning`, successful recovery/discard outcomes are `success`, and
  neutral informational outcomes remain explicit `info`.
- `MainWindow` remains the outcome/policy owner; SessionService,
  RecoveryService, TaskRunner, operation IDs, stale guards, snapshot lifecycle,
  session ordering, close guards, and persistence payloads are unchanged.
- A static AST probe proves every targeted Session/Recovery notify call has a
  legal explicit level. Compile/lint/format, handoff, package identity, and
  release no-go evidence are recorded; Qt startup, native rendering,
  accessibility, fonts, DPI, clean-machine, cross-machine, and remaining
  notification call-site evidence remain unrun. The independent review or
  explicit no-conclusion status is recorded.

## Phase 1 Plugin/extension notification severity acceptance

- The existing `MainWindow.notify(message, *, level=...)` contract is reused
  for catalog scan, catalog-result, governance, runtime enablement, and
  isolated plugin-host diagnostic outcomes. In-progress transient messages
  are explicit `info` while the permanent TaskRunner/status phase is
  `WORKING`; successful completion is `success`, user-attention/rejection/
  in-flight outcomes are `warning`, and unavailable/invalid/failure outcomes
  are `error`.
- Catalog truncation and typed scan/approval diagnostics remain warnings even
  when the scan returns a valid immutable snapshot. Host `ready` is success,
  policy `rejected` is warning, and typed timeout/crash/protocol/containment
  failures are errors.
- The two corrected Session/Recovery branches follow the same contract:
  invalid/failed persistence is `error`, and a deferred recovery snapshot is
  `warning`. No service, trust boundary, execution policy, async guard, or
  second coordinator state model is introduced.
- A static source probe, compile/lint/format, independent review, handoff,
  package identity, and release no-go evidence are recorded. Qt startup,
  native rendering, accessibility, fonts, DPI, clean-machine, and
  cross-machine evidence remain unrun under the no-launch policy.
- ADR-0060 records the bounded decision; public CloudWeGo material remains an
  engineering reference only and does not establish a private ByteDance
  standard or certification claim.

## Phase 1 Workspace/operation notification severity acceptance

- `_begin_operation()` keeps existing long-running document/workspace/
  Replace All work in the permanent `WORKING` phase and projects its transient
  copy as explicit `info`, while preserving operation IDs, busy state, phase
  precedence, and completion behavior.
- Workspace/search service availability and invalid/failure results are
  `error`; startup restore guards, missing roots, containment rejections,
  cancellation, no-match, truncation/limit, and typed diagnostics are
  `warning`; clean workspace-open and complete search results are `success`.
- Search summary severity is selected from typed result fields rather than
  localized message text. Existing WorkspaceSurface and
  WorkspaceSearchSurface inline state remains intact, and invalid search
  results now reach both inline and shell error projection.
- Generation/stale guards, cooperative cancellation, workspace root
  containment, session restore barriers, and document file activation remain
  unchanged. Static source probes, compile/lint/format, handoff, package
  identity, and release no-go evidence are recorded; Qt startup, interactive
  workspace/search/file flows, accessibility, DPI, and clean-machine evidence
  remain unrun under the no-launch policy.
- ADR-0061 records the bounded decision; public CloudWeGo material remains an
  engineering reference only and does not establish a private ByteDance
  standard or certification claim.

## Phase 1 MainWindow notification contract acceptance

- All current MainWindow `notify` call sites pass an explicit legal
  `StatusMessageLevel`; the external one-argument `notify(message)` default
  remains available and continues to mean `info`.
- New-document completion is `success`; session-restore fallback, stale
  command, and in-flight settings guards are `warning`; unavailable settings
  persistence is `error`; active-tab projection is explicit `info`.
- D34a–D36a Session/Recovery, plugin/extension, Workspace/search, containment,
  cancellation, diagnostic, and operation-progress mappings remain unchanged.
  No service, state machine, operation ID, close guard, locale route, or
  plugin compatibility boundary moves.
- An AST probe covers 81 MainWindow calls. Compile/lint/format, handoff,
  package identity, and release no-go evidence are recorded; Qt startup,
  native rendering, accessibility, fonts, DPI, clean-machine, and
  cross-machine evidence remain unrun under the no-launch policy.
- ADR-0062 records the bounded decision; public CloudWeGo material remains an
  engineering reference only and does not establish a private ByteDance
  standard or certification claim.

## Phase 1 focus-state visibility acceptance

- Centralized `presentation/theme.py` QSS adds a surface/foreground cue to
  focused command-rail and ordinary tool buttons, a surface/foreground and
  accent-alt boundary to focused document tabs, and a row/indicator cue to
  focused checkboxes.
- Existing selected, pressed, checked, disabled, keyboard, command, document,
  settings, locale, theme, and motion ownership remains unchanged. No new
  focus state model, event filter, token family, or widget-local stylesheet is
  introduced.
- A source/QSS probe, compile/lint/format, handoff, package identity, and
  release no-go evidence are recorded; QApplication startup, native QSS
  specificity, focus traversal, accessibility, fonts, DPI, clean-machine, and
  cross-machine evidence remain unrun under the no-launch policy.
- ADR-0063 records the bounded decision; public CloudWeGo material remains an
  engineering reference only and does not establish a private ByteDance
  standard or certification claim.

## Phase 2 operation-tracker boundary acceptance

- `presentation.operation_tracker.OperationTracker` owns only one
  MainWindow-local monotonic operation-ID sequence and the synchronous
  `begin`/`complete`/`cancel` stale-ID invariant. It imports no Qt, services,
  widgets, or application policy.
- MainWindow retains `_busy`, `TaskRunner` submission and pending-work
  semantics, status phase/notification projection, close guards, workspace and
  session generations, service policy, and all result callbacks. The existing
  `_next_operation_id()` facade preserves the sequence for settings, recovery,
  session, search, and plugin-related worker paths.
- A current completion or cancellation clears the active operation only when
  its ID still matches. Workspace cancellation keeps its existing generation
  invalidation, surface cleanup, document/status synchronization, and session
  restore release; stale worker delivery remains isolated by existing guards.
- Static source-boundary/lifecycle probes, compile/lint/format, handoff,
  package identity, and release no-go evidence are recorded. QApplication
  startup, callback interleaving, native rendering, accessibility, fonts, DPI,
  clean-machine, and cross-machine evidence remain unrun under the no-launch
  policy. The independent-review no-conclusion status is explicit.
- ADR-0064 records the bounded decision; public CloudWeGo material remains an
  engineering reference only and does not establish a private ByteDance
  standard, certification, or compliance claim.

## Phase 2 plugin operation-state boundary acceptance

- `presentation.plugin_operation_tracker.PluginOperationTracker` owns a
  closed typed set of independent catalog-scan, catalog-governance, and
  host-probe lifecycle IDs. Each kind has its own monotonic sequence,
  in-flight state, and stale completion guard; one kind cannot clear another.
- MainWindow retains PluginCatalog/Approval/Runtime/Host services, TaskRunner
  submission, notification severity/text, PluginSurface projection,
  `refresh_command_menus()`, and all trust/approval/enablement/containment/
  execution policy. No plugin service or Qt object enters the tracker.
- The D39 single-active operation tracker is not reused because these plugin
  domains can overlap independently; this boundary preserves their existing
  concurrency semantics while removing duplicated state fields.
- Static source-boundary/lifecycle probes, compile/lint/format, handoff,
  package identity, and release no-go evidence are recorded. QApplication
  startup, callback interleaving, native dialogs, accessibility, fonts, DPI,
  clean-machine, and cross-machine evidence remain unrun under the no-launch
  policy. ADR-0065 records the bounded decision.

## Phase 2 workspace-entry activation acceptance

- `presentation.workspace_panel.WorkspacePanel` preserves the established
  mouse contract: a file click emits `file_requested`, and a directory double
  click emits `directory_requested`.
- A small `_WorkspaceTree` presentation subclass routes only Enter/Return
  keyboard activation through one presentation-only helper, with file and
  directory kinds mapped to their matching semantic callbacks; inaccessible
  entries emit neither route. Qt's style-dependent `itemActivated` signal is
  deliberately not used, so the keyboard route cannot duplicate the existing
  mouse routes. Nullable Qt items are ignored before item data is read.
- The panel performs no filesystem, document, containment, TaskRunner, or
  notification work. MainWindow remains the owner of workspace and document
  policy and async dispatch.
- A source activation/boundary probe, compile/lint/format, handoff, package
  identity, and release no-go evidence are recorded. QApplication startup,
  native event ordering, focus traversal, accessibility, fonts, DPI,
  clean-machine, and cross-machine evidence remain unrun under the no-launch
  policy. ADR-0066 records the bounded decision.

## Phase 2 workspace action hierarchy acceptance

- The existing `primaryAction`, `workspaceBack`, and `workspaceCancel` object
  names and signal wiring remain unchanged.
- Centralized `presentation.theme._stylesheet` gives Back and Cancel distinct
  quiet secondary treatments with hover, focus, pressed, and disabled states;
  Cancel uses the selected theme's warning/gold endpoint and the primary
  action keeps its readable accent foreground calculation.
- No theme data structure, locale, application service, workspace state, or
  loading policy changes. A source hierarchy probe, compile/lint/format,
  independent-review status, handoff, package identity, and release no-go
  evidence are recorded. Native QSS rendering, font metrics, DPI,
  cross-machine appearance, and external release evidence remain unrun under
  the no-launch policy. ADR-0067 records the bounded decision.

## Phase 2 workspace-search lifecycle acceptance

- `presentation.workspace_search_operation_tracker.WorkspaceSearchOperationTracker`
  owns only search operation ID, generation, cooperative cancellation, and
  `stale`/`invalidated`/`current` callback classification.
- User cancellation sets the event without changing generation; root
  invalidation increments generation before cancellation; stale IDs cannot
  clear a current operation. MainWindow retains query validation,
  `WorkspaceSearchService`, `TaskRunner`, `WorkspaceSearchSurface`, root
  containment, notifications, startup/close guards, and result policy.
- A source lifecycle/boundary probe, compile/lint/format, review status,
  handoff, package identity, and release no-go evidence are recorded. Native
  queued delivery, thread timing, close-event interleavings, runtime search,
  cross-machine, and external release evidence remain unrun under the
  no-launch policy. ADR-0068 records the bounded decision.

## Phase 2 workspace-navigation lifecycle acceptance

- `presentation.workspace_operation_tracker.WorkspaceOperationTracker` owns
  only workspace operation identity, generation, invalidation, and
  `stale`/`invalidated`/`current` callback classification.
- MainWindow keeps the generic `OperationTracker`, `_busy`, TaskRunner,
  WorkspaceService, WorkspaceSurface, workspace containment,
  session-restore barrier, notifications, startup/close guards, and result
  policy. Cancellation invalidates the workspace tracker before bridging to
  generic operation cancellation, preserving the existing busy/status path.
- A source lifecycle/boundary probe, compile/lint/format, review status,
  handoff, package identity, and release no-go evidence are recorded. Native
  queued delivery, thread timing, close-event interleavings, runtime
  navigation, cross-machine, and external release evidence remain unrun under
  the no-launch policy. ADR-0069 records the bounded decision.

## Phase 2 session-restore state acceptance

- `presentation.session_restore_tracker.SessionRestoreTracker` owns only the
  framework-neutral session-restore values: immutable snapshot, ordered
  document cursor, active path, workspace barrier, recovery-deferred paths,
  and the pending document/open-operation binding.
- MainWindow remains the application coordinator for SessionService,
  RecoveryService, WorkspaceService, DocumentService, TaskRunner, generic
  operation/busy/status state, tab projection, notifications, startup/close
  guards, and result policy. Recovery-first ordering, serial restoration,
  duplicate-tab handling, active-tab/caret projection, fallback document
  creation, and save/close behavior remain in that boundary.
- `has_remaining_documents` is non-consuming and `next_document()` is the
  ordered cursor operation. The pending document is established before async
  dispatch, and callbacks consume only the matching operation binding.
- A source boundary probe, compile/lint/format, parent/independent review
  status, handoff, package identity, and release no-go evidence are recorded.
  Native callback timing, runtime startup, file/recovery I/O, thread
  interleavings, clean-machine, cross-machine, and external release evidence
  remain unrun under the no-launch policy. ADR-0072 records this bounded
  D47/ARCH-37/UI-33 decision.

## Phase 2 document-tab state acceptance

- `presentation.icons.IconKey.MODIFIED` owns the authored document-plus-dot
  glyph. `presentation.document_tab_surface.DocumentTabSurface` owns only the
  palette-tinted icon projection, index-aligned marker list, and explicit theme
  refresh route.
- MainWindow remains the sole owner of dirty interpretation, title asterisk,
  save/recovery/close consequences, tab lifecycle, and document operations.
  Existing tab close/current callbacks and title semantics remain unchanged.
- A source probe, compile/lint/format, parent/independent review status,
  handoff, package identity, and release no-go evidence are recorded. Native
  tab rendering, icon metrics, accessibility, DPI, clean-machine,
  cross-machine, and external release evidence remain unrun under the
  no-launch policy. ADR-0073 records this bounded D48/ARCH-38/UI-34 decision.

## Phase 2 session-save state acceptance

- `presentation.session_save_tracker.SessionSaveTracker` owns only the
  framework-neutral saved baseline, latest queued `SessionSnapshot`, in-flight
  state, positive TaskRunner operation binding, and stale/invalid/valid callback
  classification.
- MainWindow remains the coordinator for the 250 ms Qt debounce, snapshot
  capture, startup suppression, SessionService, TaskRunner, operation-ID
  allocation, notification projection, close pending-work guards, and result
  policy. No persistence schema or save interval changes.
- Matching valid completion advances the baseline; matching invalid/failure
  callbacks release the current state while preserving MainWindow's existing
  error policy and any newer queued snapshot. Stale callbacks cannot clear a
  newer save.
- A source probe, compile/lint/format, parent/independent review status,
  handoff, package identity, and release no-go evidence are recorded. Native
  callback timing, runtime startup, actual filesystem durability,
  clean-machine, cross-machine, and external release evidence remain unrun
  under the no-launch policy. ADR-0074 records this bounded D49/ARCH-39
  decision.

## Phase 2 recovery-capture lifecycle acceptance

- `presentation.recovery_capture_tracker.RecoveryCaptureTracker` owns only
  opaque capture jobs/owners, one active document/snapshot identity binding,
  discarded callback markers, worker-owned snapshot state, and deferred delete
  sequencing. It imports no Qt, editor widget, service, or TaskRunner type.
- MainWindow remains the coordinator for `TextCaptureSession` UI slices,
  `RecoveryChunkChannel` backpressure and abort/finish operations,
  `RecoveryService`, TaskRunner operation IDs/callbacks, tab identity, dirty
  policy, notifications, and close guards. The former write-only channel map is
  removed because the job/worker operation already retains the channel.
- Capture completion removes only the UI producer; document/write state is
  released by the worker callback. Aborted worker-backed captures classify the
  later callback as discarded, and deletes requested during a write/delete are
  handed back after the current lifecycle step.
- A source/integration probe, Qt-free behavior probe, compile/lint/format,
  parent/independent review status, handoff, package identity, and release
  no-go evidence are recorded. Native callback timing, actual recovery I/O,
  hard-power durability, runtime startup, clean-machine, cross-machine, and
  external release evidence remain unrun under the no-launch policy. ADR-0075
  records this bounded D50/ARCH-40 decision.

## Phase 2 settings-save callback acceptance

- `presentation.settings_save_tracker.SettingsSaveTracker` owns only one
  positive TaskRunner operation ID and `stale`/`invalid`/`valid`/failure
  classification. It imports no Qt, SettingsService, dialog, theme, locale,
  font, editor, or notification type.
- MainWindow remains the coordinator for SettingsSurface editing,
  SettingsService, TaskRunner, persisted snapshot assignment, theme/locale/
  font/editor refresh, transition animation, success/error notifications, and
  close pending-work guards.
- A matching invalid result or failure releases the current lifecycle and
  preserves existing error behavior; a stale callback cannot release or apply
  a current save.
- A source/integration probe, Qt-free behavior probe, compile/lint/format,
  parent/independent review status, handoff, package identity, and release
  no-go evidence are recorded. Native settings interaction, runtime theme/font
  rendering, clean-machine, cross-machine, and external release evidence
  remain unrun under the no-launch policy. ADR-0076 records this bounded
  D51/ARCH-41 decision.

## Phase 2 Replace All lifecycle acceptance

- `presentation.replace_all_tracker.ReplaceAllTracker` owns one typed active
  job, expected content-version guard, progress-report state, and identity
  guarded finish. It imports no Qt, editor widget, service, timer, theme,
  locale, status, or notification type.
- Every cooperative Replace All callback carries its job object and checks the
  tracker before stepping the editor session; an old queued callback cannot
  advance or release a newer job.
- MainWindow remains the coordinator for `ReplaceAllSession`, QTimer slicing,
  editor locking, operation busy/status state, cancellation, rollback,
  dirty-marker restoration, tab/find-surface state, and user-facing messages.
- A source/callback-boundary probe, Qt-free behavior probe, compile/lint/format,
  parent/independent review status, handoff, package identity, and release
  no-go evidence are recorded. Native event-loop timing, actual editor
  rollback, clean-machine, cross-machine, and external release evidence
  remain unrun under the no-launch policy. ADR-0077 records this bounded
  D52/ARCH-42 decision.

## Phase 2 Find Match snapshot acceptance

- `presentation.find_match_tracker.FindMatchTracker` owns one immutable
  document/query/case/selection/content-version snapshot and exact-match or
  clear semantics. It imports no Qt, editor widget, service, timer, theme,
  locale, status, or notification type.
- A failed find or missing selection clears the snapshot; a single replacement
  requires all five current identity dimensions before MainWindow's existing
  selected-text equality check can proceed.
- MainWindow remains the coordinator for EditorWidget find/selection and
  replacement, busy gating, criteria/document/editor/tab invalidation, and
  user-facing feedback.
- A source/integration probe, Qt-free behavior probe, compile/lint/format,
  parent/independent review status, handoff, package identity, and release
  no-go evidence are recorded. Native selection/focus timing, actual editor
  replacement, clean-machine, cross-machine, and external release evidence
  remain unrun under the no-launch policy. ADR-0078 records this bounded
  D53/ARCH-43 decision.

## Phase 2 Recovery Scan lifecycle acceptance

- `presentation.recovery_scan_tracker.RecoveryScanTracker` owns one immutable
  operation/startup-context job and identity-guarded finish. It imports no Qt,
  RecoveryService, TaskRunner, candidate, session, notification, or close
  policy type.
- Both TaskRunner callback routes carry the same typed job; stale callbacks
  cannot clear or project a newer scan. Invalid candidate results release the
  current job once and use MainWindow's existing recovery failure projection.
- MainWindow remains the coordinator for RecoveryService, TaskRunner,
  candidate validation, recovery prompts, startup/session restoration,
  notifications, and close guards; the scan remains outside generic busy
  OperationTracker policy as before.
- A source/integration probe, Qt-free behavior probe, compile/lint/format,
  parent/independent review status, handoff, package identity, and release
  no-go evidence are recorded. Native callback ordering, actual recovery I/O,
  startup interaction, clean-machine, cross-machine, and external release
  evidence remain unrun under the no-launch policy. ADR-0079 records this
  bounded D54/ARCH-44 decision.

## Phase 2 warning-background foreground acceptance

- `presentation.theme._stylesheet()` derives one local warning foreground from
  the actual `warning_bg` token using the existing pure contrast helper. The
  warning message, attention phase, workspace/search feedback, warning action,
  Find status, and workspace cancel hover/focus selectors consume that value;
  gold borders and the filled warning-action hover keep their existing
  endpoint tokens.
- The correction remains presentation-only and does not expand `ThemeColors`,
  settings, locale, motion, signals, commands, application policy, or any
  lower layer. A 12-pair all-theme/accent source probe, selector-boundary
  probe, compile/lint/format, parent/independent review status, handoff,
  package identity, and release no-go evidence are recorded. Runtime QSS
  specificity, fonts, DPI, accessibility, clean-machine, cross-machine, and
  external release evidence remain unrun. ADR-0080 records this bounded
  D55/UI-35 decision.

## Phase 2 document-tab path identity acceptance

- `presentation.document_tab_surface.DocumentTabSurface.find_by_path()` owns
  canonical full-registry lookup through `domain.path_identity.path_key()`;
  pathless records return no match and object-identity exclusion remains
  explicit. MainWindow delegates the ordinary lookup.
- At D56, MainWindow retained the `_session_restore_tabs` active-target
  selection because it was startup policy over a subset, not generic tab
  projection. D57 subsequently moves only that restore-output collection and
  selection input into the Qt-free restore tracker; no path index cache,
  file-opening behavior, or document service dependency is added.
- A source/AST boundary probe, compile/lint/format, parent/independent review
  status, handoff, package identity, and release no-go evidence are recorded.
  Native tab event ordering, filesystem case behavior, runtime startup,
  clean-machine, cross-machine, and external release evidence remain unrun.
  ADR-0081 records this bounded D56/ARCH-45 decision.

## Phase 2 session-restore tab projection acceptance

- `presentation.session_restore_tracker.SessionRestoreTracker[TabT]` owns the
  ordered opaque references produced by serial restore handling and selects the
  canonical active-path match or first recorded tab through a `path_of` callback.
  `begin()` and `finish()` clear projection references; no Qt/editor/service/
  TaskRunner dependency enters the tracker.
- MainWindow records the unchanged existing-tab and newly-opened-tab outcomes
  and delegates only final restored-tab target selection. It retains workspace,
  session, recovery, document, and TaskRunner policy, tab projection,
  notifications, startup/initial-document/close guards, and result handling.
- Source boundary probes, compile/lint/format, parent/independent review
  status, handoff, package identity, and release no-go evidence are recorded.
  Native queued callback timing, runtime startup, clean-machine, cross-machine,
  and external release evidence remain unrun. ADR-0082 records this bounded
  D57/ARCH-46 decision.

## Phase 2 session-load state simplification acceptance

- MainWindow no longer stores `_session_load_state`, because the field had no
  consumers. The existing `SessionLoadResult.state` remains the direct input to
  invalid notification classification; malformed and failed loads still use
  `DEFAULT_SESSION` for restore and save baseline.
- Recovery scanning remains scheduled with `startup=True` after every load
  outcome, and startup/close/session-restore policy remains in MainWindow. No
  replacement tracker, SessionService contract, or result schema is introduced.
- A source reachability/semantic-retention probe, compile/lint/format,
  parent/independent review status, handoff, package identity, and release
  no-go evidence are recorded. Native callback timing, runtime startup,
  clean-machine, cross-machine, and external release evidence remain unrun.
  ADR-0083 records this bounded D58/ARCH-47 decision.

## Phase 2 status-phase forwarding simplification acceptance

- `MainWindow._sync_active_document_phase()` was a pure forwarding alias with
  no state or behavior. Its existing editor dirty, workspace completion,
  current-tab, runner, and operation callers now invoke
  `_sync_status_surface()` directly.
- The unified method still gives working state precedence to active/pending
  operations, attention to a dirty active document, and ready to a clean idle
  shell. StatusSurface, notifications, QSS, and application policy remain
  unchanged.
- A source caller-retention probe, compile/lint/format, parent/independent
  review status, handoff, package identity, and release no-go evidence are
  recorded. Native event timing, runtime startup, clean-machine,
  cross-machine, and external release evidence remain unrun. ADR-0084 records
  this bounded D59/ARCH-48 decision.

## Phase 2 document-tab selection hierarchy acceptance

- The centralized theme stylesheet gives selected tabs a selection surface,
  accent border, alternate-accent leading indicator, and readable selected-hover
  foreground; the tab rail and dock title receive existing border/alternate-
  accent boundaries. All values reuse `ThemeColors`.
- The refinement is presentation-only: DocumentTabSurface behavior, tab
  signals, close/current policy, settings, locale, motion, editor colors,
  warning/gold foregrounds, and application policy remain unchanged.
- Selector/token probes and 12-pair static contrast, compile/lint/format,
  parent/independent review status, handoff, package identity, and release
  no-go evidence are recorded. Native QSS rendering, accessibility, DPI,
  fonts, runtime startup, clean-machine, cross-machine, and external release
  evidence remain unrun. ADR-0085 records this bounded D60/UI-36 decision.

## Phase 2 session-snapshot capture boundary acceptance

- `presentation.session_snapshot_builder.build_session_snapshot[TabT]` now
  owns only clean path-backed tab filtering, ordered `SessionDocument` /
  `SessionSnapshot` assembly, per-tab cursor validation, and active-index
  derivation. The boundary is Qt-free and receives opaque tabs through typed
  callbacks.
- MainWindow remains responsible for EditorWidget reads, active-tab selection,
  session-save debounce, SessionSaveTracker, SessionService, TaskRunner,
  startup/restore sequencing, notifications, and close policy. No document
  text, service, timer, or application policy crosses into the builder.
- Boundary and pure behavior probes, compile/lint/format, parent/independent
  review status, handoff, package identity, and release no-go evidence are
  recorded. Native Qt callback timing, runtime startup, clean-machine,
  cross-machine, accessibility, and external release evidence remain unrun.
ADR-0086 records this bounded D61/ARCH-49 decision.

## Phase 2 find-bar action hierarchy acceptance

- FindBar now exposes only presentation object names for previous/next,
  cancel, and close controls. Centralized theme QSS distinguishes query and
  replacement inputs, navigation, cancel, close, primary find, and Replace
  All using existing `ThemeColors` endpoints.
- Find/replace signals, callbacks, locale refresh, operation state,
  warning/gold foreground derivation, editor behavior, and MainWindow policy
  remain unchanged. No new token, action model, or layout state is introduced.
- Selector/semantic-role probes, 3-theme/4-accent static contrast, compile/
  lint/format, parent/independent review status, handoff, package identity,
  and release no-go evidence are recorded. Native QSS specificity/rendering,
  runtime startup, accessibility, DPI, fonts, clean-machine, cross-machine,
  and external release evidence remain unrun. ADR-0087 records this bounded
  D62/UI-37 decision.

## Phase 2 toolbar context chip acceptance

- The existing `QLabel#toolbarContext` now projects the localized local/safe
  context as a compact chip using existing surface, border, alternate-accent,
  and secondary-text tokens.
- CommandSurface, i18n, toolbar layout, command callbacks, shortcuts, command
  refresh, locale behavior, and application policy remain unchanged. No new
  widget, state, or theme token is introduced.
- Source/scope and 3-theme/4-accent contrast probes, compile/lint/format,
  parent/independent review status, handoff, package identity, and release
  no-go evidence are recorded. Native QSS rendering, runtime startup,
  accessibility, DPI, fonts, clean-machine, cross-machine, and external
  release evidence remain unrun. ADR-0088 records this bounded D63/UI-38
  decision.

## Phase 2 settings control hierarchy acceptance

- The existing appearance/editor groups now have presentation-only identities;
  centralized QSS provides alternate-accent/pink rails for the groups and
  theme/accent/UI-font/UI-size/editor-font/editor-size control families.
- Group-title text uses `text_primary` after a Paper/Sand contrast probe found
  direct accent text below threshold. SettingsSnapshot, language switching,
  control ranges/values, signals, SettingsSurface, SettingsService/TaskRunner,
  persistence, and MainWindow policy remain unchanged.
- Source/scope and 3-theme/4-accent/4-state contrast probes, compile/
  lint/format, parent/independent review status, handoff, package identity,
  and release no-go evidence are recorded. Native QSS rendering, runtime
  settings interaction, accessibility, DPI, fonts, clean-machine,
  cross-machine, and external release evidence remain unrun. ADR-0089 records
  this bounded D64/UI-39 decision.

## Phase 2 document-tab forwarding simplification acceptance

- MainWindow no longer contains `_active_tab`, `_find_tab`,
  `_find_tab_by_path`, or `_contains_tab` forwarding methods. Existing callers
  use `DocumentTabSurface.active_tab`, `find_by_editor`, `find_by_path`, and
  `contains` directly.
- DocumentTabSurface remains the registry/path-identity owner; path exclusion,
  session snapshot active-tab capture, save/open duplicate guards, recovery,
  Replace All, Find, close, and status policy remain unchanged at their
  existing owners.
- Source/caller-retention probe, compile/lint/format, parent/independent review
  status, handoff, package identity, and release no-go evidence are recorded.
  Native callback ordering, runtime startup, clean-machine, cross-machine, and
  external release evidence remain unrun. ADR-0090 records this bounded
  D65/ARCH-50 decision.

## Phase 2 dialog action hierarchy acceptance

- The plugin catalog and plugin status dialogs now mark approve/enable as
  `primaryAction` and revoke/disable as `warningAction`; workspace search and
  settings mark close/cancel as `quietAction` and share the existing
  `QDialogButtonBox#dialogActions` presentation rail.
- The change is presentation-only. Dialog signals, button ordering,
  enablement predicates, locale updates, settings persistence, plugin
  governance/runtime policy, workspace-search policy, and MainWindow ownership
  remain unchanged.
- Source-role and three-theme/actual-state contrast probes, compile/lint/
  format, parent/independent review status, handoff, package identity, and
  release no-go evidence are recorded. Native QSS rendering, keyboard
  traversal, accessibility, DPI, fonts, clean-machine, cross-machine, and
  external release evidence remain unrun. ADR-0091 records this bounded
  D66/UI-40 decision.

## Phase 2 MainWindow forwarding simplification acceptance

- MainWindow no longer contains `_choose_save_path` or `_show_about` pure
  forwarding methods. Save As and dirty-tab close call the existing
  `FileDialogSurface.choose_save_path` contract directly, and `help.about`
  binds directly to `MessageSurface.show_about`.
- FileDialogSurface and MessageSurface remain presentation owners; MainWindow
  retains Save As, dirty-close, command registration, document, recovery,
  locale, and application policy. Command ID/title/menu and path/default-name
  semantics remain unchanged.
- Source/caller-retention probe, compile/lint/format, parent/independent review
  status, handoff, package identity, and release no-go evidence are recorded.
  Native callback ordering, runtime startup, clean-machine, cross-machine,
  accessibility, and external release evidence remain unrun. ADR-0092 records
  this bounded D67/ARCH-51 decision.

## Phase 2 message-dialog visual hierarchy acceptance

- MessageSurface now uses equivalent instance-based QMessageBox projections for
  unsaved-close, About, and error flows, preserving icons, localized
  title/text, Save/Discard/Cancel order, default Save, modal `exec`, and
  return mapping. RecoveryPromptSurface now exposes `recoveryPrompt` and
  primary/warning/quiet action roles for restore/discard/later.
- Centralized QSS provides the QMessageBox surface, edge, label, and button
  hierarchy. MainWindow, MessageSurface decision semantics,
  RecoveryPromptSurface decision mapping, i18n, services, persistence, and
  application policy remain unchanged.
- Source-equivalence/role probe, compile/lint/format, parent/independent
  review status, handoff, package identity, and release no-go evidence are
  recorded. Native QMessageBox rendering, keyboard traversal, accessibility,
  DPI, fonts, clean-machine, cross-machine, and external release evidence
  remain unrun. ADR-0093 records this bounded D68/UI-41 decision.

## Phase 2 notification localization closure acceptance

- The presentation localization boundary now covers the remaining known
  duplicate-open, plugin-failure, invalid-workspace, extension-catalog, and
  plugin-host notification shapes. `en-US` remains source-identical and
  dynamic paths, identifiers, PIDs, limits, phase names, and raw diagnostic
  details remain visible.
- Application summary methods remain locale-free. `PluginCatalogDialog` owns
  the Qt summary label and re-projects its immutable source on locale changes;
  no application service, plugin policy, signal, or MainWindow boundary moves.
- Source localization probe, compile/lint/format, parent/independent review
  status, handoff, package identity, and release no-go evidence are recorded.
  Native dialog rendering, live language switching, accessibility, DPI, fonts,
  clean-machine, cross-machine, and external release evidence remain unrun.
  ADR-0094 records this bounded D69/UI-42 decision.

## Phase 2 plugin catalog locale projection acceptance

- Plugin Catalog row, tooltip, and empty-state projections now translate stable
  field/enum/known-reason vocabulary through the centralized i18n owner for
  `zh-CN` and `en-US`. Unknown future enum values and all dynamic names,
  paths, IDs, versions, API values, permissions, entrypoint metadata, hashes,
  and free-form errors remain raw.
- `PluginCatalogDialog.set_locale()` re-renders existing in-memory items only;
  item data, selection, governance signals, enablement predicates, catalog
  entries, and plugin trust/execution policy remain unchanged. No application
  summary or service contract gains a locale dependency.
- Source locale probe, compile/lint/format, parent/independent review status,
  handoff, package identity, and release no-go evidence are recorded. Native
  list rendering, live Qt refresh, accessibility, DPI, fonts, clean-machine,
  cross-machine, and external release evidence remain unrun. ADR-0095 records
  this bounded D70/UI-43 decision.

## Phase 2 plugin status boolean locale projection acceptance

- Plugin Status tooltips now project enabled/active booleans through the
  centralized locale catalog: `true/false` for `en-US` and `是/否` for
  `zh-CN`. IDs, versions, permissions, errors, lifecycle labels, and trust
  values remain unchanged.
- The immutable `PluginRuntimeStatus` application contract, row selection,
  signal routing, enable/disable predicates, lifecycle state, and plugin
  policy remain unchanged. No application service gains a locale dependency.
- Source locale probe, compile/lint/format, parent/independent review status,
  handoff, package identity, and release no-go evidence are recorded. Native
  tooltip rendering, live Qt refresh, accessibility, DPI, fonts,
  clean-machine, cross-machine, and external release evidence remain unrun.
  ADR-0096 records this bounded D71/UI-44 decision.

## Phase 2 plugin-dialog visual hierarchy acceptance

- Centralized `theme.py` now gives `pluginCatalogDialog` and
  `pluginStatusDialog` an accent-topped boundary, readable summary card,
  distinct list panel, focus cue, and selected-row edge/weight using existing
  theme tokens. Selected/disabled states remain distinguishable without
  making accent color the only signal.
- No dialog layout, object tree, signal, data, locale, governance, trust,
  execution, application, or MainWindow policy changes are made. Text uses
  `text_primary` on summary/selected surfaces; accents are edges/borders.
- Source selector/contrast probe, compile/lint/format, parent/independent
  review status, handoff, package identity, and release no-go evidence are
  recorded. Native QSS rendering, DPI, fonts, accessibility, clean-machine,
  cross-machine, and external release evidence remain unrun. ADR-0097 records
  this bounded D72/UI-45 decision.

### D73 / UI-46 — settings appearance preview

The Settings dialog now contains a presentation-only live preview for pending
theme, accent, interface font, and interface-size choices. `theme_colors()` and
`preview_stylesheet()` remain the canonical presentation token boundary;
`SettingsDialog` owns only preview object composition, locale projection, and
control-change refresh. The preview never applies or persists a candidate
early, so Save/Cancel, SettingsService, MainWindow application timing, editor
projection, and motion policy remain unchanged. Static contrast/selector/font
projection, compile/lint/format, package, handoff, and expected release NO-GO
evidence are recorded in ADR-0098 and the D73 handoff; native rendering,
installed-font fallback, runtime, and external release evidence remain open.

### D74 / UI-47 — settings preview surface boundary

The D73 preview object tree is isolated in `SettingsPreviewSurface` with one
presentation-only `project(...)` contract. `SettingsDialog` remains the source
of pending control values and owns snapshot construction, Save/Cancel, locale
flow, and application policy; the surface owns only its Qt object tree,
localized preview copy, and token-bound rendering. The boundary probe,
compile/lint/format, package, handoff, and expected release NO-GO evidence are
recorded in ADR-0099 and the D74 handoff. Native Qt ownership/rendering,
accessibility, runtime, and external release evidence remain open.

### D75 / UI-48 — workspace-search visual hierarchy

The workspace search surface now projects a scoped root/query/results/
diagnostics hierarchy through centralized `theme.py` QSS. The existing
diagnostic toggle receives only `workspaceSearchDiagnosticsToggle` as a
presentation identity; search service, result/diagnostic contracts, TaskRunner,
cancellation, locale, file activation, and application policy remain unchanged.
The all-theme/all-accent selector/contrast probe, compile/lint/format, package,
handoff, and expected release NO-GO evidence are recorded in ADR-0100 and the
D75 handoff. Native Qt rendering and external release evidence remain open.

### D76 / UI-49 — recovery notification dynamic localization

The existing presentation `localize_message()` boundary now recognizes the
bounded recovery-success shape `Recovered <document>; content remains unsaved`.
For `zh-CN` it translates only the stable suffix to `；内容仍未保存`, preserving
the dynamic document name verbatim; for `en-US` it returns the original message.
Recovery/document services, persistence, MainWindow policy, notification
severity, and diagnostic details remain unchanged. The targeted locale probe,
compile/lint/format, package, handoff, and expected release NO-GO evidence are
recorded in ADR-0101 and the D76 handoff. Native language switching and
external release evidence remain open.

### D77 / ARCH-52 — plugin catalog coordinator boundary

The catalog scan and descriptor governance callbacks previously concentrated in
MainWindow now live in `presentation/plugin_catalog_coordinator.py`. The new
coordinator is Qt-free and accepts narrow Protocols for task submission,
catalog projection, and typed notification; it reuses the existing operation
tracker and preserves stale completion guards, invalid-result handling,
governance enablement, and rescan behavior. MainWindow retains the shared
plugin operation tracker and close-event gates for catalog scan, catalog
governance, and host probe, as well as runtime/plugin security policy and
locale/theme projection. The Qt-free dependency probe, source boundary probe,
compile/lint/format, package, handoff, and expected release NO-GO evidence are
recorded in ADR-0102 and the D77 handoff. Native callback timing and external
release evidence remain open.

### D78 / ARCH-53 — plugin-host probe coordinator boundary

The diagnostic host probe callbacks previously concentrated in `MainWindow`
now live in `presentation/plugin_host_coordinator.py`. The coordinator is
Qt-free, reuses shared task-submitter and notification contracts, preserves
the `host-probe` operation identity and stale guard, retains the exact
unavailable/in-flight/start/invalid/result/failure notification semantics,
and maps the existing typed result states without changing the host protocol.
`MainWindow` retains the shared plugin operation tracker, close-event gates,
composition order, notification sink, and the application/infrastructure
host execution/security boundary. The Qt-free import probe, source boundary
probe, compile/lint/format, package, handoff, and expected release NO-GO
evidence are recorded in ADR-0103 and the D78 handoff. Native callback timing
and external release evidence remain open.

### D79 / ARCH-54 — plugin runtime coordinator boundary

The registered-plugin failure event, status projection, and enable/disable
callbacks previously concentrated in `MainWindow` now live in
`presentation/plugin_runtime_coordinator.py`. The coordinator is Qt-free and
accepts the application-owned `PluginRuntime` protocol, a typed runtime view,
busy predicate, command-refresh callback, and notification sink. It preserves
the existing failure/refresh, unavailable, busy-warning, exception, success,
and status-projection behavior. MainWindow retains composition, busy/close
policy, notification ownership, and the shared plugin operation gates;
`PluginRuntime` remains the runtime trust/enablement/security boundary. The
Qt-free import probe, source boundary probe, compile/lint/format, package,
handoff, and expected release NO-GO evidence are recorded in ADR-0104 and the
D79 handoff. Native event timing and external release evidence remain open.

### D80 / ARCH-55 — session-load coordinator boundary

The session-load completion callbacks previously concentrated in `MainWindow`
now live in `presentation/session_load_coordinator.py`. The coordinator is
Qt-free and accepts typed baseline setters, a recovery-scan continuation
callback, and a notification sink. It preserves absent/valid/invalid and
non-typed-result classification, the `DEFAULT_SESSION` fallback, fixed error
notification, baseline update ordering, and recovery-first continuation.
MainWindow retains `SessionService`, `TaskRunner`, startup barrier,
`SessionRestoreTracker`/`SessionSaveTracker` composition, workspace/tab
restoration, notifications outside this result boundary, and close policy.
The Qt-free import probe, source boundary probe, compile/lint/format, package,
handoff, and expected release NO-GO evidence are recorded in ADR-0105 and the
D80 handoff. Native callback timing and external release evidence remain open.

### D81 / ARCH-56 — recovery-scan coordinator boundary

The recovery inventory result callbacks previously concentrated in
`MainWindow` now live in `presentation/recovery_scan_coordinator.py`. The
coordinator is Qt-free and accepts the existing `RecoveryScanTracker`, typed
candidate prompt callback, session snapshot getter, startup continuation, and
notification sink. It preserves stale identity release, tuple/candidate
validation, manual empty-result info, invalid-inventory and worker-failure
errors, candidate dispatch, and startup continuation. MainWindow retains
RecoveryService, TaskRunner dispatch, RecoveryPromptSurface, restore/discard/
later decisions, session/workspace/tab policy, and close behavior. The
Qt-free import probe, source boundary probe, compile/lint/format, package,
handoff, and expected release NO-GO evidence are recorded in ADR-0106 and the
D81 handoff. Native callback timing and external release evidence remain open.

### D82 / ARCH-57 — session-save coordinator boundary

The session-save completion callbacks previously concentrated in `MainWindow`
now live in `presentation/session_save_coordinator.py`. The coordinator
consumes the existing `SessionSaveTracker`, classifies stale/invalid/valid and
failure callbacks, projects the existing error messages, and asks an injected
drain callback to submit the latest queued snapshot. It imports no Qt.

`MainWindow` retains `SessionService`, snapshot capture, debounce timer,
`TaskRunner`, operation IDs, startup barriers, notification ownership, and
close policy. No session format, store, persistence service, or application
contract moved. The source boundary, Qt-free import, compile/lint/format,
package, handoff, and expected release NO-GO evidence are recorded in
ADR-0107 and the D82 handoff. Native callback timing, durability, and external
release evidence remain open.

### D83 / ARCH-58 — settings-save coordinator boundary

The settings-save completion callbacks previously concentrated in `MainWindow`
now route through `presentation/settings_save_coordinator.py`. The coordinator
consumes the existing `SettingsSaveTracker`, ignores stale callbacks,
classifies invalid/valid results, and projects matching worker failures through
three explicit policy callbacks. It imports no Qt.

`MainWindow` retains `SettingsService`, candidate editing, TaskRunner,
`_settings` mutation, QApplication theme application, locale retranslation,
editor/font refresh, transition timing, notifications, and close policy. No
settings schema, store, dialog, or application contract moved. The source
boundary, Qt-free import, compile/lint/format, package, handoff, and expected
release NO-GO evidence are recorded in ADR-0108 and the D83 handoff. Native
settings interaction and external release evidence remain open.

### D84 / ARCH-59 — workspace-search coordinator boundary

Workspace-search completion callbacks previously concentrated in `MainWindow`
now route through `presentation/workspace_search_coordinator.py`. The
coordinator consumes `WorkspaceSearchOperationTracker`, classifies stale and
invalidated generations, validates `WorkspaceSearchResult`, projects the
minimal search surface, and emits the existing summary severity/failure
notifications through explicit seams. It imports no Qt.

`MainWindow` retains query construction, `WorkspaceSearchService`, TaskRunner,
cooperative cancellation, root containment, result activation, surface
construction, and close policy. No provider, search policy, filesystem
traversal, dialog, or application contract moved. The source boundary,
Qt-free import, compile/lint/format, package, handoff, and expected release
NO-GO evidence are recorded in ADR-0109 and the D84 handoff. Native search
interaction and external release evidence remain open.

### D85 / ARCH-60 — workspace-navigation coordinator boundary

Workspace open and directory-navigation completion callbacks previously
concentrated in `MainWindow` now route through
`presentation/workspace_navigation_coordinator.py`. The coordinator consumes
`WorkspaceOperationTracker`, classifies stale and invalidated generations,
releases loading, validates `WorkspaceState`/`WorkspaceDirectory`, and
projects the existing surface/failure/session-restore lifecycle. It imports no
Qt.

`MainWindow` retains `WorkspaceService`, TaskRunner, generic operation/busy
policy, workspace activation, search-root invalidation, directory-root
projection, containment, document opening, session persistence, and close
policy. No filesystem traversal, workspace provider, surface construction, or
application contract moved. The source boundary, Qt-free import,
compile/lint/format, package, handoff, and expected release NO-GO evidence are
recorded in ADR-0110 and the D85 handoff. Native workspace interaction and
external release evidence remain open.

### D86 / ARCH-61 — document-open coordinator boundary

Ordinary and session-restore document-open completion callbacks previously
concentrated in `MainWindow` now route through
`presentation/document_open_coordinator.py`. The coordinator consumes the
existing generic completion guard, session-restore identity/consumption seams,
validates `OpenedDocument`, preserves ordinary/session invalid-result and
failure semantics, and imports no Qt.

`MainWindow` retains `DocumentService`, TaskRunner, duplicate path/tab policy,
editor and tab construction, line/cursor projection, event publication,
success notifications, session persistence, and close policy. No document
store, decoding, path resolution, save callback, recovery policy, or
application contract moved. The source boundary, Qt-free import,
compile/lint/format, package, handoff, and expected release NO-GO evidence are
recorded in ADR-0111 and the D86 handoff. Native editor/session interaction and
external release evidence remain open.

### D87 / ARCH-62 — document-save coordinator boundary

Document-save completion callbacks previously concentrated in `MainWindow` now
route through `presentation/document_save_coordinator.py`. The generic
`DocumentSaveCoordinator[TabT]` consumes the existing completion guard, tab
liveness, read-only projection, and valid-save/error seams; it validates
`DocumentState`, ignores stale callbacks, and imports no Qt.

`MainWindow` retains `DocumentService`, target/duplicate policy, state/text
snapshots, editor language/title projection, recovery cleanup, `DocumentSaved`,
success notifications, session-save scheduling, optional `after` continuation,
and close policy. No document store, conflict, encoding, target-path, recovery,
or application contract moved. The source boundary, Qt-free import,
compile/lint/format, package, handoff, and expected release NO-GO evidence are
recorded in ADR-0112 and the D87 handoff. Native editor/save interaction and
external release evidence remain open.

### D88 / ARCH-63 — recovery-delete coordinator boundary

Recovery snapshot delete callbacks previously concentrated in
`MainWindow._schedule_recovery_delete` now route through
`presentation/recovery_delete_coordinator.py`. The generic
`RecoveryDeleteCoordinator[JobT, OwnerT]` consumes the existing
`RecoveryCaptureTracker`, identity-clearing, pending-resubmission, and
notification seams; it imports no Qt.

`MainWindow` retains delete request admission, operation allocation,
`RecoveryService` dispatch, capture/write lifecycle, restore/discard decisions,
persistence, and close policy. No snapshot store, filesystem, recovery prompt,
or application contract moved. The source boundary, Qt-free import,
compile/lint/format, package, handoff, and expected release NO-GO evidence are
recorded in ADR-0113 and the D88 handoff. Native recovery/delete interaction
and external release evidence remain open.

### D89 / ARCH-64 — Replace All completion coordinator boundary

Replace All completion cleanup previously concentrated in
`MainWindow._finish_replace_all` now routes through
`presentation/replace_all_completion_coordinator.py`. The generic
`ReplaceAllCompletionCoordinator[TabT, SessionT, ProgressT]` consumes the
existing `ReplaceAllTracker`, live-tab/lock, tab-bar/Find, generic operation,
and outcome seams. It imports neither PyQt6 nor the editor implementation.

`MainWindow` retains ReplaceAllSession stepping/cancellation, content-version
guards, rollback, clean-state restoration, limit/cancel/success/error status,
editor/document consequences, and close policy. No editor session, timer,
document, recovery, or application contract moved. The source boundary,
Qt/editor-free import, compile/lint/format, package, handoff, and expected
release NO-GO evidence are recorded in ADR-0114 and the D89 handoff. Native
Replace All interaction and external release evidence remain open.

### D90 / ARCH-65 — recovery-write coordinator boundary

Recovery snapshot writer callbacks previously concentrated in two MainWindow
writer paths now route through `presentation/recovery_write_coordinator.py`.
The generic `RecoveryWriteCoordinator[JobT, OwnerT]` consumes the existing
`RecoveryCaptureTracker`, matching capture abort, document/write release, and
saved/failed policy seams. It imports no Qt or editor implementation.

`MainWindow` retains chunk/channel capture, backpressure, RecoveryService
dispatch, tab liveness, dirty/content-version/snapshot matching, delete
scheduling, notifications, persistence, and close policy. No recovery store,
capture session, channel, or application contract moved. The source boundary,
Qt-free import, compile/lint/format, package, handoff, and expected release
NO-GO evidence are recorded in ADR-0115 and the D90 handoff. Native
capture/write interaction and external release evidence remain open.

### D91 / ARCH-66 — recovery-capture abort coordinator boundary

Recovery capture cancellation and failure cleanup now route through
`presentation/recovery_capture_abort_coordinator.py`. The generic
`RecoveryCaptureAbortCoordinator[JobT, OwnerT]` consumes the existing
`RecoveryCaptureTracker` and explicit owner/job adapters, preserving stale
identity suppression, worker-started discard versus pre-worker release,
channel/session termination, document lifecycle release, and optional live
owner notification without importing Qt or the editor implementation.

`MainWindow` retains editor capture stepping, QTimer scheduling, stale/cancel
decisions, channel creation, RecoveryService dispatch, retry eligibility,
notifications, persistence, and close policy. No recovery store, capture
session, channel implementation, writer callback, or application contract
moved. The source boundary, Qt-free import, compile/lint/format, package,
handoff, and expected release NO-GO evidence are recorded in ADR-0116 and the
D91 handoff. Native capture/session/channel timing and external release
evidence remain open.

### D92 / ARCH-67 — session-restore coordinator boundary

Ordered session restoration now routes its synchronous progression through
`presentation/session_restore_coordinator.py`. The generic
`SessionRestoreCoordinator[TabT]` consumes the existing
`SessionRestoreTracker[TabT]` and explicit seams for startup/workspace guards,
deferred and duplicate paths, pending document binding, async open dispatch,
active/first-tab selection, initial-document fallback, and finish/save order.
It imports no Qt, editor, service, or concrete tab implementation.

`MainWindow` retains workspace and document services, TaskRunner and
`DocumentOpenCoordinator` callbacks, tab/editor projection, startup/close
state, notification wording, persistence, and recovery decisions. No session
service, tracker contract, workspace service, document service, async framework,
or application contract moved. The source boundary, Qt-free import,
compile/lint/format, package, handoff, and expected release NO-GO evidence are
recorded in ADR-0117 and the D92 handoff. Native session/open/workspace timing
and external release evidence remain open.

### D93 / ARCH-68 — recovery-write finish boundary

The existing `RecoveryWriteCoordinator[JobT, OwnerT]` now completes its
write-finish boundary: it directly calls `RecoveryCaptureTracker.finish_write()`
and forwards the returned pending-delete projection through a generic callback.
This preserves the documented D90 success/failure ordering without importing Qt
or moving delete admission into the coordinator.

`MainWindow` retains delete request admission, RecoveryService dispatch, delete
callbacks, tab/content/snapshot/notification/persistence policy, and close
behavior. No RecoveryService, store, capture session, channel, backpressure,
delete implementation, or application contract moved. The source boundary,
Qt-free import, compile/lint/format, package, handoff, and expected release
NO-GO evidence are recorded in ADR-0118 and the D93 handoff. Native
writer/capture/delete timing and external release evidence remain open.

### D94 / ARCH-69 — document-tab removal coordinator boundary

After close policy approves removal, the finalization path now routes through
`presentation/document_tab_removal_coordinator.py`. The generic
`DocumentTabRemovalCoordinator[TabT, CaptureT]` consumes explicit liveness,
recovery cleanup, tab projection, editor teardown, event, session-save, and
empty-tab callbacks while importing no Qt, editor, EventBus, or concrete tab.

`MainWindow` retains busy/startup/index guards, dirty/save/cancel confirmation,
recovery service behavior, close notifications, and close policy. No document
store, editor implementation, event contract, session schema, or application
contract moved. The source boundary, Qt-free import, compile/lint/format,
package, handoff, and expected release NO-GO evidence are recorded in ADR-0119
and the D94 handoff. Native close/save/recovery timing and external release
evidence remain open.

### D95 / ARCH-70 — document-tab creation coordinator boundary

Document-tab assembly now routes through
`presentation/document_tab_creation_coordinator.py`. The generic
`DocumentTabCreationCoordinator[OpenedT, TabT, EditorT]` consumes factories and
semantic projection callbacks for editor creation, tab construction, surface
add/currentChanged timing, title/modified refresh, session-save request, and
status synchronization without importing Qt, editor, or concrete application
types.

`MainWindow` retains document result handling, settings/theme/editor policy,
concrete `_DocumentTab` and recovery snapshot identity construction,
open/recovery event and notification policy, and close behavior. No document
service, recovery service, editor implementation, settings schema, tab-surface
contract, or application contract moved. The source boundary, Qt-free import,
compile/lint/format, package, handoff, and expected release NO-GO evidence are
recorded in ADR-0120 and the D95 handoff. Native editor/currentChanged timing
and external release evidence remain open.

### D101 / ARCH-75 — recovery-write projection coordinator boundary

The valid recovery-writer outcomes now have a bounded boundary at
`presentation/recovery_projection_coordinator.py`. The delivered generic
Qt-free coordinator owns only the post-write policy after D90 has released the
capture/write lifecycle:

- a discarded or non-live saved owner schedules deletion and stops;
- a live saved owner schedules deletion when the snapshot identity is stale,
  clears the owner snapshot when the document is clean, or reports that newer
  edits will be captured next cycle when the content version advanced;
- a non-discarded failure reports an autosave error only for a live owner.

Tab liveness, dirty state, content version, snapshot identity, delete dispatch,
snapshot clearing, and feedback are explicit callbacks. `RecoveryWriteCoordinator`
retains discarded/capture/document/write lifecycle, pending-delete forwarding,
and callback classification; MainWindow retains concrete tab/editor/recovery,
notification, and close policy. The compatibility promise is the exact
existing branch order and no-op behavior; non-goals are changing recovery
durability, payloads, capture limits, deletion policy, or runtime claims.
Acceptance is recorded by the typed source/order probes, Qt-free boundary
probe, compile/lint/format, package identity, ADR-0126, the D101 handoff,
independent review record, simplification assessment, and expected release
NO-GO evidence.

### D102 / ARCH-76 — close-readiness coordinator boundary

The close event now has a bounded policy boundary at
`presentation/close_guard_coordinator.py`. The delivered Qt-free coordinator
owns ordered admission checks, workspace-search cancellation, dirty-tab
protection, background-operation protection, immediate session-save admission,
pending-work recheck, and timer cleanup policy.

The coordinator will return a typed close decision and preserve this exact
precedence: active operation, in-flight workspace search (cancel then block),
dirty tabs, other recovery/settings/plugin work, immediate session-save request
and pending-work recheck, then timer stop and allow. MainWindow will retain Qt
`QCloseEvent` acceptance/ignore behavior, localized/error-dialog projection,
TaskRunner and timer callbacks, and all concrete operation ownership. No close
policy, session schema, persistence, cancellation, or runtime behavior may
change. Acceptance is recorded by the typed decision/order probe, Qt-free
boundary probe, compile/lint/format, package identity, ADR-0127, the D102
handoff, independent review record, simplification assessment, and expected
release NO-GO evidence.

### UI-51 — document-tab rail state clarity

The document rail is the shell's primary work-surface navigator. This bounded
visual slice gives its native tab bar a semantic object identity,
document-mode/truncation hints, and centralized QSS states for normal, hover,
selected, selected-hover, keyboard focus, modified, and close affordances.
`DocumentTabSurface` retains identity/index mapping, close/current signals,
modified icon projection, and document policy; `theme.py` remains the only QSS
token source. No tab interaction, persistence, editor, locale, or close
decision behavior changed. Acceptance is recorded by the selector/source
probe, static theme-token/contrast probe, compile/lint/format, package
identity, ADR-0128, handoff, independent review record, simplification
assessment, and expected release NO-GO evidence.

### D100 / ARCH-74 — settings-save projection coordinator boundary

The valid branch of settings-save completion has a candidate boundary at
`presentation/settings_save_projection_coordinator.py`. The proposed generic
Qt-free coordinator owns only the observable order after a validated
`SettingsSnapshot`: apply the persisted snapshot/theme baseline, retranslate
the shell, apply editor settings to open tabs, animate the optional theme
transition, and publish success feedback. Each concrete operation remains an
explicit callback; the coordinator does not import `QApplication`, QSS, font
objects, settings services, or widgets.

`SettingsSaveCoordinator` retains tracker identity, stale suppression,
`SettingsSnapshot` validation, and invalid/failure projection. MainWindow
retains SettingsService/TaskRunner ownership, QApplication/theme application,
locale and editor surfaces, motion policy, notifications, and close behavior.
The compatibility promise is the existing side-effect order and optional
QApplication no-op; non-goals are changing settings schema, validation,
locale/theme IDs, font sizes, animation policy, or runtime claims. Acceptance
requires a typed source/order probe, Qt-free boundary probe, compile/lint/
format, package identity, one ADR, one handoff, independent review record,
simplification assessment, and expected release NO-GO evidence.

### D99 / ARCH-73 — workspace-navigation projection coordinator boundary

The valid branches of workspace navigation have a candidate boundary at
`presentation/workspace_navigation_projection_coordinator.py`. The proposed
Qt-free coordinator owns only the observable ordering for validated current
results:

- a workspace-open result invalidates the old search generation, activates the
  application workspace, updates the search root and directory surface,
  publishes success feedback, requests session persistence, and releases the
  pending session-restore workspace step;
- a directory result uses the current workspace root to project the directory
  surface.

Every concrete workspace, search-surface, notification, persistence, and
session-restore operation remains an explicit callback. The compatibility
promise is the existing open/directory ordering and no-op behavior when an
optional surface or workspace is absent. `WorkspaceNavigationCoordinator`
retains completion/stale/invalidation/loading/invalid/failure classification;
MainWindow retains admission guards, WorkspaceService calls, generation and
TaskRunner wiring, concrete surfaces, and close policy. Non-goals are changing
workspace containment, directory paging, search cancellation, session schema,
or runtime claims. Acceptance requires a typed source/order probe, Qt-free
boundary probe, compile/lint/format, package identity, one ADR, one handoff,
independent review record, simplification assessment, and expected release
NO-GO evidence.

### D98 / ARCH-72 — document-save projection coordinator boundary

The valid branch of document-save completion now has a candidate boundary at
`presentation/document_save_projection_coordinator.py`. The proposed generic
`DocumentSaveProjectionCoordinator[TabT]` owns only the ordered projection of a
validated live save result: apply the saved `DocumentState` and clean editor
state, refresh language, update the tab title, clear the recovery snapshot,
publish `DocumentSaved`, notify success, request session persistence, and run
the optional continuation. Every side effect is an explicit callback so the
coordinator remains Qt-free and does not acquire editor, tab-surface, event-bus,
filesystem, or service dependencies.

`DocumentSaveCoordinator` retains operation completion/stale suppression, tab
liveness, read-only release, `DocumentState` validation, and invalid/failure
error projection. `MainWindow` retains concrete editor state/language/title and
recovery callbacks, event/notification/persistence wiring, target/path/service
policy, and close behavior. The compatibility promise is exact ordering and
observable save success behavior; the non-goals are changing save semantics,
introducing a second save contract, changing `DocumentService`, or claiming
native/runtime evidence. Acceptance requires a typed source contract probe,
Qt-free import/boundary probe, compile/lint/format, package identity, one ADR,
one handoff, independent review record, simplification assessment, and the
expected release NO-GO record where existing gates remain open.

### D97 / ARCH-71 — document-open projection coordinator boundary

Valid document-open result projection now routes through
`presentation/document_open_projection_coordinator.py`. The generic
`DocumentOpenProjectionCoordinator[TabT]` owns only duplicate-path
classification, valid `OpenedDocument` tab projection, optional line/cursor
positioning, session restored-tab recording, `DocumentOpened`/success
projection, and restore continuation through explicit callbacks.

`DocumentOpenCoordinator` retains stale suppression, result validation,
session-binding consumption, and ordinary/session failure classification.
MainWindow retains DocumentService/TaskRunner composition, concrete editor/tab
callbacks, event/notification/error wiring, persistence, and close policy. No
Qt, editor, tab surface, EventBus, filesystem, or service dependency enters the
new coordinator. The source/contract probes, parent review, independent
no-conclusion record, compile/lint/format, package, handoff, and expected
release NO-GO evidence are recorded in ADR-0122 and the D97 handoff. Native
editor/tab/session timing and external release evidence remain open.

### D96 / UI-50 — shell visual rhythm

The centralized `presentation/theme.py` stylesheet now gives ordinary shell
surfaces a quieter baseline. The command rail uses compact transparent actions
and a restrained context chip; the status bar and rail are quiet when idle;
inactive document tabs are lighter; selected/focus/feedback/warning/error
states remain explicit; common buttons use slightly tighter geometry; and the
primary action uses a flat `accent` surface with the existing contrast-aware
`on_accent` foreground.

No widget object name, signal, shortcut, locale, settings, document,
workspace, persistence, task, or application policy moved. No second theme
engine, widget-local stylesheet, dependency, or state owner was introduced.
The D96 source/contrast/selector boundary, parent review, independent
no-conclusion record, compile/lint/format, package, handoff, and expected
release NO-GO evidence are recorded in ADR-0121 and the D96 handoff. Native
QSS specificity, layout, accessibility, runtime startup, and external release
evidence remain open.

### UI-52 — workspace resource-manager hierarchy

The workspace resource manager now has one bounded presentation refinement: the
native workspace tree keeps its existing semantic signals and keyboard route,
while the panel exposes explicit semantic identities for its path, empty state,
and tree, plus localized empty/loading copy, calmer navigation-control geometry,
and centralized QSS for the panel hierarchy. A directory with no entries no
longer leaves a visually empty tree; a missing workspace shows an explicit
choice state; an in-flight request retains the existing status feedback and
disablement policy. UI-52 acceptance and artifact evidence are recorded in
ADR-0129 and the UI-52 handoff.

The compatibility promise is unchanged `WorkspacePanel` signal behavior:
single-click files still emit `file_requested`, double-click folders still emit
`directory_requested`, and Enter/Return uses the same item-intent path. The
non-goals are changing WorkspaceService, directory paging, containment,
async-operation ownership, error classification, session restore, or the
`WorkspaceSurface` contract. Acceptance requires a source/state probe, an
all-theme contrast/selector probe, compile/lint/format, package identity, one
ADR, one handoff, independent review record, simplification assessment, and
the expected release NO-GO record while native runtime evidence remains open.

### D103 / ARCH-77 — session-save request/dispatch boundary

The existing Qt-free `SessionSaveCoordinator` now classifies completion and
also owns latest snapshot admission, operation-ID binding, single-flight drain,
and the TaskRunner submission callback without moving Qt or application
services into the coordinator. It receives typed callbacks for save eligibility,
snapshot capture, operation-ID allocation, and concrete save submission; it
owns the request-latest → begin → submit → complete/fail → drain sequence.

MainWindow will retain the QTimer debounce and immediate-save trigger, the
SessionService and TaskRunner instances, concrete snapshot capture, startup
restore guard, notification projection, and close guard. The compatibility
promise is latest-wins/single-flight behavior, stale callback suppression,
invalid-result handling, failure preservation, and queued-request draining in
the same order. Non-goals are changing the session schema/store/service,
operation IDs, timer interval, startup restore, persistence format, or close
policy. Acceptance requires a typed coordinator/source/order probe, a
Qt-free import/dependency probe, compile/lint/format, package identity, one
ADR, one handoff, independent review record, simplification assessment, and
the expected release NO-GO record.

### D104 / UI-53 — editor canvas token hierarchy

The editor currently applies the shell theme's canvas and a small set of
lexer colors, but selection text, gutter contrast, caret emphasis, current-line
highlighting, and syntax roles are not one explicit token contract. D104 adds
those presentation tokens to the existing `presentation.theme` resolver and
projects them through `EditorWidget`/`apply_editor_palette()` only. The
editor adapter keeps QScintilla details local; MainWindow, editor settings,
language selection, document state, persistence, and operation policy do not
change.

Acceptance requires one canonical resolved token set per theme/accent,
readable foregrounds for selection and syntax roles, explicit normal/focus/
selection/disabled visual selectors where QSS participates, and passing static
source, contrast, compile, lint, format, package, and handoff checks. Native
Qt rendering, DPI/font fallback, screenshot evidence, and release-owner gates
remain runtime/operator evidence rather than being inferred from static code.

### D105 / ARCH-78 — recovery-write dispatch callback boundary

The two MainWindow recovery-write paths currently repeat the same success and
failure closure binding before submitting a worker operation. D105 extends the
existing Qt-free `RecoveryWriteCoordinator` with a typed dispatch contract: it
binds owner/content-version/snapshot identity to its existing `complete()` and
`fail()` lifecycle methods, then invokes a caller-supplied generic submitter.
MainWindow still chooses the operation payload (captured tuple or bounded
channel), allocates the operation ID, owns `TaskRunner`, and retains capture,
RecoveryService, notification, persistence, and close policy.

Acceptance requires a typed source/dependency/order probe, preserved
pre-worker exception behavior, compile/lint/format, package identity, one ADR,
one handoff, independent review record, simplification assessment, and the
expected release NO-GO record. No recovery schema, channel, capture cadence,
worker operation, or close behavior may change; native worker timing and
durability remain unrun.

### D106 / UI-54 — scrollbar chrome

The centralized QSS currently styles the vertical scrollbar but includes the
horizontal bar itself in the selector group intended only for add/sub/page
subcontrols. D106 gives both orientations the same compact track, handle,
hover, and hidden-button states, while removing the accidental `height: 0`
application from `QScrollBar:horizontal`. Scroll mode, wrapping, editor
settings, signals, and QScintilla ownership remain unchanged.

Acceptance requires a selector/source probe that proves horizontal bar
visibility and orientation symmetry, compile/lint/format, package identity, one
ADR, one handoff, independent review record, simplification assessment, and
the expected release NO-GO record. Native style-engine rendering and DPI
metrics remain runtime evidence.

### D107 / ARCH-79 — recovery-delete dispatch callback boundary

The existing Qt-free `RecoveryDeleteCoordinator` already owns delete
completion classification, owner snapshot clearing, success/error feedback,
and pending-delete draining, but MainWindow still repeats the callback
closures that bind those values to `TaskRunner.submit()`. D107 adds one typed
generic dispatch method to that existing coordinator. It accepts the opaque
delete operation, operation ID, snapshot identity, optional owner and success
message, and a structural dispatcher; it binds callbacks and delegates
submission without importing Qt, TaskRunner, RecoveryService, or widgets.

MainWindow retains delete admission through `RecoveryCaptureTracker`, the
RecoveryService operation, operation-ID allocation, TaskRunner ownership,
pending-delete scheduling, notification sink, persistence, and close policy.
The compatibility promise is unchanged callback ordering, synchronous
dispatcher-exception propagation, tracker release, owner identity clearing,
success/error notification, and queued-delete drain. No recovery schema,
filesystem, capture cadence, channel, or close behavior changes.

Acceptance requires a typed source/dependency/order probe, inline success and
failure dispatch evidence, compile/lint/format, package identity, one ADR, one
handoff, independent review record, simplification assessment, and the
expected release NO-GO record. Native worker timing, filesystem durability,
restart recovery, and release-owner evidence remain runtime/operator gates.

### D108 / ARCH-80 — cross-module contract/error/observability audit

The existing `scripts/check.ps1` checks lower-layer import direction, but the
presentation contract also depends on three cross-module invariants that are
otherwise easy to regress: Qt-free coordinators must not acquire Qt shell or
concrete worker-owner dependencies; MainWindow notification calls must carry
an explicit `StatusMessageLevel`; and TaskRunner pending work must remain
observable through its signal, count/boolean API, MainWindow connection, and
WORKING phase projection. D108 adds one small AST-based static gate for those
existing contracts and runs it from `scripts/check.ps1`.

The gate is intentionally source-only. It does not attempt to infer runtime
error correctness, Qt event ordering, visual output, performance, or
observability completeness from names. No application behavior, public
contract, coordinator state, notification text, TaskRunner scheduling, or
close policy changes.

Acceptance requires the audit source probe, a deliberate negative-case probe
against in-memory AST fixtures (without creating test assets), compile/lint/
format, package identity, one ADR, one handoff, independent review record,
simplification assessment, and the expected release NO-GO record. Runtime
worker timing, startup, visual, clean-machine, and release-owner evidence
remain open.

### D109 / ARCH-81 — recovery-scan dispatch callback boundary

`RecoveryScanCoordinator` already owns recovery inventory result validation,
tracker finish, candidate prompting, failure notification, and startup
continuation, but MainWindow still creates success/failure closures solely to
bind one `RecoveryScanJob`. D109 adds typed scan operation/success/failure/
dispatcher contracts and a keyword-only `submit(...)` method to that existing
coordinator. It binds the job to the established `complete()`/`fail()` paths
and delegates to a structural dispatcher.

MainWindow retains RecoveryService, operation-ID allocation, scan admission,
TaskRunner, startup/manual-scan distinction, notification surface, session
restore, and close policy. Callback order, stale tracker suppression,
invalid-inventory handling, manual no-result feedback, and startup continuation
must remain unchanged. Synchronous dispatcher exceptions continue to propagate
to the existing caller; no lifecycle state is synthesized on dispatch failure.

Acceptance requires a typed source/dependency/order probe, inline success,
failure, stale, and dispatcher-exception evidence, compile/lint/format,
package identity, one ADR, one handoff, independent review record,
simplification assessment, and the expected release NO-GO record. Native
worker timing, filesystem/recovery durability, startup, and release-owner
evidence remain runtime/operator gates.

### D110 / ARCH-82 — document-save dispatch callback boundary

`DocumentSaveCoordinator` already owns document-save callback classification:
operation liveness, tab presence, editor read-only release, `DocumentState`
validation, invalid/failure projection, and post-save continuation. MainWindow
still repeats closures that bind the tab and optional `after` continuation to
the coordinator before submitting `DocumentService.save_document()`.

D110 adds typed save operation/success/failure/dispatcher contracts and a
keyword-only `submit(...)` method to that existing coordinator. It binds the
opaque tab and continuation callback to `complete()`/`fail()` and invokes a
structural dispatcher. MainWindow retains document state/text/path snapshots,
read-only transition, operation-ID allocation, DocumentService, TaskRunner,
busy/status/notification, persistence, and close policy.

The compatibility promise is unchanged operation completion/stale behavior,
tab-liveness handling, read-only release, invalid-result/error projection, and
post-save continuation order. Synchronous dispatcher exceptions propagate
without synthesizing a completion or changing editor state. Acceptance
requires typed source/dependency/order evidence, inline success/failure/stale
and exception dispatch evidence, compile/lint/format, package identity, one
ADR, one handoff, independent review record, simplification assessment, and
the expected release NO-GO record.

### D111 / ARCH-83 — document-open dispatch callback boundary

`DocumentOpenCoordinator` already owns open-result classification: operation
liveness, ordinary versus session-restore identity, invalid-result and error
projection, restore continuation, document application, and optional line
navigation. MainWindow still repeats the success/failure closures that bind
the requested line number before submitting `DocumentService.open_document()`.

D111 adds typed open operation/success/failure/dispatcher contracts and a
keyword-only `submit(...)` method to that existing coordinator. It binds the
optional line number to `complete()` and the error callback to `fail()` before
invoking a structural dispatcher. MainWindow retains path selection,
operation-ID allocation, session-restore binding, DocumentService, TaskRunner,
busy/status/notification, persistence, and close policy.

The compatibility promise is unchanged operation completion and stale-result
behavior, ordinary/session-restore branching, startup continuation,
invalid-result/error projection, document application, and line navigation.
Synchronous dispatcher exceptions propagate without synthesizing a completion
or changing restore state. Acceptance requires typed source/dependency/order
evidence, inline ordinary/session-restore success/failure/stale and exception
dispatch evidence, compile/lint/format, package identity, one ADR, one
handoff, independent review record, simplification assessment, and the
expected release NO-GO record.

### D112 / ARCH-84 — settings-save dispatch callback boundary

`SettingsSaveCoordinator` already owns settings-save callback classification:
tracker completion/stale suppression, `SettingsSnapshot` validation, invalid
result projection, and worker-failure projection. MainWindow still binds the
settings service operation directly to `TaskRunner.submit()` and passes the
coordinator callbacks at the composition site.

D112 adds typed settings-save operation/success/failure/dispatcher contracts
and a keyword-only `submit(...)` method to that existing coordinator. It
invokes the structural dispatcher with the existing coordinator completion
and failure paths. MainWindow retains settings dialog editing, operation-ID
allocation, tracker admission, SettingsService, TaskRunner, theme/font/locale/
editor/motion projection, notifications, persistence, and close policy.

The compatibility promise is unchanged tracker admission, stale suppression,
invalid-result/failure projection, settings projection order, synchronous
dispatcher exception propagation, and close readiness. Acceptance requires
typed source/dependency/order evidence, inline valid/invalid/failure/stale and
exception dispatch evidence, compile/lint/format, package identity, one ADR,
one handoff, independent review record, simplification assessment, and the
expected release NO-GO record.

### D113 / ARCH-85 — session-load dispatch callback boundary

`SessionLoadCoordinator` already owns session-load result classification:
`SessionLoadResult` validation, default-baseline projection, invalid-manifest
error reporting, failure projection, and recovery-first continuation. MainWindow
still submits `SessionService.load` directly to `TaskRunner` with the
coordinator callbacks.

D113 adds typed session-load operation/success/failure/dispatcher contracts
and a keyword-only `submit(...)` method to that existing coordinator. It binds
the existing `complete()`/`fail()` callbacks to a structural dispatcher.
MainWindow retains startup admission, operation-ID allocation, SessionService,
TaskRunner, session baseline/restore state, notifications, recovery ordering,
persistence, and close policy.

The compatibility promise is unchanged result validation, default baseline,
invalid-manifest preservation, failure notification, recovery-first scan
continuation, and synchronous dispatcher exception propagation. Acceptance
requires typed source/dependency/order evidence, inline valid/invalid/failure
and exception dispatch evidence, compile/lint/format, package identity, one
ADR, one handoff, independent review record, simplification assessment, and
the expected release NO-GO record.

### UI-55 — settings field hierarchy

The settings dialog already exposes language, theme, accent, interface font
and size, editor font and size, wrapping, line numbers, and motion controls,
plus the pending-appearance preview. UI-55 gives those controls stable
presentation identities and adds scoped theme QSS for field labels, compact
option rows, and the language/appearance/editor accent groups.

The slice is presentation-only. It does not change signal wiring, current
values, `SettingsSnapshot` assembly, locale updates, preview projection,
Save/Cancel behavior, SettingsService, theme application, editor settings,
motion policy, or MainWindow ownership. Acceptance requires source identity
and selector coverage, a static color/token probe, compile/lint/format,
package identity, one ADR, one handoff, parent review, simplification
assessment, and the expected release NO-GO record.

### D114 / ARCH-86 — workspace-navigation dispatch callback boundary

`WorkspaceNavigationCoordinator` already owns workspace-open and directory
callback classification: operation completion, generation/stale and
invalidated handling, loading/error projection, typed result validation, and
session-restore continuation. MainWindow still repeats TaskRunner callback
closures for `WorkspaceService.open_workspace()` and `list_directory()`.

D114 adds typed workspace-navigation operation/success/failure/dispatcher
contracts and keyword-only `submit_open(...)` and `submit_directory(...)`
methods to that existing coordinator. Each binds the generation to the
existing open/directory completion and failure paths. MainWindow retains
admission/busy state, WorkspaceService, operation IDs, generation allocation,
TaskRunner, surface ownership, containment, session barriers, persistence,
notifications, and close policy.

The compatibility promise is unchanged generation invalidation, stale
suppression, loading release, invalid-result/error projection, session-restore
finish behavior, and synchronous dispatcher exception propagation. Acceptance
requires typed source/dependency/order evidence, inline open/directory
success/failure/stale/invalidated and exception dispatch evidence,
compile/lint/format, package identity, one ADR, one handoff, independent
review record, simplification assessment, and the expected release NO-GO
record.

### UI-56 / ARCH-87 — command-rail visual role hierarchy

The command rail currently gives most toolbar actions one undifferentiated
visual treatment. UI-56 adds a bounded presentation-only role contract to
ToolbarActionSpec and assigns explicit standard, primary, quiet, and workspace
context roles at the MainWindow composition site. CommandSurface projects the
role to stable tool-button properties, and centralized theme QSS gives each
role a readable hierarchy using canonical tokens.

The compatibility promise is unchanged callbacks, shortcuts, menus, locale
retranslation, icon retinting, toolbar layout, command registry, TaskRunner,
document/workspace policy, and accessibility names/tooltips. Acceptance
requires typed source/dependency evidence, role coverage and token/readability
probe, compile/lint/format, package identity, one ADR, one handoff,
independent review record, simplification assessment, and the expected
release NO-GO record.

### UI-57 / ARCH-88 — workspace-dock chrome hierarchy

The workspace dock already has a stable WorkspaceDock identity but otherwise
falls back to native dock title and button styling. UI-57 adds only scoped
QSS for the existing dock frame, title, close button, and float button, with
readable hover/pressed/disabled states derived from canonical theme tokens.

The compatibility promise is unchanged dock placement, floating/closing
behavior, panel signals, locale text, workspace loading/error state, file and
directory activation, navigation policy, and MainWindow ownership. Acceptance
requires source/selector coverage, token/readability probe, compile/lint/
format, package identity, one ADR, one handoff, independent review record,
simplification assessment, and the expected release NO-GO record.

### D115 / ARCH-89 — workspace-search dispatch callback boundary

WorkspaceSearchCoordinator already owns search callback classification,
generation invalidation, cancellation projection, result validation, and
summary severity, but MainWindow still assembles the TaskRunner callback
closures. D115 adds typed search operation/success/failure/dispatcher
contracts and a keyword-only submit(...) method to the existing coordinator.

The compatibility promise is unchanged query construction, cooperative
cancellation, operation admission, WorkspaceSearchService, TaskRunner,
surface projection, containment, locale, notification, and close policy.
Acceptance requires typed source/dependency/order evidence, inline valid/
invalid/failure/stale/invalidated/cancel and exception dispatch evidence,
compile/lint/format, package identity, one ADR, one handoff, independent
review record, simplification assessment, and the expected release NO-GO
record.

### UI-58 — dialog-shell edge hierarchy

Settings and Command Palette already expose stable semantic dialog object
names, but their outer surfaces did not receive the same bounded frame and
accent-edge treatment as plugin and workspace-search dialogs. UI-58 adds
presentation-only QSS for those two existing identities: a token-driven
surface/border frame and a theme-specific top accent. Existing child-control
selectors remain the owners of fields, lists, buttons, preview content, and
action semantics.

The compatibility promise is unchanged dialog construction, modal/activation
behavior, signals, settings snapshot/persistence, command selection, locale,
keyboard focus, layout ownership, plugin behavior, search behavior, and
application policy. Acceptance requires selector/source coverage, token
readability evidence, compile/lint/format, package identity, one ADR, one
handoff, independent review record, simplification assessment, and the
expected release NO-GO record.

### D116 / ARCH-90 — session-save dispatch callback boundary

SessionSaveCoordinator already owns latest-wins admission, tracker sequencing,
invalid-result classification, failure notification, and drain behavior, but
MainWindow still owned the concrete TaskRunner callback binding in
`_submit_session_save`. D116 adds typed operation/success/failure/dispatcher
contracts and a coordinator `submit(...)` seam. MainWindow retains the
SessionService operation factory and injects the existing TaskRunner dispatch
callable; it no longer wires coordinator completion/failure callbacks itself.

The compatibility promise is unchanged snapshot capture/debounce,
latest-wins ordering, invalid/failure notification, session service and
persistence policy, close guards, startup restore, and synchronous dispatcher
exception propagation. Acceptance requires typed source/dependency/order
evidence, inline valid/invalid/failure/queued and dispatcher-exception
evidence, compile/lint/format, package identity, one ADR, one handoff,
independent review record, simplification assessment, and the expected
release NO-GO record.

### UI-59 — control-affordance chrome

The centralized stylesheet already styles the value surface and focus border
of `QComboBox` and `QSpinBox`, but their native dropdown/spinner affordances
fall back to low-signal platform subcontrol chrome. UI-59 adds token-driven
surface, hover, pressed, and disabled states to the existing combo dropdown
and abstract spinbox up/down subcontrols, including explicit subcontrol
placement and separator edges.

The compatibility promise is unchanged settings value ranges, signals,
keyboard focus/traversal, accessibility semantics, locale, persistence,
dialog layout, and application policy. Acceptance requires subcontrol selector
coverage, token-state evidence, compile/lint/format, package identity, one
ADR, one handoff, independent review record, simplification assessment, and
the expected release NO-GO record.

### UI-60 — dialog action-rail hierarchy

Plugin Catalog and Plugin Status already expose primary/warning button roles,
but their action rows are bare layouts without a shared presentation identity
or separator. UI-60 wraps the existing rows in `QWidget#dialogActionRail`,
preserves the same button objects and signal wiring, and adds one centralized
border/spacing contract so actions read as a deliberate dialog section.

The compatibility promise is unchanged plugin catalog/status data, button
enablement, approve/revoke/enable/disable signals, locale, plugin governance
policy, dialog layout ownership, and application behavior. Acceptance requires
source/object-name and signal coverage, token separator evidence,
compile/lint/format, package identity, one ADR, one handoff, independent
review record, simplification assessment, and the expected release NO-GO
record.

### UI-61 — accent palette swatch hierarchy

Theme and accent choices in Settings are currently text-only even though the
central theme resolver already exposes the complete palette. UI-61 adds one
stateless vector `color_swatch_icon(...)` renderer and projects token-derived
swatches onto the existing Theme and Accent combo items. The dialog refreshes
those decorative icons when pending theme/accent values change; localized item
text remains the semantic label and `ThemeId`/`AccentId` values remain the
domain contract.

The compatibility promise is unchanged settings schema, snapshot assembly,
locale flow, preview projection, Save/Cancel behavior, font/editor options,
motion setting, theme resolver, and application policy. Acceptance requires
source swatch/data/boundary coverage, token endpoint evidence, compile/lint/
format, package identity, one ADR, one handoff, independent review record,
simplification assessment, and the expected release NO-GO record.

### UI-62 — document tab close-affordance hierarchy

The document tab rail already has authored hover and pressed close styling, but
keyboard focus and disabled states are not explicit and the close target has no
minimum size. UI-62 adds an 18px minimum width/height and token-driven focus
and disabled QSS to the existing `QTabBar#documentTabBar::close-button`,
while preserving the existing danger-colored hover/pressed states.

The compatibility promise is unchanged `DocumentTabSurface` composition,
`tabCloseRequested` routing, tab identity/current-index behavior, close
confirmation, document policy, locale, persistence, and application behavior.
Acceptance requires selector/token/boundary coverage, compile/lint/format,
package identity, one ADR, one handoff, independent review record,
simplification assessment, and the expected release NO-GO record.

### D117 / ARCH-91 — operation-reserve facade simplification

`MainWindow._next_operation_id()` is a pure forwarding method to the existing
`OperationTracker.reserve()` contract. D117 initializes the tracker before
session-save composition, injects its bound reserve method into
`SessionSaveCoordinator`, replaces the seven pure call sites, and removes the
facade. `_begin_operation()` and `_complete_operation()` remain because they
own busy/status policy.

The compatibility promise is unchanged operation-ID monotonicity,
SessionSaveCoordinator callable shape, worker dispatch, stale/cancel/close
guards, busy/status projection, recovery/workspace/document/settings policy,
and persistence. Acceptance requires typed source/call-site/construction-order
evidence, compile/lint/format, package identity, one ADR, one handoff,
independent review record, simplification assessment, and the expected release
NO-GO record.

### D118 / ARCH-92 — session-restore ports contract

`SessionRestoreCoordinator[TabT]` already owned the ordered Qt-free restore
workflow, but its constructor exposed ten presentation callbacks as positional
arguments. D118 adds the frozen/slotted generic `SessionRestorePorts[TabT]`
contract and makes MainWindow map those callbacks by name. The coordinator
continues to own only restore progression; the tracker remains the state
owner, and MainWindow retains services, tabs, startup/close, notifications,
TaskRunner, and persistence policy.

The compatibility promise is unchanged inactive/workspace guards, deferred
and duplicate path handling, pending-open binding, active/first-tab selection,
initial-document fallback, finish-before-save order, and Qt-free dependency
direction. Acceptance requires a typed source contract, named composition,
Qt-free behavior probe, compile/lint/format, package identity, one ADR, one
handoff, independent review record, simplification assessment, and the
expected release NO-GO record.

### UI-63 — Find close-affordance stability

The Find bar close button now has an explicit 30px minimum height in its
existing scoped selector, plus token-driven pressed and disabled states. The
existing danger hover/focus treatment remains. FindBar button identity,
signals, keyboard focus, locale, layout, and editor behavior remain unchanged.

Acceptance requires scoped QSS/source coverage across all theme/accent
combinations, compile/lint/format, package identity, one ADR, one handoff,
independent review record, simplification assessment, and the expected release
NO-GO record.

### D119 / ARCH-93 — workspace file activation boundary

`WorkspacePanel` already emits separate file, directory, and keyboard intents,
but MainWindow previously handled raw file validation, startup/busy admission,
workspace containment, duplicate-tab focus, notification, and async-open
routing in one method. D119 adds the Qt-free generic
`WorkspaceFileActivationCoordinator[TabT]` with a frozen/slotted typed ports
contract and binds `WorkspaceSurface.file_requested` directly to it.

The compatibility promise is unchanged file first-click behavior, directory
double-click/keyboard activation, workspace containment, existing-tab focus,
notification order/text, and the single `_start_open`/DocumentOpenCoordinator
async boundary. Acceptance requires six-path behavior evidence, signal/source
coverage, compile/lint/format, package identity, one ADR, one handoff,
independent review record, simplification assessment, and the expected release
NO-GO record.

### D120 / ARCH-94 — workspace-navigation ports contract

`WorkspaceNavigationCoordinator` already owned open/directory completion
classification, but its constructor exposed seven presentation callbacks as
positional arguments. D120 adds the frozen/slotted `WorkspaceNavigationPorts`
contract and makes MainWindow map tracker completion, surface access,
valid-result projection, session-restore continuation, and notification by
named fields.

The compatibility promise is unchanged submit dispatch, generation tracking,
stale/invalidated handling, loading/error projection, invalid-result messages,
failure notification, restore continuation, close policy, and Qt-free
dependency direction. Acceptance requires behavior and source-contract
evidence, compile/lint/format, package identity, one ADR, one handoff,
independent review or explicit no-conclusion record, simplification
assessment, and the expected release NO-GO record.

### UI-64 / ARCH-95 — shell elevation and visual rhythm

The centralized stylesheet already owned command roles and document-tab states,
but the command rail and document navigator read as flat strips. UI-64 keeps
the visual refinement in `presentation/theme.py`: the command rail receives a
rounded grouped surface and a more generous target rhythm, while the document
tab rail receives a rounded container and clearer inset tabs.

The compatibility promise is unchanged widget construction, object names,
signals, shortcuts, tab identity/close behavior, focus/pressed/selected/
disabled states, locale, settings, editor behavior, and application policy.
Acceptance requires all-theme QSS/source evidence, token contrast evidence,
compile/lint/format, package identity, one ADR, one handoff, independent review
or explicit no-conclusion record, simplification assessment, and the expected
release NO-GO record.

### UI-65 / ARCH-96 — semantic message and tooltip chrome

The shell already had stable object names for common/about/error/recovery
`QMessageBox` surfaces and a shared `QToolTip` selector, but their chrome was
mostly flat. UI-65 refines only those existing selectors with rounded frames,
semantic top accents, explicit primary/informative label colors, consistent
button targets, and a compact tooltip frame using existing ThemeColors.

The compatibility promise is unchanged MessageSurface and
RecoveryPromptSurface composition, object names, localized text, button roles,
`exec()`/decision flow, locale, settings, and application policy. Acceptance
requires source/object-name coverage, all-theme contrast evidence,
compile/lint/format, package identity, one ADR, one handoff, independent review
or explicit no-conclusion record, simplification assessment, and the expected
release NO-GO record.

### D121 / ARCH-97 — close-guard ports contract

`CloseGuardCoordinator` already owned the ordered Qt-free close-readiness
classification, but its constructor exposed eight presentation callbacks as
positional arguments. D121 adds the frozen/slotted `CloseGuardPorts` contract
and maps MainWindow's existing busy/search-cancel/dirty/background/save/
pending/timer seams by name.

The compatibility promise is unchanged operation, workspace-search
cancellation, dirty-tab, background-work, immediate-save/pending-work, and
timer-stop precedence; `CloseGuardDecision`, QCloseEvent behavior, messages,
trackers, timers, persistence, and application policy remain unchanged.
Acceptance requires typed source/mapping evidence, all-branch behavior
evidence, compile/lint/format, package identity, one ADR, one handoff,
independent review or explicit no-conclusion record, simplification assessment,
and the expected release NO-GO record.

### D122 / ARCH-98 — document-open ports contract

`DocumentOpenCoordinator` already classified ordinary and session-restore
completion callbacks, but its constructor exposed seven presentation callbacks
as positional arguments. D122 adds the frozen/slotted `DocumentOpenPorts`
contract and maps operation completion, restore identity/document consumption,
valid projection, continuation, errors, and notifications by named fields.

The compatibility promise is unchanged ordinary/session-restore valid,
invalid, failure, stale, line-number, notification, projection, and
continuation ordering. `DocumentOpenProjectionCoordinator`, DocumentService,
TaskRunner, tab/editor, persistence, startup, close, and application policy
remain unchanged. Acceptance requires typed source/mapping evidence,
ordinary/restore behavior evidence, compile/lint/format, package identity, one
ADR, one handoff, independent review or explicit no-conclusion record,
simplification assessment, and the expected release NO-GO record.

### D123 / ARCH-99 — document-save ports contract

`DocumentSaveCoordinator` already classifies asynchronous save callbacks, but
its constructor exposes five presentation callbacks as positional arguments.
D123 adds the frozen/slotted `DocumentSavePorts` contract and maps operation
completion, tab liveness, read-only release, valid-save projection, and error
projection by named fields.

The compatibility promise is unchanged stale-operation suppression, missing-tab
suppression, read-only release, `DocumentState` validation, save success/failure
messages, continuation ownership, dispatch callback binding, persistence,
editor, close, and application policy. `DocumentSaveProjectionCoordinator`,
DocumentService, TaskRunner, and MainWindow save admission remain unchanged.
Acceptance requires typed source/mapping evidence, ordinary valid/invalid/
failure/stale/liveness behavior evidence, compile/lint/format, package identity,
one ADR, one handoff, independent review or explicit no-conclusion record,
simplification assessment, and the expected release NO-GO record.

### D124 / ARCH-100 — document-save projection ports contract

`DocumentSaveProjectionCoordinator` already owns the ordered projection of a
validated save result, but its constructor exposes seven presentation callbacks
as positional arguments. D124 adds the frozen/slotted generic
`DocumentSaveProjectionPorts[TabT]` contract and maps state, language, title,
recovery, event, notification, and session-save callbacks by named fields.

The compatibility promise is unchanged valid-save order: apply state, refresh
language, update title, clear recovery, publish the saved event, notify the
path, request the session save, and invoke the optional continuation. The save
service, `DocumentSaveCoordinator`, tab/editor policy, persistence, startup,
close, and application policy remain unchanged. Acceptance requires typed
source/mapping evidence, ordered projection behavior evidence, compile/lint/
format, package identity, one ADR, one handoff, independent review or explicit
no-conclusion record, simplification assessment, and the expected release
NO-GO record.

### D125 / ARCH-101 — document-open projection ports contract

`DocumentOpenProjectionCoordinator` already owns valid-open tab, line, cursor,
event, notification, and session-restore projection, but its constructor
exposes nine presentation callbacks as positional arguments. D125 adds the
frozen/slotted generic `DocumentOpenProjectionPorts[TabT]` contract and maps
existing-tab lookup, restore recording, duplicate error, tab creation, line
navigation, cursor placement, event publication, notification, and restore
continuation by named fields.

The compatibility promise is unchanged duplicate/restored branching and exact
valid-open order: find existing, handle restored duplicate or ordinary error,
add tab, optionally go to line, optionally restore cursor and record the tab,
publish the event, notify the path, and continue restore. DocumentService,
`DocumentOpenCoordinator`, tab/editor, persistence, startup, close, and
application policy remain unchanged. Acceptance requires typed source/mapping
evidence, duplicate/restored/line/cursor/order behavior evidence, compile/lint/
format, package identity, one ADR, one handoff, independent review or explicit
no-conclusion record, simplification assessment, and the expected release
NO-GO record.

### D126 / ARCH-102 — document-tab creation ports contract

`DocumentTabCreationCoordinator` already owns the deterministic assembly order
for a document tab, but its constructor exposes eight presentation callbacks as
positional arguments. D126 adds the frozen/slotted generic
`DocumentTabCreationPorts[OpenedT, TabT, EditorT]` contract and maps editor
creation, tab construction, surface insertion, title/modified queries, title
refresh, session-save request, and status synchronization by named fields.

The compatibility promise is unchanged `add()` order: create the editor, create
the tab with the optional recovery snapshot identity, add it with title and
modified state, update the title, request a session save, synchronize status,
and return the tab. EditorDocumentSurface, DocumentTabSurface, recovery,
persistence, startup, close, and application policy remain unchanged.
Acceptance requires typed source/mapping evidence, normal/recovery/order
behavior evidence, compile/lint/format, package identity, one ADR, one handoff,
independent review or explicit no-conclusion record, simplification assessment,
and the expected release NO-GO record.

### D127 / ARCH-103 — document-tab removal ports contract

`DocumentTabRemovalCoordinator` already owns finalization after close policy
approves a tab removal, but its constructor exposes eleven presentation
callbacks positionally. D127 adds the frozen/slotted generic
`DocumentTabRemovalPorts[TabT, CaptureT]` contract and maps liveness, document
identity, recovery capture lookup/cancellation, snapshot cleanup, tab removal,
editor deletion, close-event publication, session-save request, tab count, and
empty-document fallback by named fields.

The compatibility promise is unchanged: missing tabs return `False` without
side effects; live tabs cancel an active capture, clear recovery state, remove
the tab, delete the editor, publish close, request session save, ensure an
initial document only at zero count, then return `True`. Close admission,
recovery capture policy, persistence, startup, and application policy remain
unchanged. Acceptance requires typed source/mapping evidence, missing/live/
capture/empty-tab order behavior evidence, compile/lint/format, package
identity, one ADR, one handoff, independent review or explicit no-conclusion
record, simplification assessment, and the expected release NO-GO record.

### UI-66 / ARCH-104 — editor-stage visual rhythm

The central `EditorShellSurface` previously composed the existing document-tab
and Find surfaces with zero content margins and zero spacing. UI-66 gives that
existing composition boundary explicit 10/8px content margins and 8px spacing,
and projects `QWidget#editorShell` to the existing `surface_1` token so the
tab/editor `surface_0` canvas reads as a modern workspace stage.

The compatibility promise is unchanged: the tab surface remains the first
child, the Find surface remains the second child, Find visibility and locale
flow remain unchanged, and no tab/editor signal, font, motion, document,
MainWindow, or application policy moves. Acceptance requires source-contract
evidence, compile/lint/format, package identity, one ADR, one handoff,
independent review or explicit no-conclusion record, simplification
assessment, and the expected release NO-GO record.

### D128 / ARCH-105 — core command registration boundary

`MainWindow` still assembled and registered all 23 built-in commands inside its
large Qt composition class, while `CommandSurface` already owned only their Qt
menu/toolbar projection. D128 adds the Qt-free
`CoreCommandCoordinator` and frozen/slotted `CoreCommandPorts` contract. The
coordinator constructs and registers the established catalog in deterministic
order; MainWindow maps its existing behavior callbacks by name.

The compatibility promise is unchanged: command IDs, order, title, shortcut,
menu ID, callback identity, `CommandRegistry` collision behavior, plugin
registration, CommandSurface refresh, locale, shortcuts, and application
policy remain unchanged. Acceptance requires exact command behavior/contract
evidence, compile/lint/format, package identity, one ADR, one handoff,
independent review or explicit no-conclusion record, simplification
assessment, and the expected release NO-GO record.

### D129 / ARCH-106 — core toolbar composition boundary

`MainWindow` still assembled the six core toolbar action specifications and
the optional workspace action inside its large Qt composition class, while
`CommandSurface` already owned the actual QToolBar/QAction projection. D129
adds the presentation-only `CoreToolbarCoordinator` and frozen/slotted
`CoreToolbarPorts` contract. The coordinator emits the established action
specifications in deterministic order; MainWindow maps its existing
behavior callbacks by name and CommandSurface remains the projection owner.

The compatibility promise is unchanged: action count, order, text keys,
callback identity, icons, separator metadata, roles, command IDs, locale,
toolbar projection, and application policy remain unchanged. Acceptance
requires exact no-workspace/workspace behavior and contract evidence,
compile/lint/format, package identity, one ADR, one handoff, independent
review or explicit no-conclusion record, simplification assessment, and the
expected release NO-GO record.

### D130 / ARCH-107 — pure toolbar contract extraction

D129 left the action-spec composition in a coordinator that did not create Qt
widgets, but its `ToolbarActionSpec` and `IconKey` imports still originated in
Qt-bearing presentation modules. D130 corrects that dependency direction by
placing `IconKey` in `icon_contract.py` and `ToolbarActionRole`/
`ToolbarActionSpec` in `toolbar_contract.py`. `icons.py` and
`command_surface.py` retain compatibility imports; CoreToolbarCoordinator
depends only on the pure contract modules.

The compatibility promise is unchanged: action count, order, text keys,
callback identity, icon values, separator metadata, roles, locale,
QToolBar/QAction projection, and application policy remain unchanged.
Acceptance requires exact behavior and Qt-free import evidence,
compile/lint/format, package identity, one ADR, one handoff, independent
review or explicit no-conclusion record, simplification assessment, and the
expected release NO-GO record.

### D131 / ARCH-108 — Replace All admission ports boundary

`MainWindow` still owned Replace All admission checks, editor session
creation, operation/tracker binding, and the first UI-slice handoff beside
the existing cooperative QTimer loop and completion coordinator. D131 adds
the Qt-free `ReplaceAllAdmissionCoordinator` with frozen/slotted
`ReplaceAllAdmissionPorts`. It preserves the existing tracker and completion
seams while MainWindow maps editor, FindSurface, tab-bar, operation, status,
and starter callbacks by name.

The compatibility promise is unchanged: inflight and busy precedence, empty
tab/query handling, ValueError status projection, session arguments, operation
identity, dirty/content-version capture, duplicate-job rejection, first
slice scheduling, completion/rollback behavior, and application policy remain
unchanged. Acceptance requires exact admission behavior and contract evidence,
compile/lint/format, package identity, one ADR, one handoff, independent
review or explicit no-conclusion record, simplification assessment, and the
expected release NO-GO record.

### D132 / ARCH-109 — recovery capture admission ports boundary

`MainWindow` still owned recovery autosave gating, dirty-tab candidate
selection, inflight/delete-pending exclusion, snapshot identity, dirty-state
normalization, and content-version capture beside the channel/editor capture
pipeline. D132 adds the Qt-free `RecoveryCaptureAdmissionCoordinator` with
frozen/slotted `RecoveryCaptureAdmissionPorts`. It emits an immutable
candidate and invokes the existing starter in tab order.

The compatibility promise is unchanged: recovery-available/busy precedence,
tab order, dirty filtering, document inflight exclusion, delete-pending
exclusion, snapshot reuse/generation and assignment, state normalization,
content-version capture, channel/backpressure, editor capture, tracker and
writer lifecycle, QTimer slicing, abort/write, close policy, and application
behavior remain unchanged. Acceptance requires exact admission behavior and
contract evidence, compile/lint/format, package identity, one ADR, one
handoff, independent review or explicit no-conclusion record, simplification
assessment, and the expected release NO-GO record.

### UI-67 / ARCH-110 — shell surface depth and primary-work-area hierarchy

The centralized QSS previously placed several adjacent shell surfaces on the
same token value. UI-67 rebalances the existing surface ladder so the main
canvas, editor stage, command rail, status rail, document tab rail/items,
workspace dock, and workspace panel have readable visual roles. The change is
limited to `presentation/theme.py`; existing semantic interaction selectors,
contrast-derived foregrounds, locale/font/motion settings, signals, and
application policy remain the source of truth.

Acceptance requires the shell-selector and all-theme/accent contrast probes,
compile/lint/format, package identity, one ADR, one handoff, independent
review or explicit no-conclusion record, simplification assessment, and the
expected release NO-GO record. Native Qt rendering remains explicitly open.

### D133 / ARCH-111 — document-open admission boundary

`MainWindow._start_open` previously combined busy/startup-restore admission,
operation begin, session-restore binding, and asynchronous submission beside
the existing `DocumentOpenCoordinator` result boundary. D133 adds the Qt-free
`DocumentOpenAdmissionCoordinator` with frozen/slotted
`DocumentOpenAdmissionPorts`. It preserves the exact gate precedence, warning
text, operation message, binding order, dispatcher shape, `line_number`, and
`session_restore` values.

The compatibility promise is unchanged: `FileDialogSurface` file selection,
`WorkspacePanel` file/folder activation, `DocumentService`,
`DocumentOpenCoordinator` result classification, duplicate-tab projection,
line navigation, session restore, notifications, and close policy remain in
their existing owners. Acceptance requires exact admission behavior and
contract evidence, compile/lint/format, package identity, one ADR, one
handoff, independent review or explicit no-conclusion record, simplification
assessment, and the expected release NO-GO record.

### D134 / ARCH-112 — document-save admission boundary

`MainWindow._start_save` previously combined busy/startup-restore admission,
duplicate-target rejection, immutable state/text capture, editor read-only
protection, operation begin, and asynchronous submission beside the existing
`DocumentSaveCoordinator` result boundary. D134 adds the Qt-free
`DocumentSaveAdmissionCoordinator` with frozen/slotted
`DocumentSaveAdmissionPorts`. It preserves the exact gate precedence, warning
and operation text, conflict rejection order, snapshot/read-only order,
`after` continuation, and `DocumentSaveCoordinator`/TaskRunner dispatcher
shape.

The compatibility promise is unchanged: save/save-as selection,
`FileDialogSurface`, target identity, document persistence, result
classification, recovery/session persistence, notifications, and close policy
remain in their existing owners. Acceptance requires exact admission behavior
and contract evidence, compile/lint/format, package identity, one ADR, one
handoff, independent review or explicit no-conclusion record, simplification
assessment, and the expected release NO-GO record.

### D135 / ARCH-113 — workspace-navigation admission boundary

`MainWindow._start_workspace_open` and `_load_workspace_directory` previously
combined workspace availability checks, busy/startup-restore admission,
loading projection, generic operation begin, workspace-generation binding, and
asynchronous submission beside the existing `WorkspaceNavigationCoordinator`
result boundary. D135 adds the Qt-free
`WorkspaceNavigationAdmissionCoordinator` with frozen/slotted
`WorkspaceNavigationAdmissionPorts`. It preserves the exact non-`Path`
directory guard, workspace/surface/busy/restore gate order, warning text,
loading-before-operation order, open/directory operation messages, captured
service binding, generation binding, and dispatcher shape.

The compatibility promise is unchanged: `WorkspaceService`,
`WorkspaceSurface`, file/folder activation, containment,
`WorkspaceNavigationCoordinator` result classification,
`WorkspaceNavigationProjectionCoordinator`, cancellation, session restore,
notifications, and close policy remain in their existing owners. Acceptance
requires exact admission behavior and contract evidence, compile/lint/format,
package identity, one ADR, one handoff, independent review or explicit
no-conclusion record, simplification assessment, and the expected release
NO-GO record.

### D136 / ARCH-114 — settings-save admission boundary

`MainWindow._show_settings` previously combined settings-service availability,
in-flight admission, modal candidate editing, operation reservation, tracker
binding, and asynchronous submission beside the existing
`SettingsSaveCoordinator` result boundary. D136 adds the Qt-free
`SettingsSaveAdmissionCoordinator` with frozen/slotted
`SettingsSaveAdmissionPorts`. It preserves the exact service/error and
in-flight/warning precedence, candidate-cancel behavior, service capture,
operation/tracker order, and `SettingsSaveCoordinator`/TaskRunner dispatcher
shape.

The compatibility promise is unchanged: `SettingsSurface`, settings
validation/persistence, result classification, theme/locale/font/editor
application, motion transition, notifications, and close policy remain in
their existing owners. Acceptance requires exact admission behavior and
contract evidence, compile/lint/format, package identity, one ADR, one
handoff, independent review or explicit no-conclusion record, simplification
assessment, and the expected release NO-GO record.

### D137 / ARCH-115 — document-creation admission boundary

`MainWindow._new_document` previously combined busy/startup-restore admission,
document creation, tab projection, lifecycle publication, and success
notification. D137 adds the Qt-free `DocumentCreationAdmissionCoordinator`
with frozen/slotted `DocumentCreationAdmissionPorts`. It preserves the exact
busy/restore gate behavior, restore warning, explicit
`allow_during_startup` exception, document-service creation, tab projection,
`DocumentOpened` publication, and success-notification order.

The compatibility promise is unchanged: `DocumentService`,
`DocumentTabCreationCoordinator`, editor/tab construction, title/modified
projection, session-save/status synchronization, `EventBus`, startup restore,
notifications, and close policy remain in their existing owners. Acceptance
requires exact admission behavior and contract evidence, compile/lint/format,
package identity, one ADR, one handoff, independent review or explicit
no-conclusion record, simplification assessment, and the expected release
NO-GO record.

### UI-68 / ARCH-116 — semantic state contrast closure

The centralized stylesheet previously reused semantic success, danger, and
working colors as text on their state backgrounds. UI-68 keeps
`presentation/theme.py` as the sole visual owner and derives local readable
foregrounds from `success_bg`, `pressed`, and `error_bg` through the existing
contrast helper. Status, workspace/search, and FindBar feedback selectors now
share those endpoints without changing warning/gold handling, widget
identities, signals, locale, motion, or application policy.

Acceptance requires the all-theme/accent semantic contrast and generated-QSS
probes, compile/lint/format, package identity, one ADR, one handoff,
independent review or explicit no-conclusion record, simplification
assessment, and the expected release NO-GO record. Native QSS rendering,
installed-font metrics, DPI, and runtime visual evidence remain open.

### UI-69 / ARCH-117 — settings-dialog card hierarchy

The settings dialog already had semantic object names, but its canvas,
appearance/editor groups, preview, and action row shared adjacent surface
values. UI-69 keeps the centralized stylesheet and establishes a bounded
`surface_0` canvas plus distinct `surface_1`/`surface_2`/`surface_3` cards and
an object-scoped action rail. No widget, form layout, signal, settings
schema/persistence, locale, font, motion, or application policy changes.

Acceptance requires the all-theme/accent dialog hierarchy and QSS scope
probes, compile/lint/format, package identity, one ADR, one handoff,
independent review or explicit no-conclusion record, simplification
assessment, and the expected release NO-GO record. Native style-engine
rendering, installed-font metrics, DPI, and runtime visual evidence remain
open.

### D138 / ARCH-118 — document-picker admission boundary

`MainWindow._open_document` previously combined busy/startup-restore admission,
native file selection, cancellation, and handoff to `_start_open`. D138 adds
the Qt-free `DocumentPickerAdmissionCoordinator` with frozen/slotted
`DocumentPickerAdmissionPorts`. It preserves silent busy rejection, the exact
restore warning, native file selection, cancellation no-op behavior, and
handoff of the selected `Path` into the existing
`DocumentOpenAdmissionCoordinator` boundary.

The compatibility promise is unchanged: `FileDialogSurface`,
`DocumentService`, document-open result classification, workspace directory
selection, notifications, and close policy remain in their existing owners.
Acceptance requires exact picker behavior, contract, Qt-free, and wiring
evidence, compile/lint/format, package identity, one ADR, one handoff,
independent review or explicit no-conclusion record, simplification
assessment, and the expected release NO-GO record.

### D139 / ARCH-119 — workspace-picker admission boundary

`MainWindow._choose_workspace` previously combined workspace availability,
startup-restore and busy admission, native directory selection, cancellation,
and handoff to `_start_workspace_open`. D139 adds the Qt-free
`WorkspacePickerAdmissionCoordinator` with frozen/slotted
`WorkspacePickerAdmissionPorts`. It preserves unavailable-workspace error
projection, the exact restore warning, silent busy rejection, native directory
selection, cancellation no-op behavior, and handoff of the selected `Path` into
the existing `WorkspaceNavigationAdmissionCoordinator` boundary.

The compatibility promise is unchanged: `FileDialogSurface`, document file
selection/opening, `WorkspaceService`, workspace result classification, file
activation, containment, notifications, and close policy remain in their
existing owners. Acceptance requires exact picker behavior, contract, Qt-free,
and wiring evidence, compile/lint/format, package identity, one ADR, one
handoff, independent review or explicit no-conclusion record, simplification
assessment, and the expected release NO-GO record.

### UI-70 / ARCH-120 — modern shell elevation

The centralized stylesheet previously left several adjacent shell surfaces at
nearly the same visual level. UI-70 keeps `presentation/theme.py` as the sole
visual owner and refines the existing QSS for the command bar, editor shell,
document tab rail/tabs, status rail, workspace dock, and workspace empty state.
It uses only existing surface, border, and accent tokens, separates command
bar hover from its normal surface, and preserves all widget IDs, signals,
layout, locale, font, motion, semantic state, accessibility, theme/accent, and
application policy contracts.

Acceptance requires all-theme/accent shell hierarchy and hover-separation
evidence, compile/lint/format, package identity, one ADR, one handoff,
independent review or explicit no-conclusion record, simplification assessment,
and the expected release NO-GO record. Native style-engine rendering, font/DPI,
accessibility tooling, and runtime visual evidence remain open.

### D140 / ARCH-121 — workspace-search surface admission boundary

`MainWindow._show_workspace_search` previously combined startup-restore
admission, service/workspace availability, missing-root feedback, creation or
reuse of the non-modal search surface, root-change invalidation, and showing the
surface. D140 adds the Qt-free `WorkspaceSearchSurfaceAdmissionCoordinator`
with frozen/slotted `WorkspaceSearchSurfaceAdmissionPorts` and a narrow
`WorkspaceSearchSurfacePort`. It preserves the exact warning/error precedence,
surface creation/reuse, root-change invalidation/reset, and presentation order.

The compatibility promise is unchanged: `WorkspaceSearchSurface`, query
validation, operation tracking, cancellation, worker dispatch, result
classification, containment, locale, notifications, and close policy remain
in their existing owners. Acceptance requires exact surface behavior,
contract, Qt-free, and wiring evidence, compile/lint/format, package identity,
one ADR, one handoff, independent review or explicit no-conclusion record,
simplification assessment, and the expected release NO-GO record.

### D141 / ARCH-122 — document-save picker admission boundary

`MainWindow._save_document` and `_save_as_document` previously combined
active-tab/busy guards, ordinary-save target routing, Save As intent, native
save selection, cancellation, and handoff into `_start_save`. D141 adds the
Qt-free generic `DocumentSavePickerAdmissionCoordinator[TabT]` with a
frozen/slotted `DocumentSavePickerAdmissionPorts[TabT]`. It preserves silent
no-tab/busy rejection, ordinary Save path reuse, untitled Save picker
selection, Save As forced picker selection, cancellation no-op behavior, and
handoff of the selected `Path` into the existing
`DocumentSaveAdmissionCoordinator` boundary.

The compatibility promise is unchanged: `FileDialogSurface`, document
validation/conflict/read-only/persistence, result classification,
recovery/session projection, notifications, and close policy remain in their
existing owners. Acceptance requires exact Save/Save As picker behavior,
contract, Qt-free, and wiring evidence, compile/lint/format, package identity,
one ADR, one handoff, independent review or explicit no-conclusion record,
simplification assessment, and the expected release NO-GO record.

### UI-71 / ARCH-123 — workspace-search query card

The workspace search dialog already had themed root, result, diagnostic, and
feedback controls, but its query controls were a bare horizontal layout. UI-71
adds one presentation-only `QFrame#workspaceSearchQueryCard` around the
existing query row and scoped token QSS for the card, query field, and case
checkbox. Search/cancel/file signals, Return-key behavior, busy/cancel state,
results/diagnostics/status projection, locale, font, motion, theme/accent, and
application policy remain unchanged.

Acceptance requires all-theme/accent query-card projection, signal-preservation,
and QSS-scope evidence, compile/lint/format, package identity, one ADR, one
handoff, independent review or explicit no-conclusion record, simplification
assessment, and the expected release NO-GO record. Native style-engine,
font/DPI, accessibility, and runtime visual evidence remain open.

### D142 / ARCH-124 — presentation contract audit closure

The existing presentation audit now adds two source-level contract rules:
top-level coordinator classes ending in `Ports` must be declared with
`@dataclass(frozen=True, slots=True)`, and direct coordinator port notification
calls must include an explicit `level=` keyword. The existing Qt-free import,
MainWindow notification, and TaskRunner pending-work observability checks stay
in the same audit. No coordinator behavior, Qt projection, service, worker, or
application policy moves.

Acceptance requires the Ports, notification-level, dependency, and audit-run
probes, compile/lint/format, package identity, one ADR, one handoff,
independent review or explicit no-conclusion record, simplification assessment,
and the expected release NO-GO record. Runtime rendering, clean-machine,
cross-machine, legal, signing, installer/update, and release-owner evidence
remain open.

### D143 / ARCH-125 — presentation locale coordinator

`MainWindow._retranslate_ui()` now delegates the same title, command, dialog,
workspace, editor, tab, status, search, and plugin refresh sequence to the
Qt-free `PresentationLocaleCoordinator`. Its frozen/slotted Ports contract
receives callbacks, so MainWindow retains concrete Qt surfaces, translation
keys, settings, optional-surface checks, and policy. No locale catalog or
runtime behavior changes.

Acceptance requires exact locale-sequence, optional-surface, Qt-free, and
MainWindow-wiring probes, compile/lint/format, package identity, one ADR, one
handoff, independent review or explicit no-conclusion record, simplification
assessment, and the expected release NO-GO record. Native translation
rendering, font/DPI, accessibility, runtime startup, and release evidence
remain open.

### D144 / ARCH-126 — close-guard feedback coordinator

`MainWindow._project_close_guard_block()` now delegates the existing five
close-block reason messages and pending-count interpolation to the Qt-free
`CloseGuardFeedbackCoordinator`. Its frozen/slotted Ports contract receives
only pending-count and error-projection callbacks. Close classification,
cancellation, QCloseEvent handling, MessageSurface, and application policy stay
in their existing owners.

Acceptance requires exact reason-coverage, allowed-decision, Qt-free, and
MainWindow-wiring probes, compile/lint/format, package identity, one ADR, one
handoff, independent review or explicit no-conclusion record, simplification
assessment, and the expected release NO-GO record. Native dialog rendering,
event timing, runtime startup, and release evidence remain open.

### UI-72 / ARCH-127 — status notification pill

The existing centralized `QLabel#statusMessage` QSS now projects a compact
rounded state surface with a readable info baseline and semantic left color
rail. Existing success/warning/error backgrounds and contrast-safe foregrounds
remain the only semantic color inputs; StatusSurface, notification levels,
timers, signals, locale, motion, and application policy remain unchanged.

Acceptance requires 3-theme/4-accent status-pill coverage, semantic-state and
QSS-centralization probes, compile/lint/format, package identity, one ADR, one
handoff, independent review or explicit no-conclusion record, simplification
assessment, and the expected release NO-GO record. Native QSS rendering,
font/DPI, accessibility, runtime startup, and release evidence remain open.

### UI-73 / ARCH-128 — document-stage edge

The centralized `QTabWidget#documentTabs::pane` selector no longer paints a
redundant border, radius, or one-pixel top overlap. The authored document-tab
rail and `QsciScintilla#editor` canvas edge/focus cue remain the visual
boundaries, while document/find signals, keyboard routing, layout ownership,
locale, font, motion, and application policy remain unchanged. ADR-0190 records
the bounded decision.

Acceptance requires the document-stage selector probe, 3-theme/4-accent
stylesheet projection, tab-rail/editor focus-state source coverage,
compile/lint/format, package identity, one ADR, one handoff, independent review
or explicit no-conclusion record, simplification assessment, and the expected
release NO-GO record. Native QSS specificity/rendering, font/DPI,
accessibility, runtime startup, and release evidence remain open.

### D145 / ARCH-129 — status-phase policy

The Qt-free `StatusPhaseCoordinator` now owns only the existing deterministic
priority: busy or retained work projects `working`, dirty active-document state
projects `attention`, and otherwise the shell is `ready`. A canonical pure
`StatusPhase` contract is shared by the coordinator and status presentation;
MainWindow retains concrete fact queries and direct `error` projection, while
StatusSurface/StatusRail retain Qt rendering, locale, accessibility, and style
refresh. ADR-0191 records the bounded decision.

Acceptance requires the six-combination precedence probe, Qt-free boundary and
MainWindow wiring/error-path probes, compile/lint/format, package identity, one
ADR, one handoff, independent review or explicit no-conclusion record,
simplification assessment, and the expected release NO-GO record. Native
status-bar rendering, queued event timing, font/DPI, accessibility, runtime
startup, and release evidence remain open.

### UI-74 / ARCH-130 — status-rail context divider

The existing centralized `QLabel#statusContext` selector now adds a
token-driven right divider and compact padding so local context and lifecycle
phase scan as separate elements. The status-rail object tree, `statusPhase`
ready/working/attention/error states, locale, timers, signals, and application
policy remain unchanged. ADR-0192 records the bounded decision.

Acceptance requires 3-theme/4-accent context-divider coverage,
QSS-centralization and phase-state preservation probes, compile/lint/format,
package identity, one ADR, one handoff, independent review or explicit
no-conclusion record, simplification assessment, and the expected release
NO-GO record. Native status-rail rendering, font/DPI, accessibility, runtime
startup, and release evidence remain open.

### D146 / ARCH-131 — editor-change projection boundary

`MainWindow._on_editor_modified` and `_on_editor_content_changed` previously
combined editor callback lookup, stale/unchanged guards, Find invalidation,
dirty/content-version mutation, tab-title/status projection, and debounced
session-save ordering. D146 adds the Qt-free generic
`DocumentChangeProjectionCoordinator[EditorT, TabT]` with a frozen/slotted
`DocumentChangeProjectionPorts[EditorT, TabT]` contract and preserves the
exact two callback sequences.

The compatibility promise is unchanged: modified callbacks invalidate Find
before lookup, ignore stale/unchanged tabs, then project dirty state/title/
status/session-save; content callbacks ignore stale editors, increment content
identity, then invalidate Find. MainWindow retains DocumentService, concrete
tab/editor lookup and identity, FindSurface, StatusSurface, session timers,
Qt signals, and application policy. Current-tab transition, editor engine,
persistence, async workers, close policy, locale, and QSS remain outside the
boundary.

Acceptance requires exact modified/content ordering probes, MainWindow wiring,
Qt-free/dependency audit, compile/lint/format, package identity, one ADR, one
handoff, independent review or explicit no-conclusion record, simplification
assessment, and the expected release NO-GO record. Native signal timing,
editor rendering, runtime startup, clean-machine, cross-machine, legal,
installer/update, support, and release evidence remain open.

### D147 / ARCH-132 — current-document transition projection boundary

`MainWindow._on_current_tab_changed` previously combined Find invalidation,
Find-session reset, active-tab lookup, title/context notification, status
refresh, and debounced session-save request. D147 adds the Qt-free generic
`CurrentDocumentTransitionCoordinator[TabT]` with a frozen/slotted
`CurrentDocumentTransitionPorts[TabT]` contract and preserves the exact event
sequence, including the empty-tab no-notification path.

The compatibility promise is unchanged: Find invalidation and reset occur
first; an active tab emits the existing title without its dirty marker through
the semantic info-notification port; status and session-save projection still
run for both active and empty states. MainWindow retains tab identity,
FindSurface, StatusSurface, notification policy, session timers, Qt signal
ownership, and application policy. Editor mutation, document persistence,
async workers, close policy, locale, and QSS remain outside the boundary.

Acceptance requires exact transition-order/empty-tab probes, MainWindow
wiring, Qt-free/dependency audit, compile/lint/format, package identity, one
ADR, one handoff, independent review or explicit no-conclusion record,
simplification assessment, and the expected release NO-GO record. Native
signal timing, focus/rendering, runtime startup, clean-machine,
cross-machine, legal, installer/update, support, and release evidence remain
open.

### UI-75 / ARCH-133 — active-document tab emphasis

The generic `QTabBar::tab:selected` rule already carried a left accent cue,
but the more specific `QTabBar#documentTabBar::tab:selected` and selected-hover
rules replaced `border-color` and suppressed that cue in the actual document
tab rail. UI-75 adds only `border-left-color: {colors.accent_alt}` to those two
existing selectors.

The compatibility promise is unchanged: border width, padding, margins,
bottom accent, surface, text, font weight, focus/disabled/close states, tab
signals, locale, motion, and application policy remain intact. No new widget,
color token, stylesheet, icon, or layout owner is introduced.

Acceptance requires 3-theme/4-accent selected-tab projection, selected-state
preservation, compile/lint/format, package identity, one ADR, one handoff,
independent review or explicit no-conclusion record, simplification
assessment, and the expected release NO-GO record. Native QSS specificity,
tab metrics, font/DPI, accessibility, runtime startup, clean-machine,
cross-machine, legal, installer/update, support, and release evidence remain
open.

### D148 / ARCH-134 — editor-action admission boundary

`MainWindow._run_editor_action` previously combined active-tab lookup, busy
rejection, editor-action invocation, and focus restoration. D148 adds the
Qt-free generic `EditorActionAdmissionCoordinator[EditorT, TabT]` with a
frozen/slotted `EditorActionAdmissionPorts[EditorT, TabT]` contract and keeps
the exact admission sequence:

`active_tab -> busy guard -> editor_of -> action(editor) -> focus_editor`.

The compatibility promise is unchanged: an empty shell is a silent no-op and
does not read busy state; a busy shell is a silent no-op and does not invoke or
focus an editor; an admitted action runs once against the active editor and
restores focus once. MainWindow retains tab/editor identity, busy policy, Qt
focus, QAction/shortcut/menu composition, editor behavior, document policy,
and all concrete surface ownership. Command registration, undo policy, dirty
tracking, locale, QSS, asynchronous workers, and close policy remain outside
the boundary.

Acceptance requires admission/ordering probes, MainWindow wiring, Qt-free
dependency audit, compile/lint/format, package identity, one ADR, one
handoff, independent review or explicit no-conclusion record, simplification
assessment, and the expected release NO-GO record. Native event timing,
focus/rendering, font/DPI, accessibility, runtime startup, clean-machine,
cross-machine, legal, installer/update, support, and release evidence remain
open.

### D149 / ARCH-135 — settings-save projection Ports contract

`SettingsSaveProjectionCoordinator` previously accepted five positional
callbacks even though its valid-result order was already observable. D149
replaces those parameters with the frozen/slotted Qt-free
`SettingsSaveProjectionPorts` contract, naming `apply_snapshot`,
`retranslate`, `apply_editor_settings`, `animate_transition`, and
`notify_saved`.

The compatibility promise is unchanged: a valid result is projected exactly
as `apply_snapshot -> retranslate -> apply_editor_settings ->
animate_transition -> notify_saved`, with the same result object and exception
propagation. MainWindow retains settings-service/dialog ownership, concrete
editor iteration, QApplication/theme application, animation surface,
notification policy, and result classification. Settings validation,
persistence, locale catalog, theme tokens, editor behavior, async workers,
close policy, and runtime startup remain outside the boundary.

Acceptance requires projection-order/Ports probes, MainWindow wiring,
Qt-free/dependency audit, compile/lint/format, package identity, one ADR, one
handoff, independent review or explicit no-conclusion record, simplification
assessment, and the expected release NO-GO record. Native event timing,
theme/editor rendering, font/DPI, accessibility, runtime startup,
clean-machine, cross-machine, legal, installer/update, support, and release
evidence remain open.

### D150 / ARCH-136 — recovery projection Ports contract

`RecoveryProjectionCoordinator` previously accepted eight positional callbacks
for recovery-writer outcome projection. D150 replaces those parameters with
the frozen/slotted generic `RecoveryProjectionPorts[OwnerT]` contract, naming
liveness, dirty state, snapshot identity, content identity, deletion, clear,
newer-edit feedback, and failure feedback.

The compatibility promise is unchanged. Saved discarded outcomes schedule
deletion; saved non-live outcomes schedule deletion; live outcomes inspect
dirty, snapshot identity, and content version in the existing order; discarded
failures are silent; live failures notify. MainWindow retains recovery state,
tab identity, deletion/clear policy, notification text and levels, writer
lifecycle, and concrete application ownership. Capture, persistence format,
timers, delete service, close policy, Qt surfaces, async workers, and runtime
startup remain outside the boundary.

Acceptance requires recovery branch/Ports probes, MainWindow wiring,
Qt-free/dependency audit, compile/lint/format, package identity, one ADR, one
handoff, independent review or explicit no-conclusion record, simplification
assessment, and the expected release NO-GO record. Native recovery timing and
durability, filesystem behavior, font/DPI, accessibility, runtime startup,
clean-machine, cross-machine, legal, installer/update, support, and release
evidence remain open.

### UI-76 / ARCH-137 — inactive-selection contrast closure

The inactive selected-row selector for the workspace tree and list views used
`text_secondary` on the shared `pressed` background. In Paper-Sand this was
only `3.69:1`. UI-76 changes only that foreground to the existing
`text_primary` token in the centralized stylesheet.

The compatibility promise is unchanged: the pressed background, strong
border, accent-left cue, disabled selector, selection model, focus and
activation behavior, row geometry, widget IDs, locale, font, motion, token
values, and application policy remain intact. No new token, widget, signal, or
layout owner is introduced.

Acceptance requires 3-theme/4-accent contrast projection, selector/state
preservation, compile/lint/format, package identity, one ADR, one handoff,
independent review or explicit no-conclusion record, simplification
assessment, and the expected release NO-GO record. Native QSS rendering,
focus/DPI, accessibility, runtime startup, clean-machine, cross-machine,
legal, installer/update, support, and release evidence remain open.

### D153 / ARCH-140 — Replace All completion Ports contract

`ReplaceAllCompletionCoordinator` already owns the stale-guarded cleanup
boundary for a cooperative Replace All job. D153 replaces its six independent
callbacks with the frozen/slotted generic Qt-free
`ReplaceAllCompletionPorts[TabT, SessionT, ProgressT]` contract, naming tab
containment/unlock, tab-bar enablement, operation-active projection,
operation completion, and product-specific outcome projection.

The compatibility promise is unchanged. A current job releases the tracker,
unlocks a live tab, enables the tab bar, clears the operation-active state,
completes the operation, and only then projects the product outcome. Stale
jobs remain silent. MainWindow retains editor mutation, FindSurface, tab,
operation, ReplaceAllSession, rollback, cancellation, status, and policy
ownership. The session algorithm, timer, locale, theme, motion, tab model,
and runtime startup remain outside the boundary.

Acceptance requires completion-order/stale/Ports probes, MainWindow wiring,
Qt-free/dependency audit, compile/lint/format, package identity, one ADR, one
handoff, independent review or explicit no-conclusion record, simplification
assessment, and the expected release NO-GO record. Native event timing,
editor rollback, runtime startup, clean-machine, cross-machine, legal,
installer/update, support, and release evidence remain open.

### D154 / ARCH-141 — settings-save Ports contract

`SettingsSaveCoordinator` already owns settings-save result classification and
stale suppression. D154 replaces its three independent result callbacks with
the frozen/slotted Qt-free `SettingsSavePorts` contract, naming valid snapshot
application, invalid-result feedback, and matching failure projection.

The compatibility promise is unchanged. Stale completions and failures remain
silent; invalid results retain invalid feedback; valid `SettingsSnapshot`
results apply once; matching failures project once. MainWindow retains the
settings service, validation, theme/locale/font/editor refresh, transition,
notification, close policy, and concrete Qt/application ownership. Settings
validation, persistence, worker dispatch, and runtime startup remain outside
the boundary.

Acceptance requires settings-result/branch/Ports probes, MainWindow wiring,
Qt-free/dependency audit, compile/lint/format, package identity, one ADR, one
handoff, independent review or explicit no-conclusion record, simplification
assessment, and the expected release NO-GO record. Native worker/timer timing,
settings filesystem durability, runtime startup, clean-machine, cross-machine,
legal, installer/update, support, and release evidence remain open.

### D155 / ARCH-142 — workspace-search Ports contract

`WorkspaceSearchCoordinator` already owns search callback classification,
generation invalidation, result validation, summary severity, and failure
projection. D155 replaces its four independent result callbacks with the
frozen/slotted Qt-free `WorkspaceSearchPorts` contract, naming surface lookup,
summary formatting, and notification projection.

The compatibility promise is unchanged. Stale callbacks remain silent;
invalidated callbacks project cancellation; invalid results retain error
feedback; valid results retain surface projection and warning/success summary
severity; matching failures retain surface and notification error projection.
MainWindow retains the search service, query/generation cancellation,
WorkspaceSearchSurface, containment, notification policy, and concrete
Qt/application ownership. Search service behavior, result model, worker
dispatch, and runtime startup remain outside the boundary.

During the bounded inline probe, the failure branch exposed one stale
`_get_surface` reference left by the callback replacement. The smallest root
fix routed that branch through `WorkspaceSearchPorts.get_surface`; the full
branch probe passed after the correction.

Acceptance requires workspace-search branch/order/Ports probes, MainWindow
wiring, Qt-free/dependency audit, compile/lint/format, package identity, one
ADR, one handoff, independent review or explicit no-conclusion record,
simplification assessment, and the expected release NO-GO record. Native
worker/event timing, filesystem traversal, runtime startup, clean-machine,
cross-machine, legal, installer/update, support, and release evidence remain
open.

### D156 / ARCH-143 — plugin-runtime Ports contract

`PluginRuntimeCoordinator` already owns plugin failure projection, runtime
availability/busy guards, enablement exception handling, command refresh, and
status projection. D156 replaces its four independent presentation callbacks
with the frozen/slotted Qt-free `PluginRuntimePorts` contract, naming the
plugin status view, busy predicate, command refresh sink, and notification
sink while retaining `PluginRuntime` as the concrete application dependency.

The compatibility promise is unchanged. Plugin failures still notify before
refreshing commands; unavailable runtime and busy operations remain guarded;
runtime lifecycle exceptions remain error-only; successful toggles still
refresh commands, notify success, and then project status. MainWindow retains
runtime, trust/permission/enablement policy, persistence, plugin catalog/host
boundaries, command registry, surface, and concrete Qt ownership. Plugin
protocol, security, permission, enablement, host, catalog, and runtime startup
behavior remain outside the boundary.

Acceptance requires plugin-runtime branch/order/Ports probes, MainWindow
wiring, Qt-free/dependency audit, compile/lint/format, package identity, one
ADR, one handoff, independent review or explicit no-conclusion record,
simplification assessment, and the expected release NO-GO record. Native
plugin lifecycle timing, runtime startup, clean-machine, cross-machine, legal,
installer/update, support, and release evidence remain open.

### D157 / ARCH-144 — recovery-delete Ports contract

`RecoveryDeleteCoordinator` already owns the presentation callback boundary for
recovery snapshot deletion. D157 replaces its three independent callbacks with
the frozen/slotted generic Qt-free `RecoveryDeletePorts[OwnerT]` contract,
naming live-owner cleanup, pending-delete scheduling, and notification.

The compatibility promise is unchanged. A current delete completion releases
the tracker, clears a live owner, projects an optional success notification,
and then schedules a deferred delete; failures release the tracker and project
the existing error notification. MainWindow retains recovery persistence,
filesystem operations, capture/write state, tab identity, worker dispatch,
notification policy, and close ownership. Delete service behavior, pending
state, durability, and runtime startup remain outside the boundary.

Acceptance requires recovery-delete branch/order/Ports probes, MainWindow
wiring, Qt-free/dependency audit, compile/lint/format, package identity, one
ADR, one handoff, independent review or explicit no-conclusion record,
simplification assessment, and the expected release NO-GO record. Native
delete timing, filesystem durability, runtime startup, clean-machine,
cross-machine, legal, installer/update, support, and release evidence remain
open.

### D158 / ARCH-145 — recovery-scan Ports contract

`RecoveryScanCoordinator` already owns recovery inventory callback
classification, validation, candidate prompting, empty-scan information, and
startup continuation. D158 replaces its four independent callbacks with the
frozen/slotted Qt-free `RecoveryScanPorts` contract, naming session snapshot
lookup, candidate prompt, startup continuation, and notification.

The compatibility promise is unchanged. Stale callbacks remain silent;
invalid inventories retain error projection; empty non-startup scans retain
info projection; candidates are prompted in result order; startup success and
failure continue restore with the current session snapshot. MainWindow retains
RecoveryService, scan worker, restore state, filesystem, notification, startup,
close policy, and concrete Qt ownership. Scan service behavior, candidate
model, worker dispatch, and runtime startup remain outside the boundary.

Acceptance requires recovery-scan branch/order/Ports probes, MainWindow
wiring, Qt-free/dependency audit, compile/lint/format, package identity, one
ADR, one handoff, independent review or explicit no-conclusion record,
simplification assessment, and the expected release NO-GO record. Native scan
timing, startup scheduling, filesystem behavior, clean-machine, cross-machine,
legal, installer/update, support, and release evidence remain open.

### D159 / ARCH-146 — recovery-write Ports contract

`RecoveryWriteCoordinator` already owns the callback boundary around one
recovery snapshot write. D159 replaces its five independent callbacks with
the frozen/slotted generic Qt-free `RecoveryWritePorts[JobT, OwnerT]` contract,
naming document identity, capture abort, pending-delete scheduling, saved
projection, and failed projection.

This high-risk slice keeps the exact lifecycle order. Completion remains
`consume_discarded -> complete_document -> finish_write/pending delete ->
project_saved`; failure remains `consume_discarded -> abort matching capture
when present -> complete_document -> finish_write/pending delete ->
project_failed`. Submit still binds owner/content-version/snapshot identity to
the worker callbacks. MainWindow retains recovery persistence, capture/channel
concurrency, filesystem, tab identity, worker dispatch, notification, and
close policy. No tracker state model, durability, delete service, or runtime
startup behavior moves.

The Terra and final Sol architecture windows returned no conclusion in their
bounded waits; no architecture PASS is claimed. Acceptance therefore relies
on the parent source/inline/static evidence and the explicit limits below.

Acceptance requires recovery-write branch/order/discarded/pending probes,
MainWindow wiring, Qt-free/dependency audit, compile/lint/format, package
identity, one ADR, one handoff, independent review or explicit no-conclusion
record, simplification assessment, and the expected release NO-GO record.
Native callback timing, capture/write concurrency, filesystem durability,
runtime startup, clean-machine, cross-machine, legal, installer/update,
support, and release evidence remain open.

### D160 / ARCH-147 — workspace-navigation projection Ports contract

`WorkspaceNavigationProjectionCoordinator` already owns the final projection
of validated workspace-open and directory results. D160 replaces its nine
independent callbacks with the frozen/slotted Qt-free
`WorkspaceNavigationProjectionPorts` contract, naming search invalidation,
workspace activation, search-root and directory projection, surface lookup,
opened notification, session save, restore continuation, and current-root
lookup.

The compatibility promise is unchanged. Opened results remain ordered as
`invalidate search -> activate workspace -> set search root -> surface guard ->
set directory -> notify opened -> request session save -> finish session
restore`; missing surfaces still short-circuit after the guard. Directory
results still no-op without a current root or surface and otherwise project
against the current root. MainWindow retains WorkspaceService, containment,
document/file activation, TaskRunner, surface, session persistence, startup,
close policy, and concrete Qt ownership. Workspace navigation admission,
service behavior, and runtime startup remain outside the boundary.

Acceptance requires workspace-navigation open/directory order/Ports probes,
MainWindow wiring, Qt-free/dependency audit, compile/lint/format, package
identity, one ADR, one handoff, independent review or explicit no-conclusion
record, simplification assessment, and the expected release NO-GO record.
Native event timing, file/folder activation, filesystem behavior, runtime
startup, clean-machine, cross-machine, legal, installer/update, support, and
release evidence remain open.

### D161 / ARCH-148 — plugin-host probe Ports contract

`PluginHostProbeCoordinator` already owns the diagnostic host-probe callback
classification. D161 replaces its four positional dependencies with the
frozen/slotted Qt-free `PluginHostProbePorts` contract, naming the optional
host client, plugin-operation tracker, task submitter, and notification sink.

The compatibility promise is unchanged. An absent host remains an error and
does not submit work; an in-flight probe remains a warning and does not submit
work; a new probe begins tracking, emits the existing info message, and then
binds `host.probe` through `TaskSubmitter`. Only the active operation ID may
complete. Invalid results remain errors, ready/rejected/other states retain
success/warning/error severity mapping, and worker failures remain errors.
MainWindow retains host, operation tracking, worker dispatch, notification,
process containment, and external-execution policy ownership. Host security,
plugin loading, and runtime startup remain outside the boundary.

Acceptance requires plugin-host branch/order/stale/Ports probes, MainWindow
wiring, Qt-free/dependency audit, compile/lint/format, package identity, one
ADR, one handoff, independent review or explicit no-conclusion record,
simplification assessment, and the expected release NO-GO record. Native
process timing, containment, runtime startup, clean-machine, cross-machine,
legal, installer/update, support, and release evidence remain open.

### D162 / ARCH-149 — plugin-catalog Ports contract

`PluginCatalogCoordinator` already owns metadata-only catalog scan and
descriptor-governance callback classification. D162 replaces its six
positional dependencies with the frozen/slotted Qt-free `PluginCatalogPorts`
contract, naming catalog and approval services, operation tracking, task
submission, catalog view, and notification.

The compatibility promise is unchanged. Absent services and active scan or
governance operations retain their error/warning guards. A new scan begins
tracking, emits info, and dispatches; only the active ID completes. Valid
snapshots retain summary severity and view projection, while invalid results
remain errors. Governance retains target validation, action disablement,
info-before-dispatch, stale suppression, success-notify-then-rescan ordering,
failure reenablement, and error notification. MainWindow retains catalog,
approval ledger, execution gate, TaskRunner, notification, plugin surface,
external-execution policy, and concrete Qt ownership. Metadata validation,
plugin loading, security policy, and runtime startup remain outside the
boundary.

Acceptance requires plugin-catalog scan/governance branch/order/stale/Ports
probes, MainWindow wiring, Qt-free/dependency audit, compile/lint/format,
package identity, one ADR, one handoff, independent review or explicit
no-conclusion record, simplification assessment, and the expected release
NO-GO record. Metadata filesystem timing, ledger durability, plugin security,
runtime startup, clean-machine, cross-machine, legal, installer/update,
support, and release evidence remain open.

### D165 / ARCH-152 — support handoff packet check

The release workflow already records a local support owner and issue route, but
the support handoff packet was not part of the required-file contract. D165
requires the exact `docs/support/HANDOFF.md` path in `scripts/check.ps1` and
`scripts/verify_release_handoff.ps1`, then records the boolean
`support_handoff_packet_exists` in dossier checks and evidence.

The compatibility promise is unchanged. The manifest support status remains
`handoff-pending`; the ten `$openGates` entries, `decision = no-go`, dossier
output path, and external release semantics remain unchanged. The check does
not parse packet prose, contact an owner, attest a clean machine, launch Qt or
the EXE, or create test assets.

Acceptance requires PowerShell parse and scope-invariant probes, parent review,
independent review or explicit no-conclusion, simplification assessment,
compile/lint/format, package identity, one ADR, one handoff, and the expected
release NO-GO record. Owner acceptance, clean-machine, legal, signing,
installer/update, support-channel, and release evidence remain open.

### D166 / UI-78 / ARCH-153 — font-choice preview

The Settings dialog already exposed the supported interface and editor font
families, but every option appeared as unstyled text. D166 keeps the existing
`QComboBox` controls and allowlists, then assigns each item a `QFont(family)`
through `Qt.ItemDataRole.FontRole`. The popup can therefore preview the
actual family while the selected value continues to come from `currentText()`.

The compatibility promise is unchanged. No settings schema, `UserRole`,
font allowlist, fallback policy, size range, locale refresh, theme application,
editor application, persistence, or motion behavior moves. The private helper
is presentation-only and introduces no second font policy or service boundary.

Acceptance requires the FontRole/source-shape and settings-contract probes,
AST parse, parent review, independent review or explicit no-conclusion record,
simplification assessment, compile/lint/format, package identity, one ADR,
one handoff, and the expected release NO-GO record. Native popup rendering,
installed-font metrics, DPI/accessibility, clean-machine, cross-machine,
legal, installer/update, support, and release evidence remain open.

### D167 / UI-79 / ARCH-154 — Settings typography preview

The Settings preview previously reflected only the interface font and size.
D167 adds a second editor sample and localized family/size metadata, then
connects the existing editor font and editor-size controls to the same refresh
path as theme, accent, interface typography, and locale.

Both samples receive their `QFont` and point size in
`SettingsPreviewSurface.project()`. The canonical `preview_stylesheet(colors)`
now owns only named visual tokens; it has no font-family or dynamic-size
interpolation. The settings schema, font allowlists, `UserRole`,
`currentText()`, persistence, post-save application, locale architecture,
theme policy, and motion behavior remain unchanged.

Acceptance requires QFont/sample and refresh-wiring probes, scoped-QSS and
i18n placeholder probes, AST parse, parent review, independent review or
explicit no-conclusion record, simplification assessment, compile/lint/format,
package identity, one ADR, one handoff, and the expected release NO-GO record.
Native rendering, installed-font metrics, DPI/accessibility, clean-machine,
cross-machine, legal, installer/update, support, and release evidence remain
open.

### D168 / UI-80 / ARCH-155 — Workspace entry visual semantics

The workspace tree previously relied mostly on icon shape to distinguish
files, folders, and unavailable entries. D168 keeps the existing
`WorkspacePanel` interaction and application boundaries, while making the
visual semantics easier to scan: file icons use the existing Link palette role
on dark canvases, light canvases use the primary text role for pressed-row
readability, folders retain their link-colored detail fill, and unavailable
entries retain disabled warning rendering.

The panel now projects localized English/Chinese file, folder, and unavailable
item hints. A provider diagnostic remains the tooltip whenever
`WorkspaceEntry.error` is present. `set_locale()` refreshes semantic hints and
`refresh_icons()` reprojects existing entries without changing item data,
signals, click/double-click/Enter behavior, file opening, folder navigation,
containment, busy guards, or asynchronous policy. No new theme token, QSS
selector, custom delegate, domain model, or service boundary is introduced.

Acceptance requires semantic-hint, tooltip-precedence, icon-refresh,
signal-preservation, and AST probes, a 3-theme × 4-accent contrast projection,
parent review, independent review or explicit no-conclusion record,
simplification assessment, compile/lint/format, package identity, one ADR, one
handoff, and the expected release NO-GO record. Native icon rendering,
tooltip/accessibility output, DPI, clean-machine, cross-machine,
legal/installer/update/support, and release evidence remain open.

### D169 / UI-81 / ARCH-156 — FindBar authored action icons

The Find/Replace bar previously relied on text-only action buttons while the
command rail and workspace already used the authored icon provider. D169 keeps
the existing `FindBar` projection and adds compact semantic icons for previous,
next, replace, replace-all, cancel, and close. `ARROW_DOWN` is a pure icon
contract extension with a matching painter branch; no application or domain
module imports the Qt provider.

`FindBar.refresh_icons()` resolves the current ButtonText, Link, and disabled
palette roles and is called by the existing locale refresh path. Dark canvases
retain accent-alt secondary strokes; light canvases use primary button text so
the existing pressed surface remains readable. Signals, shortcuts,
query/replacement/case state, visibility, operation-active/cancel behavior,
primary-action object-name projection, theme tokens, QSS, and application
policy remain unchanged.

Acceptance requires icon-contract/provider, mapping, signal-preservation,
locale-refresh, and AST probes, a 3-theme × 4-accent icon contrast projection,
parent review, independent review or explicit no-conclusion record,
simplification assessment, compile/lint/format, package identity, one ADR, one
handoff, and the expected release NO-GO record. Native icon painting, text/icon
metrics, accessibility, DPI, clean-machine, cross-machine,
legal/installer/update/support, and release evidence remain open.

### D170 / UI-82 / ARCH-157 — Command-rail visual hierarchy

The command rail remained functionally consistent but visually dense because
its card border, brand edge, compact controls, and context badge competed for
attention. D170 keeps `presentation.theme` as the only owner and refines the
existing `QToolBar#commandBar` and `QLabel#toolbarContext` selectors: a lighter
surface, restrained two-pixel brand edge, more breathing room, consistent
button hit height/radius, quieter quiet actions, and a compact context anchor.

`CommandSurface` remains responsible for toolbar construction, command labels,
shortcuts, callbacks, icons, role properties, and locale refresh. Normal,
hover, focus, pressed, checked, disabled, primary, quiet, and context state
selectors remain explicit. No command, signal, application policy, domain
boundary, theme token, or persistence behavior changes.

Acceptance requires the QSS contract/state probe, a 3-theme × 4-accent state
contrast projection, parent review, independent review or explicit
no-conclusion record, simplification assessment, compile/lint/format, package
identity, one ADR, one handoff, and the expected release NO-GO record. Native
Qt painting, toolbar metrics, accessibility, DPI, clean-machine,
cross-machine, legal/installer/update/support, and release evidence remain
open.

### D171 / UI-83 / ARCH-158 — Document-tab visual hierarchy

The document tab rail previously combined a framed outer container with filled
and bordered inactive tabs, creating a repeated box effect. D171 keeps
`presentation.theme` as the only owner and refines the existing
`QTabBar#documentTabBar` selectors: one restrained rail, transparent inactive
tabs, hover feedback, a raised selected surface with accent edge/bottom,
explicit focus/disabled states, and a larger consistent close target.

`DocumentTabSurface` remains responsible for tab construction/removal, title
and modified-icon projection, current-change/close signals, and identity
lookup. Tab text, icons, dirty state, close behavior, keyboard routing, and
application policy remain unchanged. No delegate, coordinator, theme token,
domain boundary, or persistence behavior changes.

Acceptance requires the document-tab QSS contract/state probe, a 3-theme ×
4-accent tab-state contrast projection, parent review, independent review or
explicit no-conclusion record, simplification assessment, compile/lint/format,
package identity, one ADR, one handoff, and the expected release NO-GO record.
Native Qt painting/layout, tab metrics, accessibility, DPI, clean-machine,
cross-machine, legal/installer/update/support, and release evidence remain
open.

### D172 / UI-84 / ARCH-159 — Editor-stage visual depth

The central editor previously presented a strong outer shell frame and a
separately framed QScintilla canvas. D172 keeps `presentation.theme` as the
only owner and softens the shell into a stage surface while making the editor
canvas the primary surface with a slightly larger radius. Existing focus,
selection, palette, font, lexer, syntax, caret, line-number, and wrapping
contracts remain unchanged.

`EditorWidget` and `EditorShellSurface` retain all editing, document, signal,
and application-policy ownership. No editor adapter, theme token, delegate,
coordinator, domain boundary, or persistence behavior changes.

Acceptance requires the editor-stage QSS contract, a 3-theme × 4-accent
shell/editor/focus/selection contrast projection, parent review, independent
review or explicit no-conclusion record, simplification assessment,
compile/lint/format, package identity, one ADR, one handoff, and the expected
release NO-GO record. Native QScintilla painting/layout, editor metrics,
accessibility, DPI, clean-machine, cross-machine, legal/installer/update/
support, and release evidence remain open.

### D173 / UI-85 / ARCH-160 — Status-rail visual hierarchy

The bottom status region previously used a strong full-width boundary while
its message and permanent shell-status rail sat close to the surrounding
surface. D173 keeps `presentation.theme` as the only owner and refines the
existing status selectors: a calmer outer status surface, distinct secondary
message/status capsules, more breathing room, and explicit phase-pill states.

`StatusSurface` and `StatusRail` remain responsible for notification text,
locale, tooltip, timer, severity, phase projection, accessible names, and
state properties. Info/success/warning/error messages and ready/working/
attention/error phases remain unchanged. No coordinator, state machine, domain
boundary, theme token, or application policy changes.

Acceptance requires the status QSS contract/state probe, a 3-theme × 4-accent
message/phase contrast projection, parent review, independent review or
explicit no-conclusion record, simplification assessment, compile/lint/format,
package identity, one ADR, one handoff, and the expected release NO-GO record.
Native Qt status-bar painting/layout, metrics, accessibility, DPI,
clean-machine, cross-machine, legal/installer/update/support, and release
evidence remain open.

### D174 / UI-86 / ARCH-161 — Workspace-dock visual hierarchy

The Workspace dock already had scoped semantic styling, but its strong frame,
thick title emphasis, and undersized native close/float controls made the
left navigation region feel heavier than the modernized command rail,
document rail, editor stage, and status rail. D174 keeps `presentation.theme`
as the only owner and refines the existing WorkspaceDock rules: a normal frame
border, a 2px accent title rail, calmer 10px title corners, more vertical
breathing room, and 20px/6px native close/float controls.

`WorkspaceSurface` retains docking, workspace resources, file/folder
activation, directory search, locale, tree selection, signals, loading/error
projection, and application policy. No workspace model, coordinator, domain
boundary, persistence, theme-token derivation, or behavior changes.

Acceptance requires the WorkspaceDock QSS contract, a 3-theme × 4-accent
Workspace title/path/tree/selection contrast projection, parent review,
independent review or explicit no-conclusion record, simplification
assessment, compile/lint/format, package identity, one ADR, one handoff, and
the expected release NO-GO record. Native Qt docking/title-button
painting/layout, metrics, accessibility, DPI, clean-machine, cross-machine,
legal/installer/update/support, and release evidence remain open.

### D175 / UI-87 / ARCH-162 — Command Palette visual hierarchy

The Command Palette already had a themed query field and selected result row,
but its result list lacked a dedicated hover state and focus boundary. Its
keyboard hint was also low-emphasis text, which weakened scan order in a
high-frequency keyboard workflow. D175 keeps `presentation.theme` as the only
owner and refines the existing Command Palette rules: a token-driven list
focus border, readable hover and selected-hover states, and a compact
secondary-surface hint capsule with a restrained accent edge.

`CommandPaletteDialog` and `CommandPaletteSurface` retain query filtering,
result ordering, current-row selection, stable-ID projection, item activation,
return-key acceptance, locale, modal lifecycle, and application policy. No
command registry, coordinator, domain boundary, persistence, theme-token
derivation, or behavior changes.

Acceptance requires the Command Palette QSS contract, a command behavior source
probe, a 3-theme × 4-accent query/hover/selected/hint contrast projection,
parent review, independent review or explicit no-conclusion record,
simplification assessment, compile/lint/format, package identity, one ADR,
one handoff, and the expected release NO-GO record. Native Qt list
painting/focus/layout, metrics, accessibility, DPI, clean-machine,
cross-machine, legal/installer/update/support, and release evidence remain
open.

### D176 / UI-88 / ARCH-163 — FindBar visual rhythm

The editor Find/Replace bar already had semantic action roles and feedback
states, but its container, field labels, case-sensitivity option, and status
message did not form a consistent visual rhythm. D176 keeps
`presentation.theme` as the only owner and refines the existing FindBar rules:
a calmer 12px card with more intentional outer spacing, secondary semibold
labels, a compact hover/focus case-option surface, and semibold status-capsule
text with vertical breathing room.

`FindBar` and `FindSurface` retain query/replacement values, Find/Replace/
Replace All/Cancel/Close signals, Enter/Shift+Enter/Esc behavior, primary and
warning action roles, locale, operation-state projection, feedback meaning,
and application policy. No editor, document, coordinator, domain boundary,
persistence, theme-token derivation, or behavior changes.

Acceptance requires the FindBar QSS contract, a behavior source probe, a
3-theme × 4-accent label/checkbox/status contrast projection, parent review,
independent review or explicit no-conclusion record, simplification
assessment, compile/lint/format, package identity, one ADR, one handoff, and
the expected release NO-GO record. Native Qt FindBar painting/layout,
metrics, accessibility, DPI, clean-machine, cross-machine,
legal/installer/update/support, and release evidence remain open.

### D177 / UI-89 / ARCH-164 — Settings guidance hierarchy

Settings already exposed language, theme, accent, font, size, motion, editor
options, and a live preview, but the apply-after-save and font-fallback notes
were plain text and visually disconnected from the surrounding card system.
D177 keeps `presentation.theme` as the only owner and refines the existing
Settings note rules: the apply note becomes an information capsule with the
alternate accent edge, and the font fallback note becomes a quieter supporting
capsule with the pink accent edge. Both reuse canonical surfaces, borders,
text, radius, and spacing tokens.

`SettingsDialog` and `SettingsPreviewSurface` retain locale, preview,
`SettingsSnapshot`, font fallback messaging, Save/Cancel, persistence, motion,
and application policy. No settings service, domain boundary, coordinator,
theme-token derivation, or behavior changes.

Acceptance requires the Settings note QSS contract, a behavior source probe,
a 3-theme × 4-accent note contrast projection, parent review, independent
review or explicit no-conclusion record, simplification assessment,
compile/lint/format, package identity, one ADR, one handoff, and the expected
release NO-GO record. Native Qt dialog layout/painting, metrics,
accessibility, DPI, clean-machine, cross-machine, legal/installer/update/
support, and release evidence remain open.

### D178 / UI-90 / ARCH-165 — Message dialog action hierarchy

Common message boxes already used the shared tokenized theme, but their
standard buttons did not consistently communicate confirmation, caution, and
dismissal intent. D178 keeps `MessageSurface` as the composition boundary and
binds Save to `primaryAction`, Discard to `warningAction`, and Cancel to
`quietAction` after the existing standard buttons are created. About and
Error dialogs explicitly expose a quiet OK action. The existing centralized
`theme.py` selectors remain the only visual owner.

The compatibility promise is unchanged: `ask_save_before_close` keeps its
Save default and `save`/`discard`/`cancel` mapping; localized about/error
content stays localized; recovery, close, notification, locale, domain, and
application policy remain outside the role mapping. No new coordinator,
palette, or state owner is introduced.

Acceptance requires the message-action source probe, existing theme-role
contract probe, behavior-preservation probe, parent review, independent
review or explicit no-conclusion record, simplification assessment,
compile/lint/format, package identity, one ADR, one handoff, and the expected
release NO-GO record. Native Qt message-box layout/painting, metrics,
accessibility, DPI, clean-machine, cross-machine, legal/installer/update/
support, and release evidence remain open.

### D179 / UI-91 / ARCH-166 — Plugin catalog guidance capsule

The plugin catalog dialog already separated its summary, entry list, and
governance actions, but its localized `dialogHint` remained loose muted text.
D179 keeps `presentation.theme` as the only visual owner and scopes a
supporting capsule to `QDialog#pluginCatalogDialog QLabel#dialogHint` using
the existing surface, border, secondary-text, radius, spacing, and pink-edge
tokens.

`PluginCatalogDialog` remains responsible for hint content, locale refresh,
word-wrap, list selection, approve/revoke signals, trust/approval/execution
policy, and application ownership. No plugin state, dynamic property,
coordinator, palette token, or behavior changes.

Acceptance requires the scoped QSS contract, PluginCatalogDialog behavior
source probe, a 3-theme × 4-accent hint contrast projection, parent review,
independent review or explicit no-conclusion record, simplification
assessment, compile/lint/format, package identity, one ADR, one handoff, and
the expected release NO-GO record. Native Qt dialog layout/painting, metrics,
accessibility, DPI, clean-machine, cross-machine, legal/installer/update/
support, and release evidence remain open.

### D180 / UI-92 / ARCH-167 — font-style settings contract

D180 completes the missing font-style path without moving settings or editor
policy. The Qt-free domain contract now supports `regular`, `semibold`,
`bold`, and `italic`; application normalization advances the settings schema
to v3 and defaults absent v1/v2 style fields to regular; the atomic JSON store
round-trips both interface and editor style values. SettingsDialog owns the
two localized controls, SettingsPreviewSurface owns the pending preview, and
`presentation.font_style` centralizes the Qt QFont/QSS projection. The existing
settings-save projection coordinator then applies the interface stylesheet and
the editor adapter applies the selected style to every live tab.

The compatibility promise is unchanged for locale, theme, accent, font family,
font size, motion, wrapping, line numbers, Save/Cancel, and application policy.
No domain Qt dependency or second styling system was introduced. ADR-0229
records the boundary and review. Native font fallback/metrics, accessibility,
DPI, GUI startup, screenshots, clean-machine, cross-machine, and release-owner
evidence remain open.

### D181 / UI-93 / ARCH-168 — status accessibility localization

D181 closes the remaining status-surface locale gap without moving lifecycle or
notification policy. The presentation catalog now contains localized names for
the shell status, workspace context, shell phase, and shell notification, plus
a localized dynamic phase-description template. `StatusRail.set_locale()`
reprojects its object names and current phase description; `StatusSurface`
reprojects the notification name beside its existing visible-message
localization. Application coordinators continue to pass stable notification
payloads and status phases.

The compatibility promise is unchanged for notification text/severity,
timers, status-phase policy, theme QSS, locale storage, and application/domain
contracts. ADR-0230 records the boundary and review. Native screen-reader
output, accessibility-tree behavior, DPI, GUI startup, screenshots,
clean-machine, cross-machine, and release-owner evidence remain open.

### D182 / UI-94 / ARCH-169 — framework-neutral theme token boundary

D182 separates pure visual resolution from Qt projection without introducing a
second styling system. `presentation.theme_tokens` now owns the immutable
`ThemeColors` and `EditorColorTokens` records, bounded theme/accent palette
data, endpoint contrast helpers, and the canonical token resolvers. It imports
no Qt or application layer. `presentation.theme` remains the only owner of
the QApplication palette, generated QSS, authored icon, and editor-adapter
projection, while re-exporting the prior public imports and retaining private
renderer aliases for source compatibility.

The compatibility promise is unchanged for theme/accent IDs, gold endpoint
readability, editor syntax fallbacks, preview tokens, QSS selectors, settings,
locale, motion, and application ownership. ADR-0231 records the boundary and
review. Native Qt painting, font fallback, DPI, accessibility, GUI startup,
screenshots, clean-machine, cross-machine, and release-owner evidence remain
open.

### D183 / UI-95 / ARCH-170 — restrained surface-gradient hierarchy

D183 adds exactly three generated QSS gradients using existing
`ThemeColors.surface_0`, `surface_1`, and `surface_2` endpoints. The gradients
are scoped to `QMainWindow#mainWindow`, `QWidget#editorShell`, and
`QToolBar#commandBar` to improve surface depth; the QScintilla editor canvas
stays solid and retains its existing selection, syntax, caret, and focus
projection. No widget callback, dynamic state, setting, asset, animation,
signal, or application policy moves.

The compatibility promise is unchanged for all theme/accent IDs, text
readability, command roles, editor behavior, locale, font, motion, and
settings. ADR-0232 records the boundary and review. Native Qt QSS
parsing/painting, DPI, screenshots, GUI startup, clean-machine, cross-machine,
and release-owner evidence remain open.

### D184 / ARCH-171 — local hash-gated distribution scripts

D184 establishes a local operational boundary around the portable QuillForge
executable. `QuillForge.Distribution.psm1` owns pure-ish path normalization and
containment checks, SHA-256 artifact validation, install-state schema access,
bounded rollback naming, and exact owned-association helpers. The four entry
points own only their command-specific sequencing: install refuses an existing
state/target, update stages and validates a replacement, rollback restores a
state-recorded copy, and uninstall removes only state-owned files and
associations that still match the recorded command.

The default target is the current user's local Programs directory. No network
source, elevation, external process, EXE launch, or machine-wide registry hive
is part of the contract. File associations require an explicit switch and
validated extension list; existing HKCU associations are refused rather than
overwritten. `ShouldProcess` provides a non-destructive preview boundary, but
the project policy has not authorized executing these scripts or touching the
registry.

The compatibility promise is limited to the portable artifact and its existing
manifest; these scripts do not by themselves close the release gates for an
installer, updater, file associations, signing, clean-machine behavior, or
cross-machine repeatability. ADR-0233 records the boundary. Static PowerShell
parsing and lexical safety probes are recorded; runtime installation,
rollback, registry, and uninstall evidence remain open.

### D185 / UI-96 / ARCH-172 — form-control highlight closure

D185 keeps the existing form-control composition and settings contract while
strengthening the visual state boundary in the centralized stylesheet. The
focus state for `QLineEdit`, `QSpinBox`, and `QComboBox` now uses the existing
`ThemeColors.surface_hover` surface alongside the existing `accent_alt`
border. ComboBox popup items now have an explicit normal, hover, and selected
projection; selected items reuse `selection`, `text_primary`, and an
`accent_alt` left rail.

No object name, current value, signal, layout, locale, font, motion, settings
persistence, or application policy changes. No second style system or widget
behavior is introduced. ADR-0234 records the bounded choice. The 3-theme ×
4-accent source projection is covered by a no-GUI QSS probe; native popup
metrics, rendering, accessibility, DPI, GUI startup, clean-machine,
cross-machine, and release-owner evidence remain open.

### D186 / UI-97 / ARCH-173 — workspace file-entry closure

D186 makes the existing document-open capability discoverable from the
workspace dock. `WorkspacePanel` owns only the button and a semantic
`file_picker_requested` signal. `WorkspaceSurface` forwards that intent
through a named callback, and `MainWindow` binds it to the existing
`_open_document` entry point. That entry point continues through
`FileDialogSurface.choose_document`, `DocumentPickerAdmissionCoordinator`,
`DocumentOpenAdmissionCoordinator`, the generic worker dispatcher, and
`DocumentService.open_document`.

The folder button, workspace tree file click/double-click/Enter activation,
loading enablement, localized labels, and workspace containment policy remain
unchanged. No second file picker, direct service access, or new application
state is introduced. ADR-0235 records the boundary. Static source/call-chain
evidence is available; native dialog behavior, GUI startup, screenshots,
clean-machine, cross-machine, and release-owner evidence remain open.

### D164 / UI-77 — status-message visual weight

The status rail already has one localized `QLabel#statusMessage` presentation
contract with explicit info, success, warning, and error state selectors. D164
adds only `font-weight: 600` to its centralized base rule so transient feedback
has a stronger human-readable visual anchor beside the status context and phase
labels.

The compatibility promise is unchanged. Status text, locale projection,
visibility, timer cleanup, tooltip, state-property updates,
foreground/background tokens, margins, padding, size policy, notification
policy, and application ownership remain unchanged. `StatusSurface` remains
the Qt lifecycle boundary; no application or domain policy moves.

Acceptance requires the base-rule scope probe, four-state selector probe, AST
parse, parent review, independent review or explicit no-conclusion record,
simplification assessment, compile/lint/format, package identity, one ADR, one
handoff, and the expected release NO-GO record. Native font fallback, DPI,
size hints, screenshot/rendering, clean-machine, cross-machine, legal,
installer/update, support, and release evidence remain open.

### D163 / ARCH-150 — recovery-capture abort Ports contract

`RecoveryCaptureAbortCoordinator` already owns classification of one capture
abort/failure callback. D163 replaces its seven positional callbacks with the
frozen/slotted generic Qt-free `RecoveryCaptureAbortPorts[JobT, OwnerT]`
contract, naming owner/document/snapshot extraction, channel lookup, session
cancellation, failure notification, and live-owner lookup while the tracker
remains a separate state owner.

The compatibility promise is unchanged. A nonmatching job remains silent and
returns false. A matching job finishes capture first; if a channel exists it
marks the snapshot discarded when the worker started or releases it when the
worker did not start, then aborts the channel. It cancels the session, completes
the document, and only then checks the notify/live policy and projects failure.
MainWindow retains recovery tracker, channel, session, tab, worker, filesystem,
and close-policy ownership. Tracker state, channel implementation, worker
dispatch, durability, and runtime startup remain outside the boundary.

Acceptance requires recovery-capture branch/order/guard/Ports probes, MainWindow
wiring, Qt-free/dependency audit, compile/lint/format, package identity, one
ADR, one handoff, independent high-risk review or explicit no-conclusion
record, simplification assessment, and the expected release NO-GO record.
Native callback interleavings, filesystem durability, runtime startup,
clean-machine, cross-machine, legal, installer/update, support, and release
evidence remain open.

### D151 / ARCH-138 — session-load Ports contract

`SessionLoadCoordinator` already owns session-load result classification and
recovery-first continuation. D151 replaces its four independent constructor
callbacks with the frozen/slotted Qt-free `SessionLoadPorts` contract,
naming the baseline setters, recovery-scan continuation, and notification
sink.

The compatibility promise is unchanged. Absent and valid results project the
loaded/default baseline and then schedule recovery scanning. Invalid results
retain the existing error notification between baseline projection and scan.
Malformed and failed results retain default-baseline projection, error
notification, and recovery-first scheduling. MainWindow retains session
service, TaskRunner, startup admission, restore tracker, notification policy,
and concrete Qt/application ownership. Session-store format, validation,
recovery scan, worker dispatch, close policy, and runtime startup remain
outside the boundary.

Acceptance requires load-result/order/Ports probes, MainWindow wiring,
Qt-free/dependency audit, compile/lint/format, package identity, one ADR, one
handoff, independent review or explicit no-conclusion record, simplification
assessment, and the expected release NO-GO record. Native Qt event timing,
startup scheduling, filesystem behavior, font/DPI, accessibility,
runtime startup, clean-machine, cross-machine, legal, installer/update,
support, and release evidence remain open.

### D152 / ARCH-139 — session-save Ports contract

`SessionSaveCoordinator` already owns latest-wins request, begin, completion,
failure, stale suppression, notification, and drain sequencing. D152 replaces
its six independent callbacks with the frozen/slotted Qt-free
`SessionSavePorts` contract, naming can-save admission, snapshot capture,
operation-ID allocation, persistence, dispatch, and notification.

The compatibility promise is unchanged. Requests still capture only when
save is allowed; the tracker still owns queued/in-flight identity; stale
callbacks remain silent; invalid and matching failed callbacks retain their
existing notifications and drain the newest queued request. MainWindow keeps
the debounce timer, SessionService, snapshot builder, TaskRunner, startup,
close, and policy ownership. Session-store format, tracker state model,
worker implementation, Qt surface, and runtime startup remain outside the
boundary.

Acceptance requires latest-wins/branch/order probes, MainWindow wiring,
Qt-free/dependency audit, compile/lint/format, package identity, one ADR, one
handoff, independent review or explicit no-conclusion record, simplification
assessment, and the expected release NO-GO record. Native timer/worker timing,
filesystem durability, runtime startup, clean-machine, cross-machine, legal,
installer/update, support, and release evidence remain open.

### D187 / UI-98 / ARCH-174 — responsive Settings scroll boundary

D187 addresses a concrete presentation gap in the existing Settings surface:
appearance controls, live preview, editor controls, guidance notes, and the
action rail were composed in one tall dialog layout. The bounded change keeps
`SettingsDialog` as the sole owner of those widgets and puts only the content
surfaces into a named resizable `QScrollArea#settingsScroll`, backed by
`QWidget#settingsContent`. The existing `QDialogButtonBox#dialogActions`
remains outside the viewport so Save and Cancel stay visible at short heights
and while larger supported fonts are selected.

The compatibility promise is unchanged. `settings_snapshot()` still reads the
same controls and values; locale refresh, preview signals, persistence,
settings application, editor projection, motion preference, and application
ownership remain untouched. `theme.py` adds only object-scoped transparent
viewport/content QSS and reuses the existing global token-driven scrollbar
contract. No new settings, coordinator, signal, or runtime policy is added.

Evidence is `D187-SETTINGS-SCROLL-PROBE=PASS`,
`D187-COMPILE-RUFF-FORMAT=PASS`, `D187-PACKAGE-BUILD=PASS`, and the
artifact-bound handoff/no-go record. Herschel the 6th / Luna max architecture
and Jason the 6th / Luna max independent review windows returned
`NO_CONCLUSION`; parent review and simplification are the only claimed PASS
conclusions. Native Qt scroll metrics, size hints, keyboard/focus traversal,
accessibility, DPI, GUI startup, clean-machine, cross-machine, and release
evidence remain open.

### D189 / UI-99 / ARCH-175 — workspace-search empty-state boundary

D189 keeps the result-state projection cohesive inside
`WorkspaceSearchDialog`. The existing result `QListWidget` and one localized
`QLabel#workspaceSearchEmpty` are composed by a single `QStackedLayout`; the
stage shows the list when rows exist and the empty state for initial, loading,
no-match, cancelled, and error conditions. The existing
`WorkspaceSearchResult` input, item path/line roles, double-click file
activation, diagnostics, cancellation, locale refresh, and MainWindow/search
service ownership remain unchanged.

The surface reuses `FeedbackLevel` and the centralized `theme.py` token/QSS
owner. No search coordinator, worker, result-model, service, filesystem, or
second styling system is introduced. Evidence is
`D189-EMPTY-STATE-CONTRACT-PROBE=PASS`, `D189-LOCALE-AND-QSS-PROBE=PASS`,
`D189-STATE-BRANCH-PROBE=PASS`, `D189-SIGNAL-ROLE-PRESERVATION-PROBE=PASS`,
and `D189-PRESENTATION-AUDIT=PASS`. Descartes the 6th / Luna max architecture
and Einstein the 6th / Luna max independent review windows returned
`NO_CONCLUSION`; parent review and simplification are the only claimed PASS
conclusions. Native stack/list rendering, accessibility, DPI, GUI startup,
clean-machine, cross-machine, legal, signing, installer, updater, support,
and release evidence remain open.

### D190 / UI-100 / ARCH-176 — Command Palette empty-state boundary

D190 keeps Command Palette result-state projection cohesive inside
`CommandPaletteDialog`. The existing `QListWidget` and one localized
`QLabel#commandPaletteEmpty` are composed by a single `QStackedLayout`; the
stage shows the list when commands match and explicit initial or filtered
no-match copy when no rows exist. Command iteration/order, stable command ID
roles, current-row selection, Enter/item activation, Esc close, locale handoff,
and MainWindow/CommandRegistry execution ownership remain unchanged.

The surface adds no command registry change, execution coordinator, filtering
policy, or second styling system; QSS remains in `presentation.theme`. Evidence
is `D190-EMPTY-STATE-CONTRACT-PROBE=PASS`,
`D190-LOCALE-AND-QSS-PROBE=PASS`,
`D190-KEYBOARD-EXECUTION-PRESERVATION-PROBE=PASS`, and
`D190-PRESENTATION-AUDIT=PASS`. Poincare the 6th / Luna max architecture and
Laplace the 6th / Luna max independent review windows returned
`NO_CONCLUSION`; parent review and simplification are the only claimed PASS
conclusions. Native stack/list rendering, accessibility, DPI, GUI startup,
clean-machine, cross-machine, legal, signing, installer, updater, support,
and release evidence remain open.

### D191 / UI-101 / ARCH-177 — dynamic Find/Replace status localization closure

D191 keeps count-bearing Find/Replace message grammar cohesive inside
`presentation.i18n`. Strict matchers recognize only the existing plural result
and bounded limit shapes, then delegate to the established
`find.status.replaced_count` and `find.status.limit` catalog entries. English
identity, count text, unknown-shape fallback, and the existing exact/progress
messages remain preserved. The change adds no locale service, editor worker,
Replace All policy, signal, status-level, persistence, coordinator, or second
styling system. Evidence is `D191-I18N-MATCHER-PROBE=PASS`,
`D191-PRESENTATION-AUDIT=PASS`, and `D191-COMPILE-RUFF-FORMAT=PASS`.
Linnaeus the 6th / Luna max architecture and Ampere the 6th / Luna max
independent review windows returned `NO_CONCLUSION`; parent review and
simplification are the only claimed PASS conclusions. Native text metrics,
accessibility, DPI, GUI startup, clean-machine, cross-machine, legal,
signing, installer, updater, support, and release evidence remain open.

### D192 / UI-102 / ARCH-178 — plugin failure phase localization closure

D192 keeps stable plugin failure phase translation cohesive inside the existing
`_localize_plugin_failure` adapter in `presentation.i18n`. The bounded map
recognizes `activate`, `deactivate`, `command`, and `event`; unknown phases,
plugin IDs, and diagnostic details remain unchanged. The change adds no plugin
manager/event-bus change, runtime enablement, trust or containment policy,
command refresh behavior, notification severity, coordinator, locale service,
or second styling system. Evidence is `D192-PLUGIN-PHASE-I18N-PROBE=PASS`,
`D192-PRESENTATION-AUDIT=PASS`, and `D192-COMPILE-RUFF-FORMAT=PASS`.
Bohr the 6th / Luna max architecture and Ohm the 6th / Luna max independent
review windows returned `NO_CONCLUSION`; parent review and simplification are
the only claimed PASS conclusions. Native text metrics, accessibility, DPI,
GUI startup, clean-machine, cross-machine, legal, signing, installer,
updater, support, and release evidence remain open.

### D193 / UI-103 / ARCH-179 — close-guard pending feedback localization

D193 keeps the count-bearing close-block message cohesive inside
`presentation.i18n`. A strict matcher recognizes the existing pending-work
shape and delegates to the new `error.wait_pending` English/Simplified Chinese
catalog entry; English identity, count text, and unknown-shape fallback remain
preserved. The change adds no CloseGuard, TaskRunner, session-save, worker
drain, shutdown, coordinator, locale-service, or styling policy. Evidence is
`D193-CLOSE-GUARD-I18N-PROBE=PASS`, `D193-PRESENTATION-AUDIT=PASS`, and
`D193-COMPILE-RUFF-FORMAT=PASS`. Raman the 6th / Luna max architecture and
Aquinas the 6th / Luna max independent review windows returned
`NO_CONCLUSION`; parent review and simplification are the only claimed PASS
conclusions. Native text metrics, accessibility, DPI, GUI startup,
clean-machine, cross-machine, legal, signing, installer, updater, support,
and release evidence remain open.

## Non-goals

- Do not introduce a microservice framework, network RPC, service locator,
  dependency-injection package, global singleton, or speculative event bus.
- Do not rewrite `MainWindow`, move all files at once, or change public plugin
  behavior in the first slice.
- Do not claim ByteDance compliance, enterprise certification, or release
  readiness from this architecture baseline.

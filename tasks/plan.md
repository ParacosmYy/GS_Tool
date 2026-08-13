# Implementation Plan: QuillForge continuation

## Overview

Continue the editor toward the complete project delivery by landing small, evidence-backed slices. D105/ARCH-78 consolidated recovery-write callback binding, and D106/UI-54 restored the horizontal scrollbar visual path without changing scroll behavior.

## Completed bounded slice — D315 / UI-133 / ARCH-285 Readable checkbox indicator states

- [x] User outcome: make checked, checked-hover, focused, and checked-disabled
  checkbox indicators readable across all supported themes and accents,
  including the amber/砂金 palette.
- [x] Non-goals: no checkbox signal/state model, keyboard behavior,
  accessibility semantics, Settings roles, persistence, locale, editor,
  layout, startup, or application changes.
- [x] Compatibility promise: retain Qt's native check mark and interaction,
  reuse readable-edge fallback tokens, and apply the shared rule to existing
  ordinary and Settings behavior-toggle checkboxes.
- [x] Evidence: source contract, 3-theme/4-accent matrix, offscreen render
  probe, compile/Ruff/format, presentation audit, source diagnostic, package
  identity, PE/archive, ADR, reviews, handoff, and expected release NO-GO.

## Completed bounded slice — D314 / UI-132 / ARCH-284 Theme-aware combo-box expanded affordance

- [x] User outcome: make language, theme, accent, UI-font, editor-font, and
  font-style combo boxes clearly indicate their popup-open state with the
  selected theme's surface and accent boundary.
- [x] Non-goals: no combo-box item data, popup model, keyboard navigation,
  settings roles, locale catalog, persistence, editor policy, layout ownership,
  startup, or application changes.
- [x] Compatibility promise: keep native arrow and popup semantics, position
  the shared drop-down subcontrol explicitly, and reuse the centralized theme
  owner across all 3 themes and 4 accents.
- [x] Evidence: source contract, 3-theme/4-accent expanded-state matrix, QSS
  probe, compile/Ruff/format, presentation audit, source diagnostic, package
  identity, PE/archive, ADR, reviews, handoff, and expected release NO-GO.

## Completed bounded slice — D313 / UI-131 / ARCH-283 Theme-aware typography steppers

- [x] User outcome: make the Settings interface-font and editor-font size
  steppers compact, theme-aware, and visibly highlighted in normal, hover,
  pressed, and disabled states.
- [x] Non-goals: no QSpinBox range, keyboard stepping, persistence, settings
  schema, locale, editor policy, layout ownership, startup, or application
  changes.
- [x] Compatibility promise: scope Qt's documented up/down subcontrols to the
  existing `typographyChoice` role, retain native arrows and value semantics,
  and reuse the centralized theme owner.
- [x] Evidence: source contract, 3-theme/4-accent stepper matrix, QSS probe,
  compile/Ruff/format, presentation audit, source diagnostic, package
  identity, PE/archive, ADR, reviews, handoff, and expected release NO-GO.

## Completed bounded slice — D312 / UI-130 / ARCH-282 Theme-aware scrollbars

- [x] User outcome: make vertical and horizontal scrollbars compact,
  theme-aware, and visibly highlighted on hover/press.
- [x] Non-goals: no scrollbar range/page logic, editor behavior, widget
  callbacks, layout ownership, locale, settings, persistence, startup, or
  plugin changes.
- [x] Compatibility promise: reuse the existing `_stylesheet()` owner and
  readable-edge helper; preserve native wheel, keyboard, page, and drag
  semantics while hiding only arrow-line controls.
- [x] Evidence: source contract, 3-theme/4-accent state matrix, QSS probe,
  compile/Ruff/format, presentation audit, source diagnostic, package
  identity, PE/archive, ADR, reviews, handoff, and expected release NO-GO.

## Completed bounded slice — D311 / UI-129 / ARCH-281 Theme-aware shell separators

- [x] User outcome: make main-window and dock-panel split boundaries readable,
  theme-aware, and visibly highlighted on hover.
- [x] Non-goals: no widget logic, layout ownership, editor behavior, locale,
  settings, persistence, startup, file-open, or plugin changes.
- [x] Compatibility promise: reuse the existing `_stylesheet()` owner,
  readable edge helper, shell surfaces, and `accent_alt`; all existing widget
  and application boundaries remain unchanged.
- [x] Evidence: source contract, 3-theme/4-accent separator matrix,
  compile/Ruff/format, presentation audit, package identity, PE/archive, ADR,
  reviews, handoff, and expected release NO-GO.

## Completed bounded slice — D310 / ARCH-280 Safe startup recovery path

- [x] User outcome: provide `QuillForge.exe --safe-mode` as an explicit
  recovery launch when normal persisted state, recovery, or built-in plugin
  activation prevents the shell from appearing.
- [x] Non-goals: no normal startup order, settings/session schema, recovery
  deletion, plugin policy, locale, widget styling, or release-gate changes.
- [x] Compatibility promise: the flag is stripped before Qt parsing, normal
  startup is unchanged when absent, and explicit file paths remain available.
- [x] Evidence: safe-mode source probe and composition probe, source startup
  and file-open diagnostics, compile/Ruff/format/audit, package identity,
  PE/archive, ADR, reviews, handoff, and expected release NO-GO.

## Completed bounded slice — D309 / UI-128 / ARCH-279 Theme-aware Tooltip surface

- [x] User outcome: make every Tooltip share the selected theme's surface,
  typography, border, readable foreground, and accent edge so localized
  workspace hints remain modern and legible.
- [x] Non-goals: no Tooltip text, locale catalog, widget behavior, business
  logic, Settings persistence, animation policy, or application ownership
  changes.
- [x] Compatibility promise: the existing centralized `_stylesheet()` owner,
  UI font family, locale flow, and D308 action behavior remain unchanged; only
  the shared Tooltip visual contract is extended.
- [x] Evidence: Tooltip source probe, 3-theme/4-accent contrast matrix,
  compile/Ruff/format, presentation audit, package identity, PE/archive,
  ADR, reviews, handoff, and expected release NO-GO.

## Completed bounded slice — D308 / UI-127 / ARCH-278 Workspace open-action affordance

- [x] User outcome: make the Workspace folder picker and direct file picker
  explicit through semantic roles, localized hints, and separate visual edges.
- [x] Non-goals: no file picker, workspace containment, tree activation,
  asynchronous open, Settings, persistence, MainWindow, or application
  ownership changes.
- [x] Compatibility promise: existing folder/file signals, click/double-click/
  Enter routing, native picker, and document-open admission remain unchanged;
  only presentation metadata and centralized QSS are extended.
- [x] Evidence: four-action source probe, 3-theme/4-accent edge matrix,
  compile/Ruff/format, presentation audit, source startup/file-open
  diagnostics, package identity, PE/archive, ADR, review, handoff, and
  expected release NO-GO.

## Completed bounded slice — D307 / UI-126 / ARCH-277 Settings unsaved-draft status

- [x] User outcome: show a localized clean/changed draft status in Settings by
  comparing the current snapshot with the values captured when the dialog opens.
- [x] Non-goals: no persistence, schema, SettingsService, MainWindow, locale
  architecture, animation policy, or application ownership changes.
- [x] Compatibility promise: RestoreDefaults, Save, Cancel, preview, locale,
  accessibility, and existing control signals retain their boundaries; the
  status is presentation-only and is not persisted.
- [x] Evidence: source contract, 3-theme/4-accent clean/changed matrix,
  compile/Ruff/format, presentation audit, source startup/file-open
  diagnostics, package identity, PE/archive, ADR, review, handoff, and
  expected release NO-GO.

## Completed bounded slice — D306 / UI-125 / ARCH-276 Settings draft restore defaults

- [x] User outcome: let users restore all current Settings choices to the
  immutable product defaults without silently persisting or closing the dialog.
- [x] Non-goals: no schema, SettingsService, persistence, MainWindow, locale
  architecture, animation policy, or application ownership changes.
- [x] Compatibility promise: Save remains the only persistence boundary, Cancel
  remains the discard boundary, and existing preview/accessibility/locale paths
  are reused after the signal-blocked batch reset.
- [x] Evidence: 12-control source probe, 3-theme/4-accent reset matrix,
  compile/Ruff/format, presentation audit, source diagnostics, package
  identity, PE/archive, ADR, review, handoff, and expected release NO-GO.

## Completed bounded slice — D305 / UI-124 / ARCH-275 Localized Settings accessible names

- [x] User outcome: give the nine Settings value controls and three behavior
  controls localized accessibility names that refresh with the active locale.
- [x] Non-goals: no new translation keys, locale values, settings schema,
  persistence, signals, preview semantics, keyboard routing, or application
  ownership changes.
- [x] Compatibility promise: existing visible text, item data, current values,
  SettingsSnapshot, persistence, and locale refresh remain unchanged; the
  accessibility names are presentation-only and not persisted.
- [x] Evidence: AST source probe, compile/Ruff/format, presentation audit,
  source diagnostics, package identity, PE/archive, ADR, review, handoff, and
  expected release NO-GO record.

## Completed bounded slice — D304 / UI-123 / ARCH-274 Locale choice role

- [x] User outcome: give the Settings language selector one semantic localeChoice
  role with explicit normal, hover, focus, open, and disabled hierarchy.
- [x] Non-goals: no locale values, item data, translations, schema, persistence,
  signals, set_locale behavior, keyboard routing, or application ownership
  changes.
- [x] Compatibility promise: existing language object name, item data,
  currentData/currentIndex, locale refresh, snapshot, labels, signals, and
  keyboard behavior remain unchanged.
- [x] Evidence: semantic-role source contract, QSS state/legacy-selector audit,
  3-theme × 4-accent matrix, compile/Ruff/format, source diagnostics, package
  identity, ADR, review, handoff, and expected release NO-GO record.

## Completed bounded slice — D303 / UI-122 / ARCH-273 Behavior toggle role

- [x] User outcome: make Settings wrapping, line-number, and motion toggles
  read as one coherent behavior-control group with distinct editor/interface
  tone cues and explicit checked/focus/disabled hierarchy.
- [x] Non-goals: no settings schema/values, persistence, locale, editor
  projection, motion policy, signals, keyboard routing, or application
  ownership changes.
- [x] Compatibility promise: existing checkbox object names, checked values,
  labels, signals, SettingsSnapshot reads, persistence, and generic indicator
  states remain unchanged.
- [x] Evidence: semantic-role source contract, QSS state/legacy-selector audit,
  3-theme × 4-accent matrix, compile/Ruff/format, source diagnostics, package
  identity, ADR, review, handoff, and expected release NO-GO record.

## Completed bounded slice — D302 / UI-121 / ARCH-272 Typography choice role

- [x] User outcome: make interface/editor font family, size, and style controls
  visibly belong to one typography choice system while retaining distinct
  interface/editor accent cues.
- [x] Non-goals: no settings schema/value ranges, persistence, locale, font
  discovery, editor projection, preview, signals, keyboard routing, or
  application ownership changes.
- [x] Compatibility promise: existing typography object names, item data,
  font previews, currentText/currentData/value reads, signals, disabled state,
  and SettingsSnapshot remain unchanged.
- [x] Evidence: semantic-role source contract, QSS state/legacy-selector audit,
  3-theme × 4-accent matrix, compile/Ruff/format, source diagnostics, package
  identity, ADR, review, handoff, and expected release NO-GO record.

## Completed bounded slice — D301 / UI-120 / ARCH-271 Semantic settings choice role

- [x] User outcome: retain D300's visible theme/accent hierarchy while making
  the style boundary semantic and extensible through one `identityChoice`
  presentation role.
- [x] Non-goals: no settings values/schema, persistence, locale, theme
  resolution, preview, keyboard routing, signals, snapshot, or application
  ownership changes.
- [x] Compatibility promise: `settingsTheme`/`settingsAccent` object names,
  item order/data, icons, signals, and all existing generic/disabled QSS states
  remain available.
- [x] Evidence: semantic-property source contract, AST/QSS scope probe,
  contrast matrix, compile/Ruff/format, source diagnostics, package identity,
  ADR, review, handoff, and expected release NO-GO record.

## Completed bounded slice — D300 / UI-119 Settings choice hierarchy

- [x] User outcome: make the Settings theme and accent selectors visibly
  distinct from typography selectors, with a persistent semantic edge and a
  stronger selected/focus state in the centralized QSS.
- [x] Non-goals: no settings values, persistence/schema, locale, theme
  resolution, preview data, keyboard routing, widget ownership, or runtime
  startup changes.
- [x] Compatibility promise: existing combo item order/data, icons, current
  values, signals, and `settings_snapshot()` behavior remain unchanged.
- [x] Evidence: QSS/object-name source contract, contrast matrix, compile/Ruff/
  format, source diagnostics, package identity, ADR, review, handoff, and
  expected release NO-GO record.

## Completed bounded slice — D299 / ARCH-269 Frozen Qt plugin root selection

- [x] Audit the frozen Qt6/legacy plugin roots and identify the partial-Qt6
  directory masking a complete fallback root.
- [x] Call the Architect role; record bounded `NO_CONCLUSION` after three
  unanswered waits.
- [x] Reuse one qwindows-bearing selector for frozen configuration and startup
  diagnostics, with a targeted AST-backed static contract.
- [x] Perform parent review and behavior-preserving simplification assessment;
  revise the static contract after the first independent `REVISE` finding.
- [x] Run source diagnostics, selector simulation, static/project checks,
  rebuild the portable candidate, and inspect PE/archive identity.
- [x] Refresh D299 handoff/register/acceptance records and run final project
  and release verification.

## Completed bounded slice — D298 / ARCH-268 Frozen Qt layout fallback

- [x] Audit PyInstaller's Qt6-first/Qt fallback hook against QuillForge plugin
  and runtime dependency discovery.
- [x] Call the Architect role; record bounded `NO_CONCLUSION` after three
  unanswered waits.
- [x] Update the existing dependency inventory to accept a complete `Qt6/bin`
  or legacy `Qt/bin` candidate, preserve the report shape, and add a static
  two-layout contract.
- [x] Perform parent review and behavior-preserving simplification assessment;
  independent review returned bounded `NO_CONCLUSION` and was recorded.
- [x] Re-run source diagnostics/current-layout report, rebuild the portable
  candidate, inspect PE/archive identity, and refresh delivery evidence.
- [x] Refresh D298 handoff/register/acceptance records and run final project and
  release verification.

## Completed bounded slice — D297 / ARCH-267 Frozen Qt runtime preflight

- [x] Audit normal entry ordering, existing frozen Qt dependency checks, and
  the `__main__` startup error boundary.
- [x] Call the Architect role; record bounded `NO_CONCLUSION` after three
  unanswered waits.
- [x] Add a frozen-only pre-QApplication fail-fast check after plugin-host and
  diagnostic dispatch, reusing existing dependency inventories and emitting
  actionable missing paths; add a static contract.
- [x] Perform parent review and behavior-preserving simplification assessment;
  independent review returned bounded `NO_CONCLUSION` and was recorded.
- [x] Run source diagnostics and no-Qt frozen success/failure simulation;
  rebuild the portable candidate and inspect PE/archive identity.
- [x] Refresh D297 handoff/register/acceptance records and run final project and
  release verification.

## Completed bounded slice — D296 / ARCH-266 Frozen Qt plugin path binding

- [x] Audit the frozen PyInstaller/PyQt6 runtime hook, archive layout, entry
  import ordering, and existing startup diagnostics.
- [x] Call the Architect role; record bounded `NO_CONCLUSION` after three
  unanswered waits.
- [x] Bind frozen Qt plugin and platform-plugin paths to the current extracted
  bundle before QApplication import, with source-mode no-op behavior and a
  targeted static contract.
- [x] Perform parent review and behavior-preserving simplification assessment;
  independent review returned bounded `NO_CONCLUSION` and was recorded.
- [x] Rebuild the portable candidate, inspect PE/archive identity, refresh
  handoff records, and run project/source/release verification (expected
  no-go).

## Completed bounded slice — D295 / ARCH-265 TaskRunner abnormal termination boundary

- [x] Audit `_Task.run()`, all Exception-typed failure callback families,
  MessageSurface/localization projection, and pending-task release.
- [x] Call the Architect role; record bounded `NO_CONCLUSION` after three
  unanswered waits.
- [x] Normalize non-`Exception` worker termination into a private Exception
  wrapper while preserving coordinator interfaces, completion signaling, and
  task release; add a targeted contract.
- [x] Perform parent review and behavior-preserving simplification assessment;
  independent review returned bounded `NO_CONCLUSION` and was recorded.
- [x] Rebuild the portable candidate, inspect PE/archive identity, refresh
  handoff records, and run project/source/release verification (expected
  no-go).

## Completed bounded slice — D294 / ARCH-264 startup diagnostic QApplication lifecycle

- [x] Reproduce the repeated Qt window-class warning and distinguish it from
  the normal one-application desktop entrypoint.
- [x] Call the Architect role; record bounded `NO_CONCLUSION` after three
  unanswered waits.
- [x] Keep one diagnostic-owned QApplication across runtime and restore probes,
  clean it in `finally`, and add a targeted lifecycle contract without changing
  normal `app.main()` or masking the platform with an internal override.
- [x] Perform parent review and behavior-preserving simplification assessment;
  independent review returned bounded `NO_CONCLUSION` and was recorded.
- [x] Rebuild the portable candidate, inspect PE/archive identity, refresh
  handoff records, and run project/source/release verification (expected
  no-go).

## Completed bounded slice — D293 / ARCH-263 TaskRunner submission rollback boundary

- [x] Audit `TaskRunner.submit()`, all dispatch callers, completion delivery,
  pending-state projection, and the D292 shutdown boundary.
- [x] Call the Architect role; record bounded `NO_CONCLUSION` after three
  unanswered waits.
- [x] Roll back retained tasks when Qt signal connection or pool start raises,
  and centralize idempotent release without changing callback ordering or
  forcing worker termination.
- [x] Perform parent review and behavior-preserving simplification assessment;
  independent review returned bounded `NO_CONCLUSION` and was recorded.
- [x] Rebuild the portable candidate, inspect PE/archive identity, refresh
  handoff records, and run project/source/release verification (expected
  no-go).

## Completed bounded slice — D292 / ARCH-262 runtime shutdown activity boundary

- [x] Audit `DesktopRuntime.stop()`, MainWindow timers, TaskRunner ownership,
  and close-guard behavior after the D291 entrypoint cleanup.
- [x] Call the Architect role; record bounded `NO_CONCLUSION` when no result
  returned.
- [x] Add a presentation-owned timer-stop port and require shutdown ordering
  through a targeted AST contract without forceful worker termination.
- [x] Perform parent review and behavior-preserving simplification assessment;
  independent review returned bounded `NO_CONCLUSION` and was recorded.
- [x] Rebuild the portable candidate, inspect PE/archive identity, refresh
  handoff records, and run project/release verification (expected no-go).

## Completed bounded slice — D291 / ARCH-261 entrypoint runtime cleanup

- [x] Confirm the normal entrypoint left `runtime.start()` outside the
  existing `try/finally` cleanup boundary.
- [x] Call the Architect role; record bounded `NO_CONCLUSION` when no result
  returned.
- [x] Move only `runtime.start()` into the existing cleanup boundary and add a
  targeted AST lifecycle contract.
- [x] Perform parent review and behavior-preserving simplification assessment;
  independent review returned bounded `NO_CONCLUSION` and was recorded.
- [x] Rebuild the portable candidate, inspect PE/archive identity, refresh
  handoff records, and run the release verifier (expected no-go).

## Completed bounded slice — D290 / ARCH-260 startup preflight wait refinement

- [x] Measure the D289 source file-open preflight and confirm the event-batch
  busy-spin.
- [x] Call the Architect role; record bounded `NO_CONCLUSION` when no result
  returned.
- [x] Add a short-lived Qt wake timer and `WaitForMoreEvents` without changing
  the normal startup path or report contract.
- [x] Perform parent review and behavior-preserving simplification assessment;
  initial independent `REVISE` findings addressed and follow-up
  `NO_CONCLUSION` recorded.
- [x] Rebuild the portable candidate, inspect PE/archive identity, refresh
  handoff records, and run the release verifier (expected no-go).

## Completed bounded slice — D289 / ARCH-259 file-open preflight

- [x] Inspect explicit launch paths, workspace file activation, and async
  document-open admission.
- [x] Call the Architect role; record bounded `NO_CONCLUSION` when no result
  returned.
- [x] Add a focused regular-file diagnostic that reuses the production startup
  path queue and no-window completion boundary.
- [x] Perform parent review and behavior-preserving simplification assessment;
  independent reviewer outcome recorded.
- [x] Rebuild the portable candidate, inspect PE/archive identity, refresh
  handoff records, and run the release verifier (expected no-go).

## Completed bounded slice — D288 / ARCH-258 startup restore preflight

- [x] Inspect MainWindow recovery chain, TaskRunner completion, and timer/data
  side effects.
- [x] Call the Architect role; record bounded `NO_CONCLUSION` when no result
  returned.
- [x] Add a bounded production restore preflight, skip modal recovery prompts,
  and guard the no-window/no-write boundary.
- [x] Perform parent review and behavior-preserving simplification assessment;
  independent reviewer outcome recorded.
- [x] Rebuild the portable candidate, inspect PE/archive identity, refresh
  handoff records, and run the release verifier (expected no-go).

## Completed bounded slice — D287 / ARCH-257 startup state preflight

- [x] Inspect session/recovery storage and startup restore call chain.
- [x] Call the Architect role; record bounded `NO_CONCLUSION` when no result
  returned.
- [x] Add read-only `session_preflight` and `recovery_preflight` probes and
  static contract coverage.
- [x] Perform parent review and behavior-preserving simplification assessment;
  independent reviewer outcome recorded.
- [x] Rebuild the portable candidate, inspect PE/archive identity, refresh
  handoff records, and run the release verifier (expected no-go).

## Completed bounded slice — D286 / ARCH-256 editor-shell startup preflight

- Reuse the production first-document/editor/QScintilla construction path in
  the no-window startup diagnostic.
- Stop recovery and session-save timers in a `finally` boundary without
  changing normal session/recovery restore order or writing user data.
- Evidence: `D286-EDITOR-SHELL-CONTRACT=PASS`, source diagnostic, presentation
  audit, compileall, Ruff, formatting, project check, PE/archive, package
  identity, and handoff records.
- Review: parent `PASS`; simplification `PASS`; Averroes architecture and
  Sartre independent Luna/max windows returned `NO_CONCLUSION` after bounded
  waits.
- Safety boundary: no EXE/Qt launch, native dialog, updater/installer/
  registry operation, unit-test asset, or worktree was executed or created.

## Completed bounded slice — D285 / ARCH-255 startup preparation preflight

- Share `DesktopRuntime` pre-show plugin preparation and command refresh
  between normal startup and the no-window diagnostic.
- Preserve the original normal order around session restore, window display,
  and explicit startup paths; keep asynchronous restore out of the probe.
- Evidence: `D285-STARTUP-PREPARATION-CONTRACT=PASS`, source diagnostic,
  presentation audit, compileall, Ruff, formatting, project check, PE/archive,
  package identity, and handoff records.
- Review: parent `PASS`; simplification `PASS`; Hilbert architecture and Gauss
  independent Luna/max windows returned `NO_CONCLUSION` after bounded waits.
- Safety boundary: no EXE/Qt launch, native dialog, updater/installer/
  registry operation, unit-test asset, or worktree was executed or created.

## Completed bounded slice — D284 / ARCH-254 startup runtime-composition preflight

- Extend the existing no-window startup diagnostic from import/dependency
  checks to full `DesktopRuntime`/`MainWindow` constructor coverage.
- Keep composition ownership in `build_desktop_runtime`; the probe owns only a
  temporary QApplication and never shows a window or enters the event loop.
- Evidence: `D284-RUNTIME-COMPOSITION-CONTRACT=PASS`, source diagnostic,
  presentation audit, compileall, Ruff, formatting, project check, PE/archive,
  package identity, and handoff records.
- Review: parent `PASS`; simplification `PASS`; Chandrasekhar architecture and
  Halley independent Luna/max windows returned `NO_CONCLUSION` after bounded
  waits.
- Safety boundary: no EXE/Qt launch, native dialog, updater/installer/
  registry operation, unit-test asset, or worktree was executed or created.

## Completed bounded slice — D283 / ARCH-253 startup-error log lifecycle

- Clear only the previous QuillForge startup-error log at the `main()` entry
  boundary so a historical exception cannot be mistaken for a current crash.
- Preserve the existing fail-open startup recorder for the current exception,
  and add a Qt-free lifecycle contract for cleanup-before-dispatch.
- Evidence: `D283-STARTUP-LOG-CONTRACT=PASS`, stale-log clear, presentation
  audit, source startup diagnostic, compileall, Ruff, formatting, project
  check, PE/archive, package identity, and refreshed release handoff.
- Review: parent `PASS`; simplification `PASS`; Bohr the 7th architecture and
  Banach the 7th independent Luna/max windows returned `NO_CONCLUSION` after
  bounded waits.
- Safety boundary: no EXE/Qt launch, native dialog, updater/installer/
  registry operation, unit-test asset, or worktree was executed or created.

## Completed bounded slice — D282 / ARCH-252 MainWindow startup-state initialization

- Move the existing `_busy = False` default beside the startup state before
  coordinator/callback wiring, preserving all later runtime transitions.
- Add a Qt-free AST/source contract that prevents a future callback from
  capturing the state before its default exists.
- Evidence: `D282-STARTUP-STATE-CONTRACT=PASS`, presentation audit, source
  startup diagnostic, compileall, Ruff, formatting, project check, PE/archive,
  package identity, and refreshed release handoff.
- Review: parent `PASS`; simplification `PASS`; Gibbs the 7th architecture and
  Sagan the 7th independent Luna/max windows returned `NO_CONCLUSION` after
  bounded waits.
- Safety boundary: no EXE/Qt launch, native dialog, updater/installer/
  registry operation, unit-test asset, or worktree was executed or created.

## Completed bounded slice — D281 / ARCH-251 startup diagnostic user guidance

- Document the existing `--diagnose-startup --report` command, its
  `settings_preflight` metadata, exit codes, startup-error log, and explicit
  no-native-startup boundary in the README.
- Keep the change documentation-only; the D280 portable candidate identity is
  unchanged.
- Evidence: `D281-README-PREFLIGHT=PASS`, source startup diagnostic,
  compileall, Ruff, formatting, project check, and review/handoff records.
- Review: parent `PASS`; simplification `PASS`; Lagrange the 7th architecture
  and Bernoulli the 7th independent Luna/max windows returned `NO_CONCLUSION`
  after bounded waits.
- Safety boundary: no EXE/Qt launch, native dialog, updater/installer/
  registry operation, unit-test asset, or worktree was executed or created.

## Completed bounded slice — D280 / ARCH-250 startup settings preflight

- Extend the existing no-window startup diagnostic with a Qt-free settings
  preflight that reports the local path, file presence, decode validity, and
  normalized schema version without exposing preference values or writing data.
- Reuse the existing `JsonSettingsStore` decoder and `normalize_settings`
  policy so the report follows the same safe fallback contract as startup.
- Evidence: `D280-SETTINGS-PREFLIGHT=PASS file_present=true valid=true schema=3`,
  source startup diagnostic, compileall, Ruff, formatting, project check,
  PE/archive inspection, package identity, and refreshed release handoff.
- Review: parent `PASS`; simplification `PASS`; Kuhn the 7th architecture and
  Archimedes the 7th independent Luna/max windows returned `NO_CONCLUSION`
  after bounded waits.
- Safety boundary: no EXE/Qt launch, native dialog, updater/installer/
  registry operation, unit-test asset, or worktree was executed or created.

## Completed bounded slice — D279 / ARCH-249 font availability status projection

- Show localized installed/fallback/unknown status for the selected interface
  and editor font families in SettingsDialog.
- Keep status derived in presentation; preserve raw family values, schema,
  persistence, and editor/theme application behavior.
- Evidence: `D279-FONT-STATUS-LOCALIZATION=PASS`,
  `D279-FONT-PERSISTENCE-CONTRACT=PASS`, presentation audit, compileall, Ruff,
  formatting, source startup diagnostic, PE/archive, package identity, and
  refreshed release handoff records.
- Review: parent `PASS`; simplification `PASS`; Popper the 7th architecture
  and Curie the 7th independent Luna/max windows returned `NO_CONCLUSION`
  after bounded waits.
- Safety boundary: no EXE/Qt launch, native dialog, updater/installer/
  registry operation, unit-test asset, or worktree was executed or created.

## Completed bounded slice — D278 / ARCH-248 command-surface startup contract

- Record the historical startup root cause: `CommandSurface.create_menus()`
  previously reached a missing `_locale` member during shell construction.
- Keep the existing D225 locale-provider/accessor fix and add a Qt-free audit
  for provider storage, accessor delegation, and MainWindow startup ordering.
- Evidence: `D278-STARTUP-SURFACE-CONTRACT=PASS`, presentation audit,
  compileall, Ruff, formatting, source startup diagnostic, PE/archive,
  package identity, and refreshed release handoff records.
- Review: parent `PASS`; simplification `PASS`; Nietzsche the 7th architecture
  and Kepler the 7th independent Luna/max windows returned `NO_CONCLUSION`
  after bounded waits.
- Safety boundary: no EXE/Qt launch, native dialog, updater/installer/
  registry operation, unit-test asset, or worktree was executed or created.

## Completed bounded slice — D277 / ARCH-247 settings typography contract audit

- Add a Qt-free end-to-end contract for locale, theme/accent, interface and
  editor font family, size, style, persistence, live projection, and motion.
- Preserve SettingsSnapshot validation, schema compatibility, save projection
  order, QSS/editor application, and animation policy.
- Evidence: `D277-TYPOGRAPHY-CONTRACT=PASS routes=9`, presentation audit,
  compileall, Ruff, formatting, project check, PE/archive, package identity,
  and refreshed release handoff records.
- Review: parent `PASS`; simplification `PASS`; Dirac the 7th architecture
  and Mencius the 7th independent Luna/max windows returned `NO_CONCLUSION`
  after bounded waits.
- Safety boundary: no EXE/Qt launch, native dialog, updater/installer/
  registry operation, unit-test asset, or worktree was executed or created.

## Completed bounded slice — D276 / ARCH-246 workspace file-open contract audit

- Add a Qt-free source contract that keeps the file-vs-folder intent split
  connected from the workspace tree through the surface and activation
  coordinator into asynchronous document opening.
- Preserve click/double-click/Enter behavior, containment, busy/session gates,
  tab reuse, and document service ownership.
- Evidence: `D276-FILE-OPEN-CONTRACT=PASS routes=6`, presentation audit,
  compileall, Ruff, formatting, project check, PE/archive, package identity,
  and refreshed release handoff records.
- Review: parent `PASS`; simplification `PASS`; Erdos the 7th architecture
  and Ampere the 7th independent Luna/max windows returned `NO_CONCLUSION`
  after bounded waits.
- Safety boundary: no EXE/Qt launch, native dialog, updater/installer/
  registry operation, unit-test asset, or worktree was executed or created.

## Completed bounded slice — D275 / ARCH-245 QSS semantic contrast contract

- Centralize the existing QSS-derived foreground decisions in the pure
  `theme_tokens.py` layer and make both runtime QSS/palette projection and the
  presentation audit consume that contract.
- Cover accent-alt text/fill, selection, warning, success, working, and error
  state surfaces across all 3 themes × 4 accents.
- Evidence: `D275-QSS-CONTRAST=PASS checks=84 failures=0`, expanded
  presentation audit, compileall, Ruff, formatting, project check, PE/archive,
  package identity, and refreshed release handoff records.
- Review: parent `PASS`; simplification `PASS`; Leibniz the 7th architecture
  and Planck the 7th independent Luna/max windows returned `NO_CONCLUSION`
  after bounded waits.
- Safety boundary: no EXE/Qt launch, native dialog, updater/installer/
  registry operation, unit-test asset, or worktree was executed or created.

## Completed bounded slice — D273 / ARCH-244 theme contrast regression audit

- Extend the existing Qt-free presentation contract audit across all supported
  theme/accent combinations and semantic shell/editor text pairs.
- Preserve the existing token resolver, QSS, palette application, editor
  behavior, settings, and runtime fallback policy.
- Evidence: `D273-CONTRAST-MATRIX=PASS checks=144 failures=0`, audit,
  compileall, Ruff, formatting, project check, PE/archive/package identity,
  and handoff records.
- Review: parent `PASS`; simplification `PASS`; Rawls the 7th architecture and
  McClintock the 7th independent Luna/max windows returned `NO_CONCLUSION`.
- Safety boundary: no EXE/Qt launch, native dialog, updater/installer/
  registry operation, unit-test asset, or worktree was executed or created.

## Completed bounded slice — D270 / ARCH-243 static notification localization audit

- Extend the existing presentation AST contract audit to reject static ASCII
  `notify(...)` literals that remain unchanged through the existing zh-CN
  `localize_message` boundary.
- Preserve dynamic diagnostic text, notification levels, runtime behavior,
  Qt-free coordinator boundaries, and the single existing localization owner.
- Evidence: `D270-NOTIFICATION-LOCALIZATION=PASS static_ascii=52
  translated=52 missing=0`, audit rule exit contract, compileall, Ruff,
  formatting, project check, PE/archive/package identity, and handoff docs.
- Review: parent `PASS`; simplification `PASS`; Mill the 7th architecture and
  Euler the 7th independent Luna/max windows returned `NO_CONCLUSION` after
  bounded waits.
- Safety boundary: no EXE/Qt launch, native dialog, updater/installer/
  registry operation, unit-test asset, or worktree was executed or created.

## Completed bounded slice — D264 / ARCH-242 secondary startup fallback localization

- Extend only the existing pre-Qt fail-open handler so a Chinese locale keeps
  Chinese fallback labels even when startup error-detail construction fails.
- Preserve the final English literal if locale classification also fails, plus
  original exception/diagnostic details, MessageBox/stderr behavior, exit code,
  and normal startup.
- Evidence: `D264-STARTUP-FAILOPEN-LOCALIZATION=PASS
  nested_locale_failure=covered final_english_fallback=preserved`, source
  diagnostic, compiler/Ruff/format, PE preflight, dual-shell package identity,
  frozen archive checks, and project/handoff checks.
- Review: parent `PASS`; simplification `PASS`; Huygens the 7th architecture
  and Locke the 7th independent Luna/max windows returned `NO_CONCLUSION`
  after bounded waits.
- Safety boundary: no EXE/Qt launch, native MessageBox, updater/installer/
  registry operation, unit-test asset, or worktree was executed or created.

## Completed bounded slice — D263 / ARCH-241 workspace-search outside-workspace placeholder localization

- Add one presentation catalog key for the relative-path fallback used when a
  workspace-search diagnostic path is outside the selected root.
- Preserve the English placeholder, angle-bracket semantics, path containment,
  result data, ordering, expansion state, and the existing locale-refresh
  projection path.
- Evidence: `D263-OUTSIDE-WORKSPACE-LOCALIZATION=PASS locales=2
  placeholder=preserved`, `D263-DIAGNOSTIC-REFRESH-WIRING=PASS locale_refresh=1`,
  source diagnostic, compiler/Ruff/format, dual-shell package identity,
  frozen archive checks, and project/handoff checks.
- Review: parent `PASS`; simplification `PASS`; Herschel the 7th architecture
  and Singer the 7th independent Luna/max windows returned `NO_CONCLUSION`
  after bounded waits.
- Safety boundary: no EXE/Qt launch, native search dialog, updater/installer/
  registry operation, unit-test asset, or worktree was executed or created.

## Completed bounded slice — D262 / ARCH-240 early startup fallback localization

- Keep the pre-Qt startup exception fallback in `__main__.py` and add a small
  two-locale system-language projection for Chinese labels.
- Preserve the original English fallback, exception details, diagnostic log
  path/format, MessageBox flags, stderr fail-open behavior, and exit code `1`.
- Evidence: `D262-STARTUP-FALLBACK-LOCALIZATION=PASS current_locale=zh-CN qt_free=1`,
  `D262-STARTUP-CONTRACT-SOURCE=PASS exit_code=1 diagnostic_path_preserved=1`,
  source diagnostic, compiler/Ruff/format, dual-shell package identity,
  frozen archive checks, and project/handoff checks.
- Review: parent `PASS`; simplification `PASS`; Carson the 7th architecture
  and Arendt the 7th independent Luna/max windows returned `NO_CONCLUSION`
  after bounded waits.
- Safety boundary: no EXE/Qt launch, native MessageBox, updater/installer/
  registry operation, unit-test asset, or worktree was executed or created.

## Completed bounded slice — D261 / ARCH-239 file-dialog default-name localization

- Reuse the existing `document.untitled` catalog value for the no-current-
  document Save As fallback and retain the `.txt` extension.
- Preserve `str(current)` for existing document paths and leave dialog title,
  filter, selected-path, document, filesystem, and persistence contracts
  unchanged.
- Evidence: `D261-SAVE-NAME-LOCALIZATION=PASS locales=2 extension=.txt`,
  `D261-SAVE-NAME-WIRING=PASS current_path_preserved=1`, source diagnostic,
  compiler/Ruff/format, dual-shell package identity, frozen archive checks,
  and project/handoff checks.
- Review: parent `PASS`; simplification `PASS`; Hubble the 7th architecture
  and Heisenberg the 7th independent Luna/max windows returned
  `NO_CONCLUSION` after bounded waits.
- Safety boundary: no EXE/Qt launch, native dialog, updater/installer/
  registry operation, unit-test asset, or worktree was executed or created.

## Completed bounded slice — D260 / ARCH-238 font-size unit locale refresh

- Replace the Settings dialog’s hard-coded `pt` suffix with one catalog value
  applied to both interface/editor font-size controls in the existing locale
  refresh path.
- Preserve numeric values, ranges, settings persistence, preview behavior,
  and English output.
- Evidence: `D260-FONT-SUFFIX-CATALOG=PASS locales=2`,
  `D260-FONT-SUFFIX-WIRING=PASS spinboxes=2 locale_refresh=1`, source
  diagnostic, compiler/Ruff/format, dual-shell package identity, frozen
  archive checks, and project/handoff checks.
- Review: parent `PASS`; simplification `PASS`; Darwin the 7th architecture
  and Harvey the 7th independent Luna/max windows returned `NO_CONCLUSION`
  after bounded waits.
- Safety boundary: no EXE/Qt launch, native dialog, updater/installer/
  registry operation, unit-test asset, or worktree was executed or created.

## Completed bounded slice — D259 / ARCH-237 message-dialog standard button localization

- Add four catalog-backed labels and project them after common QMessageBox
  standard-button creation.
- Preserve StandardButton return semantics, button roles, default selection,
  modal flow, recovery/settings ownership, and host-independent locale text.
- Evidence: `D259-DIALOG-BUTTON-CATALOG=PASS keys=4`, source diagnostic,
  compiler/Ruff/format, dual-shell package identity, frozen archive checks,
  and project/handoff checks.
- Review: parent `PASS`; simplification `PASS`; Carver the 7th architecture
  and Wegener the 7th independent Luna/max windows returned `NO_CONCLUSION`
  after bounded waits.
- Safety boundary: no EXE/Qt launch, native dialog, updater/installer/
  registry operation, unit-test asset, or worktree was executed or created.

## Completed bounded slice — D258 / ARCH-236 Replace All error localization

- Extend only the existing presentation localizer with one anchored dynamic
  match-limit mapping and three exact text-capture/rollback mappings.
- Preserve editor transaction and rollback behavior, en-US output, unknown
  diagnostic details, and the existing error propagation path.
- Evidence: `D258-REPLACE-LOCALIZATION=PASS cases=7`, catalog parity, source
  diagnostic, compiler/Ruff/format, dual-shell package identity, frozen
  archive checks, and project/handoff checks.
- Review: parent `PASS`; simplification `PASS`; Socrates the 7th architecture
  and Hooke the 7th independent Luna/max windows returned `NO_CONCLUSION`
  after bounded waits.
- Safety boundary: no EXE/Qt launch, native dialog, updater/installer/
  registry operation, unit-test asset, or worktree was executed or created.

## Completed bounded slice — D257 / ARCH-235 built-in plugin status-name localization

- Route the built-in `quillforge.document-stats` plugin name through the
  existing presentation i18n boundary and reuse the plugin-status locale
  refresh path.
- Preserve en-US output, external plugin manifest names, plugin API/status
  contracts, lifecycle policy, and normal startup.
- Evidence: `D257-PLUGIN-NAME-BOUNDARY=PASS cases=3`, source diagnostic,
  compiler/Ruff/format, dual-shell package identity, frozen archive checks,
  and project/handoff checks.
- Review: parent `PASS`; simplification `PASS`; Hume the 7th architecture and
  Russell the 7th independent Luna/max windows returned `NO_CONCLUSION` after
  bounded waits.
- Safety boundary: no EXE/Qt launch, native dialog, updater/installer/
  registry operation, unit-test asset, or worktree was executed or created.

## Completed bounded slice — D256 / ARCH-234 built-in document-statistics localization

- Add the built-in `tools.document-stats` command title and exact/dynamic
  notification mappings to the existing presentation i18n boundary.
- Preserve plugin API ownership, built-in plugin behavior, dynamic counters,
  en-US output, unknown external fallback, and normal startup.
- Evidence: `D256-BUILTIN-PLUGIN-LOCALIZATION=PASS cases=3`, compiler/Ruff/
  format, dual-shell package identity, frozen archive checks, and project/
  handoff checks.
- Review: parent `PASS`; simplification `PASS`; Einstein the 7th architecture
  and Lovelace the 7th independent Luna/max windows returned `NO_CONCLUSION`
  after bounded waits.
- Safety boundary: no EXE/Qt launch, native dialog, updater/installer/
  registry operation, unit-test asset, or worktree was executed or created.

## Completed bounded slice — D255 / ARCH-233 application-error localization boundary

- Extend only the shared presentation localizer with bounded user-visible
  application error mappings and nested coordinator-wrapper handling.
- Preserve dynamic paths, names, plugin IDs, unknown details, en-US behavior,
  application policy, worker boundaries, and normal startup.
- Evidence: `D255-LOCALIZATION-BOUNDARY=PASS cases=7`,
  `D255-SOURCE-DIAGNOSTIC-EXIT=0`, compiler/Ruff/format, dual-shell package
  identity, frozen archive checks, and project/handoff checks.
- Review: parent `PASS`; simplification `PASS`; Zeno the 7th architecture and
  Ptolemy the 7th independent Luna/max windows returned `NO_CONCLUSION` after
  bounded waits.
- Safety boundary: no EXE/Qt launch, native dialog, updater/installer/
  registry operation, unit-test asset, or worktree was executed or created.

## Completed bounded slice — D254 / ARCH-232 frozen-startup runtime dependency diagnostic

- Extend only the no-window frozen startup diagnostic with explicit presence
  checks for Qt Core/Gui/Widgets, QScintilla, and the platform bundle.
- Preserve normal QApplication/runtime startup, qwindows discovery,
  diagnostic exit semantics, and package contents; report missing relative
  paths without loading or repairing dependencies.
- Evidence: `D254-SOURCE-DIAGNOSTIC-EXIT=0`,
  `D254-DIAGNOSTIC-DEPENDENCIES-STATUS=not_applicable`, compiler/Ruff/format,
  dual-shell package identity, seven-path frozen dependency/archive probe, and
  project/handoff checks.
- Review: parent `PASS`; simplification `PASS`; Bacon the 7th architecture and
  Aristotle the 7th independent Luna/max windows returned `NO_CONCLUSION`
  after bounded waits.
- Safety boundary: no EXE/Qt launch, native dialog, updater/installer/
  registry operation, unit-test asset, or worktree was executed or created.

## Completed bounded slice — D253 / ARCH-231 workspace-entry error locale refresh

- Retain each `WorkspaceEntry.error` source in a private workspace-tree item
  role, localize known provider reasons for disabled-row tooltips, and refresh
  those tooltips when the locale changes.
- Preserve inaccessible-row disabled state, path/kind data, file-first-click
  and folder-double-click/Enter activation, provider ordering/bounds, and
  application ownership.
- Evidence: `D253-WORKSPACE-ERROR-SOURCE-RETENTION=PASS`,
  `D253-WORKSPACE-ERROR-TOOLTIP-LOCALIZATION=PASS`,
  `D253-WORKSPACE-DISABLED-LOCALE-REFRESH=PASS`,
  `D253-WORKSPACE-UNKNOWN-ERROR-FALLBACK=PASS`, compiler/Ruff/format checks,
  dual-shell package identity, recursive frozen archive checks, and
  project/handoff checks.
- Review: parent `PASS`; simplification `PASS`; Hypatia the 7th architecture
  and Peirce the 7th independent Luna/max windows returned `NO_CONCLUSION`
  after bounded waits.
- Safety boundary: no EXE/Qt launch, native dialog, updater/installer/
  registry operation, unit-test asset, or worktree was executed or created.

## Completed bounded slice — D252 / ARCH-230 plugin diagnostic locale refresh

- Route plugin runtime status errors and catalog entry reasons through the
  existing shared localizer at tooltip composition, covering stable
  application-owned reasons/prefixes while preserving unknown provider text.
- Keep trust, approval, enablement, runtime registration, external execution,
  catalog scanning, permissions, and application ownership unchanged.
- Evidence: `D252-PLUGIN-STATUS-ERROR-ROUTING=PASS`,
  `D252-PLUGIN-CATALOG-REASON-ROUTING=PASS`,
  `D252-PLUGIN-UNKNOWN-TEXT-FALLBACK=PASS`,
  `D252-PLUGIN-LOCALIZER-BEHAVIOR=PASS cases=7`, compiler/Ruff/format checks,
  dual-shell package identity, recursive frozen archive checks, and
  project/handoff checks.
- Review: parent `PASS`; simplification `PASS`; Noether the 7th architecture
  and Turing the 7th independent Luna/max windows returned `NO_CONCLUSION`
  after bounded waits.
- Safety boundary: no EXE/Qt launch, native dialog, updater/installer/
  registry operation, unit-test asset, or worktree was executed or created.

## Completed bounded slice — D251 / ARCH-229 workspace-search diagnostic locale refresh

- Retain the latest immutable search result only for diagnostic-row
  re-projection, localize known provider reasons/prefixes, and preserve paths,
  detail suffixes, ordering, counts, marker behavior, and expansion state.
- Keep search provider/service policy, query/results, cancellation, and
  application ownership unchanged.
- Evidence: `D251-DIAGNOSTIC-SOURCE-RETENTION=PASS`,
  `D251-DIAGNOSTIC-LOCALE-REFRESH=PASS`,
  `D251-DIAGNOSTIC-PREFIX-CATALOG=PASS`,
  `D251-DIAGNOSTIC-EXPANSION-PRESERVATION=PASS`, compiler/Ruff/format checks,
  dual-shell package identity, recursive frozen archive checks, and
  project/handoff checks.
- Review: parent `PASS`; simplification `PASS`; Avicenna the 7th architecture
  and Newton the 7th independent Luna/max windows returned `NO_CONCLUSION`
  after bounded waits.
- Safety boundary: no EXE/Qt launch, native dialog, updater/installer/
  registry operation, unit-test asset, or worktree was executed or created.

## Completed bounded slice — D250 / ARCH-228 workspace search status locale refresh

- Retain source data for workspace-search catalog, error, and immutable summary
  statuses, then rebuild the current text after locale changes.
- Preserve feedback levels, query/results/diagnostics, cancellation, search
  service policy, and application ownership.
- Evidence: `D250-STATUS-SOURCE-RETENTION=PASS`,
  `D250-LOCALE-ERROR-REPROJECTION=PASS`,
  `D250-LOCALE-SUMMARY-REPROJECTION=PASS`,
  `D250-CATALOG-STATUS-REPROJECTION=PASS`, compiler/Ruff/format checks,
  dual-shell package identity, recursive frozen archive checks, and
  project/handoff checks.
- Review: parent `PASS`; simplification `PASS`; Godel the 7th architecture
  and Lorentz the 7th independent Luna/max windows returned `NO_CONCLUSION`
  after bounded waits.
- Safety boundary: no EXE/Qt launch, native dialog, updater/installer/
  registry operation, unit-test asset, or worktree was executed or created.

## Completed bounded slice — D249 / ARCH-227 workspace error locale refresh

- Retain the current workspace error source inside `WorkspacePanel` and
  reproject it through the shared typed/string localizer after a locale change.
- Clear obsolete errors on a successful directory result or the start of a
  new load; keep coordinator, service, session, notification, and page
  retention policy unchanged.
- Evidence: `D249-ERROR-SOURCE-RETENTION=PASS`,
  `D249-LOCALE-ERROR-REPROJECTION=PASS`,
  `D249-SUCCESS-AND-LOAD-CLEARING=PASS`, compiler/Ruff/format checks,
  dual-shell package identity, recursive frozen archive checks, and
  project/handoff checks.
- Review: parent `PASS`; simplification `PASS`; Epicurus the 7th architecture
  and Feynman the 7th independent Luna/max windows returned `NO_CONCLUSION`
  after bounded waits.
- Safety boundary: no EXE/Qt launch, native dialog, updater/installer/
  registry operation, unit-test asset, or worktree was executed or created.

## Completed bounded slice — D248 / ARCH-226 workspace typed-error projection

- Preserve the original workspace navigation exception through the existing
  view/surface protocol and localize it at `WorkspacePanel` with the shared
  typed exception resolver.
- Keep invalid-result strings, English behavior, navigation/session policy,
  operation tracking, and workspace service ownership unchanged.
- Evidence: `D248-WORKSPACE-TYPED-ERROR-ROUTING=PASS`,
  `D248-WORKSPACE-CHINESE-ERROR=PASS`,
  `D248-WORKSPACE-ENGLISH-COMPATIBILITY=PASS`, source/compiler/Ruff/format
  checks, dual-shell package identity, recursive frozen archive checks, and
  project/handoff checks.
- Review: parent `PASS`; simplification `PASS`; Dewey the 7th architecture
  and Beauvoir the 7th independent Luna/max windows returned `NO_CONCLUSION`
  after bounded waits.
- Safety boundary: no EXE/Qt launch, native dialog, updater/installer/
  registry operation, unit-test asset, or worktree was executed or created.

## Completed bounded slice — D247 / ARCH-225 typed secondary error localization

- Preserve the original `Exception` object at the settings-save and Replace
  All presentation error handlers so the shared Chinese localizer can retain
  filesystem/codec type and metadata.
- Keep English output, service contracts, async behavior, persistence, and
  application policy unchanged.
- Evidence: `D247-TYPED-ERROR-BOUNDARY=PASS`,
  `D247-SETTINGS-ERROR-LOCALIZATION=PASS`,
  `D247-REPLACE-ERROR-LOCALIZATION=PASS`, source/compiler/Ruff/format checks,
  dual-shell package identity, recursive frozen archive checks, and project/
  handoff checks.
- Review: parent `PASS`; simplification `PASS`; Volta the 7th architecture
  and Euclid the 7th independent Luna/max windows returned `NO_CONCLUSION`
  after bounded waits.
- Safety boundary: no EXE/Qt launch, native dialog, updater/installer/
  registry operation, unit-test asset, or worktree was executed or created.

## Completed bounded slice — D246 / ARCH-224 startup path stat routing

- Replace the startup route's swallowing `Path.is_dir()`/`Path.is_file()`
  predicates with one `Path.stat()` and `S_ISDIR/S_ISREG` classification.
- Preserve directory/file/special-file routing and the existing async
  admission boundary while retaining concrete missing/invalid path details.
- Evidence: `D246-STARTUP-STAT-ROUTING=PASS`,
  `D246-STARTUP-ERROR-CONTEXT=PASS`, source/compiler/Ruff/format checks,
  dual-shell package identity, recursive frozen archive checks, and project/
  handoff checks.
- Review: parent `PASS`; simplification `PASS`; Boole the 7th architecture
  and Maxwell the 7th independent Luna/max windows returned `NO_CONCLUSION`
  after bounded waits.
- Safety boundary: no EXE/Qt launch, native dialog, updater/installer/
  registry operation, unit-test asset, or worktree was executed or created.

## Completed bounded slice — D245 / UI-27 typed file-error localization

- Preserve the existing typed exception flow in the document open/save
  coordinators and project it through the shared `MessageSurface` boundary.
- Add one presentation-only `localize_exception()` helper: English remains
  byte-for-byte compatible, while Chinese gives common filesystem/codec
  failures stable wording and retains path/encoding/position diagnostics.
- Evidence: `D245-EXCEPTION-LOCALIZATION=PASS`,
  `D245-CONFLICT-LOCALIZATION=PASS`, `D245-SOURCE-GATE=PASS`, compileall,
  Ruff, format, dual-shell package identity, recursive frozen archive checks,
  and project/handoff checks.
- Review: parent `PASS`; simplification `PASS`; Ohm the 7th architecture and
  Boyle the 7th independent Luna/max windows returned `NO_CONCLUSION` after
  bounded waits.
- Safety boundary: no EXE/Qt launch, native dialog, updater/installer/
  registry operation, unit-test asset, or worktree was executed or created.

## Completed bounded slice — D244 / UI-26 startup-path error localization

- Preserve the existing Qt-free `localize_message()` compatibility path and
  add only the two missing startup-path error prefixes emitted by
  `MainWindow._drain_startup_paths()`.
- Keep English output byte-for-byte unchanged and preserve the original path
  and exception detail after the translated Chinese prefix.
- Evidence: `D244-COMPILEALL=PASS`, `D244-RUFF=PASS`,
  `D244-FORMAT=PASS`, `D244-STARTUP-PATH-LOCALIZATION=PASS`, dual-shell
  package identity, recursive frozen-archive essentials, project/handoff
  checks, and expected release NO-GO.
- Review: parent `PASS`; simplification `PASS`; Pasteur the 7th architecture
  and Cicero the 7th independent review windows returned `NO_CONCLUSION`
  after bounded waits.
- Safety boundary: no EXE/Qt launch, native dialog, updater/installer/registry
  operation, unit-test asset, or worktree was executed or created.

## Completed bounded slice — D243 / UI-25 readable accent text endpoints

- Keep `ThemeColors` framework-neutral and keep filled accent foregrounds
  (`on_accent*`) separate from text-on-surface foregrounds.
- Route the application link palette, workspace eyebrow, and generic checkbox
  hover text through one private `theme.py` resolver. It uses the deepest
  shared surface as the conservative contrast background and falls back to
  `text_primary` when the selected accent is too light.
- Evidence: `D243-COMPILEALL=PASS`, `D243-RUFF=PASS`,
  `D243-FORMAT=PASS`, `D243-THEME-ENDPOINT-CONTRAST=PASS min=9.207`,
  `D243-DIRECT-ACCENT-TEXT-PROBE=PASS`, QSS endpoint matrix, dual-shell
  package identity, archive/resource/warning checks, and project/handoff
  checks.
- Review: parent `PASS`; simplification `PASS`; Fermat the 7th architecture
  windows and Linnaeus the 7th independent review window returned
  `NO_CONCLUSION` after bounded waits.
- Safety boundary: no EXE/Qt launch, native dialog, updater/installer/registry
  operation, unit-test asset, or worktree was executed or created.

## Completed bounded slice — D242 / ARCH-223 file dialog all-files default

- Keep `FileDialogSurface` and the existing async document boundary unchanged.
- Make the shared localized `dialog.text_filter` begin with Qt's `All files (*)`
  filter, retaining the text/source filter as a secondary choice so extensionless
  and uncommon source/configuration files are visible by default.
- Evidence: `D242-AST=PASS`, `D242-COMPILEALL=PASS`, `D242-RUFF=PASS`,
  `D242-FORMAT=PASS`, QtCore wildcard and extensionless probes, source startup
  diagnostic, dual-shell package builds, archive/resource/warning checks, and
  package identity.
- Review: parent `PASS`; simplification `PASS`; Plato/Poincare architecture
  windows and Raman independent window returned `NO_CONCLUSION` after bounded
  waits.
- Safety boundary: no EXE/Qt launch, native dialog, updater/installer/registry
  operation, unit-test asset, or worktree was executed or created.

## Completed bounded slice — D241 / ARCH-222 startup provenance diagnostic

- Keep normal GUI startup unchanged; extend only the existing no-window
  `--diagnose-startup` path.
- Record `quillforge.app` import module/package/origin/loader provenance and
  frozen Qt plugin-root/platform-plugin presence without mutating the runtime
  environment.
- Reuse the selected Qt plugin path for the existing Windows platform-plugin
  check so Qt6 and legacy Qt bundle layouts cannot disagree.
- Evidence: `D241-COMPILEALL=PASS`, `D241-RUFF=PASS`, `D241-FORMAT=PASS`,
  source diagnostic/parser probes, `D241-FROZEN-ARCHIVE-ENTRYPOINT=PASS`,
  dual-shell package identity, project/handoff checks, and expected release
  NO-GO.
- Review: parent `PASS`; simplification `PASS`; Pauli the 7th architecture
  follow-up `PASS`; Confucius the 7th final independent review `PASS`.
- Safety boundary: no EXE/Qt launch, updater/installer/registry operation,
  unit-test asset, or worktree was executed or created.

## Completed bounded slice — D240 / ARCH-221 frozen entrypoint path hardening

- Keep the package-relative import for normal module execution.
- Restrict the `sys.path` insertion to direct source-file execution; frozen
  PyInstaller execution now uses its installed importer without source-style
  `__file__` path surgery.
- Preserve the public `main(argv)` contract, startup diagnostics, direct-source
  convenience, Qt argument/path routing, and the no-launch boundary.
- Evidence: `D240-FROZEN-ENTRYPOINT-STATIC-CONTRACT=PASS`,
  `D240-COMPILEALL=PASS`, `D240-RUFF=PASS`, `D240-FORMAT=PASS`,
  `D240-FROZEN-ARCHIVE-ESSENTIALS=PASS`, dual-shell package builds and
  `D240-PACKAGE-IDENTITY=PASS`, project/handoff checks, and expected release
  NO-GO.
- Review: parent `PASS`; simplification `PASS`; Mendel the 7th and Galileo
  the 7th architecture windows returned `NO_CONCLUSION`; James the 7th
  independent Luna/max window returned `NO_CONCLUSION`.
- Safety boundary: no EXE/Qt launch, updater/installer/registry operation,
  unit-test asset, or worktree was executed or created.

## Completed bounded slice — D239 / ARCH-220 target identity preflight

- Reuse `Get-QfArtifactInfo` after `ShouldProcess` and before any updater move
  to validate target identity against `state.sha256`.
- For rollback, validate both target and backup against their state hashes and
  reject identical paths.
- Preserve D238 recovery, containment, state schema, and no-launch boundaries;
  do not introduce a second hash or transaction abstraction.
- Evidence: `D239-STATE-IDENTITY-STATIC-CONTRACT=PASS`, dual PowerShell AST,
  distribution/startup archive audits, compile/Ruff/presentation audit,
  dual-shell package builds, archive/PYZ, and package identity.
- Review: parent `PASS`; simplification `PASS`; Parfit/Dalton Luna/max
  windows explicitly `NO_CONCLUSION`.
- Safety boundary: no updater/rollback script, file move, EXE/Qt, registry,
  unit-test asset, or worktree was executed or created.

## Completed bounded slice — D238 / ARCH-219 update transaction safety

## Completed bounded slice — D238 / ARCH-219 update transaction safety

- Keep `Write-QfState` as the existing update/rollback commit boundary.
- Track both file moves and restore ordinary update or rollback state only when
  exact expected hashes and state-owned paths still hold.
- Preserve externally changed targets, occupied backup paths, or uncertain
  files with a warning; do not add a new transaction framework or state schema.
- Evidence: `D238-UPDATE-TRANSACTION-STATIC-CONTRACT=PASS`, dual PowerShell
  AST, compile/Ruff/presentation audit, dual-shell package builds,
  archive/PYZ, and final package identity.
- Review: parent `PASS`; simplification `PASS`; Laplace/Ramanujan Luna/max
  windows explicitly `NO_CONCLUSION`.
- Safety boundary: no updater/rollback script, file move, EXE/Qt, registry,
  unit-test asset, or worktree was executed or created.

## Completed bounded slice — D237 / ARCH-217 multi-association safety

## Completed bounded slice — D237 / ARCH-217 multi-association safety

- Keep the portable EXE association-free; change only the opt-in local
  PowerShell distribution boundary.
- Create the shared ProgId once per install operation, reuse it only with the
  operation-owned marker and exact command match, and keep each extension
  independently state-recorded.
- Remove extensions before the shared ProgId, scan for unknown references, and
  preserve the executable/state when cleanup is incomplete.
- Evidence: `D237-DISTRIBUTION-STATIC-CONTRACT=PASS`, dual PowerShell AST,
  compile/Ruff/presentation audit, dual-shell package builds, archive/PYZ,
  source boundary, and final package identity.
- Review: parent `PASS`; simplification `PASS`; Franklin/Anscombe Luna/max
  windows explicitly `NO_CONCLUSION`; the initial Meitner window found the
  pre-fix defect and was not used as post-fix approval.
- Safety boundary: no installer/uninstaller script, registry provider, EXE/Qt,
  shell association, unit-test asset, or worktree was executed or created.

## Completed bounded slice — D233 / ARCH-215 desktop file-launch routing

## Completed bounded slice — D233 / ARCH-215 desktop file-launch routing

- Keep command-line and drag-to-EXE argument classification in the Qt-free
  `application.desktop_launch` boundary.
- Keep `app.main` responsible for dispatch and Qt argument construction,
  `DesktopRuntime` responsible for transfer/lifecycle ordering, and
  `MainWindow` responsible for recovery/session barriers and existing async
  document/workspace admission.
- Preserve ordered multi-path handling, invalid-path warnings, and the
  portable artifact's `file_associations: not-configured` release boundary.
- Evidence: `D233-DESKTOP-LAUNCH-PARSER-PROBE=PASS`,
  `D233-STARTUP-WIRING-SOURCE-PROBE=PASS`, `D233-COMPILEALL=PASS`,
  `D233-RUFF=PASS`, `D233-PRESENTATION-AUDIT=PASS`, package archive/PYZ
  inclusion, and final package identity.
- Review: parent `PASS`; simplification `PASS`; Pascal/Descartes Luna/max
  windows explicitly `NO_CONCLUSION`.
- Safety boundary: no GUI/QApplication, EXE launch, shell drag-and-drop, file
  association, or test-only asset was created or run. Embedded requirements do
  not apply.

## Completed bounded slice — D235 / ARCH-216 startup-path edge routing

- Preserve the D233 parser/admission ownership; normalize only Qt value-option
  dash spelling and make permanent admission rejection visible.
- Keep temporary busy/session-restore rejection retriable and keep portable
  file associations explicit/opt-in/not-configured.
- Evidence: `D235-DESKTOP-LAUNCH-PARSER-PROBE=PASS`, compile/Ruff,
  presentation audit, source startup diagnostic, package archive/PYZ,
  source-revision boundary, and final package identity.
- Review: parent `PASS`; simplification `PASS`; Schrodinger/Tesla Luna/max
  windows explicitly `NO_CONCLUSION`.
- Safety boundary: no GUI/QApplication, EXE launch, shell drag-and-drop,
  registry operation, or test-only asset was created or run. Embedded
  requirements do not apply.

## Architecture Decisions

- Keep UI-13 in `presentation`; do not add domain/application state or a second styling system.
- Reuse the existing theme tokens and semantic QSS action selectors, with explicit disabled/focus states.
- Preserve `FindBar` signals, Enter/Shift+Enter/Esc keyboard routing, operation locking, and `EditorEngine` ownership.
- Keep D98/ARCH-72 limited to the post-validation save projection sequence. `DocumentSaveCoordinator` retains stale-operation, tab-liveness, read-only, invalid-result, and failure classification; `MainWindow` retains concrete editor, recovery, event, notification, persistence, and close policy.
- Keep D99/ARCH-73 limited to valid workspace-open and directory projection. `WorkspaceNavigationCoordinator` retains stale/invalidation/loading/invalid/failure classification; `MainWindow` retains workspace admission, service calls, concrete surface/search callbacks, session restore, notification, and close policy.
- Keep D100/ARCH-74 limited to the valid settings-save projection sequence. `SettingsSaveCoordinator` retains tracker/stale/invalid/failure classification; `MainWindow` retains concrete QApplication/theme, locale, editor, motion, notification, and close policy callbacks.
- Keep D101/ARCH-75 limited to the post-write recovery result policy. `RecoveryWriteCoordinator` retains discarded/capture/document/write lifecycle and pending-delete release; the new projection boundary retains tab liveness, dirty/content-version/snapshot identity decisions, and feedback ordering without importing Qt.
- Keep D102/ARCH-76 limited to close-readiness classification and ordered close side effects. `CloseGuardCoordinator` owns only busy/search/dirty/background/pending-work precedence and the existing immediate-session-save/timer-stop sequence; MainWindow retains Qt event acceptance, error dialog projection, concrete TaskRunner/timer/session callbacks, and all operation policy.
- Keep UI-51 limited to document-tab rail presentation. `DocumentTabSurface` may expose semantic tab-bar identity and native presentation hints; centralized theme QSS owns selected/modified/hover/focus/close states, while tab signals, index mapping, document dirty policy, and close behavior remain unchanged.
- Keep UI-52 limited to `WorkspacePanel` presentation. Semantic empty-state/path/tree identities, localized copy, icon sizing, and centralized QSS may change; `WorkspaceSurface` callbacks, file-first-click/directory-double-click/Enter routing, loading/error state ownership, workspace service calls, and containment policy remain unchanged.
- Keep D103/ARCH-77 limited to the complete session-save request/dispatch seam. A Qt-free `SessionSaveCoordinator` may own latest snapshot admission, operation binding, TaskRunner submission callback, and completion drain; MainWindow retains QTimer debounce, concrete SessionService/TaskRunner wiring, snapshot capture, startup guard, notification, and close policy.
- Keep D104/UI-53 limited to the editor's presentation token projection. `presentation.theme` remains the single source for canvas, gutter, selection, caret, current-line, and syntax-color tokens; `EditorWidget` only applies those tokens through its existing adapter boundary. No editor operation, language policy, persistence, or MainWindow ownership moves.
- Keep D105/ARCH-78 limited to recovery-write callback binding. The existing Qt-free `RecoveryWriteCoordinator` may own only the typed success/failure callback closures passed to a generic dispatch callable; MainWindow retains operation-ID allocation, RecoveryService payload/channel selection, TaskRunner ownership, capture lifecycle, notification, persistence, and close policy.
- Keep D106/UI-54 limited to centralized scrollbar presentation. `presentation.theme` may define matching vertical/horizontal track and handle states and remove the accidental horizontal-bar collapse; editor wrap/scroll policy, widget signals, and QScintilla ownership remain unchanged.
- Keep D219 / ARCH-203 limited to application-owned exception categories in the nine remaining Qt-free application modules. Reuse `ApplicationValidationError` and `ApplicationStateError`, preserve built-in compatibility and exact messages, leave dedicated recovery-channel exceptions unchanged, and do not migrate domain/infrastructure/presentation validators.
- Treat runtime Qt/EXE visual acceptance as user-owned and unrun under the current no-launch policy.
- Keep D180 / UI-92 / ARCH-167 limited to the font-style contract and its
  existing settings/editor projections: the domain remains Qt-free, schema v3
  owns validation and v1/v2 fallback, the JSON store owns compatibility, and
  presentation owns localized controls, preview, QSS, QFont, and editor
  adapter application. Reuse the existing settings-save coordinator and do not
  move editor operations, lexer policy, theme ownership, or application policy.
- Keep D181 / UI-93 / ARCH-168 limited to stable status accessibility
  projection: `presentation.i18n` owns labels/templates, `StatusRail` owns its
  Qt object tree, and `StatusSurface` owns the notification label. Reuse the
  existing locale refresh path; do not translate or store messages in
  application coordinators, and do not change status policy, severity, timers,
  or theme ownership.
- Keep D182 / UI-94 / ARCH-169 limited to the visual token ownership split:
  `presentation.theme_tokens` owns immutable Qt-free palette/contrast
  resolution, while `presentation.theme` remains the only Qt palette/QSS,
  icon, and editor-adapter projection owner. Preserve existing theme imports,
  fallback semantics, and semantic selectors; do not add a second styling
  system, runtime theme service, or settings contract.
- Keep D183 / UI-95 / ARCH-170 limited to three existing QSS surface
  selectors: `QMainWindow#mainWindow`, `QWidget#editorShell`, and
  `QToolBar#commandBar`. Use only `ThemeColors.surface_0/1/2`; keep the editor
  canvas solid and do not alter control states, signals, settings, locale,
  motion, or application policy.
- Keep D184 / ARCH-171 limited to local distribution scripts. The shared
  PowerShell module owns path containment, SHA-256 artifact validation,
  install-state/rollback contracts, and exact owned-association cleanup; the
  entry points remain user-local, `ShouldProcess`-gated, offline, and opt-in
  for explicit HKCU extensions. Do not execute scripts, touch the registry,
  start an EXE, or claim verified installer/update/runtime evidence.
- Keep D185 / UI-96 / ARCH-172 limited to centralized QSS affordance
  highlighting. Reuse `ThemeColors.surface_hover`, `selection`, and
  `accent_alt` for existing form-control focus and ComboBox popup item states;
  do not alter object names, values, signals, layout, locale, fonts, motion,
  settings persistence, or application policy.
- Keep D186 / UI-97 / ARCH-173 limited to the workspace file-entry edge.
  `WorkspacePanel` may emit one localized file-picker intent and
  `WorkspaceSurface` may forward it, but `MainWindow` must reuse
  `_open_document` and the existing `FileDialogSurface`/
  `DocumentPickerAdmissionCoordinator`; do not add a second file-open service,
  direct DocumentService call, or change folder/tree activation semantics.
- Keep D187 / UI-98 / ARCH-174 limited to a responsive Settings
  presentation boundary. `SettingsDialog` may place its existing appearance,
  preview, editor, and guidance surfaces inside one named `QScrollArea`, while
  the Save/Cancel rail remains pinned to the dialog. Do not change settings
  values, locale, persistence, signals, theme application, motion policy, or
  application ownership.
- Keep UI-73 limited to the document-stage presentation edge: remove the
  duplicated tab-pane outline while retaining the authored document-tab rail,
  editor canvas border, focus cue, and all existing tab/find signals. Do not
  change layout owners, editor behavior, locale, font, motion, or application
  policy. The architecture consultation for this continuation returned
  `NO_CONCLUSION`; the parent decision is a one-file centralized-QSS slice.
- Keep D145/ARCH-129 limited to the deterministic status-phase policy. A
  Qt-free coordinator may map `busy`, retained runner work, and active-document
  dirty state to `working`, `attention`, or `ready`; `MainWindow` retains
  concrete state queries and `StatusSurface` retains Qt rendering. The existing
  direct `error` projection, notifications, close policy, and task ownership
  remain unchanged. The architecture consultation returned `NO_CONCLUSION`.

## Task List

### Phase 1: UI-13 vertical slice

- [x] Confirm the action-role design with bounded read-only role input and record independent no-conclusion results honestly.
- [x] Assign stable presentation roles to FindBar actions and extend centralized QSS states.
- [x] Add UI-13 acceptance, review, handoff, register, index, roadmap, package, and artifact evidence.

### Checkpoint: UI-13 static/package evidence

- [x] Compileall, Ruff, format, source boundary probe, JSON validation, handoff verifier, and repository check pass.
- [x] Rebuild the portable package and confirm root/dist manifest identity.
- [x] Release verifier remains an explicit NO-GO because known report mismatches/external gates remain.

### Phase 1b: UI-14 visual surface rhythm

- [x] Deploy and validate the project-local UI visual-quality and enterprise-architecture skills.
- [x] Confirm the bounded command-palette/surface-rhythm design with fixed-role input.
- [x] Flatten ordinary-control gradients, tighten excessive rounding/spacing, and add command-palette semantic selectors without changing signals.
- [x] Add UI-14 acceptance, review, handoff, register, index, roadmap, package, and artifact evidence.

### Phase 2: Remaining project delivery

- [ ] D7.3 permission/disk-pressure and authorized workload evidence, if an operator authorizes it.
- [ ] D7.4 fresh packaged/interactive/cross-machine search evidence, if authorized.
- [ ] D8 legal decision, signing/installer/update decisions, clean-machine/support evidence, and release-owner approval.
- [x] D42/UI-28 workspace action hierarchy: distinguish the existing primary open-workspace action from quiet Back/Cancel states through centralized QSS only, preserving signals and loading policy.
- [x] D43/ARCH-33/UI-29 workspace-search lifecycle boundary: isolate active ID, generation invalidation, cooperative cancellation, and stale/current callback classification in a Qt-free tracker while preserving MainWindow policy ownership.
- [x] D44/ARCH-34/UI-30 workspace-navigation lifecycle boundary: isolate workspace open/list identity and generation invalidation while preserving generic busy/status, TaskRunner, service, surface, session, notification, containment, and result policy in MainWindow.
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
- [x] D55/UI-35 warning-background foreground contract: derive readable warning text from warning_bg through the existing centralized theme helper without expanding the theme schema.
- [x] D56/ARCH-45 document-tab identity lookup boundary: add one path-identity lookup to DocumentTabSurface and remove MainWindow's full-tab iteration without moving session-restore policy.
- [ ] Further bounded UI slices only when they address a distinct user-visible gap.
- [x] D187/UI-98/ARCH-174 settings responsive scroll boundary: keep the
  existing settings content usable at short heights while preserving the
  pinned Save/Cancel rail and every settings contract.

### Current continuation: UI-73 document-stage edge

- [x] Add the smallest centralized-QSS adjustment that makes the document tab
  rail flow into the editor canvas without a redundant pane outline.
- [x] Verify the normal, selected, hover, focus, disabled, and editor-canvas
  focus selectors remain explicit in the source contract.
- [x] Record the UI-73 acceptance, independent review, simplification result,
  handoff, package identity, and known no-launch/release limitations.

### Next bounded architecture slice: D145/ARCH-129 status phase policy

- [x] Define the pure status-phase contract and coordinator before changing the
  composition root.
- [x] Wire only `_sync_status_surface` through the coordinator and preserve the
  existing `error` path and status-rendering owner.
- [x] Record the independent review, simplification assessment, static/package
  evidence, and no-launch/release limits.

### Next bounded visual slice: UI-74/ARCH-130 status-rail context divider

- [x] Add one centralized token-driven divider/padding treatment to
  `QLabel#statusContext` so the local context and phase pill scan as separate
  elements.
- [x] Verify all theme/accent projections and preserve every phase selector,
  locale, timer, signal, and status-owner contract.
- [x] Record the independent review, simplification assessment, package
  identity, and no-launch/release limitations.

### Phase 3: Enterprise architecture migration

- [x] Research public ByteDance/CloudWeGo sources and record applicability without private-standard claims.
- [x] Write the architecture baseline/spec and ADR-0036 before the first source migration.
- [x] Extract desktop composition into `composition.py` and Qt-free diagnostic composition into `diagnostic_composition.py` while preserving entry-point, lifecycle, and diagnostic boundaries.
- [x] Extract the menu and command-rail projection into `presentation/command_surface.py` while keeping MainWindow callbacks and the public refresh seam unchanged.
- [x] Extract the document-tab widget projection and record/index lookup into `presentation/document_tab_surface.py` while keeping MainWindow document behavior and lifecycle callbacks unchanged.
- [x] Extract workspace dock/panel composition, semantic signal wiring, and locale projection into `presentation/workspace_surface.py` while keeping MainWindow workspace/search/session behavior unchanged.
- [x] Extract workspace-search dialog composition, semantic signal wiring, activation, and projection into `presentation/workspace_search_surface.py` while keeping MainWindow search lifecycle and containment policy unchanged.
- [x] Close the workspace projection boundary by moving current-path/loading/directory/error delegation behind `presentation/workspace_surface.py` and removing MainWindow's direct `WorkspacePanel` dependency.
- [x] Extract FindBar composition, six semantic signal routes, shell placement, and presentation projection into `presentation/find_surface.py` while keeping editor and Replace All policy in MainWindow.
- [x] Extract SettingsDialog composition and modal snapshot editing into `presentation/settings_surface.py` while keeping SettingsService/TaskRunner, theme, locale, editor, animation, and error policy in MainWindow.
- [x] Extract recovery prompt composition, locale-aware text, and typed restore/discard/later projection into `presentation/recovery_prompt_surface.py` while keeping RecoveryService, session, document, cleanup, and notification policy in MainWindow.
- [x] Extract StatusRail composition and semantic locale/phase projection into `presentation/status_surface.py` while keeping operation/document phase policy and close guards in MainWindow.
- [x] Extract native file/folder/save dialog selection into `presentation/file_dialog_surface.py` while keeping MainWindow startup/busy guards, async dispatch, save/session/recovery policy, and containment decisions.
- [x] Extract command-palette modal composition and stable-ID projection into `presentation/command_palette_surface.py` while keeping MainWindow live command resolution, execution, stale-ID feedback, menu refresh, and localized notifications.
- [x] Extract plugin catalog/status dialog lifecycle, locale, activation, semantic callbacks, and governance projection into `presentation/plugin_surface.py` while keeping MainWindow plugin services and policy ownership.
- [x] Extract localized save-before-close, About, and recoverable error message composition into `presentation/message_surface.py` while keeping MainWindow dirty/save/close/error/status policy.
- [x] Extract bounded theme-transition effect and animation lifecycle into `presentation/theme_transition_surface.py` while keeping MainWindow motion policy, target selection, and trigger timing.
- [x] Complete the StatusSurface host boundary for QStatusBar attachment, size-grip configuration, and localized transient notifications while keeping MainWindow phase/operation policy.
- [x] Extract central `editorShell` QWidget/layout, `DocumentTabSurface`/`FindSurface` composition, initial FindBar hidden state, and FindBar locale routing into `presentation/editor_shell_surface.py` while keeping MainWindow callbacks and document/editor/operation policy.
- [x] Extract per-document EditorWidget creation, presentation settings/theme application, Save As language refresh, and semantic signal wiring into `presentation/editor_document_surface.py` while keeping MainWindow document/operation policy.
- [x] Harden centralized accent endpoint foregrounds, including the amber/砂金 warning action, and refine command-rail/document-tab QSS rhythm without changing behavior.
- [x] Replace platform-dependent shell icons with a theme-tinted authored vector icon surface, keep icon refresh explicit, and preserve command/workspace behavior.
- [x] Add semantic transient notification levels behind the existing StatusSurface boundary, with a localized statusMessage widget, preserved notify compatibility, and centralized contrast-safe QSS states.
- [x] D32a inline feedback hierarchy: give workspace and Find in Files status labels explicit presentation-only loading/success/warning/error states through one shared contract, without moving policy out of MainWindow.
- [x] D33 FindBar feedback projection: add explicit semantic status levels at the existing FindSurface seam without changing editor policy or keyboard routing.
- [x] D34a Session/Recovery notification severity closure: explicitly project existing recovery/session outcomes through the shared notification level contract without moving state policy.
- [x] D34a correction: classify invalid/failed session persistence as error and deferred recovery as warning after independent source review.
- [x] D35a plugin/extension notification severity closure: explicitly project existing catalog, governance, runtime, and host outcomes through the shared notification contract without moving plugin policy.
- [x] D36a workspace/operation notification severity closure: explicitly project existing workspace, Find in Files, containment, cancellation, and long-running operation outcomes without moving async or file policy.
- [x] D37a MainWindow notification contract closure: make all current coordinator notification call sites explicit while preserving the external one-argument compatibility default.
- [x] D38 focus-state visibility: strengthen centralized QSS focus cues for command/tool buttons, document tabs, and checkboxes without introducing a new state owner.
- [x] D39 operation-tracker boundary: extract monotonic operation-ID allocation and the current-operation stale guard while keeping MainWindow busy/status/TaskRunner policy.
- [x] D40 plugin operation-state boundary: centralize independent catalog-scan, catalog-governance, and host-probe lifecycle IDs without moving plugin policy or TaskRunner ownership.
- [x] D41 workspace-entry activation: keep single-click file opening and double-click folder navigation, add Enter/Return keyboard activation through one semantic item-intent route, and preserve MainWindow containment/open policy.
- [x] D57/ARCH-46 session-restore tab projection boundary: move only ordered restored-tab recording and active-tab selection input behind a Qt-free tracker contract; retain session restore policy, service calls, task execution, tab projection, notifications, startup, and close ownership in MainWindow.
- [x] D58/ARCH-47 session-load coordinator state simplification: remove the write-only load-state field after reachability review; retain load normalization, restore/recovery sequencing, notifications, and save baseline policy.
- [x] D59/ARCH-48 status-phase forwarding simplification: remove the coordinator-only `_sync_active_document_phase()` alias and route existing callers to `_sync_status_surface()` without moving phase policy.
- [x] UI-36 document-tab selection hierarchy: strengthen selected/hover/focus QSS distinction and dock-title boundary through the existing centralized theme owner; preserve tab behavior, tokens, signals, and policy.
- [x] D61/ARCH-49 session-snapshot capture boundary: move only pure clean-tab metadata filtering and active-index assembly behind a Qt-free contract; retain editor/widget reads, session service, debounce, save, startup, and close policy in MainWindow.
- [x] D62/UI-37 find-bar action hierarchy: add presentation-only action identities and strengthen find/replace input/action QSS while preserving signals, locale, operation state, warning semantics, and MainWindow policy.
- [x] D63/UI-38 toolbar context chip: turn the existing localized local/safe context label into a token-driven visual chip without changing CommandSurface, i18n, layout, shortcuts, or command policy.
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
- [x] D98/ARCH-72 document-save projection coordinator boundary: move the valid-save state/language/title/recovery/event/notification/session-save/continuation ordering behind a Qt-free generic coordinator while preserving D87 classification, concrete editor policy, service ownership, and close behavior.
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
- [x] Continue splitting `MainWindow` orchestration into smaller presentation coordinators with typed application contracts; D163 global inventory records 43 frozen/slotted Ports contracts and zero legacy positional coordinator constructors.
- [x] Complete cross-module contract/error/observability audit and add only the necessary static gates. Revalidated as D188 static evidence: the existing AST audit passes, 43 frozen/slotted Ports contracts are inventoried, forbidden coordinator imports are zero, and MainWindow notification calls have zero missing explicit levels.
- [ ] Obtain authorized runtime, clean-machine, and release evidence for the architecture migration.

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

## Next bounded slice — editor-change projection

Status: completed as D146 / ARCH-131; accepted-with-limits.

- Outcome: reduce `MainWindow` editor-event coupling by giving the existing modified/content side effects one typed, Qt-free presentation boundary.
- Scope: preserve tab/editor lookup and document-service dirty mutation ownership in `MainWindow`; extract only deterministic find invalidation, stale-tab guards, dirty/content mutation, tab-title/status projection, and debounced session-save ordering.
- Non-goals: no current-tab transition behavior, editor behavior, document persistence, tab identity, locale catalog, QSS, async worker, close policy, or runtime startup changes.
- Compatibility promise: modified events still invalidate Find before lookup, ignore stale/unchanged tabs, then mark dirty/title/status/save in the existing order; content events still ignore stale editors, increment content version, then invalidate Find.
- Evidence required: coordinator contract/projection probe, MainWindow wiring/source probe, Qt-free import probe, compile/lint/format, package identity, ADR, handoff, independent review/no-conclusion, simplification assessment, and expected release NO-GO record.
- Recorded evidence: `D146-PROJECTION-PROBE=PASS`, `D146-SOURCE-WIRING-PROBE=PASS`, `D146-PRESENTATION-AUDIT=PASS`, `D146-COMPILEALL=PASS`, `D146-RUFF=PASS`, `D146-FORMAT=PASS`, `D146-PACKAGE-BUILD=PASS`, `D146-PACKAGE-IDENTITY-PROBE=PASS`, `D146-CHECK=PASS`, `D146-VERIFY-HANDOFF=PASS`; architect and independent review windows are recorded as `NO_CONCLUSION`, parent and simplification are `PASS`.

## Next bounded slice — current-document transition projection (D147 / ARCH-132)

- Outcome: reduce the remaining current-tab event-handler coupling while keeping the concrete tab surface, FindSurface, StatusSurface, notification, and session-save owners in `MainWindow`.
- Scope: extract only the established transition order: invalidate Find match, reset Find session, resolve active-tab context, emit the localized info notification, synchronize status, and request debounced session save.
- Non-goals: no editor mutation, tab identity, document service, persistence implementation, async worker, close policy, locale refresh, QSS, or runtime startup change.
- Compatibility promise: current-tab changes preserve the exact existing order and no-op behavior when there is no active tab.
- Evidence required: exact transition-order probe, MainWindow wiring/source probe, Qt-free import audit, compile/lint/format, package identity, ADR, handoff, independent review/no-conclusion, simplification assessment, and expected release NO-GO record.
- Status: completed as D147 / ARCH-132; accepted-with-limits.
- Recorded evidence: `D147-TRANSITION-PROBE=PASS`, `D147-SOURCE-WIRING-PROBE=PASS`, `D147-PRESENTATION-AUDIT=PASS`, `D147-COMPILEALL=PASS`, `D147-RUFF=PASS`, `D147-FORMAT=PASS`, `D147-PACKAGE-BUILD=PASS`, `D147-PACKAGE-IDENTITY-PROBE=PASS`, `D147-CHECK=PASS`, `D147-VERIFY-HANDOFF=PASS`; architect and independent review windows are recorded as `NO_CONCLUSION`, parent and simplification are `PASS`.

## Next bounded slice — active document tab emphasis (UI-75 / ARCH-133)

- Outcome: make the selected document tab scan as the active work surface again by restoring a token-driven left accent edge that the specific document-tab selector currently overrides.
- Scope: only `QTabBar#documentTabBar::tab:selected` and its selected-hover selector in `presentation/theme.py`; retain existing surface, bottom accent, text, focus, disabled, close-button, tab metrics, and theme/accent tokens.
- Non-goals: no tab widget, signal, keyboard, close lifecycle, document state, icon, locale, font, motion, or application-policy change.
- Compatibility promise: all existing selected/hover/focus/disabled rules remain active; the only new projection is the selected tab's explicit left accent edge across supported theme/accent combinations.
- Evidence required: QSS selector/token projection probe, selected-state preservation probe, compile/lint/format, package identity, ADR, handoff, independent review/no-conclusion, simplification assessment, and expected release NO-GO record.
- Status: completed as UI-75 / ARCH-133; accepted-with-limits.
- Recorded evidence: `UI75-TAB-HIGHLIGHT-PROBE=PASS:12 theme/accent projections`, `UI75-STATE-PRESERVATION-PROBE=PASS`, `UI75-PRESENTATION-AUDIT=PASS`, `UI75-COMPILEALL=PASS`, `UI75-RUFF=PASS`, `UI75-FORMAT=PASS`, `UI75-PACKAGE-BUILD=PASS`, `UI75-PACKAGE-IDENTITY-PROBE=PASS`, `UI75-CHECK=PASS`, `UI75-VERIFY-HANDOFF=PASS`; architect and independent review windows are recorded as `NO_CONCLUSION`, parent and simplification are `PASS`.

## Next bounded slice — editor-action admission (D148 / ARCH-134)

- Outcome: make the editor command gate explicit without moving editor behavior or Qt ownership into a coordinator.
- Scope: extract only the existing no-active-tab/busy guard, action application to the active editor, and focus restoration after the action.
- Non-goals: no command registry, shortcut/menu/toolbar composition, undo implementation, document mutation policy, dirty tracking, tab identity, locale, QSS, async worker, close policy, or runtime startup change.
- Compatibility promise: actions remain silent no-ops with no active tab or while busy; admitted actions run against the active editor once and restore focus once.
- Evidence required: admission/ordering probe, MainWindow wiring/source probe, Qt-free import audit, compile/lint/format, package identity, ADR, handoff, independent review/no-conclusion, simplification assessment, and expected release NO-GO record.
- Status: completed as D148 / ARCH-134; accepted-with-limits.
- Recorded evidence: `D148-ADMISSION-PROBE=PASS`, `D148-SOURCE-WIRING-PROBE=PASS`, `D148-PRESENTATION-AUDIT=PASS`, `D148-COMPILEALL=PASS`, `D148-RUFF=PASS`, `D148-FORMAT=PASS`, `D148-PACKAGE-BUILD=PASS`, `D148-PACKAGE-IDENTITY-PROBE=PASS`; architecture and independent review windows are recorded as `NO_CONCLUSION`, parent and simplification are `PASS`.

## Next bounded slice — settings-save projection Ports contract (D149 / ARCH-135)

- Outcome: make the existing valid-settings projection order explicit through a frozen/slotted named callback contract without changing settings behavior.
- Scope: replace only the positional constructor callbacks of `SettingsSaveProjectionCoordinator` with a Qt-free `SettingsSaveProjectionPorts` dataclass and preserve apply snapshot, retranslation, editor settings, theme transition, and success-notification order.
- Non-goals: no settings validation/persistence, dialog controls, locale catalog, theme tokens, editor behavior, animation implementation, async worker, close policy, or runtime startup change.
- Compatibility promise: a valid settings result runs the same five callbacks in the same order; the coordinator remains Qt-free and MainWindow retains all concrete ownership.
- Evidence required: projection-order/ports probe, MainWindow wiring/source probe, Qt-free import audit, compile/lint/format, package identity, ADR, handoff, independent review/no-conclusion, simplification assessment, and expected release NO-GO record.
- Status: completed as D149 / ARCH-135; accepted-with-limits.
- Recorded evidence: `D149-PROJECTION-ORDER-PROBE=PASS`, `D149-SOURCE-WIRING-PROBE=PASS`, `D149-PRESENTATION-AUDIT=PASS`, `D149-COMPILEALL=PASS`, `D149-RUFF=PASS`, `D149-FORMAT=PASS`, `D149-PACKAGE-BUILD=PASS`, `D149-PACKAGE-IDENTITY-PROBE=PASS`; architecture and independent review windows are recorded as `NO_CONCLUSION`, parent and simplification are `PASS`.

## Next bounded slice — recovery projection Ports contract (D150 / ARCH-136)

- Outcome: make recovery-writer outcome projection explicit through a frozen/slotted named callback contract without changing recovery policy.
- Scope: replace only the positional callbacks of `RecoveryProjectionCoordinator` with Qt-free `RecoveryProjectionPorts`; preserve discarded/live/snapshot/content-version branch order and failure projection.
- Non-goals: no recovery capture, writer lifecycle, timers, persistence format, delete service, close policy, notification text, Qt surface, async worker, or runtime startup change.
- Compatibility promise: saved and failed recovery outcomes produce the same schedule/clear/notify/no-op decisions in the same order; MainWindow retains all concrete recovery and application ownership.
- Evidence required: recovery branch/ports probe, MainWindow wiring/source probe, Qt-free import audit, compile/lint/format, package identity, ADR, handoff, independent review/no-conclusion, simplification assessment, and expected release NO-GO record.
- Status: completed as D150 / ARCH-136; accepted-with-limits.
- Recorded evidence: `D150-RECOVERY-BRANCH-PROBE=PASS`, `D150-SOURCE-WIRING-PROBE=PASS`, `D150-PRESENTATION-AUDIT=PASS`, `D150-COMPILEALL=PASS`, `D150-RUFF=PASS`, `D150-FORMAT=PASS`, `D150-PACKAGE-BUILD=PASS`, `D150-PACKAGE-IDENTITY-PROBE=PASS`; architecture and independent review windows are recorded as `NO_CONCLUSION`, parent and simplification are `PASS`.

## Next bounded slice — inactive-selection contrast closure (UI-76 / ARCH-137)

- Outcome: keep inactive selected workspace/list rows readable across every supported theme and accent, including Paper-Sand.
- Scope: change only the existing `QTreeWidget#workspaceTree::item:selected:!active, QListWidget::item:selected:!active` foreground from `text_secondary` to `text_primary` in centralized `presentation/theme.py`.
- Non-goals: no selection model, focus/activation signals, row geometry, palette, token values, locale, font, motion, widget IDs, or application policy change.
- Compatibility promise: inactive selected rows keep the existing `pressed` background, border, accent-left cue, disabled state, selection semantics, and layout; only the readable foreground endpoint changes.
- Evidence required: 3-theme/4-accent contrast projection, selector/state preservation probe, compile/lint/format, package identity, ADR, handoff, independent review/no-conclusion, simplification assessment, and expected release NO-GO record.
- Status: completed as UI-76 / ARCH-137; accepted-with-limits.
- Recorded evidence: `UI76-INACTIVE-SELECTION-PROBE=PASS:12 theme/accent projections`, `UI76-STATE-PRESERVATION-PROBE=PASS`, `UI76-PRESENTATION-AUDIT=PASS`, `UI76-COMPILEALL=PASS`, `UI76-RUFF=PASS`, `UI76-FORMAT=PASS`, `UI76-PACKAGE-BUILD=PASS`, `UI76-PACKAGE-IDENTITY-PROBE=PASS`; architecture and independent review windows are recorded as `NO_CONCLUSION`, parent and simplification are `PASS`.

## Next bounded slice — session-load Ports contract (D151 / ARCH-138)

- Outcome: make session-load baseline projection explicit through a frozen/slotted named callback contract without changing result classification or recovery-first startup ordering.
- Scope: replace only the positional `SessionLoadCoordinator` projection callbacks with Qt-free `SessionLoadPorts`; preserve typed-result validation, `DEFAULT_SESSION` fallback, notification text/level, and recovery-scan scheduling order.
- Non-goals: no session-store format, load validation, recovery scan, restore tracker, worker dispatch, Qt surface, locale, theme, motion, close policy, or runtime startup change.
- Compatibility promise: valid, invalid, malformed, and failed load outcomes keep the same baseline writes, error notification, and recovery-scan scheduling decisions in the same order; MainWindow retains concrete ownership.
- Evidence required: load-result/order/Ports probe, MainWindow wiring/source probe, Qt-free import audit, compile/lint/format, package identity, ADR, handoff, independent review/no-conclusion, simplification assessment, and expected release NO-GO record.
- Status: completed as D151 / ARCH-138; accepted-with-limits.
- Recorded evidence: `D151-LOAD-PROJECTION-PROBE=PASS`, `D151-LOAD-ORDER-PROBE=PASS`, `D151-SOURCE-WIRING-PROBE=PASS`, `D151-QT-FREE-CONTRACT-PROBE=PASS`, `D151-PRESENTATION-AUDIT=PASS`, `D151-COMPILEALL=PASS`, `D151-RUFF=PASS`, `D151-FORMAT=PASS`, `D151-PACKAGE-BUILD=PASS`, `D151-PACKAGE-IDENTITY-PROBE=PASS`; architecture and independent review windows are recorded as `NO_CONCLUSION`, parent and simplification are `PASS`.

## Next bounded slice — session-save Ports contract (D152 / ARCH-139)

- Outcome: make latest-wins session-save admission and callback ownership explicit through a frozen/slotted named Qt-free contract without changing tracker state or persistence policy.
- Scope: replace only the six existing `SessionSaveCoordinator` projection/dispatch callbacks with `SessionSavePorts`; preserve request capture, operation ID allocation, tracker begin/complete/fail, stale suppression, invalid/failure notification, and drain ordering.
- Non-goals: no session-store format, snapshot builder, debounce timer, tracker state model, worker implementation, close policy, notification text, Qt surface, locale, theme, motion, or runtime startup change.
- Compatibility promise: request_latest, drain, submit, complete, and fail keep the same guards, tracker mutations, callback order, and latest-wins behavior; MainWindow retains all concrete session-service, timer, TaskRunner, and policy ownership.
- Evidence required: latest-wins/branch/order probe, MainWindow wiring/source probe, Qt-free import audit, compile/lint/format, package identity, ADR, handoff, independent review/no-conclusion, simplification assessment, and expected release NO-GO record.
- Status: completed as D152 / ARCH-139; accepted-with-limits.
- Recorded evidence: `D152-LATEST-WINS-PROBE=PASS`, `D152-STALE-INVALID-FAILURE-PROBE=PASS`, `D152-PORTS-IMMUTABILITY-PROBE=PASS`, `D152-SOURCE-WIRING-PROBE=PASS`, `D152-QT-FREE-CONTRACT-PROBE=PASS`, `D152-PRESENTATION-AUDIT=PASS`, `D152-COMPILEALL=PASS`, `D152-RUFF=PASS`, `D152-FORMAT=PASS`, `D152-PACKAGE-BUILD=PASS`, `D152-PACKAGE-IDENTITY-PROBE=PASS`; architecture and independent review windows are recorded as `NO_CONCLUSION`, parent and simplification are `PASS`.

## Next bounded slice — Replace All completion Ports contract (D153 / ARCH-140)

- Outcome: make Replace All completion lifecycle ownership explicit through a frozen/slotted named Qt-free contract without changing tracker or editor outcome policy.
- Scope: replace only the six positional `ReplaceAllCompletionCoordinator` callbacks with `ReplaceAllCompletionPorts`; preserve tracker finish/stale suppression, tab unlock, tab-bar enablement, operation completion, and product-specific outcome order.
- Non-goals: no ReplaceAllSession algorithm, slice timer, editor mutation, rollback, cancellation policy, status wording, locale, theme, motion, tab model, or runtime startup change.
- Compatibility promise: a current job runs `tracker.finish -> unlock tab -> enable tab bar -> clear operation active -> complete operation -> project outcome` exactly once; stale jobs remain silent; MainWindow retains concrete editor, FindSurface, tab, operation, and policy ownership.
- Evidence required: completion-order/stale probe, MainWindow wiring/source probe, Qt-free import audit, compile/lint/format, package identity, ADR, handoff, independent review/no-conclusion, simplification assessment, and expected release NO-GO record.
- Status: completed as D153 / ARCH-140; accepted-with-limits.
- Recorded evidence: `D153-COMPLETION-ORDER-PROBE=PASS`, `D153-STALE-JOB-PROBE=PASS`, `D153-PORTS-IMMUTABILITY-PROBE=PASS`, `D153-SOURCE-WIRING-PROBE=PASS`, `D153-QT-FREE-CONTRACT-PROBE=PASS`, `D153-PRESENTATION-AUDIT=PASS`, `D153-COMPILEALL=PASS`, `D153-RUFF=PASS`, `D153-FORMAT=PASS`, `D153-PACKAGE-BUILD=PASS`, `D153-PACKAGE-IDENTITY-PROBE=PASS`; architecture and independent review windows are recorded as `NO_CONCLUSION`, parent and simplification are `PASS`.

## Completed bounded slice — settings-save Ports contract (D154 / ARCH-141)

- Outcome: make settings-save result projection explicit through a frozen/slotted named Qt-free contract without changing tracker classification or settings policy.
- Scope: replace only the three positional `SettingsSaveCoordinator` result callbacks with `SettingsSavePorts`; preserve dispatch binding, stale suppression, invalid-result projection, valid snapshot application, and matching failure projection.
- Non-goals: no settings validation/persistence, dialog controls, theme/locale/font/motion projection, settings tracker state, worker dispatch, notification wording, Qt surface, or runtime startup change.
- Compatibility promise: stale callbacks remain silent; invalid results project invalid feedback; valid `SettingsSnapshot` results apply once; matching failures project failure once; MainWindow retains all concrete settings and policy ownership.
- Evidence required: settings-result/branch/order probe, MainWindow wiring/source probe, Qt-free import audit, compile/lint/format, package identity, ADR, handoff, independent review/no-conclusion, simplification assessment, and expected release NO-GO record.
- Status: completed as D154 / ARCH-141; accepted-with-limits.
- Recorded evidence: `D154-SETTINGS-BRANCH-PROBE=PASS`, `D154-STALE-INVALID-VALID-FAILURE-PROBE=PASS`, `D154-PORTS-IMMUTABILITY-PROBE=PASS`, `D154-SOURCE-WIRING-PROBE=PASS`, `D154-QT-FREE-CONTRACT-PROBE=PASS`, `D154-PRESENTATION-AUDIT=PASS`, `D154-COMPILEALL=PASS`, `D154-RUFF=PASS`, `D154-FORMAT=PASS`, `D154-PACKAGE-BUILD=PASS`, `D154-PACKAGE-IDENTITY-PROBE=PASS`; architecture and independent review windows are recorded as `NO_CONCLUSION`, parent and simplification are `PASS`.

## Next bounded slice — workspace-search Ports contract (D155 / ARCH-142)

- Outcome: make workspace-search result projection explicit through a frozen/slotted named Qt-free contract without changing tracker classification, cancellation, result validation, summary severity, or search policy.
- Scope: replace only the four positional `WorkspaceSearchCoordinator` result callbacks with `WorkspaceSearchPorts`; preserve dispatch binding, stale suppression, invalidation cancellation, invalid-result feedback, valid-result projection, summary notification, and matching failure projection.
- Non-goals: no workspace search service, query/cancellation generation, result model, surface behavior, containment policy, notification wording, Qt surface, locale/theme/motion projection, or runtime startup change.
- Compatibility promise: stale callbacks remain silent; invalidated callbacks project cancellation; invalid results retain error projection; valid results retain summary severity; matching failures retain error projection; MainWindow retains concrete search and policy ownership.
- Evidence required: workspace-search branch/order probe, MainWindow wiring/source probe, Qt-free import audit, compile/lint/format, package identity, ADR, handoff, independent review/no-conclusion, simplification assessment, and expected release NO-GO record.
- Status: completed as D155 / ARCH-142; accepted-with-limits.
- Recorded evidence: `D155-WORKSPACE-SEARCH-BRANCH-PROBE=PASS`, `D155-STALE-INVALIDATED-INVALID-VALID-FAILURE-PROBE=PASS`, `D155-PORTS-IMMUTABILITY-PROBE=PASS`, `D155-SOURCE-WIRING-PROBE=PASS`, `D155-QT-FREE-CONTRACT-PROBE=PASS`, `D155-PRESENTATION-AUDIT=PASS`, `D155-COMPILEALL=PASS`, `D155-RUFF=PASS`, `D155-FORMAT=PASS`, `D155-PACKAGE-BUILD=PASS`, `D155-PACKAGE-IDENTITY-PROBE=PASS`; architecture and independent review windows are recorded as `NO_CONCLUSION`, parent and simplification are `PASS`; an inline probe caught and the parent fixed one stale `_get_surface` reference before acceptance.

## Next bounded slice — plugin-runtime Ports contract (D156 / ARCH-143)

- Outcome: make plugin runtime status/control projection explicit through a frozen/slotted named Qt-free contract without changing runtime policy, permissions, enablement, or failure handling.
- Scope: replace only the four positional `PluginRuntimeCoordinator` presentation callbacks with `PluginRuntimePorts`; preserve failure notification/command refresh order, unavailable and busy guards, runtime toggle exception handling, success refresh/notification/status order, and MainWindow ownership.
- Non-goals: no plugin runtime protocol, trust/permission policy, enablement persistence, external plugin host, catalog governance, command registry, notification wording, Qt surface, locale/theme/motion projection, or runtime startup change.
- Compatibility promise: plugin failure notifies then refreshes commands; unavailable and busy paths remain guarded; runtime exceptions remain error-only; successful toggles refresh commands, notify success, and then show status; MainWindow retains concrete runtime and policy ownership.
- Evidence required: plugin-runtime branch/order probe, MainWindow wiring/source probe, Qt-free import audit, compile/lint/format, package identity, ADR, handoff, independent review/no-conclusion, simplification assessment, and expected release NO-GO record.
- Status: completed as D156 / ARCH-143; accepted-with-limits.
- Recorded evidence: `D156-PLUGIN-RUNTIME-BRANCH-PROBE=PASS`, `D156-FAILURE-UNAVAILABLE-BUSY-EXCEPTION-SUCCESS-PROBE=PASS`, `D156-PORTS-IMMUTABILITY-PROBE=PASS`, `D156-SOURCE-WIRING-PROBE=PASS`, `D156-QT-FREE-CONTRACT-PROBE=PASS`, `D156-PRESENTATION-AUDIT=PASS`, `D156-COMPILEALL=PASS`, `D156-RUFF=PASS`, `D156-FORMAT=PASS`, `D156-PACKAGE-BUILD=PASS`, `D156-PACKAGE-IDENTITY-PROBE=PASS`; architecture and independent review windows are recorded as `NO_CONCLUSION`, parent and simplification are `PASS`.

## Next bounded slice — recovery-delete Ports contract (D157 / ARCH-144)

- Outcome: make recovery-delete result projection explicit through a frozen/slotted named Qt-free contract without changing tracker deletion, pending-delete draining, notifications, or recovery policy.
- Scope: replace only the three positional `RecoveryDeleteCoordinator` callbacks with `RecoveryDeletePorts`; preserve delete completion/failure classification, owner cleanup, success notification, pending-delete scheduling, and error notification order.
- Non-goals: no recovery capture/write/delete service, filesystem durability, pending state model, worker dispatch, tab lifecycle, close policy, notification wording, Qt surface, locale/theme/motion projection, or runtime startup change.
- Compatibility promise: current delete completion releases the tracker, clears a live owner, notifies success when supplied, then schedules pending delete; failures release the tracker and notify error; MainWindow retains concrete recovery and policy ownership.
- Evidence required: recovery-delete branch/order probe, MainWindow wiring/source probe, Qt-free import audit, compile/lint/format, package identity, ADR, handoff, independent review/no-conclusion, simplification assessment, and expected release NO-GO record.
- Status: completed as D157 / ARCH-144; accepted-with-limits.
- Recorded evidence: `D157-RECOVERY-DELETE-BRANCH-PROBE=PASS`, `D157-COMPLETE-PENDING-FAILURE-ORDER-PROBE=PASS`, `D157-PORTS-IMMUTABILITY-PROBE=PASS`, `D157-SOURCE-WIRING-PROBE=PASS`, `D157-QT-FREE-CONTRACT-PROBE=PASS`, `D157-PRESENTATION-AUDIT=PASS`, `D157-COMPILEALL=PASS`, `D157-RUFF=PASS`, `D157-FORMAT=PASS`, `D157-PACKAGE-BUILD=PASS`, `D157-PACKAGE-IDENTITY-PROBE=PASS`; architecture and independent review windows are recorded as `NO_CONCLUSION`, parent and simplification are `PASS`.

## Next bounded slice — recovery-scan Ports contract (D158 / ARCH-145)

- Outcome: make recovery inventory result projection explicit through a frozen/slotted named Qt-free contract without changing tracker classification, inventory validation, candidate prompting, startup continuation, or recovery policy.
- Scope: replace only the four positional `RecoveryScanCoordinator` result callbacks with `RecoveryScanPorts`; preserve stale suppression, invalid-inventory failure, empty non-startup info, candidate prompt order, startup continuation, and failure continuation.
- Non-goals: no RecoveryService, scan worker, recovery candidate model, startup restore policy, filesystem behavior, notification wording, Qt surface, locale/theme/motion projection, close policy, or runtime startup change.
- Compatibility promise: stale callbacks remain silent; invalid inventories retain error projection; empty non-startup scans retain info projection; candidates prompt in order; startup success/failure continues restore with the current session snapshot; MainWindow retains concrete recovery and policy ownership.
- Evidence required: recovery-scan branch/order probe, MainWindow wiring/source probe, Qt-free import audit, compile/lint/format, package identity, ADR, handoff, independent review/no-conclusion, simplification assessment, and expected release NO-GO record.
- Status: completed as D158 / ARCH-145; accepted-with-limits.
- Recorded evidence: `D158-RECOVERY-SCAN-BRANCH-PROBE=PASS`, `D158-STALE-INVALID-EMPTY-CANDIDATE-STARTUP-FAILURE-PROBE=PASS`, `D158-PORTS-IMMUTABILITY-PROBE=PASS`, `D158-SOURCE-WIRING-PROBE=PASS`, `D158-QT-FREE-CONTRACT-PROBE=PASS`, `D158-PRESENTATION-AUDIT=PASS`, `D158-COMPILEALL=PASS`, `D158-RUFF=PASS`, `D158-FORMAT=PASS`, `D158-PACKAGE-BUILD=PASS`, `D158-PACKAGE-IDENTITY-PROBE=PASS`; architecture and independent review windows are recorded as `NO_CONCLUSION`, parent and simplification are `PASS`.

## Next bounded slice — recovery-write Ports contract (D159 / ARCH-146)

- Outcome: make recovery-write callback projection explicit through a frozen/slotted generic named Qt-free contract without changing capture abort, document release, discarded snapshot classification, pending-delete scheduling, saved/failed projection, or recovery policy.
- Scope: replace only the five positional `RecoveryWriteCoordinator` callbacks with `RecoveryWritePorts[JobT, OwnerT]`; preserve submit identity binding and exact complete/fail order.
- Non-goals: no RecoveryService, capture channel, worker dispatch, filesystem durability, tracker state model, recovery projection policy, delete service, tab lifecycle, close policy, notification wording, Qt surface, locale/theme/motion projection, or runtime startup change.
- Compatibility promise: complete remains consume-discarded -> complete document -> finish write/pending delete -> project saved; fail remains consume-discarded -> abort matching capture when present -> complete document -> finish write/pending delete -> project failed; MainWindow retains all concrete recovery and policy ownership.
- Evidence required: recovery-write branch/order/discarded/pending probe, MainWindow wiring/source probe, Qt-free import audit, compile/lint/format, package identity, ADR, handoff, independent review/no-conclusion, simplification assessment, and expected release NO-GO record.
- Status: completed as D159 / ARCH-146; accepted-with-limits. Terra and Sol architecture windows returned `NO_CONCLUSION` in bounded waits.
- Recorded evidence: `D159-RECOVERY-WRITE-BRANCH-PROBE=PASS`, `D159-COMPLETE-FAIL-DISCARDED-PENDING-ORDER-PROBE=PASS`, `D159-SUBMIT-BINDING-PROBE=PASS`, `D159-PORTS-IMMUTABILITY-PROBE=PASS`, `D159-SOURCE-WIRING-PROBE=PASS`, `D159-QT-FREE-CONTRACT-PROBE=PASS`, `D159-PRESENTATION-AUDIT=PASS`, `D159-COMPILEALL=PASS`, `D159-RUFF=PASS`, `D159-FORMAT=PASS`, `D159-PACKAGE-BUILD=PASS`, `D159-PACKAGE-IDENTITY-PROBE=PASS`; Terra, Sol, and independent review windows are recorded as `NO_CONCLUSION`, parent and simplification are `PASS`.

## Next bounded slice — workspace-navigation projection Ports contract (D160 / ARCH-147)

- Outcome: make workspace open/directory projection explicit through a frozen/slotted named Qt-free contract without changing file/folder activation, workspace containment, search invalidation, session persistence, or restore policy.
- Scope: replace only the nine positional `WorkspaceNavigationProjectionCoordinator` callbacks with `WorkspaceNavigationProjectionPorts`; preserve opened-result order, missing-surface short circuit, directory-root lookup, and directory projection.
- Non-goals: no WorkspaceService, document open policy, workspace navigation admission/tracker, file activation, TaskRunner, Qt surface behavior, notification wording, locale/theme/motion projection, close policy, or runtime startup change.
- Compatibility promise: opened result remains invalidate search -> activate workspace -> set search root -> surface guard -> set directory -> notify opened -> request session save -> finish session restore; directory result still no-ops without root/surface and otherwise projects against current root; MainWindow retains concrete ownership.
- Evidence required: workspace-navigation open/directory order probe, MainWindow wiring/source probe, Qt-free import audit, compile/lint/format, package identity, ADR, handoff, independent review/no-conclusion, simplification assessment, and expected release NO-GO record.
- Status: completed as D160 / ARCH-147; accepted-with-limits.
- Recorded evidence: `D160-WORKSPACE-NAVIGATION-BRANCH-PROBE=PASS`, `D160-OPENED-DIRECTORY-ORDER-SHORT-CIRCUIT-PROBE=PASS`, `D160-PORTS-IMMUTABILITY-PROBE=PASS`, `D160-SOURCE-WIRING-PROBE=PASS`, `D160-QT-FREE-CONTRACT-PROBE=PASS`, `D160-PRESENTATION-AUDIT=PASS`, `D160-COMPILEALL=PASS`, `D160-RUFF=PASS`, `D160-FORMAT=PASS`, `D160-PACKAGE-BUILD=PASS`, `D160-PACKAGE-IDENTITY-PROBE=PASS`; architecture and independent review windows are recorded as `NO_CONCLUSION`, parent and simplification are `PASS`.

## Next bounded slice — plugin-host probe Ports contract (D161 / ARCH-148)

- Outcome: make isolated plugin-host diagnostic dependencies explicit through a frozen/slotted named Qt-free contract without changing host security, worker dispatch, or diagnostic result policy.
- Scope: replace only the four positional `PluginHostProbeCoordinator` dependencies with `PluginHostProbePorts`; preserve unavailable/busy guards, start notification, task submission binding, stale operation suppression, typed-result validation, severity mapping, and failure notification.
- Non-goals: no PluginHostClient protocol, process containment, external execution policy, worker implementation, plugin catalog/governance, notification wording, Qt surface, locale/theme/motion projection, close policy, or runtime startup change.
- Compatibility promise: unavailable host remains an error and does not submit work; an in-flight host probe remains a warning and does not submit work; a new probe notifies info before dispatch; only the active operation ID completes; invalid results remain errors; ready/rejected/other states retain success/warning/error mapping; failures remain errors; MainWindow retains host, tracker, runner, notification, and security-policy ownership.
- Evidence required: plugin-host branch/order/stale probe, MainWindow wiring/source probe, Qt-free import audit, compile/lint/format, package identity, ADR, handoff, independent review/no-conclusion, simplification assessment, and expected release NO-GO record.
- Status: completed as D161 / ARCH-148; accepted-with-limits. Architecture and independent review windows are recorded as `NO_CONCLUSION`.
- Recorded evidence: `D161-PLUGIN-HOST-BRANCH-PROBE=PASS`, `D161-PLUGIN-HOST-ORDER-STALE-GUARD-PROBE=PASS`, `D161-PORTS-IMMUTABILITY-PROBE=PASS`, `D161-SOURCE-WIRING-PROBE=PASS`, `D161-QT-FREE-CONTRACT-PROBE=PASS`, `D161-PRESENTATION-AUDIT=PASS`, `D161-COMPILEALL=PASS`, `D161-RUFF=PASS`, `D161-FORMAT=PASS`, `D161-PACKAGE-BUILD=PASS`, `D161-PACKAGE-IDENTITY-PROBE=PASS`; parent and simplification are `PASS`, runtime and release gates remain open.

## Next bounded slice — plugin-catalog Ports contract (D162 / ARCH-149)

- Outcome: make catalog scan and descriptor-governance dependencies explicit through a frozen/slotted named Qt-free contract without changing metadata validation, approval ledger, external execution policy, or plugin loading boundaries.
- Scope: replace only the six positional `PluginCatalogCoordinator` dependencies with `PluginCatalogPorts`; preserve unavailable/busy guards, scan info-before-dispatch, typed snapshot validation, summary severity, governance target validation, scan/governance mutual exclusion, disable/reenable actions, governance success/rescan order, stale suppression, and failure notification.
- Non-goals: no PluginCatalogService, PluginApprovalService, approval ledger, execution gate, plugin host, plugin loading, worker implementation, notification wording, Qt surface, locale/theme/motion projection, close policy, or runtime startup change.
- Compatibility promise: absent catalog and absent approval service remain errors; active scan/governance remains a warning; valid scans notify summary then project the snapshot; invalid/stale results remain silent/error as before; governance remains disabled during work, success notifies then rescans, failures reenable actions and notify error; MainWindow retains concrete plugin/security policy ownership.
- Evidence required: plugin-catalog scan/governance branch/order/stale probe, MainWindow wiring/source probe, Qt-free import audit, compile/lint/format, package identity, ADR, handoff, independent review/no-conclusion, simplification assessment, and expected release NO-GO record.
- Status: completed as D162 / ARCH-149; accepted-with-limits. Architecture and independent review windows are recorded as `NO_CONCLUSION`.
- Recorded evidence: `D162-PLUGIN-CATALOG-BRANCH-PROBE=PASS`, `D162-PLUGIN-CATALOG-ORDER-STALE-GUARD-PROBE=PASS`, `D162-PORTS-IMMUTABILITY-PROBE=PASS`, `D162-SOURCE-WIRING-PROBE=PASS`, `D162-QT-FREE-CONTRACT-PROBE=PASS`, `D162-PRESENTATION-AUDIT=PASS`, `D162-COMPILEALL=PASS`, `D162-RUFF=PASS`, `D162-FORMAT=PASS`, `D162-PACKAGE-BUILD=PASS`, `D162-PACKAGE-IDENTITY-PROBE=PASS`; parent and simplification are `PASS`, runtime, security, and release gates remain open.

## Next bounded slice — recovery-capture abort Ports contract (D163 / ARCH-150)

- Outcome: make recovery capture-abort callbacks explicit through a frozen/slotted generic named Qt-free contract without changing capture identity, snapshot classification, channel cancellation, session cancellation, document release, or notification policy.
- Scope: replace only the seven positional `RecoveryCaptureAbortCoordinator` callbacks with `RecoveryCaptureAbortPorts[JobT, OwnerT]`; preserve matching-capture guard, finish-capture order, worker-started discarded/release branch, channel abort, session cancel, document completion, live-owner notification, and stale/duplicate suppression.
- Non-goals: no RecoveryCaptureTracker state model, RecoveryService, capture channel implementation, write/delete coordinator, worker dispatch, filesystem durability, tab lifecycle, close policy, notification wording, Qt surface, locale/theme/motion projection, or runtime startup change.
- Compatibility promise: nonmatching jobs remain silent and return false; matching jobs finish capture first, mark discarded or release an unstarted snapshot when a channel exists, abort the channel, cancel the session, complete the document, then notify only when requested and live; MainWindow retains recovery, channel, session, tab, and policy ownership.
- Evidence required: recovery-capture branch/order/guard probe, MainWindow wiring/source probe, Qt-free import audit, compile/lint/format, package identity, ADR, handoff, independent high-risk review/no-conclusion, simplification assessment, and expected release NO-GO record.
- Status: completed as D163 / ARCH-150; accepted-with-limits. Terra architecture and independent high-risk windows are `NO_CONCLUSION`; Sol final adversarial source/order review is `PASS`, while runtime concurrency/durability assurance remains unclaimed.
- Recorded evidence: `D163-RECOVERY-CAPTURE-ABORT-BRANCH-PROBE=PASS`, `D163-RECOVERY-CAPTURE-ABORT-ORDER-GUARD-PROBE=PASS`, `D163-PORTS-IMMUTABILITY-PROBE=PASS`, `D163-SOURCE-WIRING-PROBE=PASS`, `D163-QT-FREE-CONTRACT-PROBE=PASS`, `D163-SOL-ADVERSARIAL-REVIEW=PASS`, `D163-GLOBAL-PRESENTATION-PORTS-INVENTORY=PASS`, `D163-PRESENTATION-AUDIT=PASS`, `D163-COMPILEALL=PASS`, `D163-RUFF=PASS`, `D163-FORMAT=PASS`, `D163-PACKAGE-BUILD=PASS`, `D163-PACKAGE-IDENTITY-PROBE=PASS`; parent and simplification are `PASS`, runtime and release gates remain open.

## D164 / UI-77 — status-message visual weight

- Outcome: strengthen the human-readable hierarchy of transient status messages through one centralized `QLabel#statusMessage` QSS declaration.
- Scope: add only `font-weight: 600` to the existing base selector; preserve info/success/warning/error selectors, text/locale projection, timer/visibility, tooltip, geometry, status policy, and application ownership.
- Non-goals: no status widget, feedback state, notification wording, locale catalog, theme token, font-size, layout, animation, editor, application, or release policy change.
- Compatibility promise: all existing status-message state colors, borders, text, visibility, timer cleanup, dynamic-property projection, and status-rail layout remain source-equivalent.
- Evidence required: base-rule/scope probe, AST parse, four-state selector probe, independent review or explicit no-conclusion, simplification assessment, compile/lint/format, package identity, ADR, handoff, and expected release NO-GO record.
- Status: completed as D164 / UI-77; accepted-with-limits. Architect and independent source reviews are `PASS`; native font metrics, runtime visual review, and release gates remain open.
- Recorded evidence: `D164-AST-PROBE=PASS`, `D164-STATUS-MESSAGE-QSS-PROBE=PASS`, `D164-SINGLE-SCOPE-PROBE=PASS`, `D164-INDEPENDENT-REVIEW=PASS`, `D164-SIMPLIFICATION-ASSESSMENT=PASS`.

## D165 / ARCH-152 — support handoff packet check

- Outcome: make the local support handoff packet a required release traceability artifact without marking support or release readiness complete.
- Scope: require only `docs/support/HANDOFF.md` in `scripts/check.ps1` and `scripts/verify_release_handoff.ps1`; project `support_handoff_packet_exists` in dossier checks/evidence.
- Non-goals: no packet parsing, owner contact, manifest status, `$openGates`, no-go decision, runtime launch, Qt/EXE execution, network, or test asset.
- Compatibility promise: missing packet yields a mechanical failure; existing support/status/open-gate semantics and JSON output remain compatible.
- Evidence required: PowerShell parse, path/scope invariant, packet-check dossier evidence, parent review, independent review/no-conclusion, simplification assessment, static checks, package identity, ADR, handoff, and expected release NO-GO record.
- Status: completed as D165 / ARCH-152; accepted-with-limits. Architect is `PASS`; independent window is `NO_CONCLUSION`; owner acceptance and external release gates remain open.
- Recorded evidence: `D165-POWERSHELL-PARSE-PROBE=PASS`, `D165-SCOPE-INVARIANT-PROBE=PASS`, `D165-SUPPORT-PACKET-CHECK-PROBE=PASS`, `D165-SIMPLIFICATION-ASSESSMENT=PASS`.

## D166 / UI-78 / ARCH-153 — font-choice preview

- Outcome: make the existing Settings font selectors human-readable by previewing each supported interface and editor font family in its own popup item.
- Scope: only `src/quillforge/presentation/settings_dialog.py`; add `QFont`/`FontRole` item data through one private presentation helper.
- Non-goals: no font allowlist, `UserRole`, `currentText()` contract, settings schema, persistence, locale, theme, size, motion, editor application, service, or runtime policy change.
- Compatibility promise: both combos keep their existing item text/order and selected values; `settings_snapshot()` and all existing save/apply paths remain unchanged.
- Evidence required: FontRole/source-shape and settings-contract probes, AST parse, independent review or explicit no-conclusion, simplification assessment, compile/lint/format, package identity, ADR, handoff, and expected release NO-GO record.
- Status: completed as D166 / UI-78 / ARCH-153; accepted-with-limits. Architect is `PASS` with bounded conditions; independent window is `NO_CONCLUSION` because the checkout has no Git baseline; native popup rendering and release gates remain open.
- Recorded evidence: `D166-AST-PROBE=PASS`, `D166-FONT-ROLE-SHAPE-PROBE=PASS`, `D166-SETTINGS-CONTRACT-PROBE=PASS`, `D166-COMPILEALL=PASS`, `D166-RUFF=PASS`, `D166-FORMAT=PASS`, `D166-PACKAGE-BUILD=PASS`, `D166-PACKAGE-IDENTITY-PROBE=PASS`, `D166-CHECK=PASS`, `D166-VERIFY-HANDOFF=PASS`, `D166-RELEASE-DOSSIER-INVARIANT-PROBE=PASS`, `D166-SIMPLIFICATION-ASSESSMENT=PASS`.

## D167 / UI-79 / ARCH-154 — Settings typography preview

- Outcome: make both interface and editor font/size choices visible in the live Settings preview before saving.
- Scope: `settings_preview.py`, `settings_dialog.py`, `i18n.py`, and `theme.py`; use QFont for both samples, localized editor metadata, and object-scoped visual tokens.
- Non-goals: no SettingsSnapshot/SettingsService, font allowlist, UserRole, persistence, post-save application, editor adapter, locale architecture, theme policy, size range, motion, or runtime startup change.
- Compatibility promise: existing preview/theme/accent/locale refresh ordering remains; editor font and size changes now join the same projection path; `preview_stylesheet()` loses typography interpolation inputs.
- Evidence required: QFont/sample, refresh-wiring, scoped-QSS, i18n key/placeholder, AST, compile/lint/format, package identity, ADR, handoff, independent review, simplification, and expected release NO-GO record.
- Status: completed as D167 / UI-79 / ARCH-154; accepted-with-limits. Architect and final independent review are `PASS` after correction; native rendering, fallback metrics, accessibility, and release gates remain open.
- Recorded evidence: `D167-AST-PROBE=PASS`, `D167-EDITOR-FONT-SIZE-PROJECTION-PROBE=PASS`, `D167-REFRESH-WIRING-PROBE=PASS`, `D167-SCOPED-QSS-PROBE=PASS`, `D167-I18N-KEY-PROBE=PASS`, `D167-I18N-PLACEHOLDER-PROBE=PASS`, `D167-QFONT-BOTH-SAMPLES-PROBE=PASS`, `D167-SAFE-PREVIEW-QSS-PROBE=PASS`, `D167-COMPILEALL=PASS`, `D167-RUFF=PASS`, `D167-FORMAT=PASS`, `D167-PACKAGE-BUILD=PASS`, `D167-PACKAGE-IDENTITY-PROBE=PASS`, `D167-CHECK=PASS`, `D167-VERIFY-HANDOFF=PASS`, `D167-RELEASE-DOSSIER-INVARIANT-PROBE=PASS`.

## D168 / UI-80 / ARCH-155 — Workspace entry visual semantics

- Outcome: make file, directory, and inaccessible workspace entries easier to
  distinguish during visual scanning while preserving all existing activation
  and application policy.
- Scope: `src/quillforge/presentation/workspace_panel.py` and
  `src/quillforge/presentation/i18n.py`; reuse QPalette roles, preserve the
  existing authored icon provider, and add localized semantic hints.
- Non-goals: no WorkspaceEntry/domain change, WorkspaceService/MainWindow
  change, signal or click/double-click/Enter behavior change, theme token/QSS
  change, custom delegate, file-open policy, containment, async policy,
  runtime startup, or test asset.
- Architecture window: Dalton the 5th / Luna max — `NO_CONCLUSION` after
  bounded waits; no child PASS claimed.
- Independent review window: Socrates the 5th / Luna max — `NO_CONCLUSION`
  after bounded waits; no child PASS claimed.
- Parent review: `PASS`; simplification assessment: `PASS`.
- Recorded evidence: `D168-AST-PROBE=PASS`,
  `D168-I18N-SEMANTIC-HINT-PROBE=PASS`,
  `D168-TOOLTIP-PRECEDENCE-PROBE=PASS`, `D168-ICON-REFRESH-PROBE=PASS`,
  `D168-SIGNAL-PRESERVATION-PROBE=PASS`, `D168-CONTRAST-PROBE=PASS`,
  `D168-COMPILEALL=PASS`, `D168-RUFF=PASS`, `D168-FORMAT=PASS`,
  `D168-PACKAGE-BUILD=PASS`, `D168-PACKAGE-IDENTITY-PROBE=PASS`,
  `D168-CHECK=PASS`, `D168-VERIFY-HANDOFF=PASS`,
  `D168-RELEASE-DOSSIER-INVARIANT-PROBE=PASS`,
  `D168-SIMPLIFICATION-ASSESSMENT=PASS`.
- Limits: native icon/tooltip/accessibility rendering, DPI, GUI/EXE runtime,
  clean-machine, cross-machine, legal, support, signing, installer, updater,
  and release-owner gates remain open.

## D169 / UI-81 / ARCH-156 — FindBar authored action icons

- Outcome: make Find/Replace actions easier to scan through authored icons while
  preserving all existing text, keyboard, signal, and operation semantics.
- Scope: `src/quillforge/presentation/find_bar.py`,
  `src/quillforge/presentation/icon_contract.py`, and
  `src/quillforge/presentation/icons.py`; use the existing icon provider and
  locale refresh seam.
- Non-goals: no FindSurface/MainWindow/application/domain/service/coordinator,
  query/replacement/case behavior, visibility, busy/cancel, primary-action,
  theme token, QSS, runtime startup, or test asset change.
- Architecture window: Heisenberg the 5th / Luna max — `NO_CONCLUSION` after
  bounded waits; no child PASS claimed.
- Independent review window: Carson the 5th / Luna max — `NO_CONCLUSION`
  after bounded waits; no child PASS claimed.
- Parent review: `PASS`; simplification assessment: `PASS`.
- Recorded evidence: `D169-CONTRACT-AST-PROBE=PASS`,
  `D169-ICONS-AST-PROBE=PASS`, `D169-FIND-AST-PROBE=PASS`,
  `D169-ICON-MAPPING-PROBE=PASS`, `D169-SIGNAL-PRESERVATION-PROBE=PASS`,
  `D169-LOCALE-REFRESH-PROBE=PASS`, `D169-CONTRAST-PROBE=PASS`,
  `D169-COMPILEALL=PASS`, `D169-RUFF=PASS`, `D169-FORMAT=PASS`,
  `D169-PACKAGE-BUILD=PASS`, `D169-PACKAGE-IDENTITY-PROBE=PASS`,
  `D169-CHECK=PASS`, `D169-VERIFY-HANDOFF=PASS`,
  `D169-RELEASE-DOSSIER-INVARIANT-PROBE=PASS`,
  `D169-SIMPLIFICATION-ASSESSMENT=PASS`.
- Limits: native icon painting, text/icon metrics, accessibility, DPI,
  GUI/EXE runtime, clean-machine, cross-machine, legal, support, signing,
  installer, updater, and release-owner gates remain open.

## D170 / UI-82 / ARCH-157 — Command-rail visual hierarchy

- Outcome: reduce the command rail's visual density and improve first-read
  hierarchy without changing command behavior.
- Scope: `src/quillforge/presentation/theme.py` only; refine the existing
  command-bar/context QSS surface, spacing, hit height, radius, and role states.
- Non-goals: no `CommandSurface`, command registry, labels, shortcuts,
  callbacks, signals, icons, theme tokens, application policy, runtime
  startup, or test asset change.
- Architecture window: Raman the 5th / Luna max — `NO_CONCLUSION` after
  bounded waits; no child PASS claimed.
- Independent review window: Copernicus the 5th / Luna max — `NO_CONCLUSION`
  after bounded waits; no child PASS claimed.
- Parent review: `PASS`; simplification assessment: `PASS`.
- Recorded evidence: `D170-QSS-CONTRACT-PROBE=PASS`,
  `D170-CONTRAST-PROBE=PASS`, `D170-STATE-CONTRAST-PROBE=PASS`,
  `D170-COMPILEALL=PASS`, `D170-RUFF=PASS`, `D170-FORMAT=PASS`,
  `D170-PACKAGE-BUILD=PASS`, `D170-PACKAGE-IDENTITY-PROBE=PASS`,
  `D170-CHECK=PASS`, `D170-VERIFY-HANDOFF=PASS`,
  `D170-RELEASE-DOSSIER-INVARIANT-PROBE=PASS`,
  `D170-SIMPLIFICATION-ASSESSMENT=PASS`.
- Limits: native Qt painting, toolbar metrics, accessibility, DPI, GUI/EXE
  runtime, clean-machine, cross-machine, legal, support, signing, installer,
  updater, and release-owner gates remain open.

## D171 / UI-83 / ARCH-158 — Document-tab visual hierarchy

- Outcome: remove repeated box noise from the document tab rail and make the
  current document easier to identify without changing document behavior.
- Scope: `src/quillforge/presentation/theme.py` only; refine the existing
  document-tab rail, tab, state, and close-button QSS.
- Non-goals: no `DocumentTabSurface`, tab creation/removal, title, modified
  state, icon contract, signal, shortcut, locale, application policy, runtime
  startup, or test asset change.
- Architecture window: McClintock the 5th / Luna max — `NO_CONCLUSION` after
  bounded waits; no child PASS claimed.
- Independent review window: Descartes the 5th / Luna max — `NO_CONCLUSION`
  after bounded waits; no child PASS claimed.
- Parent review: `PASS`; simplification assessment: `PASS`.
- Recorded evidence: `D171-QSS-CONTRACT-PROBE=PASS`,
  `D171-CONTRAST-PROBE=PASS`, `D171-COMPILEALL=PASS`, `D171-RUFF=PASS`,
  `D171-FORMAT=PASS`, `D171-PACKAGE-BUILD=PASS`,
  `D171-PACKAGE-IDENTITY-PROBE=PASS`, `D171-CHECK=PASS`,
  `D171-VERIFY-HANDOFF=PASS`, `D171-RELEASE-DOSSIER-INVARIANT-PROBE=PASS`,
  `D171-SIMPLIFICATION-ASSESSMENT=PASS`.
- Limits: native Qt painting/layout, tab metrics, accessibility, DPI, GUI/EXE
  runtime, clean-machine, cross-machine, legal, support, signing, installer,
  updater, and release-owner gates remain open.

## D172 / UI-84 / ARCH-159 — Editor-stage visual depth

- Outcome: make the central editor read as one primary canvas inside a softer
  stage instead of two equally heavy frames.
- Scope: `src/quillforge/presentation/theme.py` only; refine the existing
  `editorShell`, `editor`, focus, and selection QSS projection.
- Non-goals: no `EditorWidget`, `EditorShellSurface`, font, lexer, syntax,
  caret, selection, line-number, wrapping, document, signal, application
  policy, runtime startup, or test asset change.
- Architecture window: Peirce the 5th / Luna max — `NO_CONCLUSION` after
  bounded waits; no child PASS claimed.
- Independent review window: Galileo the 5th / Luna max — `NO_CONCLUSION`
  after bounded waits; no child PASS claimed.
- Parent review: `PASS`; simplification assessment: `PASS`.
- Recorded evidence: `D172-QSS-CONTRACT-PROBE=PASS`,
  `D172-CONTRAST-PROBE=PASS`, `D172-COMPILEALL=PASS`, `D172-RUFF=PASS`,
  `D172-FORMAT=PASS`, `D172-PACKAGE-BUILD=PASS`,
  `D172-PACKAGE-IDENTITY-PROBE=PASS`, `D172-CHECK=PASS`,
  `D172-VERIFY-HANDOFF=PASS`, `D172-RELEASE-DOSSIER-INVARIANT-PROBE=PASS`,
  `D172-SIMPLIFICATION-ASSESSMENT=PASS`.
- Limits: native QScintilla painting/layout, editor metrics, accessibility,
  DPI, GUI/EXE runtime, clean-machine, cross-machine, legal, support,
  signing, installer, updater, and release-owner gates remain open.

## D173 / UI-85 / ARCH-160 — Status-rail visual hierarchy

- Outcome: make the bottom status area calmer and more scannable through
  clearer message, shell-status, and phase-pill hierarchy.
- Scope: `src/quillforge/presentation/theme.py` only; refine the existing
  status-bar/message/rail/context/phase QSS.
- Non-goals: no `StatusSurface`, `StatusRail`, notification text, locale,
  tooltip, timer, severity, phase precedence, accessible-name, state-machine,
  application-policy, runtime startup, or test asset change.
- Architecture window: Chandrasekhar the 5th / Luna max — `NO_CONCLUSION`
  after bounded waits; no child PASS claimed.
- Independent review window: Poincare the 5th / Luna max — `NO_CONCLUSION`
  after bounded waits; no child PASS claimed.
- Parent review: `PASS`; simplification assessment: `PASS`.
- Recorded evidence: `D173-QSS-CONTRACT-PROBE=PASS`,
  `D173-CONTRAST-PROBE=PASS`, `D173-COMPILEALL=PASS`, `D173-RUFF=PASS`,
  `D173-FORMAT=PASS`, `D173-PACKAGE-BUILD=PASS`,
  `D173-PACKAGE-IDENTITY-PROBE=PASS`, `D173-CHECK=PASS`,
  `D173-VERIFY-HANDOFF=PASS`, `D173-RELEASE-DOSSIER-INVARIANT-PROBE=PASS`,
  `D173-SIMPLIFICATION-ASSESSMENT=PASS`.
- Limits: native Qt status-bar painting/layout, metrics, accessibility, DPI,
  GUI/EXE runtime, clean-machine, cross-machine, legal, support, signing,
  installer, updater, and release-owner gates remain open.

## D174 / UI-86 / ARCH-161 — Workspace-dock visual hierarchy

- Outcome: make the left Workspace dock lighter, more breathable, and more
  consistent with the modernized shell while preserving workspace behavior.
- Scope: `theme.py` WorkspaceDock frame/title/close/float QSS only; docking,
  tree data, file/folder activation, search, locale, signals, and policy remain
  unchanged.
- Architecture window: Euler the 5th / Luna max — `NO_CONCLUSION` after
  bounded waits; no child PASS claimed.
- Independent review window: Jason the 5th / Luna max — `NO_CONCLUSION` after
  bounded waits; no child PASS claimed.
- Parent review: `PASS`; simplification assessment: `PASS`.
- Recorded evidence: `D174-QSS-CONTRACT-PROBE=PASS`,
  `D174-WORKSPACE-CONTRAST-PROBE=PASS`, `D174-COMPILEALL=PASS`,
  `D174-RUFF=PASS`, `D174-FORMAT=PASS`, `D174-PACKAGE-BUILD=PASS`,
  `D174-PACKAGE-IDENTITY-PROBE=PASS`, `D174-CHECK=PASS`,
  `D174-VERIFY-HANDOFF=PASS`, `D174-RELEASE-DOSSIER-INVARIANT-PROBE=PASS`,
  `D174-SIMPLIFICATION-ASSESSMENT=PASS`.
- Limits: native Qt docking/title-button metrics, accessibility, DPI,
  GUI/EXE runtime, clean-machine, cross-machine, legal, support, signing,
  installer, updater, and release-owner gates remain open.

## D175 / UI-87 / ARCH-162 — Command Palette visual hierarchy

- Outcome: make the keyboard-first Command Palette easier to scan through
  explicit result hover/focus hierarchy and a readable hint capsule.
- Scope: `theme.py` Command Palette list/hint QSS only; query filtering,
  ordering, stable IDs, selection, item activation, return-key acceptance,
  locale, and modal policy remain unchanged.
- Architecture window: Herschel the 5th / Luna max — `NO_CONCLUSION` after
  bounded waits; no child PASS claimed.
- Independent review window: Euclid the 5th / Luna max — `NO_CONCLUSION` after
  bounded waits; no child PASS claimed.
- Parent review: `PASS`; simplification assessment: `PASS`.
- Recorded evidence: `D175-QSS-CONTRACT-PROBE=PASS`,
  `D175-COMMAND-BEHAVIOR-SOURCE-PROBE=PASS`,
  `D175-COMMAND-PALETTE-CONTRAST-PROBE=PASS`, `D175-COMPILE-RUFF-FORMAT=PASS`,
  `D175-PACKAGE-BUILD=PASS`, `D175-PACKAGE-IDENTITY-PROBE=PASS`,
  `D175-CHECK=PASS`, `D175-VERIFY-HANDOFF=PASS`,
  `D175-RELEASE-DOSSIER-INVARIANT-PROBE=PASS`,
  `D175-SIMPLIFICATION-ASSESSMENT=PASS`.
- Limits: native Qt list/focus metrics, accessibility, DPI, GUI/EXE runtime,
  clean-machine, cross-machine, legal, support, signing, installer, updater,
  and release-owner gates remain open.

## D176 / UI-88 / ARCH-163 — FindBar visual rhythm

- Outcome: make the editor Find/Replace bar calmer and easier to scan while
  preserving all search and replacement behavior.
- Scope: `theme.py` FindBar container/label/checkbox/status QSS only; query,
  replacement, signals, keyboard handling, action roles, locale, and operation
  state remain unchanged.
- Architecture window: Noether the 5th / Luna max — `NO_CONCLUSION` after
  bounded waits; no child PASS claimed.
- Independent review window: Mendel the 5th / Luna max — `NO_CONCLUSION` after
  bounded waits; no child PASS claimed.
- Parent review: `PASS`; simplification assessment: `PASS`.
- Recorded evidence: `D176-FINDBAR-QSS-CONTRACT-PROBE=PASS`,
  `D176-FINDBAR-BEHAVIOR-SOURCE-PROBE=PASS`,
  `D176-FINDBAR-CONTRAST-PROBE=PASS`, `D176-COMPILE-RUFF-FORMAT=PASS`,
  `D176-PACKAGE-BUILD=PASS`, `D176-PACKAGE-IDENTITY-PROBE=PASS`,
  `D176-CHECK=PASS`, `D176-VERIFY-HANDOFF=PASS`,
  `D176-RELEASE-DOSSIER-INVARIANT-PROBE=PASS`,
  `D176-SIMPLIFICATION-ASSESSMENT=PASS`.
- Limits: native Qt FindBar metrics, accessibility, DPI, GUI/EXE runtime,
  clean-machine, cross-machine, legal, support, signing, installer, updater,
  and release-owner gates remain open.

## D177 / UI-89 / ARCH-164 — Settings guidance hierarchy

- Outcome: make Settings guidance easier to scan by turning the existing
  apply-after-save and font-fallback notes into supporting capsules.
- Scope: `theme.py` `settingsNote`/`settingsFontNote` QSS only; locale,
  preview, SettingsSnapshot, Save/Cancel, font fallback, persistence, motion,
  and application policy remain unchanged.
- Architecture window: Wegener the 5th / Luna max — `NO_CONCLUSION` after
  bounded waits; no child PASS claimed.
- Independent review window: Nash the 5th / Luna max — `NO_CONCLUSION` after
  bounded waits; no child PASS claimed.
- Parent review: `PASS`; simplification assessment: `PASS`.
- Recorded evidence: `D177-SETTINGS-NOTE-QSS-CONTRACT-PROBE=PASS`,
  `D177-SETTINGS-BEHAVIOR-SOURCE-PROBE=PASS`,
  `D177-SETTINGS-NOTE-CONTRAST-PROBE=PASS`, `D177-COMPILE-RUFF-FORMAT=PASS`,
  `D177-PACKAGE-BUILD=PASS`, `D177-PACKAGE-IDENTITY-PROBE=PASS`,
  `D177-CHECK=PASS`, `D177-VERIFY-HANDOFF=PASS`,
  `D177-RELEASE-DOSSIER-INVARIANT-PROBE=PASS`,
  `D177-SIMPLIFICATION-ASSESSMENT=PASS`.
- Limits: native Qt dialog metrics, accessibility, DPI, GUI/EXE runtime,
  clean-machine, cross-machine, legal, support, signing, installer, updater,
  and release-owner gates remain open.

## D178 / UI-90 / ARCH-165 — Message dialog action hierarchy

- Outcome: make common message-box actions easier to scan through explicit
  primary, warning, and quiet roles while preserving all decisions.
- Scope: `message_surface.py` standard-button role projection only; reuse the
  existing `theme.py` roles and keep message, recovery, close, locale, and
  application policy unchanged.
- Architecture window: Confucius the 5th / Luna max — `NO_CONCLUSION` after
  bounded waits; no child PASS claimed.
- Independent review window: Boyle the 5th / Luna max — `NO_CONCLUSION` after
  bounded waits; no child PASS claimed.
- Parent review: `PASS`; simplification assessment: `PASS`.
- Recorded evidence: `D178-MESSAGE-ACTION-SOURCE-PROBE=PASS`,
  `D178-MESSAGE-THEME-ROLE-CONTRACT-PROBE=PASS`,
  `D178-MESSAGE-BEHAVIOR-SOURCE-PROBE=PASS`, `D178-COMPILE-RUFF-FORMAT=PASS`,
  `D178-PACKAGE-BUILD=PASS`, `D178-PACKAGE-IDENTITY-PROBE=PASS`,
  `D178-CHECK=PASS`, `D178-VERIFY-HANDOFF=PASS`,
  `D178-RELEASE-DOSSIER-INVARIANT-PROBE=PASS`,
  `D178-SIMPLIFICATION-ASSESSMENT=PASS`.
- Limits: native Qt message-box metrics, accessibility, DPI, GUI/EXE
  runtime, clean-machine, cross-machine, legal, support, signing, installer,
  updater, and release-owner gates remain open.

## D179 / UI-91 / ARCH-166 — Plugin catalog guidance capsule

- Outcome: make plugin governance guidance easier to scan through a scoped
  supporting capsule while preserving catalog behavior.
- Scope: `theme.py` `QDialog#pluginCatalogDialog QLabel#dialogHint` QSS only;
  catalog content, locale, list selection, approve/revoke signals,
  trust/approval/execution policy, and application ownership remain unchanged.
- Architecture window: Cicero the 5th / Luna max — `NO_CONCLUSION` after
  bounded waits; no child PASS claimed.
- Independent review window: Pauli the 5th / Luna max — `NO_CONCLUSION` after
  bounded waits; no child PASS claimed.
- Parent review: `PASS`; simplification assessment: `PASS`.
- Recorded evidence: `D179-PLUGIN-HINT-QSS-CONTRACT-PROBE=PASS`,
  `D179-PLUGIN-HINT-BEHAVIOR-SOURCE-PROBE=PASS`,
  `D179-PLUGIN-HINT-CONTRAST-PROBE=PASS`, `D179-COMPILE-RUFF-FORMAT=PASS`,
  `D179-PACKAGE-BUILD=PASS`, `D179-PACKAGE-IDENTITY-PROBE=PASS`,
  `D179-CHECK=PASS`, `D179-VERIFY-HANDOFF=PASS`,
  `D179-RELEASE-DOSSIER-INVARIANT-PROBE=PASS`,
  `D179-SIMPLIFICATION-ASSESSMENT=PASS`.
- Limits: native Qt dialog metrics, accessibility, DPI, GUI/EXE runtime,
  clean-machine, cross-machine, legal, support, signing, installer, updater,
  and release-owner gates remain open.

## D182 / UI-94 / ARCH-169 — Framework-neutral theme token boundary

- Outcome: separate pure theme/editor token resolution from Qt-specific visual
  projection so future UI iterations have a high-cohesion, low-coupling token
  seam.
- Scope: new `presentation/theme_tokens.py` and the import/compatibility edge
  in `presentation/theme.py`; no QSS selector, widget behavior, settings,
  locale, motion, editor operation, or application policy change.
- Architecture window: Averroes the 5th / Luna max — `NO_CONCLUSION` after
  bounded wait; no child PASS claimed.
- Independent review window: Curie the 5th / Luna max — `NO_CONCLUSION` after
  bounded wait; no child PASS claimed.
- Parent review: `PASS`; simplification assessment: `PASS`.
- Recorded evidence: `D182-TOKEN-BOUNDARY-PROBE=PASS`,
  `D182-TOKEN-COMPATIBILITY-PROBE=PASS`,
  `D182-CONTRAST-ENDPOINT-PROBE=PASS`, `D182-QSS-WIRING-PROBE=PASS`,
  `D182-COMPILE-RUFF-FORMAT=PASS`, `D182-SIMPLIFICATION-ASSESSMENT=PASS`.
- Limits: native Qt rendering, font fallback, accessibility, DPI, GUI/EXE
  runtime, clean-machine, cross-machine, legal, support, signing, installer,
  updater, and release-owner gates remain open.

## D183 / UI-95 / ARCH-170 — Restrained surface-gradient hierarchy

- Outcome: add modern layered depth to the main window, editor shell, and
  command rail without turning controls or the editor canvas into decoration.
- Scope: three generated QSS gradients in `presentation/theme.py`; reuse only
  `ThemeColors.surface_0/1/2`, with all behavior, state selectors, settings,
  locale, motion, editor, and application policy unchanged.
- Architecture window: Halley the 5th / Luna max — `NO_CONCLUSION` after
  bounded wait; no child PASS claimed.
- Independent review window: Ohm the 5th / Luna max — `NO_CONCLUSION` after
  bounded wait; no child PASS claimed.
- Parent review: `PASS`; simplification assessment: `PASS`.
- Recorded evidence: `D183-GRADIENT-CONTRACT-PROBE=PASS`,
  `D183-SURFACE-ENDPOINT-CONTRAST-PROBE=PASS`,
  `D183-COMPILE-RUFF-FORMAT=PASS`, `D183-SIMPLIFICATION-ASSESSMENT=PASS`.
- Limits: native Qt QSS parsing/painting, DPI, screenshots, GUI/EXE runtime,
  clean-machine, cross-machine, legal, support, signing, installer, updater,
  and release-owner gates remain open.

## D184 / ARCH-171 — Local hash-gated distribution scripts

- Outcome: add a cohesive local distribution boundary for a portable QuillForge
  executable without introducing network delivery, silent execution, or broad
  registry ownership.
- Scope: `packaging/QuillForge.Distribution.psm1`, `packaging/install.ps1`,
  `packaging/update.ps1`, and `packaging/uninstall.ps1`; no application Python,
  settings, UI, or runtime behavior changes.
- Contract: accept only a `.exe` whose caller-provided SHA-256 matches, install
  under the user-local Programs directory by default, retain bounded rollback
  copies, and require explicit `-RegisterFileAssociations` plus safe extension
  values for HKCU-only associations. Existing user associations are never
  overwritten; uninstaller cleanup is limited to state-owned paths/values.
- Architecture window: Kierkegaard the 5th / Luna max — `NO_CONCLUSION` after
  bounded wait; no child PASS claimed.
- Independent security/simplification window: Hooke the 5th / Luna max —
  `NO_CONCLUSION` after timeout; no child PASS claimed.
- Parent review: `PASS`; simplification assessment: `PASS` for the shared
  module/entry-point split and bounded, explicit operations.
- Public-source applicability: Python/PyQt6 application distribution only;
  embedded C/C++, MCU, RTOS, and manufacturer requirements are not applicable.
  Shipping/security skill guidance is engineering guidance; public CloudWeGo
  material remains an engineering reference, not a private ByteDance standard.
- Recorded evidence: `D184-STATIC-DISTRIBUTION-PROBE=PASS`,
  `D184-POWERSHELL-PARSE=PASS`, `D184-PACKAGE-IDENTITY-PROBE=PASS`,
  `D184-MANIFEST-TRACEABILITY-PROBE=PASS`,
  `D184-HANDOFF-IDENTITY-PROBE=PASS`,
  `D184-RELEASE-DOSSIER-INVARIANT-PROBE=PASS`.
- Limits: installer/update/registry execution, clean-machine, signing,
  file-association runtime behavior, cross-machine repeatability, and release
  owner approval remain unrun or open.

## D185 / UI-96 / ARCH-172 — Form-control highlight closure

- Outcome: make settings and other existing form controls easier to scan by
  strengthening focus surfaces and adding explicit ComboBox popup hover and
  selected states.
- Scope: two existing centralized QSS regions in
  `src/quillforge/presentation/theme.py`; no widget composition, settings,
  locale, font, motion, signal, or application policy change.
- Architecture window: Hypatia the 5th / Luna max — `NO_CONCLUSION` after
  bounded wait; no child PASS claimed.
- Independent review window: Russell the 5th / Luna max — `NO_CONCLUSION` after
  bounded wait; no child PASS claimed.
- Parent review: `PASS`; simplification assessment: `PASS` because the change
  reuses existing ThemeColors and selectors instead of adding a visual system.
- Recorded evidence: `D185-QSS-HIGHLIGHT-CONTRACT-PROBE=PASS combos=12`,
  `D185-COMPILE-RUFF-FORMAT=PASS`.
- Public-source applicability: Python/PyQt6 presentation only; embedded C/C++,
  MCU, RTOS, and manufacturer requirements are not applicable. Public
  engineering guidance is not a private ByteDance standard or compliance claim.
- Limits: native popup rendering/metrics, accessibility, DPI, GUI/EXE runtime,
  clean-machine, cross-machine, signing, installer, updater, support, and
  release-owner gates remain open.

## D186 / UI-97 / ARCH-173 — Workspace file-entry closure

- Outcome: expose an explicit localized `Open file` action in the workspace
  dock for the already-supported file picker/document-open flow, closing the
  discoverability gap behind the report that the app only opens folders.
- Scope: `workspace_panel.py`, `workspace_surface.py`, `main_window.py`,
  `i18n.py`, and the scoped workspace QSS in `theme.py`; reuse the existing
  `FileDialogSurface` and `DocumentPickerAdmissionCoordinator` boundary. No
  second picker, open service, folder/tree activation policy, or application
  ownership change.
- Architecture window: Arendt the 5th / Luna max — `NO_CONCLUSION` after
  bounded wait; no child PASS claimed.
- Independent review window: Cicero the 6th / Luna max — `NO_CONCLUSION` after
  bounded wait; no child PASS claimed.
- Parent review: `PASS`; simplification assessment: `PASS` because the new
  intent is a single presentation callback that reuses the canonical open
  path instead of duplicating file-opening orchestration.
- Recorded evidence: `D186-WORKSPACE-FILE-ACTION-PROBE=PASS`,
  `D186-SURFACE-CALLBACK-PROBE=PASS`,
  `D186-REUSE-OPEN-BOUNDARY-PROBE=PASS`, `D186-I18N-PROBE=PASS en+zh-CN`,
  `D186-FILE-DIALOG-CHAIN-PROBE=PASS`,
  `D186-FILE-ACTION-QSS-PROBE=PASS`, `D186-COMPILE-RUFF-FORMAT=PASS`,
  `D186-PACKAGE-IDENTITY-PROBE=PASS`,
  `D186-MANIFEST-TRACEABILITY-PROBE=PASS`,
  `D186-HANDOFF-IDENTITY-PROBE=PASS`,
  `D186-RELEASE-DOSSIER-INVARIANT-PROBE=PASS`.
- Public-source applicability: Python/PyQt6 presentation and existing document
  admission flow only; embedded C/C++, MCU, RTOS, and manufacturer
  requirements are not applicable. Public engineering references do not prove
  a private ByteDance standard or any compliance claim.
- Limits: native file-dialog behavior, accessibility-tree output, DPI, GUI/EXE
  runtime, clean-machine, cross-machine, legal, support, signing, installer,
  updater, and release-owner gates remain unrun or open. Release verification
  remains expected no-go because packaged/startup runtime artifacts are absent.

## D187 / UI-98 / ARCH-174 — Responsive Settings scroll boundary

- User-visible gap: the Settings surface contains appearance, preview, editor,
  notes, and actions in one tall layout; at short window heights or larger
  interface fonts, the content can compete with the action rail rather than
  remaining comfortably reachable.
- Proposed outcome: put the existing settings content in one named,
  resizable `QScrollArea`, leave Save/Cancel outside that viewport, and give
  the viewport a transparent token-driven surface so scrolling feels like a
  deliberate modern panel rather than a legacy nested dialog.
- Non-goals: no new settings, no value/range/schema change, no locale or
  persistence change, no signal rewiring, no theme-application timing change,
  no animation change, and no native/runtime acceptance claim.
- Acceptance evidence: source ownership probe, scroll/action containment probe,
  object-name/QSS scope probe, compile/Ruff/format, package identity,
  handoff/register/acceptance/index synchronization, parent review,
  independent review or honest no-conclusion, simplification assessment, and
  expected release no-go record.
- Architecture window: Herschel the 6th / Luna max — `NO_CONCLUSION` after
  bounded wait; no child PASS claimed.
- Independent review window: Jason the 6th / Luna max — `NO_CONCLUSION` after
  bounded wait; no child PASS claimed.
- Parent review: `PASS`; simplification assessment: `PASS` because one named
  scroll boundary preserves the existing Settings owner and pins the existing
  action rail without adding a coordinator or duplicate controls.
- Recorded evidence: `D187-SETTINGS-SCROLL-PROBE=PASS`,
  `D187-COMPILE-RUFF-FORMAT=PASS`, `D187-SIMPLIFICATION-ASSESSMENT=PASS`,
  `D187-PACKAGE-BUILD=PASS`, `D187-PACKAGE-IDENTITY-PROBE=PASS`,
  `D187-CHECK=PASS`, `D187-VERIFY-HANDOFF=PASS`,
  `D187-RELEASE-DOSSIER-INVARIANT-PROBE=PASS`.
- Public-source applicability: Python/PyQt6 presentation only; embedded
  C/C++, MCU, RTOS, and manufacturer requirements are not applicable. Public
  CloudWeGo material is engineering reference only, not a private ByteDance
  standard or compliance claim.
- Limits: native Qt scroll metrics, size hints, keyboard/focus traversal,
  accessibility, DPI, GUI/EXE runtime, screenshots, clean-machine,
  cross-machine, legal, support, signing, installer, updater, and
  release-owner gates remain unrun or open. Release verification remains
  expected no-go because artifact-bound runtime reports are stale.

## Completed bounded visual slice — workspace-search empty-state boundary (D189 / UI-99 / ARCH-175)

- User-visible gap: the workspace-search result viewport is visually blank when
  the dialog is first opened or a completed query has no matches; the status
  line alone does not provide a clear content-state anchor.
- Proposed outcome: add one localized empty-state projection inside the existing
  result stage, switch it with the existing `QListWidget`, and retain the
  current result list unchanged when matches exist.
- Scope: `workspace_search_dialog.py`, the existing search locale catalog, and
  scoped QSS in `presentation/theme.py`; preserve query validation, result item
  identity, double-click file activation, diagnostics, cancellation, close
  behavior, and MainWindow/search-service ownership.
- Non-goals: no search algorithm, worker, filesystem traversal, result model,
  async operation, notification policy, new settings, locale architecture,
  application coordinator, or runtime startup change.
- Acceptance evidence: empty-state source/state probe for initial, loading,
  no-match, and populated branches; localized-key/placeholder probe; scoped
  QSS selector probe; Qt-free policy boundary audit; compile/Ruff/format;
  package identity; ADR, handoff, register, acceptance, index, roadmap;
  independent review or honest no-conclusion; simplification assessment; and
  expected release no-go record.
- Architecture window: Descartes the 6th / Luna max — `NO_CONCLUSION` after
  bounded wait; no child PASS claimed.
- Independent review window: Einstein the 6th / Luna max — `NO_CONCLUSION` after
  bounded wait; no child PASS claimed.
- Parent review: `PASS`; simplification assessment: `PASS` because one stacked
  result stage and one synchronization helper preserve the existing surface
  owner without a new coordinator or styling system.
- Recorded evidence: `D189-EMPTY-STATE-CONTRACT-PROBE=PASS`,
  `D189-LOCALE-AND-QSS-PROBE=PASS`, `D189-STATE-BRANCH-PROBE=PASS`,
  `D189-SIGNAL-ROLE-PRESERVATION-PROBE=PASS`,
  `D189-COMPILE-RUFF-FORMAT=PASS`, `D189-PRESENTATION-AUDIT=PASS`,
  `D189-SIMPLIFICATION-ASSESSMENT=PASS`, `D189-PACKAGE-BUILD=PASS`,
  `D189-PACKAGE-IDENTITY-PROBE=PASS`,
  `D189-MANIFEST-TRACEABILITY-PROBE=PASS`,
  `D189-HANDOFF-IDENTITY-PROBE=PASS`,
  `D189-RELEASE-DOSSIER-INVARIANT-PROBE=PASS`.
- Public-source applicability: Python 3.12/PyQt6 presentation only; embedded
  C/C++, MCU, RTOS, and manufacturer requirements are not applicable. Public
  CloudWeGo material is engineering reference only, not a private ByteDance
  standard or compliance claim.
- Limits: native Qt stack/list metrics, accessibility, DPI, GUI/EXE runtime,
  screenshots, clean-machine, cross-machine, legal, support, signing,
  installer, updater, permission/disk-pressure, and release-owner gates remain
  open. Release verification remains expected no-go because artifact-bound
  runtime reports are stale.

## Completed bounded visual slice — command-palette empty-state boundary (D190 / UI-100 / ARCH-176)

- User-visible gap: Command Palette renders a blank result list when the command
  registry is empty or the query filters every command, leaving keyboard users
  without a clear explanation or next action.
- Proposed outcome: add one localized empty-state projection inside the
  existing Command Palette result stage and keep the existing list, stable
  command IDs, filtering, Enter acceptance, and Esc close behavior unchanged.
- Scope: `command_palette.py`, the existing command-palette locale catalog, and
  scoped QSS in `presentation/theme.py`; preserve command iteration order,
  `Command` ownership, selected ID projection, modal policy, and MainWindow
  command execution ownership.
- Non-goals: no command registry changes, command metadata, filtering
  algorithm, shortcut policy, execution callback, application coordinator,
  locale architecture, or runtime startup change.
- Acceptance evidence: empty/filtered/populated source-state probe;
  localized-key probe; result-role/signal/keyboard preservation probe; scoped
  QSS probe; Qt-free policy boundary audit; compile/Ruff/format; package
  identity; ADR, handoff, register, acceptance, index, roadmap; independent
  review or honest no-conclusion; simplification assessment; and expected
  release no-go record.
- Architecture window: Poincare the 6th / Luna max — `NO_CONCLUSION` after
  bounded wait; no child PASS claimed.
- Independent review window: Laplace the 6th / Luna max — `NO_CONCLUSION` after
  bounded wait; no child PASS claimed.
- Parent review: `PASS`; simplification assessment: `PASS` because one stacked
  result stage and one synchronization helper preserve the existing Command
  Palette owner without a registry change or execution coordinator.
- Recorded evidence: `D190-EMPTY-STATE-CONTRACT-PROBE=PASS`,
  `D190-LOCALE-AND-QSS-PROBE=PASS`,
  `D190-KEYBOARD-EXECUTION-PRESERVATION-PROBE=PASS`,
  `D190-COMPILE-RUFF-FORMAT=PASS`, `D190-PRESENTATION-AUDIT=PASS`,
  `D190-SIMPLIFICATION-ASSESSMENT=PASS`, `D190-PACKAGE-BUILD=PASS`,
  `D190-PACKAGE-IDENTITY-PROBE=PASS`,
  `D190-MANIFEST-TRACEABILITY-PROBE=PASS`,
  `D190-HANDOFF-IDENTITY-PROBE=PASS`,
  `D190-RELEASE-DOSSIER-INVARIANT-PROBE=PASS`.
- Public-source applicability: Python 3.12/PyQt6 presentation only; embedded
  C/C++, MCU, RTOS, and manufacturer requirements are not applicable. Public
  CloudWeGo material is engineering reference only, not a private ByteDance
  standard or compliance claim.
- Limits: native Qt stack/list metrics, accessibility, DPI, GUI/EXE runtime,
  screenshots, clean-machine, cross-machine, legal, support, signing,
  installer, updater, permission/disk-pressure, and release-owner gates remain
  open. Release verification remains expected no-go because artifact-bound
  runtime reports are stale.

## D191 / UI-101 / ARCH-177 — Dynamic find-status localization closure

- User-visible gap: the Chinese Find/Replace surface still leaks English in
  dynamic result messages such as `Replaced N matches` and the bounded
  `Stopped: more than N matches; document unchanged` outcome because the
  existing prefix fallback cannot translate plural suffixes.
- Proposed outcome: add bounded, catalog-backed message matchers at the
  existing presentation localization boundary so singular, plural, and limit
  outcomes remain fully localized while preserving counts and diagnostic
  meaning.
- Scope: `presentation/i18n.py` and its static delivery evidence only; no
  editor operation, worker, command, signal, status-state, or persistence
  behavior changes.
- Non-goals: no new locale backend, no translation service, no user-visible
  wording change in English, no unit-test asset, no runtime/GUI launch, and no
  changes to the Replace All admission or cancellation policy.
- Acceptance evidence: Qt-free matcher/locale probe for English and Chinese,
  exact count preservation, no-match regression probe, compile/Ruff/format,
  package identity, ADR/handoff/register/acceptance/index/roadmap, architect
  consultation, independent review or honest no-conclusion, simplification
  assessment, and expected release no-go record.
- Architecture window: Linnaeus the 6th / Luna max — `NO_CONCLUSION` after
  bounded wait; no child PASS claimed.
- Independent review window: Ampere the 6th / Luna max — `NO_CONCLUSION` after
  bounded wait; no child PASS claimed.
- Parent review: `PASS`; simplification assessment: `PASS` because the strict
  matchers reuse the existing catalog boundary and remove only the unsafe
  partial prefix fallback.
- Recorded evidence: `D191-I18N-MATCHER-PROBE=PASS`,
  `D191-COMPILE-RUFF-FORMAT=PASS`, `D191-PRESENTATION-AUDIT=PASS`,
  `D191-SIMPLIFICATION-ASSESSMENT=PASS`, `D191-PACKAGE-BUILD=PASS`,
  `D191-PACKAGE-IDENTITY-PROBE=PASS`.
- Public-source applicability: Python 3.12/PyQt6 presentation only; embedded
  C/C++, MCU, RTOS, and manufacturer requirements are not applicable. Public
  CloudWeGo material remains engineering reference only, not a private
  ByteDance standard or compliance claim.
- Limits: native Qt rendering, accessibility, DPI, GUI/EXE runtime,
  screenshots, clean-machine, cross-machine, legal, support, signing,
  installer, updater, permission/disk-pressure, and release-owner gates remain
  open. Release verification remains expected no-go because artifact-bound
  runtime reports are stale.

## D192 / UI-102 / ARCH-178 — Plugin failure phase localization closure

- User-visible gap: plugin failure notifications localize the message shell but
  expose stable phases such as `activate`, `deactivate`, `command`, and
  `event` in English inside the Chinese interface.
- Proposed outcome: translate the bounded phase vocabulary inside the existing
  presentation-only plugin failure adapter while preserving plugin IDs and
  diagnostic details verbatim.
- Scope: `presentation/i18n.py` and its static delivery evidence only; no
  plugin manager, event bus, runtime enablement, trust, containment, command
  refresh, or notification severity changes.
- Non-goals: no new locale backend, no phase inference from arbitrary errors,
  no unit-test asset, no GUI/EXE launch, and no runtime plugin activation.
- Acceptance evidence: known-phase/unknown-phase locale probe, plugin ID and
  diagnostic preservation probe, compile/Ruff/format, presentation audit,
  package identity, ADR/handoff/register/acceptance/index/roadmap, architect
  consultation, independent review or honest no-conclusion, simplification
  assessment, and expected release no-go record.
- Architecture window: Bohr the 6th / Luna max — `NO_CONCLUSION` after
  bounded wait; no child PASS claimed.
- Independent review window: Ohm the 6th / Luna max — `NO_CONCLUSION` after
  bounded wait; no child PASS claimed.
- Parent review: `PASS`; simplification assessment: `PASS` because one
  bounded phase map stays inside the existing failure localizer and leaves
  unknown diagnostics visible.
- Recorded evidence: `D192-PLUGIN-PHASE-I18N-PROBE=PASS`,
  `D192-COMPILE-RUFF-FORMAT=PASS`, `D192-PRESENTATION-AUDIT=PASS`,
  `D192-SIMPLIFICATION-ASSESSMENT=PASS`, `D192-PACKAGE-BUILD=PASS`,
  `D192-PACKAGE-IDENTITY-PROBE=PASS`.
- Public-source applicability: Python 3.12/PyQt6 presentation only; embedded
  C/C++, MCU, RTOS, and manufacturer requirements are not applicable. Public
  CloudWeGo material remains engineering reference only, not a private
  ByteDance standard or compliance claim.
- Limits: native Qt rendering, accessibility, DPI, GUI/EXE runtime,
  screenshots, clean-machine, cross-machine, legal, support, signing,
  installer, updater, permission/disk-pressure, and release-owner gates remain
  open. Release verification remains expected no-go because artifact-bound
  runtime reports are stale.

## D193 / UI-103 / ARCH-179 — Close-guard pending feedback localization

- User-visible gap: when close is blocked by retained background work, the
  dynamic pending-count message remains entirely English in the Chinese UI.
- Proposed outcome: add one catalog-backed, count-preserving matcher at the
  existing presentation localization boundary for that known close-guard
  message shape.
- Scope: `presentation/i18n.py` and its static delivery evidence only; no
  close guard, pending-work count, TaskRunner lifecycle, session save, or
  shutdown policy changes.
- Non-goals: no new locale backend, no close behavior change, no unit-test
  asset, no GUI/EXE launch, and no forced worker termination.
- Acceptance evidence: English identity/Chinese count-bearing probe, unknown
  shape fallback, compile/Ruff/format, presentation audit, package identity,
  ADR/handoff/register/acceptance/index/roadmap, architect consultation,
  independent review or honest no-conclusion, simplification assessment, and
  expected release no-go record.
- Architecture window: Raman the 6th / Luna max — `NO_CONCLUSION` after
  bounded wait; no child PASS claimed.
- Independent review window: Aquinas the 6th / Luna max — `NO_CONCLUSION`
  after bounded wait; no child PASS claimed.
- Parent review: `PASS`; simplification assessment: `PASS` because one
  catalog-backed matcher stays inside the existing localization boundary and
  leaves close/TaskRunner policy untouched.
- Recorded evidence: `D193-CLOSE-GUARD-I18N-PROBE=PASS`,
  `D193-COMPILE-RUFF-FORMAT=PASS`, `D193-PRESENTATION-AUDIT=PASS`,
  `D193-SIMPLIFICATION-ASSESSMENT=PASS`, `D193-PACKAGE-BUILD=PASS`,
  `D193-PACKAGE-IDENTITY-PROBE=PASS`.
- Public-source applicability: Python 3.12/PyQt6 presentation only; embedded
  C/C++, MCU, RTOS, and manufacturer requirements are not applicable. Public
  CloudWeGo material remains engineering reference only, not a private
  ByteDance standard or compliance claim.
- Limits: native Qt rendering, accessibility, DPI, GUI/EXE runtime,
  screenshots, clean-machine, cross-machine, legal, support, signing,
  installer, updater, permission/disk-pressure, and release-owner gates remain
  open. Release verification remains expected no-go because artifact-bound
  runtime reports are stale.

## D194 / UI-104 / ARCH-180 — Native control affordance cohesion

- User-visible gap: themed combo boxes, spin boxes, and checkboxes still leave
  their small arrows/indicators to the Fusion default, so the control language
  can look mixed across themes and accent choices.
- Proposed outcome: keep the existing token-driven QSS in `presentation.theme`
  as the single owner and add explicit geometry/color states for the combo
  down-arrow, spin up/down arrows, and checkbox indicator without changing
  widget behavior or settings contracts.
- Scope: `presentation/theme.py` plus static delivery evidence only; no widget
  construction, signals, values, locale, motion policy, application service,
  or domain changes.
- Non-goals: no custom widget class, no image/resource pipeline, no native
  GUI/EXE startup, no screenshot claim, and no unit-test asset.
- Acceptance evidence: generated-QSS selector/contrast probe, compile/Ruff/
  format, presentation audit, package identity, ADR/handoff/register/
  acceptance/index/roadmap, architect consultation or honest no-conclusion,
  independent review or honest no-conclusion, simplification assessment, and
  expected release no-go record.
- Architecture window: Huygens the 6th / Luna max — `NO_CONCLUSION` after
  two bounded waits; the child was closed without a PASS claim.
- Architecture window: Huygens the 6th / Luna max — `NO_CONCLUSION` after
  two bounded waits; no child PASS claimed.
- Independent review window: Bernoulli the 6th / Luna max — `NO_CONCLUSION`
  after two bounded waits; no child PASS claimed.
- Parent review: `PASS`; simplification assessment: `PASS` because explicit
  subcontrol geometry is the smallest cohesive QSS projection.
- Recorded evidence: `D194-CONTROL-AFFORDANCE-QSS-PROBE=PASS`,
  `D194-CONTROL-AFFORDANCE-CONTRAST-PROBE=PASS`,
  `D194-COMPILE-RUFF-FORMAT=PASS`, `D194-PRESENTATION-AUDIT=PASS`,
  `D194-SIMPLIFICATION-ASSESSMENT=PASS`, `D194-PACKAGE-BUILD=PASS`,
  `D194-PACKAGE-IDENTITY-PROBE=PASS`.
- Packaging note: Windows PowerShell 5.1 reached EXE creation but failed at
  the existing `Path.GetRelativePath` manifest step; PowerShell 7 completed
  the package and identity refresh. Compatibility remains a follow-up gap.
- Public-source applicability: Python 3.12/PyQt6 presentation only; embedded
  C/C++, MCU, RTOS, and manufacturer requirements are not applicable. Qt's
  public Style Sheets Reference is an applicable framework source; public
  CloudWeGo material remains engineering reference only, not a private
  ByteDance standard or compliance claim.
- Limits: native QSS parsing/painting, accessibility, DPI, GUI/EXE runtime,
  screenshots, clean-machine, cross-machine, legal, support, signing,
  installer, updater, permission/disk-pressure, and release-owner gates remain
  open. Release verification remains expected no-go because artifact-bound
  runtime reports are stale.

## D195 / ARCH-181 — Windows PowerShell packaging compatibility

- User-visible delivery gap: `scripts/package.ps1` is documented as a Windows
  PowerShell entry point but currently calls .NET Core-only helpers
  (`Path.GetRelativePath`, `SHA256.HashData`, and `Convert.ToHexString`), so
  Windows PowerShell 5.1 can build the EXE and then fail before writing the
  release manifest.
- Proposed outcome: replace only those helper calls with compatibility
  functions based on the existing .NET Framework APIs while preserving source
  file ordering, SHA-256 bytes, manifest schema, artifact copy semantics, and
  rollback cleanup.
- Scope: `scripts/package.ps1` and static delivery evidence only; no installer,
  registry, network, EXE launch, application behavior, or release-gate policy
  changes.
- Non-goals: no new test-only asset, no shell installation, no registry touch,
  no deployment, and no claim that external release gates are closed.
- Acceptance evidence: PowerShell 5.1 and 7 parse/probe, package identity,
  manifest/source traceability, compile/Ruff/format, handoff checks, ADR/
  handoff/register/acceptance/index/roadmap, architect consultation,
  independent review or honest no-conclusion, simplification assessment, and
  expected release no-go record.
- Architecture window: Archimedes the 6th / Luna max — `NO_CONCLUSION` after
  two bounded waits; no child PASS claimed.
- Independent review window: Sartre the 6th / Luna max — `NO_CONCLUSION` after
  two bounded waits; no child PASS claimed.
- Parent review: `PASS`; simplification assessment: `PASS` because the patch
  replaces only APIs unavailable to Windows PowerShell 5.1 and preserves the
  existing source inventory, hash, and manifest flow.
- Recorded evidence: `D195-PS51-PARSE-PROBE=PASS`,
  `D195-PS7-PARSE-PROBE=PASS`, `D195-PS51-PACKAGE=PASS`,
  `D195-PS7-PACKAGE=PASS`, `D195-POWERSHELL-COMPAT-SOURCE-PROBE=PASS`,
  `D195-COMPILE-RUFF-FORMAT=PASS`, `D195-PRESENTATION-AUDIT=PASS`,
  `D195-PACKAGE-IDENTITY-PROBE=PASS`.

## D196 / ARCH-182 — Release verifier PowerShell compatibility

- User-visible delivery gap: the release dossier verifier still passes
  `-DateKind String` to `ConvertFrom-Json`, which makes Windows PowerShell 5.1
  stop before producing the expected no-go dossier even though PowerShell 7
  can run it.
- Proposed outcome: add one local JSON compatibility adapter that preserves
  the PowerShell 7 `DateKind String` behavior when available and falls back to
  the Windows PowerShell 5.1-compatible conversion otherwise. Keep all gate
  predicates, dossier fields, and non-zero no-go semantics unchanged.
- Scope: `scripts/verify_release_handoff.ps1` and static delivery evidence
  only; no performance report refresh, EXE launch, installer, registry,
  network, or release-gate closure.
- Non-goals: no test-only asset, no report mutation beyond the existing current
  dossier write, and no suppression of mechanical failures or open gates.
- Acceptance evidence: PowerShell 5.1/7 parser and verifier probes with the
  same expected no-go failure set, JSON dossier invariant, compile/Ruff/format,
  ADR/handoff/register/acceptance/index/roadmap, architect consultation,
  independent review or honest no-conclusion, simplification assessment, and
  current package identity.
- Architecture window: Helmholtz the 6th / Luna max — `NO_CONCLUSION` after
  two bounded waits; the UTF-8 correction was re-consulted with Franklin the
  6th / Luna max and also returned `NO_CONCLUSION`; no child PASS claimed.
- Independent review window: Euler the 6th / Luna max — `NO_CONCLUSION` after
  two bounded waits; no child PASS claimed.
- Parent review: `PASS`; simplification assessment: `PASS` because one local
  conversion adapter and explicit UTF-8 reads preserve the existing predicates.
- Recorded evidence: `D196-PS51-PARSE-PROBE=PASS`,
  `D196-PS7-PARSE-PROBE=PASS`, `D196-COMPAT-SOURCE-PROBE=PASS`,
  `D196-PS51-VERIFIER=EXPECTED-NO-GO`,
  `D196-PS7-VERIFIER=EXPECTED-NO-GO`,
  `D196-SAME-MECHANICAL-FAILURES-PROBE=PASS`,
  `D196-COMPILE-RUFF-FORMAT=PASS`, `D196-PRESENTATION-AUDIT=PASS`,
  `D196-PACKAGE-BUILD=PASS`, `D196-PACKAGE-IDENTITY-PROBE=PASS`.

## D197 / ARCH-183 — Packaged measurement PowerShell compatibility

- User-visible delivery gap: `scripts/measure_packaged.ps1` still required
  PowerShell 7's JSON `DateKind` parameter and default-encoded report reads.
- Proposed outcome: reuse one local compatibility adapter and explicit UTF-8
  reads for the release manifest and per-run capture report while leaving
  artifact binding, process lifecycle, Qt offscreen policy, capture validation,
  cleanup, and measurement ownership unchanged.
- Scope: `scripts/measure_packaged.ps1` and static/package evidence only; no
  EXE launch, capture, performance report refresh, or external release gate.
- Non-goals: no new runtime abstraction, no test-only asset, no measurement
  claim, and no change to cleanup or failure semantics.
- Acceptance evidence: dual-shell parser/source probes, compile/Ruff/format,
  handoff checks, package identity, ADR/handoff/register/acceptance/index/
  roadmap, architect consultation, independent review or honest
  no-conclusion, simplification assessment, and explicit unrun measurement
  boundary.
- Architecture window: Kierkegaard the 6th / Luna max — `NO_CONCLUSION` after
  two bounded waits; no child PASS claimed.
- Independent review window: Planck the 6th / Luna max — `NO_CONCLUSION` after
  two bounded waits; no child PASS claimed.
- Parent review: `PASS`; simplification assessment: `PASS` because the one
  existing JSON boundary is the smallest compatible change.
- Recorded evidence: `D197-PS51-PARSE-PROBE=PASS`,
  `D197-PS7-PARSE-PROBE=PASS`, `D197-COMPAT-SOURCE-PROBE=PASS`,
  `D197-COMPILE-RUFF-FORMAT=PASS`, `D197-PRESENTATION-AUDIT=PASS`,
  `D197-PACKAGE-BUILD=PASS`, `D197-PACKAGE-IDENTITY-PROBE=PASS`.

## D198 / UI-105 / ARCH-184 — Menu affordance cohesion

- User-visible gap: the centralized menu stylesheet exposed a checked item
  background but no independent indicator contract, so future checkable menu
  actions could depend on color alone and fall back to mixed native chrome.
- Proposed outcome: add token-driven `QMenu::indicator` normal, hover,
  checked, checked-hover, and disabled states plus an explicit
  selected-and-checked item rule without changing any existing QAction.
- Scope: `src/quillforge/presentation/theme.py` and synchronized static
  delivery evidence only; command registration, shortcuts, callbacks, locale,
  settings, motion, and application ownership remain unchanged.
- Non-goals: no custom menu widget, icon/resource pipeline, command behavior,
  GUI/QApplication launch, screenshot, native-rendering claim, or unit-test
  asset.
- Acceptance evidence: generated-QSS selector/contrast probe across 12
  theme/accent combinations, compile/Ruff/format, presentation audit, dual
  shell package identity, ADR/handoff/register/acceptance/index/roadmap,
  architect consultation or honest no-conclusion, independent review or
  honest no-conclusion, simplification assessment, and expected release
  no-go record.
- Architecture window: Hume the 6th / Luna max — `NO_CONCLUSION` after two
  bounded waits; no child PASS claimed.
- Independent review window: Locke the 6th / Luna max — `NO_CONCLUSION` after
  two bounded waits; no child PASS claimed.
- Parent review: `PASS`; simplification assessment: `PASS` because the
  presentation-only QSS block is the smallest cohesive fix.
- Recorded evidence: `D198-MENU-AFFORDANCE-QSS-PROBE=PASS`,
  `D198-CONTRAST-PROBE=PASS`, `D198-COMPILE-RUFF-FORMAT=PASS`,
  `D198-PRESENTATION-AUDIT=PASS`, `D198-PACKAGE-BUILD-PS51=PASS`,
  `D198-PACKAGE-BUILD-PS7=PASS`, `D198-PACKAGE-IDENTITY-PROBE=PASS`.
- Public-source applicability: Python 3.12/PyQt6 presentation only; embedded
  C/C++, MCU, RTOS, and manufacturer requirements are not applicable. Qt's
  public Style Sheets Reference is the applicable framework source; public
  CloudWeGo material remains engineering reference only, not a private
  ByteDance standard or compliance claim.
- Limits: native Qt menu painting, accessibility, DPI, GUI/runtime,
  screenshots, clean-machine, cross-machine, signing, installer, updater,
  legal, support, permission/disk-pressure, hard-power, and release-owner
  evidence remain open. Release verification remains expected no-go because
  artifact-bound runtime reports are stale.

## Risks and Mitigations

| Risk | Impact | Mitigation |
|---|---|---|
| QSS specificity or native metrics differ at runtime | Medium | Keep selectors centralized; record runtime visual evidence as unrun until authorized. |
| Replace All appears too actionable | Medium | Give it a distinct warning treatment and preserve text/keyboard semantics. |
| UI scope leaks into editor/application layers | High | Static source probe and architecture review enforce presentation-only changes. |
| External release evidence is unavailable | High | Keep D7/D8 and release status open; do not infer completion from static checks. |

## D199 / ARCH-185 — Command menu contract closure

- User-visible/operational gap: a plugin can construct a command with an
  arbitrary `menu_id`; the registry accepts it, while the Qt projection later
  drops it silently when no matching menu exists.
- Proposed outcome: define one application-owned supported `MenuId` contract
  and make `CommandRegistry.register` reject unknown menu IDs before the
  command becomes visible to any consumer. Keep menu labels and Qt widgets in
  presentation, and preserve the existing four menu IDs/order.
- Scope: `application.commands`, its existing plugin registration seam, and
  static delivery evidence; no command execution, callback, shortcut, locale,
  menu layout, or plugin trust/enablement policy changes.
- Non-goals: no new dynamic menu system, no silent fallback to Tools, no
  custom Qt menu widget, no unit-test asset, and no GUI/EXE launch.
- Acceptance evidence: source/AST contract probe, valid/invalid registration
  boundary probe without a test asset, dependency-direction audit,
  compile/Ruff/format, package identity, ADR/handoff/register/acceptance/index/
  roadmap, architect consultation or honest no-conclusion, independent review
  or honest no-conclusion, simplification assessment, and expected release
  no-go record.
- Risk: rejecting an unknown value is an intentional compatibility tightening;
  it prevents invisible commands and surfaces plugin integration errors at the
  registration boundary. Existing built-ins and the documented plugin command
  path use `tools`, and remain unchanged.

## D199 / ARCH-185 — completion record

- Outcome: completed with limits. `CommandRegistry` now owns an explicit
  four-value `MenuId` contract and rejects unsupported IDs before admission;
  `CommandSurface` reuses that ordered contract for its existing projection.
- Review: parent `PASS`; behavior-preserving simplification `PASS`.
  `Carson the 6th / Luna max` architecture and `Dirac the 6th / Luna max`
  independent windows both returned `NO_CONCLUSION` after two bounded waits;
  no child PASS is claimed.
- Evidence: `D199-MENU-CONTRACT-PROBE=PASS valid=4 invalid=reject`,
  `D199-BOUNDARY-SCOPE-PROBE=PASS`, `D199-STATIC-AUDIT=PASS`,
  `D199-COMPILE-RUFF-FORMAT=PASS`, `D199-PACKAGE-BUILD-PS51=PASS`,
  `D199-PACKAGE-BUILD-PS7=PASS`.
- Records: ADR-0247, S251, parent/independent review records, and the D199
  handoff are synchronized. Runtime, GUI, plugin integration, and external
  release gates remain open.

## D200 / ARCH-186 — Package source-revision determinism

- User-visible/operational gap: culture-sensitive `Sort-Object FullName` made
  the same 166-file source inventory produce different source revisions under
  Windows PowerShell 5.1 and PowerShell 7.
- Outcome: collect existing canonical source lines in a .NET generic list and
  sort with `System.StringComparer.Ordinal`; source inventory, per-file hashes,
  manifest schema, artifact binding, and packaging ownership remain unchanged.
- Scope: `scripts/package.ps1` and static/package provenance evidence only.
- Non-goals: no application behavior, GUI/EXE launch, installer, updater,
  runtime capture, deployment, or release-gate closure.
- Review: parent `PASS`; simplification `PASS`. `Carver the 6th / Luna max`
  architecture returned `NO_CONCLUSION` after two bounded waits; `Hubble the
  6th / Luna max` independently returned bounded `PASS` after confirming the
  source-line, manifest, and packaging-boundary evidence.
- Evidence: `D200-STABLE-SORT-PROTOTYPE=PASS shells_identical`,
  `D200-PS51-PARSE-PROBE=PASS`, `D200-PS7-PARSE-PROBE=PASS`,
  `D200-DETERMINISTIC-SORT-SOURCE-PROBE=PASS`, matching source revision
  `tree-sha256:79cca7bc5704fca205ef41385a588c49d11c9cef77f7ec9a90d574acafad7646`,
  `D200-COMPILE-RUFF-FORMAT=PASS`, and dual-shell package builds.
- Records: ADR-0248, S252, parent/independent review records, and the D200
  handoff are synchronized. Separate PyInstaller invocations may have
  different artifact bytes; each manifest remains independently bound.

## D201 / UI-106 / ARCH-187 — Disabled menu state hierarchy

- User-visible gap: `QMenu::item:selected` and `QMenu::item:checked` could
  leave a disabled item on an accent/selection background while the generic
  disabled rule changed only the foreground, producing a minimum measured
  disabled-text contrast of about 1.02:1 across the 12 projections.
- Outcome: add a presentation-only compound selector after the generic
  disabled rule so selected-disabled and checked-disabled items use `surface_2`,
  `border_strong`, `text_muted`, and semibold weight.
- Scope: `src/quillforge/presentation/theme.py` QSS only; no QAction state,
  menu projection, command, locale, motion, or application policy changes.
- Non-goals: no dynamic menu, widget subclass, palette branch, GUI/EXE launch,
  screenshot, test-only asset, or external release-gate closure.
- Architecture: `Ramanujan the 6th / Luna max` returned `NO_CONCLUSION` after
  two bounded waits; independent `Harvey the 6th / Luna max` also returned
  `NO_CONCLUSION` after two bounded waits. No child PASS is claimed.
- Parent review: `PASS`; simplification assessment: `PASS` because one
  centralized selector block is the smallest behavior-preserving correction.
- Evidence: `D201-MENU-DISABLED-QSS-PROBE=PASS combinations=12 selectors=2`,
  `D201-DISABLED-CONTRAST-PROBE=PASS minimum=text_muted/surface_2>=4.5`,
  `D201-COMPILE-RUFF-FORMAT=PASS`, `D201-PACKAGE-BUILD-PS51=PASS`,
  `D201-PACKAGE-BUILD-PS7=PASS`, `D201-PACKAGE-IDENTITY-PROBE=PASS`.
- Artifact: PS7 `BFAC209D26B4190515957D2C057CF1DBA22B81F87581E08650722644C2A3CAE6`,
  38,562,358 bytes; `tree-sha256:cecf78580582c4a2feba27753b93022678d4d581441f31d95717291afc2ece90`.
- Limits: native Qt selector specificity, rendering, accessibility, DPI,
  runtime menu interaction, clean-machine, cross-machine, signing, installer,
  updater, legal, support, and release-owner gates remain open.

## D202 / ARCH-188 — Workspace directory capability boundary

- User-visible/operational gap: `application.workspace.WorkspaceService`
  directly called `Path.is_dir()`, so a concrete filesystem predicate leaked
  through the application layer despite the existing `WorkspaceProvider` port.
- Proposed outcome: add one `WorkspaceProvider.is_directory(path)` capability,
  implement it in `FileWorkspaceProvider`, and keep workspace path
  normalization, containment, entry limits, and the existing `ValueError`
  messages in `WorkspaceService`.
- Scope: `application.ports.WorkspaceProvider`,
  `application.workspace`, `infrastructure.workspace_provider`, and static
  delivery records only. `WorkspaceSearchProvider`, document path
  normalization, UI signals, async dispatch, and filesystem enumeration
  remain unchanged.
- Non-goals: no new provider implementation, no search-service refactor, no
  GUI/EXE launch, no unit-test asset, no file deletion, and no release-gate
  closure.
- Compatibility promise: the sole current file adapter retains `Path.is_dir()`
  semantics; invalid workspace and directory paths keep their existing error
  text and containment checks.
- Acceptance evidence: Port/adapter implementation probe, application
  filesystem-predicate static audit, compile/Ruff/format, package identity,
  ADR/handoff/register/acceptance/index/roadmap, architect consultation or
  honest no-conclusion, independent review or honest no-conclusion,
  simplification assessment, and expected release no-go record.

## D202 / ARCH-188 — completion record

- Outcome: completed with limits. `WorkspaceProvider.is_directory` is now the
  sole application-facing directory capability; `WorkspaceService` no longer
  calls `Path.is_dir()` directly and `FileWorkspaceProvider` preserves the
  existing filesystem result semantics.
- Review: parent `PASS`; simplification `PASS`. `Singer the 6th / Luna max`
  architecture and `Bacon the 6th / Luna max` independent windows both
  returned `NO_CONCLUSION` after two bounded waits; no child PASS is claimed.
- Evidence: `D202-WORKSPACE-PREDICATE-PROBE=PASS port=1 adapter=1 app_direct_is_dir=0`,
  `D202-COMPILEALL=PASS`, `D202-RUFF=PASS`, `D202-FORMAT=PASS`,
  `D202-PRESENTATION-AUDIT=PASS`, `D202-PACKAGE-BUILD-PS51=PASS`,
  `D202-PACKAGE-BUILD-PS7=PASS`, `D202-PACKAGE-IDENTITY-PROBE=PASS`.
- Records: ADR-0250, S254, parent/independent review records, and the D202
  handoff are synchronized. Runtime, GUI, filesystem-race, and external
  release gates remain open.

## D203 / ARCH-189 — Shared directory capability for workspace search

- User-visible/operational gap: `application.workspace_search` and its file
  search adapter repeated direct `Path.is_dir()` calls after D202 had already
  established the workspace provider capability.
- Proposed outcome: define one reusable `DirectoryCapability` Protocol in
  `application.ports`; have both workspace provider contracts inherit it, and
  route `WorkspaceSearchService` plus `FileWorkspaceSearchProvider` through
  that same capability.
- Scope: `application.ports`, `application.workspace_search`, and
  `infrastructure.workspace_search_provider`, plus static delivery records.
  Search limits, cancellation, result validation, path containment, UI
  signals, composition, and D202 workspace behavior remain unchanged.
- Non-goals: no new filesystem service, no search-policy rewrite, no GUI/EXE
  launch, no unit-test asset, no file deletion, and no release-gate closure.
- Compatibility promise: the current search adapter preserves the existing
  `Path.is_dir()` predicate and error text; `WorkspaceProvider` keeps the D202
  capability through the shared contract.
- Acceptance evidence: shared-Port/adapter implementation probe, full
  application direct-directory-predicate audit, compile/Ruff/format, package
  identity, ADR/handoff/register/acceptance/index/roadmap, architect
  consultation or honest no-conclusion, independent review or honest
  no-conclusion, simplification assessment, and expected release no-go record.

## D203 / ARCH-189 — completion record

- Outcome: completed with limits. `DirectoryCapability` is now the single
  directory predicate contract reused by workspace navigation and workspace
  search; the application layer contains no direct `.is_dir()` call in either
  use-case module.
- Review: parent `PASS`; simplification `PASS`. `Nash the 6th / Luna max`
  architecture and `Turing the 6th / Luna max` independent windows both
  returned `NO_CONCLUSION` after two bounded waits; no child PASS is claimed.
- Evidence: `D203-DIRECTORY-CAPABILITY-PROBE=PASS shared=1 workspace=1 search=1 application_is_dir=0`,
  `D203-COMPILEALL=PASS`, `D203-RUFF=PASS`, `D203-FORMAT=PASS`,
  `D203-PRESENTATION-AUDIT=PASS`, `D203-PACKAGE-BUILD-PS51=PASS`,
  `D203-PACKAGE-BUILD-PS7=PASS`, `D203-PACKAGE-IDENTITY-PROBE=PASS`.
- Records: ADR-0251, S255, parent/independent review records, and the D203
  handoff are synchronized. Runtime, GUI, filesystem-race, and external
  release gates remain open.

## D204 / UI-107 / ARCH-190 — Workspace-search diagnostics disabled state

- User-visible gap: the Find in Files diagnostics disclosure control is
  disabled during an active search, but the centralized stylesheet had no
  scoped disabled projection; the warning capsule could still look
  actionable.
- Proposed outcome: add one scoped `:disabled` rule after the existing
  base/hover/focus/checked rules, using `surface_2`, `border`,
  `border_strong`, and `text_muted` while preserving checked-and-disabled
  precedence.
- Scope: `presentation.theme` QSS and synchronized delivery records only.
  `WorkspaceSearchDialog.set_busy()`, search query/cancellation/results,
  diagnostics data, locale, signals, and application ownership remain
  unchanged.
- Non-goals: no new widget, token, state coordinator, search-policy rewrite,
  GUI/EXE launch, unit-test asset, file deletion, or release-gate closure.
- Compatibility promise: available diagnostics retains its warning,
  hover/focus, and checked hierarchy; only the disabled busy projection is
  subdued.
- Acceptance evidence: scoped disabled/checked selector probe,
  behavior-preservation source probe, 3-theme × 4-accent token projection,
  compile/Ruff/format, package identity, ADR/handoff/register/acceptance/
  index/roadmap, architect consultation or honest no-conclusion,
  independent review or honest no-conclusion, simplification assessment, and
  expected release no-go record.

## D204 / UI-107 / ARCH-190 — completion record

- Outcome: completed with limits. The diagnostics toggle now has an explicit
  subdued disabled state ordered after checked styling, so a busy search is
  visibly non-actionable without changing enablement or search behavior.
- Review: parent `PASS`; simplification `PASS`. `Hypatia the 6th / Luna max`
  architecture and `Kant the 6th / Luna max` independent windows both
  returned `NO_CONCLUSION` after two bounded waits; no child PASS is claimed.
- Evidence: `D204-DISABLED-STATE-PROBE=PASS`, `D204-COMPILEALL=PASS`,
  `D204-RUFF=PASS`, `D204-FORMAT=PASS`, `D204-PRESENTATION-AUDIT=PASS`,
  `D204-PACKAGE-BUILD-PS51=PASS`, `D204-PACKAGE-BUILD-PS7=PASS`,
  `D204-PACKAGE-IDENTITY-PROBE=PASS`.
- Records: ADR-0252, S256, parent/independent review records, and the D204
  handoff are synchronized. Native rendering, runtime, accessibility, and
  external release gates remain open.

## D205 / UI-108 / ARCH-191 — Find/Replace navigation disabled state

- User-visible gap: `FindBar.set_operation_active()` disables previous/next
  navigation during a cooperative editor operation, but their more-specific
  normal and hover/focus QSS rules had no scoped disabled projection.
- Proposed outcome: add one grouped scoped `:disabled` rule after those rules,
  using `surface_2`, `border`, and `text_muted` so an active Replace All
  operation visibly owns navigation without changing the operation.
- Scope: `presentation.theme` QSS and synchronized delivery records only.
  `FindBar.set_operation_active()`, query/replacement, cancellation, icons,
  locale, signals, and editor policy remain unchanged.
- Non-goals: no new widget, token, state coordinator, editor-policy rewrite,
  GUI/EXE launch, unit-test asset, file deletion, or release-gate closure.
- Compatibility promise: available previous/next retains its existing normal,
  hover, focus, pressed, icon, and keyboard hierarchy; only disabled state is
  subdued.
- Acceptance evidence: scoped disabled/specificity probe, FindBar behavior
  source probe, 3-theme × 4-accent contrast projection, compile/Ruff/format,
  package identity, ADR/handoff/register/acceptance/index/roadmap, architect
  consultation or honest no-conclusion, independent review or honest
  no-conclusion, simplification assessment, and expected release no-go record.

## D205 / UI-108 / ARCH-191 — completion record

- Outcome: completed with limits. Find/Replace previous and next now have an
  explicit subdued disabled state ordered after their higher-specificity
  available-state rules, so active operations are visually distinguishable.
- Review: parent `PASS`; simplification `PASS`. `Gauss the 6th / Luna max`
  architecture and `Pauli the 6th / Luna max` independent windows both
  returned `NO_CONCLUSION` after two bounded waits; no child PASS is claimed.
- Evidence: `D205-DISABLED-STATE-SOURCE-PROBE=PASS`,
  `D205-FINDBAR-BEHAVIOR-PROBE=PASS`,
  `D205-QSS-DISABLED-CONTRAST-PROBE=PASS combinations=12 min=5.14`,
  `D205-COMPILEALL=PASS`, `D205-RUFF=PASS`, `D205-FORMAT=PASS`,
  `D205-PRESENTATION-AUDIT=PASS`, `D205-PACKAGE-BUILD-PS51=PASS`,
  `D205-PACKAGE-BUILD-PS7=PASS`, `D205-PACKAGE-IDENTITY-PROBE=PASS`.
- Records: ADR-0253, S257, parent/independent review records, and the D205
  handoff are synchronized. Native rendering, runtime, accessibility, and
  external release gates remain open.

## D207 / UI-109 / ARCH-192 — Workspace tree disabled state

- User-visible gap: an already-populated Workspace tree remains visible while
  `WorkspacePanel.set_loading(True)` disables it for a provider refresh, but
  the centralized stylesheet had no scoped disabled container projection.
- Proposed outcome: add one scoped `QTreeWidget#workspaceTree:disabled` rule
  after normal/focus rules using `surface_2`, `border`, and `text_muted`, while
  preserving row-level item states.
- Scope: `presentation.theme` QSS and synchronized delivery records only.
  Workspace data, loading/refresh, selection, activation, navigation,
  cancellation, locale, signals, and provider policy remain unchanged.
- Non-goals: no new widget, token, workspace state service, provider-policy
  rewrite, GUI/EXE launch, unit-test asset, file deletion, or release-gate
  closure.
- Compatibility promise: available tree and item hierarchy remain unchanged;
  only the visible busy container is subdued.
- Acceptance evidence: visible-tree loading source probe, scoped disabled and
  item-state probe, 3-theme × 4-accent contrast projection, compile/Ruff/
  format, package identity, ADR/handoff/register/acceptance/index/roadmap,
  architect consultation or honest no-conclusion, independent review or
  honest no-conclusion, simplification assessment, and expected release
  no-go record.

## D207 / UI-109 / ARCH-192 — completion record

- Outcome: completed with limits. The visible populated Workspace tree now
  has an explicit subdued disabled container during provider refresh, while
  row states and navigation behavior remain unchanged.
- Review: parent `PASS`; simplification `PASS`. `Boyle the 6th / Luna max`
  architecture and `Parfit the 6th / Luna max` independent windows both
  returned `NO_CONCLUSION` after two bounded waits; no child PASS is claimed.
- Evidence: `D207-DISABLED-STATE-SOURCE-PROBE=PASS`,
  `D207-QSS-DISABLED-CONTRAST-PROBE=PASS combinations=12 min=5.14`,
  `D207-COMPILEALL=PASS`, `D207-RUFF=PASS`, `D207-FORMAT=PASS`,
  `D207-PRESENTATION-AUDIT=PASS`, `D207-PACKAGE-BUILD-PS51=PASS`,
  `D207-PACKAGE-BUILD-PS7=PASS`, `D207-PACKAGE-IDENTITY-PROBE=PASS`.
- Records: ADR-0254, S258, parent/independent review records, and the D207
  handoff are synchronized. Native rendering, runtime, accessibility, and
  external release gates remain open.

## D208 / UI-110 / ARCH-193 — Editor locked disabled state

- User-visible gap: Replace All locks the active QScintilla editor through the
  existing adapter, but the editor's normal/focus QSS had no disabled canvas
  projection, so the main work area could still look editable.
- Proposed outcome: add one scoped `QsciScintilla#editor:disabled` rule after
  normal/focus rules using `surface_1`, `border_strong`, and `pressed`, while
  preserving text/lexer colors and all editor operation ownership.
- Scope: `presentation.theme` QSS and synchronized delivery records only.
  Editor lock/unlock, Replace All, selection, caret, syntax, locale, signals,
  cancellation, tab, and close policy remain unchanged.
- Non-goals: no broad disabled text color, lexer mutation, new editor state
  service, GUI/EXE launch, unit-test asset, file deletion, or release-gate
  closure.
- Compatibility promise: available editor canvas/text/selection behavior
  remains unchanged; only the locked canvas projection is subdued.
- Acceptance evidence: locked-editor call-chain and no-text-muting probes,
  3-theme × 4-accent canvas contrast, compile/Ruff/format, package identity,
  ADR/handoff/register/acceptance/index/roadmap, architect consultation or
  honest no-conclusion, independent review or honest no-conclusion,
  simplification assessment, and expected release no-go record.

## D208 / UI-110 / ARCH-193 — completion record

- Outcome: completed with limits. The Replace All locked editor now has a
  subdued canvas/boundary and pressed selection projection without muting
  document text or syntax colors.
- Review: parent `PASS`; simplification `PASS`. `Feynman the 6th / Luna max`
  architecture and `Lovelace the 6th / Luna max` independent windows both
  returned `NO_CONCLUSION` after two bounded waits; no child PASS is claimed.
- Evidence: `D208-DISABLED-STATE-SOURCE-PROBE=PASS`,
  `D208-QSS-CANVAS-CONTRAST-PROBE=PASS combinations=12 min=12.87`,
  `D208-COMPILEALL=PASS`, `D208-RUFF=PASS`, `D208-FORMAT=PASS`,
  `D208-PRESENTATION-AUDIT=PASS`, `D208-PACKAGE-BUILD-PS51=PASS`,
  `D208-PACKAGE-BUILD-PS7=PASS`, `D208-PACKAGE-IDENTITY-PROBE=PASS`.
- Records: ADR-0255, S259, parent/independent review records, and the D208
  handoff are synchronized. Native rendering, runtime, accessibility, and
  external release gates remain open.

## D209 / UI-111 / ARCH-194 — Plugin disabled action hierarchy

- User-visible gap: Plugin Catalog and Plugin Status disable their existing
  governance/lifecycle buttons for invalid, untrusted, or already-resolved
  states, but those buttons previously used only the generic disabled button
  projection and did not retain a clear dialog-scoped policy boundary.
- Proposed outcome: add one grouped scoped `:disabled` rule for the existing
  `primaryAction` and `warningAction` identities in both plugin dialogs using
  `surface_2`, `border`, `border_strong`, and `text_muted`.
- Scope: `presentation.theme` QSS and synchronized delivery records only.
  Plugin enablement predicates, signals, identity, trust/approval/lifecycle
  policy, list data, locale, and application ownership remain unchanged.
- Non-goals: no plugin policy rewrite, new widget, state service, GUI/EXE
  launch, unit-test asset, file deletion, or release-gate closure.
- Compatibility promise: enabled primary/warning actions retain their current
  normal, hover, focus, pressed, icon, and keyboard hierarchy; only disabled
  plugin actions gain a subdued boundary.
- Acceptance evidence: real-object source probe, selector-order probe,
  3-theme × 4-accent disabled contrast projection, compile/Ruff/format,
  package identity, ADR/handoff/register/acceptance/index/roadmap,
  architecture consultation or honest no-conclusion, independent review or
  honest no-conclusion, simplification assessment, and expected release
  no-go record.

## D209 / UI-111 / ARCH-194 — completion record

- Outcome: completed with limits. Plugin Catalog approve/revoke and Plugin
  Status enable/disable now expose a readable scoped disabled boundary without
  changing plugin action policy or lifecycle behavior.
- Review: parent `PASS`; simplification `PASS`. `Averroes the 6th / Luna max`
  architecture and `Goodall the 6th / Luna max` independent windows both
  returned `NO_CONCLUSION` after two bounded waits; no child PASS is claimed.
- Evidence: `D209-PLUGIN-DISABLED-SOURCE-PROBE=PASS`,
  `D209-QSS-DISABLED-CONTRAST-PROBE=PASS combinations=12 min=5.14`,
  `D209-COMPILEALL=PASS`, `D209-RUFF=PASS`, `D209-FORMAT=PASS`,
  `D209-PRESENTATION-AUDIT=PASS`, `D209-PACKAGE-BUILD-PS51=PASS`,
  `D209-PACKAGE-BUILD-PS7=PASS`, `D209-PACKAGE-IDENTITY-PROBE=PASS`.
- Records: ADR-0256, S260, parent/independent review records, and the D209
  handoff are synchronized. Native rendering, runtime, accessibility, and
  external release gates remain open.

## D210 / UI-112 / ARCH-195 — Default document filter coverage

- User-visible gap: the native open/save filter listed only five common
  extensions although the existing document store accepts ordinary text files
  beyond that list; the All files fallback existed but was easy to miss.
- Proposed outcome: expand the existing bilingual `dialog.text_filter` entry
  to 18 common text/source extensions while retaining exactly one All files
  fallback group.
- Scope: `presentation.i18n` catalog and synchronized delivery records only.
  FileDialogSurface, folder selection, async document admission/open/save,
  encoding, persistence, and application policy remain unchanged.
- Non-goals: no new file-open path, MIME registry, document-service rewrite,
  native dialog replacement, GUI/EXE launch, unit-test asset, file deletion,
  or release-gate closure.
- Compatibility promise: both existing open and save dialogs continue sharing
  the same filter; choose_workspace remains directory-only and All files still
  exposes unlisted files.
- Acceptance evidence: bilingual filter grammar/coverage probe, existing
  boundary source probe, compile/Ruff/format, package identity,
  ADR/handoff/register/acceptance/index/roadmap, architecture consultation or
  honest no-conclusion, independent review or honest no-conclusion,
  simplification assessment, and expected release no-go record.

## D210 / UI-112 / ARCH-195 — completion record

- Outcome: completed with limits. Common text/source files are now visible in
  the existing localized file picker by default, while the All files fallback
  and document architecture remain unchanged.
- Review: parent `PASS`; simplification `PASS`. `Beauvoir the 6th / Luna max`
  architecture and `Noether the 6th / Luna max` independent windows both
  returned `NO_CONCLUSION` after two bounded waits; no child PASS is claimed.
- Evidence: `D210-DIALOG-FILTER-PROBE=PASS locales=2 extensions=18
  all-files-fallback=present`, `D210-COMPILEALL=PASS`, `D210-RUFF=PASS`,
  `D210-FORMAT=PASS`, `D210-PRESENTATION-AUDIT=PASS`,
  `D210-PACKAGE-BUILD-PS51=PASS`, `D210-PACKAGE-BUILD-PS7=PASS`,
  `D210-PACKAGE-IDENTITY-PROBE=PASS`.
- Records: ADR-0257, S261, parent/independent review records, and the D210
  handoff are synchronized. Native rendering, runtime, accessibility, and
  external release gates remain open.

## D211 / UI-113 / ARCH-196 — Workspace disabled-item hierarchy

- Outcome: make inaccessible workspace entries and the disabled truncation
  marker read as explicitly non-actionable rows instead of inheriting only
  muted text.
- Scope: add one scoped `QTreeWidget#workspaceTree::item:disabled` QSS rule in
  `presentation.theme`, using existing `surface_2`, `border`,
  `border_strong`, and `text_muted` tokens. Keep the existing selected-disabled
  rule more specific and preserve `WorkspacePanel` state, signals, icons,
  locale, loading behavior, and application ownership.
- Non-goals: no custom delegate, new workspace state service, provider or
  filesystem policy, item data change, file/folder activation change,
  GUI/EXE launch, unit-test asset, file deletion, or release-gate closure.
- Compatibility promise: normal rows, selected rows, selected-disabled rows,
  hover/focus states, the disabled loading container, file first-click,
  folder double-click, Enter/Return activation, and locale/icon refresh remain
  behaviorally unchanged; only ordinary disabled workspace rows gain a clear
  subdued surface and neutral boundary.
- Evidence required: workspace disabled-item source probe, QSS specificity
  probe, 3-theme × 4-accent muted-on-surface contrast probe, independent
  review or honest no-conclusion, simplification assessment, compile/Ruff/
  format, presentation audit, package identity, ADR/handoff/register/
  acceptance/index/roadmap, and expected release NO-GO record.

## D211 / UI-113 / ARCH-196 — completion record

- Outcome: completed with limits. Ordinary disabled workspace rows now expose
  a subdued card surface, neutral left boundary, and readable muted text;
  selected-disabled specificity and all workspace activation policy remain
  intact.
- Review: parent `PASS`; simplification assessment `PASS`. `Maxwell the 6th /
  Luna max` architecture and `Kuhn the 6th / Luna max` independent review
  windows both returned `NO_CONCLUSION` after two bounded waits and were
  closed; no child PASS is claimed.
- Evidence: `D211-WORKSPACE-DISABLED-SOURCE-PROBE=PASS`,
  `D211-QSS-SPECIFICITY-SOURCE-PROBE=PASS`,
  `D211-DISABLED-CONTRAST-PROBE=PASS combinations=12 min=5.14`,
  `D211-COMPILEALL=PASS`, `D211-RUFF=PASS`, `D211-FORMAT=PASS`,
  `D211-PRESENTATION-AUDIT=PASS`, `D211-PACKAGE-BUILD-PS51=PASS`,
  `D211-PACKAGE-BUILD-PS7=PASS`, `D211-PACKAGE-IDENTITY-PROBE=PASS`.
- Records: ADR-0258, S262, parent/independent review records, and the D211
  handoff are synchronized. Native rendering, runtime, accessibility, and
  external release gates remain open.

## D212 / UI-114 / ARCH-197 — Localized untitled tab title

- Outcome: replace the fixed English pathless-document tab title with the
  active localized label and refresh already-open tab titles on locale changes.
- Scope: add `document.untitled` to the existing English/Chinese catalog, pass
  `Locale` through `_tab_title`, and add one `refresh_tab_titles` callback to
  the existing Qt-free locale ports. Preserve saved basenames, dirty markers,
  document state, tab ownership, and open/save policy.
- Non-goals: no title service, document model mutation, file rename, GUI/EXE
  launch, native tab rendering, unit-test asset, or release-gate closure.
- Evidence required: localized title source and i18n probes, refresh-order and
  Ports immutability probe, independent review or honest no-conclusion,
  simplification assessment, compile/Ruff/format, presentation audit, dual
  shell package identity, ADR/handoff/register/acceptance/index/roadmap, and
  expected release NO-GO.

## D212 / UI-114 / ARCH-197 — completion record

- Outcome: completed with limits. Pathless tabs now use the active English or
  Simplified Chinese label, and existing tabs reproject after locale changes;
  saved basenames and dirty markers remain unchanged.
- Review: parent `PASS`; simplification assessment `PASS`. `Faraday the 6th /
  Luna max` architecture and `Kepler the 6th / Luna max` independent review
  windows both returned `NO_CONCLUSION` after two bounded waits and were
  closed; no child PASS is claimed.
- Evidence: `D212-LOCALIZED-TAB-TITLE-SOURCE-PROBE=PASS`,
  `D212-LOCALE-REFRESH-ORDER-PROBE=PASS`,
  `D212-FIXED-UNTITLED-REGRESSION-PROBE=PASS`,
  `D212-I18N-KEY-PROBE=PASS locales=2`, `D212-COMPILEALL=PASS`,
  `D212-RUFF=PASS`, `D212-FORMAT=PASS`, `D212-PRESENTATION-AUDIT=PASS`,
  `D212-PACKAGE-BUILD-PS51=PASS`, `D212-PACKAGE-BUILD-PS7=PASS`,
  `D212-PACKAGE-IDENTITY-PROBE=PASS`.
- Records: ADR-0259, S263, parent/independent review records, and the D212
  handoff are synchronized. Native rendering, runtime, accessibility, and
  external release gates remain open.

## D213 / UI-115 / ARCH-198 — Editor-shell brand edge

- Outcome: give the central editor stage one clear token-driven top brand edge
  so it scans with the same modern hierarchy as the command rail and workspace
  dock.
- Scope: add one `border-top` declaration to the existing
  `QWidget#editorShell` rule using `accent_pink`; preserve the gradient,
  neutral border, radius, editor/tab/FindBar composition, child state rules,
  locale, motion, and document policy.
- Non-goals: no wrapper widget, palette mutation, state service, editor/tab
  behavior, GUI/EXE launch, native rendering, unit-test asset, or release-gate
  closure.
- Evidence required: scoped selector/order and theme-token probes, independent
  review or honest no-conclusion, simplification assessment, compile/Ruff/
  format, presentation audit, dual-shell package identity,
  ADR/handoff/register/acceptance/index/roadmap, and expected release NO-GO.

## D213 / UI-115 / ARCH-198 — completion record

- Outcome: completed with limits. The central editor shell now exposes a
  two-pixel token-driven pink brand edge; child widgets and behavior remain
  unchanged.
- Review: parent `PASS`; simplification assessment `PASS`. `Mencius the 6th /
  Luna max` architecture and `Tesla the 6th / Luna max` independent review
  windows both returned `NO_CONCLUSION` after two bounded waits and were
  closed; no child PASS is claimed.
- Evidence: `D213-EDITOR-SHELL-ACCENT-SOURCE-PROBE=PASS`,
  `D213-ACCENT-PINK-TOKEN-PROBE=PASS themes=3`, `D213-COMPILEALL=PASS`,
  `D213-RUFF=PASS`, `D213-FORMAT=PASS`, `D213-PRESENTATION-AUDIT=PASS`,
  `D213-PACKAGE-BUILD-PS51=PASS`, `D213-PACKAGE-BUILD-PS7=PASS`,
  `D213-PACKAGE-IDENTITY-PROBE=PASS`.
- Records: ADR-0260, S264, parent/independent review records, and the D213
  handoff are synchronized. Native rendering, runtime, accessibility, and
  external release gates remain open.

## D218 / ARCH-202 — Workspace-search application error taxonomy

- Outcome: complete the Phase 3 application error taxonomy at the Qt-free
  workspace-search boundary while preserving built-in `ValueError`/`TypeError`
  compatibility and existing search behavior.
- Scope: add `ApplicationTypeError` and migrate only the 39 workspace-search
  policy, value-object, request, and provider-result validation branches;
  preserve the two `Path.relative_to()` built-in catches, exact messages,
  validation order, provider protocol, directory capability, and result shape.
- Non-goals: no result-union redesign, adapter wrapper, search limit change,
  filesystem traversal change, GUI/EXE launch, unit-test asset, runtime
  concurrency claim, or release-gate closure.
- Evidence required: 39-branch taxonomy/compatibility probe, dependency and
  path-catch boundary probe, independent review or honest no-conclusion,
  simplification assessment, compile/Ruff/format, presentation audit,
  dual-shell package identity, ADR/handoff/register/acceptance/index/roadmap,
  and expected release NO-GO.

## D218 / ARCH-202 — completion record

- Outcome: completed with limits. Workspace-search now uses stable
  `ApplicationValidationError` and `ApplicationTypeError` categories while
  preserving exact messages, built-in catches, validation order, provider
  contracts, and search policy.
- Review: parent `PASS`; simplification `PASS`; `Pascal the 6th / Luna max`
  architecture and `James the 6th / Luna max` independent windows returned
  `NO_CONCLUSION` after two bounded waits and were closed.
- Evidence: `D218-WORKSPACE-SEARCH-TAXONOMY-PROBE=PASS branches=39`,
  `D218-COMPILEALL=PASS`, `D218-RUFF=PASS`, `D218-FORMAT=PASS`,
  `D218-PRESENTATION-AUDIT=PASS`, `D218-PACKAGE-BUILD-PS51=PASS`,
  `D218-PACKAGE-BUILD-PS7=PASS`,
  `D218-PACKAGE-IDENTITY-PROBE=PASS`.
- Records: ADR-0265, S269, parent/independent review records, and the D218
  handoff are synchronized. Runtime provider behavior, concurrency,
  accessibility, and external release gates remain open.

## D217 / UI-117 — Toolbar brand anchor

- Outcome: give the existing top command rail a stable QuillForge identity
  anchor so the shell starts with a deliberate visual landmark.
- Scope: add one non-interactive `QLabel#toolbarBrand` before the existing
  command actions, refresh its product title metadata through the existing
  locale path, and style it through centralized `theme.py` QSS. No QAction,
  command ID, callback, shortcut, context, settings, motion, or application
  policy changes.
- Non-goals: no new brand service, command, icon-registry entry, widget-local
  stylesheet, palette mutation, GUI/EXE launch, unit-test asset, native
  rendering claim, or release-gate closure.
- Evidence required: source/order probe, 12-combination brand contrast probe,
  independent review or honest no-conclusion, simplification assessment,
  compile/Ruff/format, presentation audit, dual-shell package identity,
  ADR/handoff/register/acceptance/index/roadmap, and expected release NO-GO.

## D217 / UI-117 — completion record

- Outcome: completed with limits. The command rail now begins with a compact
  `✦ QuillForge` identity chip that refreshes its title metadata on locale
  changes; existing QAction order, callbacks, shortcuts, context, and
  application ownership remain unchanged.
- Review: parent `PASS`; simplification `PASS`; `Godel the 6th / Luna max`
  architecture and `Darwin the 6th / Luna max` independent windows returned
  `NO_CONCLUSION` after two bounded waits and were closed.
- Evidence: `D217-BRAND-ANCHOR-SOURCE-PROBE=PASS`,
  `D217-BRAND-CONTRAST-PROBE=PASS combos=12 min=11.16`,
  `D217-COMPILEALL=PASS`, `D217-RUFF=PASS`, `D217-FORMAT=PASS`,
  `D217-PRESENTATION-AUDIT=PASS`, `D217-PACKAGE-BUILD-PS51=PASS`,
  `D217-PACKAGE-BUILD-PS7=PASS`,
  `D217-PACKAGE-IDENTITY-PROBE=PASS`.
- Records: ADR-0264, S268, parent/independent review records, and the D217
  handoff are synchronized. Native rendering, accessibility, runtime, and
  external release gates remain open.

## D216 / ARCH-201 — Editor policy error taxonomy

- Outcome: advance the existing Phase 3 application error taxonomy to the
  composition-boundary `EditorOperationPolicy` without changing policy
  defaults, validation messages, dataclass shape, or presentation behavior.
- Scope: migrate only the eight `EditorOperationPolicy` validation branches to
  `ApplicationValidationError`; workspace-search, command-registry,
  domain-model, and presentation validation remain out of scope.
- Non-goals: no catch-all adapter wrapper, result-union redesign, policy
  default change, GUI/EXE launch, unit-test asset, runtime editor claim, or
  release-gate closure.
- Evidence required: eight-field validation/compatibility probe,
  independent review or honest no-conclusion, simplification assessment,
  compile/Ruff/format, presentation audit, dual-shell package identity,
  ADR/handoff/register/acceptance/index/roadmap, and expected release NO-GO.

## D216 / ARCH-201 — completion record

- Outcome: completed with limits. All eight application-owned
  `EditorOperationPolicy` limit invariants now raise the shared
  `ApplicationValidationError` while preserving exact messages, defaults,
  dataclass shape, `ValueError` compatibility, and downstream consumers.
- Review: parent `PASS`; simplification `PASS`; `Leibniz the 6th / Luna max`
  architecture and `Dewey the 6th / Luna max` independent windows returned
  `NO_CONCLUSION` after two bounded waits and were closed.
- Evidence: `D216-EDITOR-POLICY-TAXONOMY-PROBE=PASS fields=8`,
  `D216-COMPILEALL=PASS`, `D216-RUFF=PASS`, `D216-FORMAT=PASS`,
  `D216-PRESENTATION-AUDIT=PASS`, `D216-PACKAGE-BUILD-PS51=PASS`,
  `D216-PACKAGE-BUILD-PS7=PASS`,
  `D216-PACKAGE-IDENTITY-PROBE=PASS`.
- Records: ADR-0263, S267, parent/independent review records, and the D216
  handoff are synchronized. Broader taxonomy coverage, native/runtime,
  accessibility, and external release gates remain open.

## D215 / UI-116 / ARCH-200 — Semantic accent endpoint foregrounds

- Outcome: make the existing primary-action pink hover endpoint use an
  explicit readable foreground token across all supported theme/accent pairs.
- Scope: add `ThemeColors.on_accent_pink` through the existing Qt-free
  `best_on_accent()` resolver and consume it only in the existing
  `QPushButton#primaryAction:hover` selector. No action, settings, locale,
  motion, or application policy changes.
- Non-goals: no new theme ID, color algorithm, widget-local stylesheet,
  palette mutation, GUI/EXE launch, runtime visual claim, or release-gate
  closure.
- Evidence required: complete-token/import probe, 12-combination pink
  contrast and generated-QSS wiring probes, architect/independent review or
  honest no-conclusion, simplification assessment, compile/Ruff/format,
  presentation audit, dual-shell package identity, synchronized records, and
  expected release NO-GO.

## D215 / UI-116 / ARCH-200 — completion record

- Outcome: completed with limits. Primary-action hover text now resolves from
  the endpoint-specific `on_accent_pink` token while existing action behavior,
  settings, locale, fonts, motion, and QSS ownership remain unchanged.
- Review: parent `PASS`; simplification `PASS`; `Newton the 6th / Luna max`
  architecture and `Galileo the 6th / Luna max` independent windows both
  returned `NO_CONCLUSION` after two bounded waits and were closed.
- Evidence: `D215-PINK-ENDPOINT-CONTRAST-PROBE=PASS combos=12`,
  `D215-QSS-PINK-WIRING-PROBE=PASS combos=12`,
  `D215-THEME-IMPORT-PROBE=PASS`, `D215-COMPILEALL=PASS`,
  `D215-RUFF=PASS`, `D215-FORMAT=PASS`, `D215-PRESENTATION-AUDIT=PASS`,
  `D215-PACKAGE-BUILD-PS51=PASS`, `D215-PACKAGE-BUILD-PS7=PASS`,
  `D215-PACKAGE-IDENTITY-PROBE=PASS`.
- Records: ADR-0262, S266, parent/independent review records, and the D215
  handoff are synchronized. Native rendering, runtime, and external release
  gates remain open.

## D214 / ARCH-199 — Application error taxonomy

- Outcome: advance enterprise architecture Phase 3 by classifying
  application-owned validation/state failures without changing behavior.
- Scope: add dependency-free `application.errors` categories and migrate only
  `DocumentService` missing-target validation plus `WorkspaceService` limit,
  missing-root, missing-directory, and containment failures. Preserve exact
  messages, built-in catch compatibility, domain conflict ownership, provider
  calls, and presentation behavior.
- Non-goals: no catch-all adapter wrapper, result-union redesign, domain error
  migration, GUI/EXE launch, runtime filesystem execution, unit-test asset, or
  release-gate closure.
- Evidence required: taxonomy/compatibility/boundary probes, independent
  review or honest no-conclusion, simplification assessment, compile/Ruff/
  format, presentation audit, dual-shell package identity,
  ADR/handoff/register/acceptance/index/roadmap, and expected release NO-GO.

## D214 / ARCH-199 — completion record

- Outcome: completed with limits. Document and workspace use cases now expose
  stable application validation/state categories while existing messages,
  ValueError/RuntimeError catches, document conflicts, containment, provider
  behavior, and UI projection remain unchanged.
- Review: parent `PASS`; simplification assessment `PASS`. `Arendt the 6th /
  Luna max` architecture returned `NO_CONCLUSION`; `Fermat the 6th` completed
  a bounded independent static review with `PASS`; a separate `Aristotle the
  6th / Luna max` window returned `NO_CONCLUSION` after two bounded waits.
- Evidence: `D214-APPLICATION-ERROR-TAXONOMY-SOURCE-PROBE=PASS`,
  `D214-EXCEPTION-COMPATIBILITY-PROBE=PASS`, `D214-COMPILEALL=PASS`,
  `D214-RUFF=PASS`, `D214-FORMAT=PASS`, `D214-PRESENTATION-AUDIT=PASS`,
  `D214-PACKAGE-BUILD-PS51=PASS`, `D214-PACKAGE-BUILD-PS7=PASS`,
  `D214-PACKAGE-IDENTITY-PROBE=PASS`.
- Records: ADR-0261, S265, parent/independent review records, and the D214
  handoff are synchronized. Broader taxonomy coverage, runtime, and external
  release gates remain open.

## D219 / ARCH-203 — Remaining application error taxonomy

- Outcome: completed with limits. The remaining Qt-free application contracts
  now reuse stable validation/state categories without changing behavior.
- Scope: `commands.py`, `events.py`, `plugin_enablement.py`,
  `plugin_execution.py`, `plugin_governance.py`, `plugin_host.py`, `ports.py`,
  `recovery.py`, and `release_metadata.py`; dedicated recovery-channel
  protocol exceptions and other layers remain unchanged.
- Review: parent `PASS`; simplification assessment `PASS`. `Sagan the 6th /
  Luna max` architecture and `Pasteur the 6th / Luna max` independent windows
  returned `NO_CONCLUSION` after two bounded waits and closure.
- Evidence: `D219-APPLICATION-ERROR-TAXONOMY-PROBE=PASS files=9
  direct_generic_raises=0`, `D219-COMPILEALL=PASS`, `D219-RUFF=PASS`,
  `D219-FORMAT=PASS`, `D219-PRESENTATION-AUDIT=PASS`,
  `D219-PACKAGE-BUILD-PS51=PASS`, `D219-PACKAGE-BUILD-PS7=PASS`, and
  `D219-PACKAGE-IDENTITY-PROBE=PASS`.
- Records: ADR-0266, S270, parent/independent review records, and the D219
  handoff are synchronized. Native/runtime and external release gates remain
  open.

## D220 / UI-118 — Main-shell low-noise visual hierarchy

- User outcome: make the main window read as a calmer modern workspace by
  reducing decorative gradients and nested rounded-card noise in the existing
  shell, editor stage, command rail, and document-tab rail.
- Scope: centralized selectors in `src/quillforge/presentation/theme.py` only;
  keep theme tokens, accent endpoints, fonts, locale, motion policy, widget
  object names, actions, signals, focus/hover/pressed/selected/disabled states,
  layout, and application ownership unchanged.
- Non-goals: no new theme, palette token, widget-local stylesheet, custom title
  bar, animation rewrite, GUI/EXE launch, or release-gate closure.
- Evidence required: scoped QSS/state source probe, 3-theme × 4-accent token
  projection/contrast probe, independent review or honest no-conclusion,
  simplification assessment, compile/Ruff/format, presentation audit,
  dual-shell package identity, synchronized records, and expected release
  NO-GO.

## D221 / ARCH-204 — i18n literal-key static gate

- User outcome: prevent future presentation code from silently introducing a
  missing translation key while preserving the existing English/Chinese
  catalog and runtime fallback behavior.
- Scope: extend the existing `scripts/audit_presentation_contracts.py` AST
  audit to compare literal `tr()` keys in `presentation/*.py` with the
  canonical `_ENGLISH` catalog in `presentation/i18n.py`.
- Non-goals: no translation copy changes, no dynamic-key restriction, no
  runtime locale behavior change, no new test-only asset, no GUI/EXE launch,
  and no release-gate closure.
- Evidence required: missing-key source probe, existing presentation audit,
  independent review or honest no-conclusion, simplification assessment,
  compile/Ruff/format, package identity, synchronized records, and expected
  release NO-GO.

## D221 / ARCH-204 — completion record

- Outcome: completed with limits. Literal `tr()` keys in the presentation
  package are now checked against the canonical English catalog while dynamic
  keys and runtime fallback behavior remain unchanged.
- Review: parent `PASS`; simplification assessment `PASS`. `Hegel the 6th /
  Luna max` architecture and `Heisenberg the 6th / Luna max` independent
  windows returned `NO_CONCLUSION` after bounded waits and closure.
- Evidence: `D221-I18N-LITERAL-KEY-PROBE=PASS catalog_keys=262 missing=0
  dynamic_keys=allowed`, `D221-COMPILEALL=PASS`, `D221-RUFF=PASS`,
  `D221-FORMAT=PASS`, `D221-PRESENTATION-AUDIT=PASS`,
  `D221-PACKAGE-BUILD-PS51=PASS`, `D221-PACKAGE-BUILD-PS7=PASS`, and
  `D221-PACKAGE-IDENTITY-PROBE=PASS`.
- Records: ADR-0268, S272, parent/independent review records, and the D221
  handoff are synchronized. Native/runtime and external release gates remain
  open.

## D222 / ARCH-205 — Windowed startup-failure boundary

- User outcome: make early failures in the console-disabled portable EXE
  visible and recoverable instead of silently disappearing before Qt can show.
- Scope: executable entry boundary in `src/quillforge/__main__.py` only;
  write a UTF-8 traceback to user-local QuillForge app data, attempt a native
  Windows error message, and return exit code 1.
- Non-goals: no application/domain/infrastructure/presentation behavior,
  theme/settings/session/plugin policy, PyInstaller console-mode change,
  GUI/EXE launch, or release-gate closure.
- Evidence required: temporary-path startup report probe, source import smoke,
  archive coverage, independent review or honest no-conclusion,
  simplification assessment, compile/Ruff/format, dual-shell package
  identity, synchronized records, and expected release NO-GO.

## D222 / ARCH-205 — completion record

- Outcome: completed with limits. Windowed startup failures now produce a
  user-local diagnostic log and a native Windows fallback message at the
  entry boundary; business behavior and layer ownership are unchanged.
- Review: parent `PASS`; simplification assessment `PASS`. `Russell the 6th /
  Luna max` architecture and `Hooke the 6th / Luna max` independent windows
  returned `NO_CONCLUSION` after bounded waits and closure.
- Evidence: `D222-STARTUP-REPORT-PROBE=PASS bytes=218`,
  `D222-SOURCE-IMPORT-SMOKE=PASS modules=143`, `D222-COMPILEALL=PASS`,
  `D222-RUFF=PASS`, `D222-FORMAT=PASS`, `D222-PRESENTATION-AUDIT=PASS`,
  `D222-PACKAGE-BUILD-PS51=PASS`, `D222-PACKAGE-BUILD-PS7=PASS`,
  `D222-PACKAGE-ARCHIVE-PROBE=PASS`, and
  `D222-PACKAGE-IDENTITY-PROBE=PASS`.
- Records: ADR-0269, S273, parent/independent review records, and the D222
  handoff are synchronized. GUI/runtime and external release gates remain
  open.

## D223 / ARCH-206 — No-window packaged startup diagnostic

- User outcome: make the currently failing-to-open portable EXE diagnosable
  without requiring the normal Qt window to appear.
- Scope: add `--diagnose-startup --report <path>` to the existing
  `quillforge.app.main()` dispatcher before `QApplication`; emit versioned JSON
  covering source/frozen mode, runtime, Qt/QScintilla, composition, and frozen
  resource checks; preserve normal GUI startup and one-file packaging.
- Non-goals: no GUI/EXE launch, native dialog, console-mode change, runtime
  repair claim, application-policy rewrite, or release-gate closure.
- Evidence required: no-window source probe, independent review or honest
  no-conclusion, simplification assessment, compile/Ruff/format, dual-shell
  package build, archive coverage, package identity, synchronized records, and
  expected release NO-GO.

## D223 / ARCH-206 — completion record

- Outcome: completed with limits. The entry dispatcher now exposes a stable
  no-window startup diagnostic contract with deterministic JSON and exit codes;
  normal GUI composition remains unchanged.
- Review: parent `PASS`; simplification assessment `PASS`. `Peirce the 6th /
  Luna max` architecture and `Meitner the 6th / Luna max` independent windows
  returned `NO_CONCLUSION` after bounded waits and closure.
- Evidence: `D223-STATIC-EXIT=0`,
  `D223-SOURCE-DIAGNOSTIC-PROBE=PASS exit=0 failed=0`,
  `D223-PACKAGE-BUILD-PS51=PASS`, `D223-PACKAGE-BUILD-PS7=PASS`,
  `D223-PACKAGE-ARCHIVE-PROBE=PASS`, and
  `D223-PACKAGE-IDENTITY-PROBE=PASS`.
- Records: ADR-0270, S274, parent/independent review records, and the D223
  handoff are synchronized. Frozen runtime and external release gates remain
  open.

## D225 / ARCH-207 — Command-surface locale accessor startup fix

- User outcome: restore startup for the supplied EXE by fixing the captured
  `CommandSurface._locale` AttributeError before the editor shell appears.
- Scope: add one `_locale() -> Locale` accessor over the existing
  `_locale_provider` in `presentation/command_surface.py`; preserve menu,
  toolbar, command, locale, theme, icon, and MainWindow ownership.
- Non-goals: no locale catalog rewrite, command registry change, GUI/EXE launch,
  native rendering, or release-gate closure.
- Evidence required: captured root-cause log, minimal source probe,
  compile/import, no-window diagnostic, independent review or honest
  no-conclusion, simplification assessment, dual-shell package build, archive
  coverage, package identity, synchronized records, and expected release
  NO-GO.

## D225 / ARCH-207 — completion record

- Outcome: completed with limits. The first recorded startup exception is fixed
  at its source boundary and the PS7 portable candidate was rebuilt.
- Review: parent `PASS`; simplification assessment `PASS`. `Volta the 6th /
  Luna max` architecture and `Lorentz the 6th / Luna max` independent windows
  returned `NO_CONCLUSION` after bounded waits and closure.
- Evidence: `D225-STARTUP-LOG-ROOT-CAUSE=PASS`,
  `D225-LOCALE-STARTUP-FIX-PROBE=PASS`, `D225-COMMAND-SURFACE-IMPORT=PASS`,
  `D225-COMPILEALL=PASS`, `D225-SOURCE-DIAGNOSTIC-PROBE=PASS exit=0 failed=0`,
  `D225-PACKAGE-BUILD-PS51=PASS`, `D225-PACKAGE-BUILD-PS7=PASS`,
  `D225-PACKAGE-ARCHIVE-PROBE=PASS`, and
  `D225-PACKAGE-IDENTITY-PROBE=PASS`.
- Records: ADR-0271, S275, parent/independent review records, and the D225
  handoff are synchronized. Native runtime and external release gates remain
  open.

## D226 / ARCH-208 — Private self-call contract audit

- User outcome: prevent another D225-shaped missing presentation accessor from
  reaching shell startup by extending the existing static contract audit.
- Scope: add `_audit_private_self_calls()` to
  `scripts/audit_presentation_contracts.py`; inspect direct `self._private()`
  calls in top-level presentation classes and allow class methods or assigned
  callable/provider attributes.
- Non-goals: no runtime reflection, type-checker migration, Qt/application
  behavior, dynamic `getattr` proof, GUI/EXE launch, unit-test asset, or
  release-gate closure.
- Evidence required: D225 regression-shape probe, presentation audit,
  compile/Ruff/format, independent review or honest no-conclusion,
  simplification assessment, dual-shell package identity, synchronized
  records, and expected release NO-GO.

## D226 / ARCH-208 — completion record

- Outcome: completed with limits. The presentation contract audit now reports
  direct private self calls that have neither a class method nor an assigned
  attribute, while injected locale/provider fields remain valid.
- Review: parent `PASS`; simplification assessment `PASS`. `Lagrange the 6th
  / Luna max` architecture and `Popper the 6th / Luna max` independent
  windows returned `NO_CONCLUSION` after bounded waits and closure.
- Evidence: `D226-PRIVATE-CALL-GATE-PROBE=PASS method=_locale calls=9
  violations=0`, `D226-COMPILEALL=PASS`, `D226-RUFF=PASS`,
  `D226-FORMAT=PASS`, `D226-PRESENTATION-AUDIT=PASS`,
  `D226-PACKAGE-BUILD-PS51=PASS`, `D226-PACKAGE-BUILD-PS7=PASS`,
  `D226-PACKAGE-ARCHIVE-PROBE=PASS`, and
  `D226-PACKAGE-IDENTITY-PROBE=PASS`.
- Records: ADR-0272, S276, parent/independent review records, and the D226
  handoff are synchronized. Native/runtime and external release gates remain
  open.

## D227 / ARCH-209 — Private attribute declaration-aware audit

- User outcome: prevent declaration-related false alarms in the D226 startup
  contract gate when a presentation provider or field is declared at class
  scope.
- Scope: add `_class_attribute_names()` to
  `scripts/audit_presentation_contracts.py` and merge direct class-body
  `Assign`, `AnnAssign`, and `AugAssign` names with existing instance
  assignments for the narrow private self-call check.
- Non-goals: no broad private-read audit, runtime callability inference,
  provider registry, Qt/application behavior, GUI/EXE launch, unit-test asset,
  or release-gate closure.
- Evidence required: class declaration structural probe, presentation audit,
  compile/Ruff/format, independent review or honest no-conclusion,
  simplification assessment, dual-shell package identity, synchronized
  records, and expected release NO-GO.

## D227 / ARCH-209 — completion record

- Outcome: completed with limits. The existing presentation AST audit now
  recognizes direct class-level declarations and annotated fields without
  widening its direct private-call scope.
- Review: parent `PASS`; simplification assessment `PASS`. `Dalton the 6th /
  Luna max` architecture and `Socrates the 6th / Luna max` independent
  windows returned `NO_CONCLUSION` after bounded waits and closure.
- Evidence: `D227-CLASS-ATTRIBUTE-PROBE=PASS names=3`,
  `D227-COMPILEALL=PASS`, `D227-RUFF=PASS`, `D227-FORMAT=PASS`,
  `D227-PRESENTATION-AUDIT=PASS`, package/identity probes, and dual-shell
  project/handoff checks recorded in the D227 handoff.
- Records: ADR-0273, S277, parent/independent review records, and the D227
  handoff are synchronized. Native/runtime and external release gates remain
  open.

## D229 / ARCH-211 — Startup diagnostic path fail-open

- User outcome: ensure a failure in the early diagnostic path cannot mask the
  original startup exception when the report path or log environment is broken.
- Scope: keep the existing `__main__.py` boundary, protect report-path
  resolution, preserve D228's unavailable-context fallback, and protect
  payload construction and writes.
- Non-goals: no Qt/application rewrite, second logger, settings or document
  logging, native EXE launch, unit-test asset, or release-gate closure.
- Evidence required: path fail-open and context fallback probes, compile/Ruff/
  format/presentation audit, parent and independent review records, refreshed
  package identity, synchronized records, project/handoff checks, and expected
  release NO-GO.

## D229 / ARCH-211 — completion record

- Outcome: completed with limits. The existing startup diagnostics boundary now
  fails open for report-path resolution, context collection, payload
  construction, and writes while retaining D228's unavailable-context
  fallback.
- Review: parent `PASS`; independent `Confucius the 6th / Luna max` source
  review `PASS`; simplification assessment `PASS`. `Boole the 6th / Luna max`
  architecture window returned `NO_CONCLUSION` after bounded waiting and
  closure.
- Evidence: `D229-FAIL-OPEN-PATH-PROBE=PASS`,
  `D229-CONTEXT-FALLBACK-PROBE=PASS`, `D229-COMPILEALL=PASS`,
  `D229-RUFF=PASS`, `D229-FORMAT=PASS`,
  `D229-PRESENTATION-AUDIT=PASS`, dual-shell package builds,
  `D229-PACKAGE-ARCHIVE-PROBE=PASS`,
  `D229-PACKAGE-IDENTITY-PROBE=PASS`, and project/handoff checks.
- Records: ADR-0275, S279, parent/independent review records, and the D229
  handoff are synchronized. Native/runtime and external release gates remain
  open.

## D230 / ARCH-212 — Startup fallback display fail-open

- User outcome: ensure an unusual exception object, native fallback failure, or
  broken stderr cannot hide the original early startup failure.
- Scope: keep `_show_startup_failure` at the existing entry boundary, retain
  ordinary error text and exit behavior, and make exception stringification,
  MessageBox, and stderr output best-effort.
- Non-goals: no Qt/application rewrite, new logger, settings or document
  logging, native EXE launch, unit-test asset, or release-gate closure.
- Evidence required: error-string and MessageBox fail-open probes,
  compile/Ruff/format/presentation audit, parent and independent review
  records, refreshed package identity, synchronized records, project/handoff
  checks, and expected release NO-GO.

## D230 / ARCH-212 — completion record

- Outcome: completed with limits. The existing early fallback now treats
  exception stringification, native MessageBox display, and stderr output as
  best-effort while preserving ordinary messages and the final startup exit
  path.
- Review: parent `PASS`; independent `Mendel the 6th / Luna max` static source
  review `PASS`; simplification assessment `PASS`. `Copernicus the 6th / Luna
  max` architecture window returned `NO_CONCLUSION` after bounded waiting and
  closure.
- Evidence: `D230-ERROR-STRING-FAIL-OPEN-PROBE=PASS`,
  `D230-MESSAGEBOX-FAIL-OPEN-PROBE=PASS`, `D230-COMPILEALL=PASS`,
  `D230-RUFF=PASS`, `D230-FORMAT=PASS`,
  `D230-PRESENTATION-AUDIT=PASS`, dual-shell package builds,
  `D230-PACKAGE-ARCHIVE-PROBE=PASS`,
  `D230-PACKAGE-IDENTITY-PROBE=PASS`, and project/handoff checks.
- Records: ADR-0276, S280, parent/independent review records, and the D230
  handoff are synchronized. Native/runtime and external release gates remain
  open; `MessageBoxW` zero-return interpretation is a follow-up observation.

## D232 / ARCH-214 — Guarded entry-point application import

- User outcome: ensure an early failure while loading the application dispatcher
  reaches the existing startup diagnostics instead of terminating before the
  entry boundary is active.
- Scope: keep the public `main(argv)` wrapper in `src/quillforge/__main__.py`,
  resolve `app.main` inside it, and preserve package-relative/direct-source
  import paths and ordinary argument dispatch.
- Non-goals: no Qt/application rewrite, new logger, settings or document
  logging, native EXE launch, unit-test asset, or release-gate closure.
- Evidence required: public main contract and import-failure guard probes,
  source startup diagnostic, compile/Ruff/format/presentation audit, parent and
  independent review records, refreshed package identity, synchronized records,
  project/handoff checks, and expected release NO-GO.

## D232 / ARCH-214 — completion record

- Outcome: completed with limits. `app.main` is now loaded inside the guarded
  public entry call, so import failures reach the existing traceback, native
  fallback, and stderr diagnostics without changing normal dispatch.
- Review: parent `PASS`; simplification assessment `PASS`. `Rawls the 6th /
  Luna max` architecture and `Ptolemy the 6th / Luna max` independent windows
  returned `NO_CONCLUSION` after bounded waiting and closure.
- Evidence: `D232-LAZY-MAIN-IMPORT-CONTRACT-PROBE=PASS`,
  `D232-ENTRY-IMPORT-FAILURE-GUARD-PROBE=PASS`,
  `D232-STARTUP-DIAGNOSTIC-SOURCE-PROBE=PASS`, `D232-COMPILEALL=PASS`,
  `D232-RUFF=PASS`, `D232-PRESENTATION-AUDIT=PASS`,
  `D232-PACKAGE-ARCHIVE-PROBE=PASS`, and
  `D232-PACKAGE-IDENTITY-PROBE=PASS` for the 38,571,175-byte candidate
  `4161A707CA67AA739F2596DD1378111A7BAC7F979A95B22E3DF45114031EAAF8`.
- Records: ADR-0278, S282, parent/independent review records, and the D232
  handoff are synchronized. Native/runtime and external release gates remain
  open.

## D231 / ARCH-213 — MessageBox zero-return fallback

- User outcome: preserve a visible fallback attempt when Win32 reports that the
  startup MessageBox failed to display.
- Scope: capture `MessageBoxW`'s return value, retain direct return for nonzero
  success, and continue the existing stderr route for zero.
- Non-goals: no Qt/application rewrite, new logger, message-text change,
  native EXE launch, unit-test asset, or release-gate closure.
- Evidence required: zero-return and nonzero-preservation probes,
  compile/Ruff/format/presentation audit, parent and independent review
  records, refreshed package identity, synchronized records, project/handoff
  checks, and expected release NO-GO.

## D231 / ARCH-213 — completion record

- Outcome: completed with limits. The startup fallback now distinguishes a
  successful nonzero `MessageBoxW` return from a zero failure result and
  continues stderr for the latter.
- Review: parent `PASS`; simplification assessment `PASS`. `Mill the 6th /
  Luna max` architecture and `Gibbs the 6th / Luna max` independent windows
  returned `NO_CONCLUSION` after bounded waiting and closure.
- Evidence: `D231-MESSAGEBOX-ZERO-RETURN-FALLBACK-PROBE=PASS`,
  `D231-MESSAGEBOX-NONZERO-RETURN-PRESERVE-PROBE=PASS`,
  `D231-COMPILEALL=PASS`, `D231-RUFF=PASS`, `D231-FORMAT=PASS`,
  `D231-PRESENTATION-AUDIT=PASS`, dual-shell package builds,
  `D231-PACKAGE-ARCHIVE-PROBE=PASS`,
  `D231-PACKAGE-IDENTITY-PROBE=PASS`, and project/handoff checks.
- Records: ADR-0277, S281, parent/independent review records, and the D231
  handoff are synchronized. Native/runtime and external release gates remain
  open.

## D228 / ARCH-210 — Startup failure context record

- User outcome: make early packaged startup failures actionable by recording
  fixed executable, working-directory, frozen/runtime, Python-version, and
  bundle-root context beside the existing exception and traceback.
- Scope: add fail-open context collection at the existing entry boundary in
  `src/quillforge/__main__.py`; preserve normal GUI entry, MessageBox behavior,
  exit code, and the no-Qt early-failure path.
- Non-goals: no Qt/application rewrite, settings or document logging,
  environment dump, native EXE launch, unit-test asset, or release-gate
  closure.
- Review: parent `PASS`; simplification assessment `PASS`. `Avicenna the 6th /
  Luna max` architecture and `Epicurus the 6th / Luna max` independent
  bounded windows returned `NO_CONCLUSION` and were closed.
- Evidence: `D228-STARTUP-CONTEXT-PROBE=PASS fields=5`,
  `D228-FAIL-OPEN-PATH-PROBE=PASS`, `D228-COMPILEALL=PASS`,
  `D228-RUFF=PASS`, `D228-FORMAT=PASS`, `D228-PRESENTATION-AUDIT=PASS`,
  dual-shell package builds, `D228-PACKAGE-ARCHIVE-PROBE=PASS`, and
  `D228-PACKAGE-IDENTITY-PROBE=PASS` for the 38,570,902-byte candidate
  `E7E361635C0E8B4003EA68AD743AF906B2094F4B37F8C036DB32EA62B6241CC7`.
- Records: ADR-0274, S278, parent/independent review records, and the D228
  handoff are synchronized. Native/runtime and external release gates remain
  open.

## D220 / UI-118 — completion record

- Outcome: completed with limits. The main shell now uses a calmer flat surface
  ladder across the main window, editor stage, command rail, and document-tab
  rail; existing behavior and state selectors remain unchanged.
- Review: parent `PASS`; simplification assessment `PASS`. `Halley the 6th /
  Luna max` architecture and `Hilbert the 6th / Luna max` independent windows
  returned `NO_CONCLUSION` after two bounded waits and closure.
- Evidence: `D220-MAIN-SHELL-QSS-PROBE=PASS selectors=4 combos=12 gradients=0`,
  `D220-COMPILEALL=PASS`, `D220-RUFF=PASS`, `D220-FORMAT=PASS`,
  `D220-PRESENTATION-AUDIT=PASS`, `D220-PACKAGE-BUILD-PS51=PASS`,
  `D220-PACKAGE-BUILD-PS7=PASS`, and `D220-PACKAGE-IDENTITY-PROBE=PASS`.
- Records: ADR-0267, S271, parent/independent review records, and the D220
  handoff are synchronized. Native rendering, runtime, and external release
  gates remain open.

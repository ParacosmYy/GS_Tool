# QuillForge long-term product objective

## North star

QuillForge will become a dependable, keyboard-first Windows editor for long editing sessions and enterprise extension. It must remain useful offline, preserve user data predictably, keep the editor engine replaceable, and expose only explicit, versioned capabilities to extensions.

The project is delivered as a sequence of independently usable increments. A roadmap item is not complete because its code exists; it is complete only when its user outcome, acceptance evidence, review record, documentation, and applicable package verification are present.

## Product pillars

1. **Editing correctness** — fast keyboard workflows, predictable selection/undo behavior, multiple documents, and clear dirty state.
2. **Data safety** — encoding and line-ending preservation, external-change protection, atomic saves, recovery snapshots, and explicit failure paths.
3. **Workspace scale** — project navigation, search, incremental analysis, large-file measurements, and bounded background work.
4. **Extension platform** — versioned contracts, command/event capabilities, settings and panels without widget leakage, permission and trust policy.
5. **Enterprise delivery** — deterministic builds, diagnostics, notices, signing and update decisions, supportable configuration, and evidence-backed release claims.

## Delivery sequence

| ID | Increment | User outcome | Status | Main dependencies |
|---|---|---|---|---|
| D0 | Extensible editor shell | Open/save documents, tabs, safe replacement, plugin lifecycle, and packaged startup foundation | accepted-with-limits | — |
| D1 | Core editing commands | Undo/redo, cut/copy/paste, and select-all are available from keyboard and Edit menu | completed | D0 |
| D2 | Current-document find and bounded replace | Search the active document, replace one/all, and navigate literal matches without filesystem work | completed | D1 |
| D3 | Recovery and autosave | Recover unsaved work after an enabled failure path and explain recovery decisions | completed | D0, D1 |
| D4 | Workspace navigation | Open a project folder, browse files, and keep filesystem work bounded and explicit | completed | D2, D3 |
| D5 | Command palette and settings | Discover commands, configure editor behavior, and persist versioned settings | completed | D1, D4 |
| D6 | Governed extension ecosystem | Enable/disable trusted plugins with permissions, settings, diagnostics, compatibility policy, and bounded metadata-only catalog | accepted-with-limits | D5 |
| D7 | Scale and performance | Measure and improve large files, long lines, search latency, memory, and recovery behavior | in-progress | D2, D4 |
| D8 | Enterprise release | Notices, version metadata, signing/installer decisions, clean-machine verification, and support handoff | in-progress | D3, D6, D7 |
| D9 | Modern UI iteration | Rapid, bounded modernization of the editor shell with a centralized visual system, modern command rail, and authored anime-forge brand asset | accepted-with-limits | D5, D8 |
| D10 | Team workflow governance | Fixed six-role Architect-led delivery, configurable constraints, and an indexed handoff record for every material slice | accepted-with-limits | D8 |

The machine-readable source of delivery status is [`docs/agent-team/delivery-register.json`](agent-team/delivery-register.json). This document explains the product intent and sequencing; the register carries the current evidence state.

## Current bounded delivery — D315 / UI-133 / ARCH-285

D315 / UI-133 / ARCH-285 keeps checked checkboxes and Settings behavior toggles
readable in every supported theme/accent combination. Native check marks remain
owned by Qt; checked and checked-hover indicators now use high-contrast
pressed/hover surfaces, while their accent borders use readable-edge fallbacks
and disabled indicators use the muted surface pair. Source, contrast matrix,
offscreen render probe, source diagnostic, package, archive, review, and handoff
evidence pass; native Windows rendering, accessibility, DPI, clean-machine
behavior, and remaining release gates are explicit limits.

## Completed bounded delivery — D314 / UI-132 / ARCH-284

D314 / UI-132 / ARCH-284 gives the language, theme, accent, UI-font,
editor-font, and font-style combo boxes an explicit theme-aware popup-open
affordance. The native drop-down button is positioned against the border and
uses the pressed surface/accent boundary when open, while the native arrow,
popup, item data, keyboard behavior, settings roles, persistence, and locale
behavior remain unchanged. Source, contrast matrix, QSS probe, source
diagnostic, package, archive, review, and handoff evidence pass; native
rendering, popup interaction, DPI, clean-machine behavior, and remaining
release gates are explicit limits.

## Completed bounded delivery — D313 / UI-131 / ARCH-283

D313 / UI-131 / ARCH-283 gives the Settings interface-font and editor-font
size controls compact, theme-aware up/down stepper states through the existing
centralized QSS renderer. Native arrows, value ranges, keyboard stepping,
persistence, and locale behavior remain unchanged. Source, contrast matrix,
QSS probe, source diagnostic, package, archive, review, and handoff evidence
pass; native rendering, click timing, DPI, clean-machine behavior, and
remaining release gates are explicit limits.

## Completed bounded delivery — D312 / UI-130 / ARCH-282

D312 / UI-130 / ARCH-282 gives vertical and horizontal scrollbars a compact,
theme-aware surface, rounded handle, and readable hover/pressed hierarchy
through the existing centralized QSS renderer. Native range, page, wheel,
keyboard, and drag behavior remain unchanged. Source, contrast matrix, QSS
probe, source diagnostic, package, archive, review, and handoff evidence pass;
native rendering, dragging, DPI, clean-machine behavior, and remaining release
gates are explicit limits.

## Completed bounded delivery — D311 / UI-129 / ARCH-281

D311 / UI-129 / ARCH-281 makes main-window and dock-panel split boundaries
readable and theme-aware through the existing centralized QSS renderer. Normal
separators use a contrast-checked border-derived edge across the shell
surfaces, while hover uses the existing alternate accent. Source, contrast
matrix, package, archive, review, and handoff evidence pass; native rendering,
interactive dragging, DPI, clean-machine behavior, and remaining release gates
are explicit limits.

## Completed bounded delivery — D310 / ARCH-280

D310 / ARCH-280 provides an explicit `QuillForge.exe --safe-mode` recovery
launch. It selects product-default appearance settings, skips built-in plugin
activation and persisted session/recovery restore, creates an initial document,
and preserves explicit file paths. Source, composition, diagnostic, package,
archive, and handoff evidence pass; native safe-mode startup, clean-machine
behavior, and remaining release gates are explicit limits.

## Completed bounded delivery — D309 / UI-128 / ARCH-279

D309 / UI-128 / ARCH-279 gives all tooltips one theme-aware surface through the
existing centralized QSS renderer. It reuses surface, text, and border tokens,
derives a readable accent edge with a fallback, and preserves existing
typography and behavior. Source, contrast matrix, diagnostics, package,
archive, and handoff evidence pass; native Tooltip rendering, accessibility,
clean-machine behavior, and remaining release gates are explicit limits.

## Completed bounded delivery — D308 / UI-127 / ARCH-278

D308 / UI-127 / ARCH-278 makes the Workspace folder and direct-file opening
actions explicit through semantic roles, localized hints, accessible
descriptions, and distinct QSS edges. Existing file/folder signals, tree
activation, native picker, containment, and asynchronous document opening are
unchanged. Source, contrast matrix, diagnostics, package, archive, and handoff
evidence pass; native rendering, screen-reader behavior, clean-machine
behavior, and remaining release gates are explicit limits.

## Completed bounded delivery — D307 / UI-126 / ARCH-277

D307 / UI-126 / ARCH-277 adds localized clean/changed draft feedback to
Settings. The dialog compares the current `SettingsSnapshot` with the values
captured at open, refreshes the status after preview and behavior edits, and
uses the existing locale/accessibility/QSS boundaries. Source, contrast matrix,
diagnostics, package, archive, and handoff evidence pass; native rendering,
screen-reader behavior, clean-machine behavior, and remaining release gates
are explicit limits.

## Completed bounded delivery — D306 / UI-125 / ARCH-276

D306 / UI-125 / ARCH-276 adds a localized RestoreDefaults action to Settings.
It resets the 12 current controls from `DEFAULT_SETTINGS` with signals blocked,
refreshes the existing preview/accessibility/locale projection, and leaves
Save as the only persistence boundary. Source, contrast matrix, diagnostics,
package, archive, and handoff evidence pass; native dialog interaction,
screen-reader behavior, clean-machine behavior, and remaining release gates are
explicit limits.

## Completed bounded delivery — D305 / UI-124 / ARCH-275

D305 / UI-124 / ARCH-275 gives nine Settings value controls and three behavior
controls localized accessible names sourced from existing labels/text. The
names refresh after locale-visible text and before preview projection, with an
AST-backed exact 9+3 mapping audit. Source, diagnostics, package, archive, and
handoff evidence pass; native screen-reader behavior, clean-machine behavior,
and remaining release gates are explicit limits.

## Completed bounded delivery — D304 / UI-123 / ARCH-274

D304 / UI-123 / ARCH-274 gives the Settings language selector one semantic
`localeChoice` role with centralized normal/hover/focus/open/disabled QSS and a
cross-surface readable edge. Existing English/Chinese item data, current data,
signals, immediate locale refresh, snapshot, persistence, and keyboard
behavior remain unchanged. Source, matrix, diagnostics, package, archive, and
handoff evidence pass; native visual review, clean-machine behavior, and
remaining release gates are explicit limits.

## Completed bounded delivery — D303 / UI-122 / ARCH-273

D303 / UI-122 / ARCH-273 gives the Settings wrapping, line-number, and motion
checkboxes one semantic `behaviorToggle` role with `editor` and `interface`
tones. Centralized QSS covers normal/hover/focus/checked/disabled states, and a
pure token helper keeps tone edges above the 3:1 non-text floor across all
state surfaces. Existing values, labels, signals, snapshot, persistence, and
keyboard behavior remain unchanged. Source, matrix, diagnostics, package,
archive, and handoff evidence pass; native visual review, clean-machine
behavior, and remaining release gates are explicit limits.

## Completed bounded delivery — D302 / UI-121 / ARCH-272

D302 / UI-121 / ARCH-272 gives the six Settings interface/editor typography
controls one semantic `typographyChoice` role with `interface` and `editor`
tones. The centralized QSS now has shared normal/hover/focus/open/disabled
states, independent tone cues, and a pure contrast-safe edge fallback. Existing
values, previews, signals, locale, persistence, keyboard behavior, and
snapshot semantics remain unchanged. Source, matrix, diagnostics, package,
archive, and handoff evidence pass; native visual review, clean-machine
behavior, and remaining release gates are explicit limits.

## Completed bounded delivery — D301 / UI-120 / ARCH-271

D301 / UI-120 / ARCH-271 retains D300's theme/accent visual hierarchy while
moving its active-state styling behind the semantic
`settingsRole="identityChoice"` presentation role. The existing controls,
values, item data, swatches, signals, locale, persistence, keyboard behavior,
and disabled compatibility remain unchanged. The canonical audit, 3-theme ×
4-accent matrix, source diagnostics, package/archive checks, and handoff pass;
native visual review, clean-machine behavior, and remaining release gates are
explicit limits.

## Completed bounded delivery — D300 / UI-119 / ARCH-270

D300 / UI-119 / ARCH-270 gives the Settings theme and accent selectors a
stronger visual hierarchy through centralized semantic QSS: distinct surface,
accent edge, stronger weight, and hover/focus/open states. Existing values,
icons, locale, keyboard behavior, persistence, and application ownership are
unchanged. Source, contrast-matrix, diagnostic, package, and archive evidence
passed; native visual review, clean-machine behavior, and remaining release
gates remain explicit limits.

## Completed bounded delivery — D299 / ARCH-269

D299 / ARCH-269 closes the split-brain plugin-root selection gap left by D296:
the frozen selector prefers the first Qt6/legacy root containing
`platforms/qwindows.dll`, and configuration plus startup diagnostics reuse the
same helper. The static contract was hardened after independent review to use
AST function scopes for ordering and reuse. Source, selector simulation,
package, and archive evidence passed; architecture and independent review
returned no conclusion for the bounded final slice, while native startup,
clean-machine behavior, physical split-layout loading, and remaining release
gates remain explicit limits.

D298 / ARCH-268 aligns the frozen Qt DLL inventory with the PyInstaller PyQt6
runtime-hook layout policy: `Qt6/bin` first, then legacy `Qt/bin`. This closes
the false-negative gap where D297's preflight could reject a valid older-layout
bundle even though plugin discovery supported it. Current Qt6 source,
diagnostic, package, and archive evidence passed; architecture and independent
review returned no conclusion, while physical legacy-layout, native startup,
clean-machine behavior, and remaining release gates remain explicit limits.

D297 / ARCH-267 adds a frozen-only fail-fast Qt runtime preflight after the

D297 / ARCH-267 adds a frozen-only fail-fast Qt runtime preflight after the
explicit plugin-host and diagnostic branches and before normal QApplication
construction. It reuses the existing `qwindows.dll` and Qt/QScintilla
dependency checks and projects missing bundle-relative paths through the
existing localized startup-error fallback. Source, diagnostic, and plugin-host
routes remain unchanged. Static/source/package/archive/simulation evidence
passed; architecture and independent review returned no conclusion, while
native startup, clean-machine behavior, and remaining release gates remain
explicit limits.

D296 / ARCH-266 binds frozen Qt plugin discovery to the current PyInstaller

D296 / ARCH-266 binds frozen Qt plugin discovery to the current PyInstaller
bundle before QApplication construction. The entry boundary supports both the
current `PyQt6/Qt6/plugins` layout and the legacy `PyQt6/Qt/plugins` layout and
sets both Qt plugin environment paths only for frozen execution. This reduces
the chance that a machine-level Qt installation makes the portable EXE fail to
show by selecting a foreign platform plugin. Static/source/package/archive
evidence passed; architecture and independent review returned no conclusion,
and native startup, clean-machine behavior, and remaining release gates remain
explicit limits.

D295 / ARCH-265 closes a concrete TaskRunner false-success path left by the
D293 lifecycle review. A worker operation that raises a non-`Exception`
`BaseException` is now normalized into a private `RuntimeError` and projected
through the existing failure callback instead of being reported as success with
`None`. Existing coordinator contracts and release ordering remain unchanged.
Independent review, direct abnormal injection, native startup, clean-machine
behavior, and remaining release gates remain explicit limits.

D294 / ARCH-264 closes a diagnostic-only Qt lifecycle defect found during the
D293 verification loop. The startup diagnostic now owns one QApplication across
the runtime-composition and startup-restore probes and closes it in `finally`
when owned. The repeated `Qt6111ThemeChangeObserverWindow` registration warning
disappeared while the normal desktop entrypoint remained unchanged. Independent
review, native startup, clean-machine behavior, and remaining release gates
remain explicit limits.

D293 / ARCH-263 closes a concrete TaskRunner bookkeeping hole found during the
D292 lifecycle review. Submission retains the task before Qt signal connection
and pool start, but synchronous connection/start failures now roll that
retention back and re-raise the original exception. Normal completion and
rollback share an idempotent release boundary, preserving callback ordering and
preventing stale or duplicate pending-state transitions. Independent review,
native startup, clean-machine behavior, and remaining release gates remain
explicit limits.

D291 closes a concrete startup lifecycle hole: normal `runtime.start()` now
shares the existing `try/finally` with `application.exec()` and
`runtime.stop()`. The change preserves the established startup order and
composition boundary, while a targeted AST contract prevents the cleanup
boundary from drifting. It is accepted only with explicit limits until the
independent review, package identity, native startup, clean-machine, and
remaining release gates are evidenced.

D292 / ARCH-262 extends the same boundary to periodic UI activity. Runtime
shutdown now stops recovery/session-save timers through a presentation-owned
port before plugin deactivation, while deliberately leaving queued worker
callbacks under the existing Qt lifecycle instead of introducing forceful
thread termination. Independent review, refreshed package identity, native
startup, and remaining release gates remain explicit limits.

D286 / ARCH-256 extends the no-window startup diagnostic through the first
empty editor/QScintilla tab construction. The production editor path is reused
and its recovery/session-save timers are stopped during the probe; normal
session/recovery restore order is unchanged. Native startup remains
intentionally unrun.

D287 / ARCH-257 extends the no-window startup diagnostic into the persisted
user-state boundary. The report now reuses the production session loader and
recovery snapshot store to expose manifest validity, missing remembered files,
and malformed recovery counts without returning document contents or path
lists. Normal asynchronous restore behavior and native startup remain
intentionally unrun.

D288 / ARCH-258 extends the diagnostic through the production asynchronous
startup restore chain when no recovery prompt is required. A bounded Qt event
processing window now proves session/recovery/file-open completion, worker
drain, and timer cleanup without showing the main window or writing user
state. Recovery candidates are reported as an explicit safe skip; native
startup remains intentionally unrun.

D289 / ARCH-259 adds a focused file-open diagnostic for the reported
“只能打开文件夹，不能打开文件” failure. A requested regular file now travels
through the same startup-path queue and asynchronous document-open admission
used by normal launch; the source diagnostic proved the file reached a tab
without writing session/settings. Native shell activation and frozen EXE launch
remain intentionally unrun.

D290 / ARCH-260 removes the measured busy-spin from the no-window startup/file
open preflight. The wait now uses an unparented, short-lived 2ms Qt wake timer
with `WaitForMoreEvents`; repeated source diagnostics dropped from 8,679 event
batches to 7–11 while still opening the requested file and preserving the
five-second soft timeout and no-window boundary. Native startup remains
intentionally unrun.

D285 / ARCH-255 extends the no-window startup diagnostic through the shared
pre-show lifecycle. Plugin capability binding/activation and command refresh
now use the same `DesktopRuntime` stages as normal startup, while session and
recovery restoration, window display, and the event loop remain outside the
probe. Native startup remains intentionally unrun.

D284 / ARCH-254 extends the no-window startup diagnostic through the existing
composition root. It constructs and releases a temporary QApplication and
DesktopRuntime without showing a window or entering the event loop, so
constructor-time failures become reportable. Native startup remains
intentionally unrun.

D283 / ARCH-253 removes stale startup evidence at the entry boundary. Each
attempt clears only the prior QuillForge startup-error log, while the existing
fail-open recorder writes a fresh traceback when the current attempt fails.
Native startup remains intentionally unrun.

D282 / ARCH-252 closes a construction-order risk in the desktop shell. The
`MainWindow` busy default is now established before coordinator callback
wiring, and a Qt-free AST contract guards that invariant while preserving
runtime busy transitions. Native startup remains intentionally unrun.

D281 / ARCH-251 makes the existing startup diagnostic discoverable. README now
documents the `settings_preflight` metadata, exit codes, startup-error log, and
the distinction between source/package preflight and native Windows startup.
This documentation-only slice does not change the D280 candidate artifact.

D280 / ARCH-250 makes the reported startup failure easier to isolate without
adding a second settings policy. The existing no-window startup diagnostic now
reads the user-local settings path once through `JsonSettingsStore`, reports
only presence/decode validity/normalized schema metadata, and reuses the
existing default-and-normalize contract. It never writes settings or exposes
locale, theme, or font values. Native launch remains intentionally unrun.

D279 / ARCH-249 makes the requested font switching more transparent. Settings
now reports whether the selected interface/editor family is installed or will
use the system fallback, in English and Simplified Chinese, while persisting
only the validated family value. Native font metrics, rendering, and startup
remain intentionally unrun.

D278 / ARCH-248 closes a concrete startup-regression gap behind the reported
"EXE cannot open" failure. The historical local startup log identified the
missing `CommandSurface._locale` accessor during menu construction; the current
D225 source fix is retained and a Qt-free contract now guards its provider,
accessor, and MainWindow ordering. Native launch remains intentionally unrun.

D277 / ARCH-247 closes a regression-confidence gap across the requested
language and typography controls. A Qt-free contract now verifies locale,
theme/accent, interface/editor font family, size, style, persistence, live
application, and motion-transition routes without moving settings policy or
changing save order. Native rendering, installed-font metrics, startup, and
release gates remain open.

D276 / ARCH-246 closes a regression-confidence gap in the reported file-open
bug. A Qt-free presentation contract now verifies the file-vs-folder intent
split from the workspace tree through the surface, activation coordinator, and
asynchronous document-open admission, while containment, busy/session gates,
tab reuse, and folder navigation remain unchanged. Native event ordering,
startup, and release gates remain open.

D275 / ARCH-245 closes the remaining QSS semantic foreground gap behind the
reported 砂金/readability regression. The framework-neutral token layer now
owns the derived foregrounds for accent-alt text/fill, selection, warning,
success, working, and error surfaces; runtime QSS/palette projection and the
Qt-free audit consume the same resolver across all 3 themes × 4 accents.
Native rendering, startup, accessibility, and release gates remain open.

D273 / ARCH-244 adds a Qt-free contrast regression gate for all supported
theme/accent combinations and semantic shell/editor foregrounds. It reuses the
existing token resolver and keeps runtime palette/QSS behavior unchanged. The
D273 package is identity-bound; native startup, rendering, accessibility, and
release gates remain open.

D270 / ARCH-243 adds a static notification-localization rule to the existing
presentation contract audit. New literal `notify(...)` strings are checked
against the current zh-CN projection without changing runtime behavior or
dynamic diagnostic handling. The D270 candidate is rebuilt and package-bound;
native startup, native rendering, and release gates remain open.

D264 / ARCH-242 closes the remaining English text in the secondary pre-Qt
startup fail-open path. Chinese fallback labels are preserved through nested
locale failure handling while the final English fallback, diagnostics, native
startup, and release gates remain unchanged/open.

D263 / ARCH-241 closes the remaining English workspace-search path placeholder
when a diagnostic path lies outside the selected root. Chinese now shows
`<位于所选工作区之外>` while English, containment, search state, native
startup, and release gates remain unchanged/open.

D262 / ARCH-240 closes the Chinese-language gap in the pre-Qt startup failure
fallback. System Chinese locale hints now project Chinese startup labels while
unknown/other locales retain English; exception details, diagnostics, exit
code, native startup, and release gates remain unchanged/open.

D261 / ARCH-239 closes the no-current-document Save As default-name locale gap
at the existing `FileDialogSurface` presentation boundary. English now uses
`Untitled.txt` and Simplified Chinese uses `未命名文档.txt`, while existing
current paths, the `.txt` extension, dialog/save contracts, native startup, and
release gates remain unchanged/open.

D260 / ARCH-238 closes the Settings font-size unit locale gap. Both font-size
controls now refresh their display suffix between `pt` and `磅` with the
selected language while numeric settings, native startup, and release gates
remain unchanged/open.

D259 / ARCH-237 closes the common message-dialog button locale gap at the
existing presentation boundary. Save, Discard, Cancel, and OK are explicitly
projected from the selected locale after Qt standard-button creation while
button semantics, native startup, and release gates remain unchanged/open.

D258 / ARCH-236 closes the Replace All error-localization gap at the existing
presentation boundary. The match-limit, text-capture, and rollback-failure
forms now render in Chinese while en-US, unknown details, editor transaction
behavior, native startup, and release gates remain unchanged/open.

D257 / ARCH-235 closes the built-in plugin-status name locale gap at the
existing presentation boundary. The stable Document Statistics plugin ID now
projects through the command catalog, while external plugin names remain as
provided and the existing locale-refresh path is reused. Native startup,
native dialogs, and release gates remain open.

D256 / ARCH-234 closes the built-in Document Statistics locale gap at the
existing presentation i18n boundary. The command title, empty-document notice,
and dynamic line/character counters now render in Chinese while en-US output,
unknown external plugin fallback, plugin API ownership, native startup,
native dialogs, and release gates remain unchanged/open.

D255 / ARCH-233 closes the remaining user-visible application-error locale gap
at the existing presentation localizer boundary. Common document, workspace,
search, plugin, recovery, autosave, and coordinator-wrapper details now render
in Chinese while dynamic paths/names/IDs and unknown text remain visible;
application policy, English behavior, native startup, native dialogs, and
release gates remain open.

D254 / ARCH-232 strengthens the existing no-window frozen startup diagnostic
with explicit Qt Core/Gui/Widgets, QScintilla, and platform dependency paths.
It reports missing relative files without loading libraries or changing normal
startup behavior. Native startup, native dialogs, and release gates remain
open.

D253 / ARCH-231 closes the workspace-entry diagnostic locale gap at the
existing disabled-row presentation boundary. Known provider reasons now appear
in the selected locale and refresh when language changes, while unknown text,
inaccessible-row state, file/folder activation, and workspace provider policy
remain unchanged. Native startup, native dialogs, and release gates remain
open.

D252 / ARCH-230 closes the plugin diagnostic locale gap at the existing
presentation boundary. Known runtime-status and catalog-entry reasons/prefixes
are localized in visible tooltips, unknown provider text remains visible, and
plugin trust, approval, enablement, and runtime policy remain unchanged.
Native startup, native dialogs, and release gates remain open.

D251 / ARCH-229 closes the workspace-search diagnostic locale gap at the
existing presentation boundary. Known provider reasons and failure prefixes
are localized while paths and diagnostic detail remain visible; existing rows
are reprojected on locale changes without changing search policy, ordering,
counts, or expansion state. Native startup, native dialogs, and release gates
remain open.

D250 / ARCH-228 closes the workspace-search status locale gap at the existing
presentation seam. Catalog keys, raw failure messages, and immutable result
summaries are retained as sources so the current status is rebuilt after a
language change, without changing search execution, results, cancellation, or
diagnostics policy. Native startup, native dialogs, and release gates remain
open.

D249 / ARCH-227 closes a locale-refresh state gap at the existing workspace
presentation seam. A retained typed or string error source is reprojected when
the user changes language, while successful directory results and new loads
clear obsolete errors. Navigation, service, session, notification, and
last-good-page policy remain unchanged. Native startup, native dialogs, and
release gates remain open.

D248 / ARCH-226 closes the workspace navigation typed-error gap at the
existing presentation seam. Workspace failures now reach the panel as
exceptions and reuse the shared filesystem/codec localizer, so Chinese users
receive actionable permission/path wording while invalid-result strings,
English behavior, navigation policy, and session restoration remain unchanged.
Native startup, native dialogs, and release gates remain open.

D247 / ARCH-225 closes the remaining typed-error forwarding gap at two
existing `MainWindow` presentation handlers. Settings persistence and Replace
All now pass the original exception object to `MessageSurface`, so the
already-established Chinese filesystem/codec localization can preserve type
and diagnostic metadata while English output remains compatible. Native
startup, native dialogs, and release gates remain open.

D246 / ARCH-224 closes a startup-path diagnostic gap at the existing
`MainWindow._drain_startup_paths()` boundary. One `Path.stat()` now separates
directories, regular files, and unsupported special files while preserving
concrete stat failures for the existing localized warning path. Native
startup, shell associations, and release gates remain open.

D245 / UI-27 closes a file-failure comprehension gap at the existing
presentation error boundary. Open/save coordinators now preserve typed
exceptions through `MessageSurface`; `zh-CN` gives common filesystem and
codec failures stable Chinese wording while retaining path, encoding, and
position details, and `en-US` remains unchanged. File I/O, native dialogs,
startup, and release gates remain open.

D244 / UI-26 closes a localization gap in the explicit desktop-file launch
failure path. The existing presentation message compatibility mapper now
translates both `Cannot open path:` and `Unable to open path:` prefixes for
`zh-CN`, while preserving English output and all path/exception details. It
does not change startup argument parsing, file admission, or document loading;
native startup and release gates remain open.

D243 / UI-25 closes a confirmed text-contrast gap at the centralized
`theme.py` projection boundary. Application links, the workspace eyebrow, and
generic checkbox hover text now use one conservative `accent_alt` foreground
resolver against the deepest shared surface, falling back to `text_primary`
when needed; filled accent buttons and their `on_accent*` tokens remain
unchanged. All supported theme/accent combinations pass the source QSS
contrast matrix. Runtime Qt rendering, native metrics, and release gates
remain open.

D242 / ARCH-223 closes the default file-dialog visibility gap at the existing
`FileDialogSurface` presentation boundary. The localized shared filter now
starts with `All files (*)` and retains a text/source filter as a secondary
choice, so extensionless and uncommon source/configuration files are visible
without adding a second open path. Qt's first-party `QFileDialog` documentation
is the applicable framework reference; native dialog and startup evidence
remain open under the no-launch policy.

## Delivery contract for every increment

Each increment follows the project team loop:

1. Product writes the user outcome, in/out-of-scope behavior, and Given/When/Then scenarios.
2. Architect records dependency direction, ownership, compatibility, and risk decisions.
3. The assigned developers implement one bounded change slice; only one shared-checkout writer is active.
4. QA records deterministic checks, integration smoke evidence, package evidence where applicable, and unrun checks.
5. The Architect integrates, runs the review hooks, updates the register and handoff index, and writes `docs/handoffs/<handoff-id>/handoff.md`.
6. Under the current user no-launch instruction, startup and visual checks are explicitly unrun and user-owned; static checks, compilation, and packaging remain allowed.

The next increment must not silently absorb later roadmap work. If a later capability is needed, it becomes an explicit dependency or a new delivery item.

## Current D7 slices

- **D7.1 — cooperative active-document operations:** the current implementation slices Replace All on the Qt UI thread, preflights the match limit, exposes cancellation, and rolls back one undo transaction on cancellation or failure. It is still subject to the post-change review and packaged evidence gates.
- **D7.2 — recovery capture scale:** delivered with explicit limits. The default application composition reads position-safe chunks cooperatively into a bounded 64-chunk/1 MiB handoff and atomically commits them in the worker-side JSON store. Source/offscreen and packaged-EXE evidence cover queue capacity, worker completion, stale cancellation, round-trip, and temporary-file cleanup; larger workloads, clean-machine behavior, filesystem pressure, cross-machine behavior, hard-power durability, and a native-memory ceiling remain unclaimed. ADR-0012 records the state machine and support boundary.
- **D7.3 — measured workload expansion:** in progress with the current support boundary accepted with limits. The first matrix, cancellation/rollback, match-limit, stale-capture, injected-writer-failure, bounded recovery handoff, 16 MiB source/offscreen probe, three-run source repeat, three-run packaged-EXE capture diagnostic, and current-machine interactive startup record are recorded; authorized disk-pressure evidence, clean-machine/cross-machine repeatability, and broader support remain open.
- **D7.4 — bounded asynchronous workspace search:** in progress, with D74-AC01..03 and S22 accepted with limits. Find in Files now has a separate application contract, recursive deterministic filesystem adapter, scan-time enforcement of file/total byte budgets, configurable file/result/depth/line bounds, binary/symlink/excluded-directory handling, cooperative cancellation, stale-result protection, containment-checked line navigation, and tab-expanded previews that honor the display bound; current packaged report freshness, packaged interactive search measurement, cross-machine evidence, indexing, regex, and cross-file replacement remain open.
- **D7.4.1 — visible bounded search diagnostics:** accepted with limits. Find in Files now projects bounded file-level issues through a collapsed read-only panel using workspace-relative paths, explicitly reports when the diagnostic ledger is truncated, and binds packaged diagnostic payloads to the producing artifact; packaged report regeneration, interactive evidence, and directory-order determinism remain open.
- **D7.4.2 — TaskRunner lifecycle guard:** accepted with limits. All retained background work is observable through a single pending-task contract, the main window refuses close until worker execution plus queued completion delivery drain without blocking or force termination, and cancelling a startup workspace restore now releases the session barrier while the stale completion remains contained. The post-fix independent Luna source audit returned PASS; scenario S24 now projects this evidence, while provider-specific cooperative cancellation and runtime evidence remain open.
- **D7.5.1 — local session continuity:** accepted-with-limits. A bounded session manifest remembers the workspace root, clean path-backed document-tab order, active tab, and adapter-level caret; startup recovery is ordered ahead of session reopening, invalid manifests are preserved until an explicit later session change, and dirty/untitled content remains owned by D3 Recovery. ADR-0023 and ADR-0026 are aligned, and the post-fix independent Luna review returned PASS; hard-power, cross-machine, multi-instance, and dirty/untitled session payloads remain outside the slice.

## Current D8 slices

- **D8.1 — release candidate identity and handoff contract:** implementation delivered with limits. Windows version resources, a typed atomic release manifest, a mechanically lockfile-covered NOTICE sidecar, and explicit unsigned/portable/manual-update/support states are now generated; legal clearance, signing, installer, clean-machine, and final support handoff remain open.
- **D8.2 — signing, installer, and update decisions:** decision contract delivered with limits. The current candidate is explicitly portable/manual/unsigned with no file associations or updater; approved signing, installer implementation, and update channel remain release gates.
- **D8.3 — clean-machine and support handoff:** delivered with limits. The handoff verifier now checks artifact identity, current-machine startup, an assigned Project Manager support owner, and the local issue route; clean Windows x64 evidence and final go/no-go remain open.
- **D8.4 — acceptance evidence traceability preflight:** accepted with limits. `scripts/check.ps1` validates configurable acceptance statuses, required evidence coverage, explicit unrun limits, and repository-backed evidence paths; it does not turn descriptive evidence into runtime proof or close external release gates.
- **D8.5 — artifact-bound handoff freshness:** accepted with limits. `scripts/verify_release_handoff.ps1` now writes a current artifact-bound no-go dossier with exact mechanical failures before returning non-zero for stale or inconsistent evidence; it does not refresh prohibited runtime measurements or close external release gates.
- **D8.6 — release manifest completeness and local provenance:** accepted with limits. The typed manifest and handoff now record explicit no-file-association posture and a deterministic local source tree identifier; this does not establish signed provenance, legal clearance, installer/update behavior, or runtime release evidence.

## Current D9 slices

- **UI-01 — ink/violet shell foundation:** delivered as a source-and-package slice. Centralize the theme tokens, dark editor/workspace surfaces, native workspace icons, FindBar mode visibility, and runtime application icon registration while preserving existing presentation/application seams.
- **UI-02 — command rail:** accepted-with-limits. The compact top action rail for New/Open/Save/Find/Replace/Command palette/Workspace is source- and package-verified, keeps menu commands and application ownership unchanged, and is recorded through configurable acceptance evidence. Runtime visual review is intentionally deferred while the user prohibits starting the software.
- **UI-03 — dialog surfaces:** accepted-with-limits. The command palette, settings, plugin, and workspace-search dialogs now share the same spacing, hierarchy labels, focus colors, and list surfaces without changing their modal or asynchronous behavior; runtime visual acceptance remains open.
- **UI-04 — explicit shell status rail:** accepted-with-limits. The presentation-only READY/WORKING/ATTENTION/ERROR rail is driven by explicit lifecycle calls without parsing notification strings or changing task/close semantics; the follow-up coverage slice is tracked separately below and runtime visual review remains open.
- **UI-05 — modern editor canvas:** accepted-with-limits. The QScintilla adapter applies centralized canvas, caret, margin, current-line, and Python syntax tokens without leaking editor-engine details into application services; font, lexer-version, DPI, accessibility, non-Python themes, and runtime visual review remain open.
- **UI-06 — TaskRunner status coverage:** accepted-with-limits. The shared pending-task signal now drives WORKING while any retained worker or queued completion remains, then restores dirty-document ATTENTION or clean-idle READY; runtime visual, accessibility, DPI, and cross-machine evidence remain open.
- **UI-07 — static accessibility baseline:** accepted with limits. Recoverable modal errors now return the shell to its explicit lifecycle phase, common controls/lists retain centralized keyboard focus styling, the muted token is strengthened, and the status rail exposes accessible names; runtime screen-reader, DPI, contrast, and visual evidence remain open.
- **UI-08 — localization, appearance preferences, and workspace file activation:** accepted-with-limits. The shell now persists English/Simplified Chinese selection, interface/editor font family and size, wrapping, line numbers, dark/light theme surfaces, four accent colors, and motion preference; menu/dialog/status text reprojects through the bounded catalog; workspace files activate through the existing async document boundary while folders remain navigable. Runtime visual acceptance, native dialog behavior, installed-font coverage, DPI, and double-click timing remain open.
- **UI-09 — modern Sakura Pop / anime-cute shell:** accepted-with-limits. The visual system now has a candy-toned Sakura Pop theme, gradient surfaces, rounded cards and pills, warmer brand copy, and a fresh-install default that still preserves explicit legacy theme choices; editor readability and application/service seams remain unchanged. Runtime visual acceptance, installed-font coverage, DPI, and native style metrics remain open.
- **UI-10 — visual hierarchy and highlight states:** accepted-with-limits. Selected, focus, hover, pressed, checked, disabled, readonly, menu, popup, tab, workspace, input, primary-action, warning, and error states now have explicit centralized QSS distinctions across Sakura Pop and legacy themes. Runtime visual state review, formal contrast measurement, fonts, DPI, and native style metrics remain open.
- **UI-11 — visual state closure and focus clarity:** accepted-with-limits. Focused item views now receive a visible accent boundary, alternate rows use an explicit secondary surface, selected-disabled entries and the primary workspace action resolve to muted states, and disabled checkbox indicators remain distinguishable through the same centralized QSS. Runtime visual state review, formal contrast, fonts, DPI, screen-reader output, and native style metrics remain open.
- **UI-12 — dialog primary-action hierarchy:** accepted-with-limits. Settings Save and workspace Search now use the centralized primary-action contract while Cancel and Close remain secondary; dialog signals and application ownership are unchanged. Runtime visual, native-style, font, DPI, contrast, and screen-reader evidence remain open.
- **UI-13 — FindBar action hierarchy and accent contrast:** accepted-with-limits. Find mode and replace mode now expose one current primary action, Replace All has a warning role, and dynamic foreground selection covers bright Sakura/ink/paper accent endpoints, including the reported amber regression. Runtime Qt visual acceptance, formal cross-machine rendering, and release gates remain open.
- **UI-14 — command-palette hierarchy and mature surface rhythm:** accepted-with-limits. Ordinary controls now use calmer flat surfaces and restrained radii, the primary action remains the visual anchor, and the command palette has dedicated query/list/selection/hint selectors. Runtime Qt visual acceptance, native-style/DPI/font evidence, and release gates remain open.
- **UI-15 — visual endpoint contrast and shell rhythm:** accepted-with-limits. ThemeColors now derives a dedicated readable foreground for the amber/砂金 filled endpoint, the warning action uses that token, and command-rail/document-tab spacing and radius hierarchy are refined through centralized QSS. Runtime visual acceptance, native-style/DPI/font evidence, and release gates remain open.
- **UI-16 — authored vector iconography:** accepted-with-limits. The command rail and workspace navigation now use one theme-tinted, presentation-only `IconKey` provider instead of platform-dependent `QStyle.StandardPixmap` icons; explicit visual refresh keeps theme and locale concerns separate. Runtime Qt painting, high-DPI/native metrics, accessibility, and release evidence remain open.
- **UI-17 — semantic transient notification hierarchy:** accepted-with-limits. The status surface now projects localized temporary messages through one centralized, theme-aware `statusMessage` control with explicit info/success/warning/error levels; success and error foregrounds are checked against all supported message backgrounds. Runtime status-bar rendering, timeout behavior, accessibility, and release evidence remain open.
- **UI-18 — inline workspace feedback hierarchy:** accepted-with-limits. WorkspacePanel and WorkspaceSearchDialog now share a presentation-only feedback state contract for info/working/success/warning/error, with centralized borders, surfaces, and readable foregrounds across all supported theme/accent combinations. FindBar mapping remains D33; runtime QSS specificity, fonts, DPI, accessibility, and release evidence remain open.
- **UI-19 — FindBar semantic feedback projection:** accepted-with-limits. Find/Replace and cooperative Replace All now project success, warning, working, and error outcomes through the existing FindSurface seam while preserving the one-argument status API, editor policy, cancellation, rollback, and keyboard routing. Runtime QSS specificity, interactive behavior, fonts, DPI, accessibility, and release evidence remain open.
- **UI-20 — Session/Recovery notification severity closure:** accepted-with-limits. Nineteen existing MainWindow Session/Recovery notifications now carry explicit info/success/warning/error levels through the D31 seam without changing persistence, recovery, TaskRunner, stale-guard, or close policy. Runtime rendering, remaining notification call sites, accessibility, and release evidence remain open.
- **UI-21 — Plugin and extension notification severity closure:** accepted-with-limits. Catalog scans, governance mutations, runtime enablement, and isolated host diagnostics now project explicit info/success/warning/error outcomes through the existing status-message seam; in-progress work also keeps the permanent WORKING phase. Invalid session persistence is error and deferred recovery is warning. Plugin trust, containment, execution policy, async guards, and command refresh remain unchanged. Runtime interaction, remaining document/workspace call sites, and release evidence remain open.
- **UI-22 — Workspace and operation notification severity closure:** accepted-with-limits. Workspace navigation, Find in Files, containment rejection, cancellation, no-match/limit diagnostics, and long-running operation progress now project explicit info/success/warning/error outcomes through the existing status-message seam; the permanent status phase remains WORKING during operations. Generation, cancellation, containment, session barriers, and file-open policy remain unchanged. Runtime interaction and remaining document/settings call sites remain open.
- **UI-23 — MainWindow notification contract closure:** accepted-with-limits. All 81 MainWindow notification calls now pass explicit info/success/warning/error metadata, including document creation, session fallback, stale commands, settings guards, and active-tab projection; the permanent status phase continues to show WORKING and the external one-argument compatibility default remains intact. Runtime interaction and release gates remain open.
- **UI-24 — focus-state visibility:** accepted-with-limits. Centralized QSS now makes keyboard focus readable on command-rail/tool buttons, document tabs, and settings checkboxes by combining the existing accent boundary with a restrained surface/foreground cue; selected, pressed, checked, and disabled semantics remain unchanged. Native QSS specificity, keyboard traversal, DPI, accessibility, and release evidence remain open.
- **UI-25 — operation lifecycle seam:** accepted-with-limits. MainWindow now delegates monotonic operation-ID allocation and the one-current-operation stale guard to a Qt-free tracker while retaining busy/status/TaskRunner and all document/workspace/session policy. Runtime callback timing, visual acceptance, and release evidence remain open.
- **UI-26 — plugin operation-state seam:** accepted-with-limits. Catalog scanning, governance mutation, and host diagnostics now use one typed presentation tracker with independent lifecycle slots while dialogs, notifications, TaskRunner, plugin policy, and close guards remain unchanged. Runtime callback timing and release evidence remain open.
- **UI-27 — workspace-entry activation:** accepted-with-limits. Workspace files retain single-click opening and folders retain double-click navigation, while keyboard activation now routes each entry kind through one semantic presentation helper. Native event ordering, focus traversal, accessibility, DPI, and release evidence remain open.
- **UI-28 — workspace action hierarchy:** accepted-with-limits. Existing Back and Cancel workspace actions now have explicit quiet, hover/focus, pressed, and disabled QSS states while the open-workspace action remains primary; signals, loading policy, locale, and theme-token ownership remain unchanged. Native QSS rendering, font metrics, DPI, and release evidence remain open.
- **UI-29 — workspace-search lifecycle clarity:** accepted-with-limits. Search operation identity, generation invalidation, cooperative cancellation, and stale/current completion classification now live in a Qt-free tracker while MainWindow retains search service, TaskRunner, surface, containment, notification, and startup/close policy. Native queued delivery, thread timing, and release evidence remain open.
- **UI-30 — workspace-navigation lifecycle clarity:** accepted-with-limits. Workspace open/list operation identity and generation invalidation now have a Qt-free tracker while cancellation, busy/status, TaskRunner, session restore, result projection, and workspace policy remain in MainWindow. Native callback timing and release evidence remain open.
- **UI-31 — workspace file activation consistency:** accepted-with-limits. Workspace files now activate on click, double-click, or Enter/Return, folders remain navigable, and an already-open path reuses its tab; containment, busy gating, and asynchronous document dispatch remain in MainWindow. Native event ordering, runtime interaction, accessibility, DPI, and release evidence remain open.
- **UI-32 — workspace tree visual rhythm:** accepted-with-limits. The workspace tree now uses alternating rows, full-row single selection, stable row heights, readable long-name elision, and explicit alternate-hover surfaces from existing theme tokens. Runtime QSS rendering, font/DPI, accessibility, and release evidence remain open.
- **UI-33 — session-restore state clarity:** accepted-with-limits. Ordered session restore state, the workspace barrier, recovery deferrals, active path, and the async document/open binding now have a typed Qt-free boundary while MainWindow retains services, TaskRunner, tabs, notifications, startup/close guards, and result policy. Native callback timing, runtime startup, and release evidence remain open.
- **UI-34 — document-tab state clarity:** accepted-with-limits. Modified documents now retain the title asterisk and gain a theme-tinted authored document marker with an explicit refresh route; dirty calculation, save/recovery/close policy, and tab lifecycle remain in MainWindow. Native tab icon metrics, DPI, accessibility, and release evidence remain open.
- **UI-35 — warning-background foreground contract:** accepted-with-limits. Warning message, attention phase, workspace/search feedback, warning action, Find status, and workspace cancel hover/focus text now share a readable foreground derived from the actual warning background instead of directly reusing the decorative 砂金 endpoint or a second generic foreground; gold borders, filled warning hover, and application policy remain unchanged. Native QSS specificity, runtime visual, DPI, font, accessibility, and release evidence remain open.

## Current D10 slices

- **GOV-01 — workflow policy and handoff contract:** accepted with limits. The fixed Architect + Project Manager + Product + Developer 1 + Developer 2 + QA roster, Luna → Terra → Sol routing ladder, current-checkout/single-writer rule, no-launch boundary, required `handoff.md` structure, index, scaffold, and static verifier are now repository artifacts. Runtime startup and visual acceptance remain intentionally unrun.
- **GOV-02 — indexed handoff status consistency:** accepted with limits. `scripts/verify_handoff.ps1` now rejects disagreement between each indexed status and its Markdown handoff status row; the D6.6, D7.4.2, and UI-07 stale projections were synchronized. This is structural traceability, not runtime or external approval.

## Current D11 slices

- **ARCH-01 — public-source enterprise architecture baseline:** accepted-with-limits. The project now records the transferable public CloudWeGo principles, target layer invariants, migration phases, contract-first rules, and explicit non-goals; it does not claim access to ByteDance private standards or enterprise certification.
- **ARCH-02 — desktop composition-root isolation:** accepted-with-limits. `app.py` now owns mode dispatch and the Qt event loop, `composition.py` owns desktop concrete adapter selection and the desktop/plugin lifecycle, and `diagnostic_composition.py` owns the Qt-free diagnostic adapter choice. MainWindow decomposition, runtime startup, clean-machine evidence, and release gates remain open.
- **ARCH-03 — MainWindow command-surface coordinator:** accepted-with-limits. Menu and command-rail Qt projection now lives in `presentation/command_surface.py`; MainWindow keeps core command registration, callbacks, and the stable refresh seam. Document, workspace, recovery, session, plugin, and authorized runtime coordinators remain future slices.
- **ARCH-04 — MainWindow document-tab surface:** accepted-with-limits. `presentation/document_tab_surface.py` now owns the tab widget, record/index mapping, active/editor lookup, title projection, and tab-bar enablement; MainWindow keeps document state, path uniqueness, close/save/recovery/session policy, and lifecycle callbacks. Runtime Qt interaction and the remaining coordinator/release slices remain open.
- **ARCH-05 — MainWindow workspace surface:** accepted-with-limits. `presentation/workspace_surface.py` now owns workspace dock/panel composition, the five semantic navigation signal routes, and dock/panel locale projection; MainWindow keeps async workspace/search/session policy, generation/cancellation guards, containment checks, and error handling. Runtime dock/file interaction and remaining coordinator/release slices remain open.
- **ARCH-06 — MainWindow workspace-search surface:** accepted-with-limits. `presentation/workspace_search_surface.py` now owns search-dialog composition, three semantic signal routes, activation, locale, busy/cancel feedback, and result/error projection; MainWindow keeps query validation, `WorkspaceSearchService`/`TaskRunner`, operation/generation/cancellation state, containment checks, document opening, and notifications. Runtime search/file interaction and remaining coordinator/release slices remain open.
- **ARCH-07 — MainWindow workspace projection closure:** accepted-with-limits. `WorkspaceSurface` now owns semantic current-path/loading/directory/error projection in addition to dock/panel composition and navigation intents; MainWindow no longer imports or directly accesses `WorkspacePanel` while retaining workspace service, TaskRunner, generation/cancellation, session barrier, containment, and error policy. Runtime workspace interaction and remaining coordinator/release slices remain open.
- **ARCH-08 — MainWindow Find surface:** accepted-with-limits. `presentation/find_surface.py` now owns FindBar composition, six semantic signal routes, shell placement, mode/locale/status/operation projection; MainWindow keeps active-editor lookup, literal find/replace, Replace All lifecycle, rollback/cancellation, tab locking, and error policy. Runtime keyboard/Replace All interaction and remaining coordinator/release slices remain open.
- **ARCH-09 — MainWindow settings surface:** accepted-with-limits. `presentation/settings_surface.py` now owns SettingsDialog parentage and modal snapshot editing; MainWindow keeps SettingsService/TaskRunner persistence, result validation, theme/locale/editor projection, motion policy and transition trigger timing, notifications, and errors. Runtime settings interaction and remaining coordinator/release slices remain open.
- **ARCH-10 — MainWindow recovery prompt surface:** accepted-with-limits. `presentation/recovery_prompt_surface.py` now owns recovery prompt parentage, locale-aware text, button roles, and typed restore/discard/later projection; MainWindow keeps RecoveryService, snapshot/session/document consequences, cleanup, and notifications. Runtime recovery interaction and remaining coordinator/release slices remain open.
- **ARCH-11 — MainWindow status surface:** accepted-with-limits. `presentation/status_surface.py` now owns StatusRail creation, status-bar widget exposure, locale, and phase projection; MainWindow keeps working/attention/ready/error precedence, operation/document policy, TaskRunner lifecycle, and close guards. Runtime status rendering and remaining coordinator/release slices remain open.
- **ARCH-12 — MainWindow file-dialog surface:** accepted-with-limits. `presentation/file_dialog_surface.py` now owns localized file-only, folder-only, and Save As native dialog selection; MainWindow keeps startup/busy guards, async document/workspace dispatch, save/session/recovery policy, and containment decisions. Runtime native dialog interaction and remaining coordinator/release slices remain open.
- **ARCH-13 — MainWindow command-palette surface:** accepted-with-limits. `presentation/command_palette_surface.py` now owns command-palette modal composition, locale, parentage, and stable-ID projection; MainWindow keeps live registry resolution, execution, stale-ID feedback, menu refresh, and localized notifications. Runtime palette interaction and remaining coordinator/release slices remain open.
- **ARCH-14 — MainWindow plugin surface:** accepted-with-limits. `presentation/plugin_surface.py` now owns extension-catalog and plugin-status dialog lifecycle, locale, activation, semantic callbacks, and governance-button projection; MainWindow keeps plugin services, TaskRunner/operation state, trust/approval/enablement policy, validation, notifications, and errors. Runtime plugin-dialog interaction and remaining coordinator/release slices remain open.
- **ARCH-15 — MainWindow message surface:** accepted-with-limits. `presentation/message_surface.py` now owns localized save-before-close, About, and recoverable error message composition with a typed Save/Discard/Cancel result; MainWindow keeps dirty/save/close consequences, error phase, and status synchronization. Runtime message interaction and remaining coordinator/release slices remain open.
- **ARCH-16 — Theme transition surface:** accepted-with-limits. `presentation/theme_transition_surface.py` now owns the bounded opacity effect, easing, animation lifetime, interruption cleanup, and owned-effect guard; MainWindow keeps motion policy, central-widget selection, and trigger timing. Runtime animation rendering and remaining coordinator/release slices remain open.
- **ARCH-17 — Status surface host projection:** accepted-with-limits. `presentation/status_surface.py` now owns QStatusBar attachment, size-grip configuration, permanent rail placement, locale, and transient notification projection; MainWindow keeps phase precedence, TaskRunner/document policy, and notification call sites. Runtime status-bar interaction and remaining coordinator/release slices remain open.
- **ARCH-18 — Central editor shell surface:** accepted-with-limits. `presentation/editor_shell_surface.py` owns the `editorShell` QWidget/layout, `DocumentTabSurface`/`FindSurface` composition, initial hidden state, and FindBar locale; MainWindow keeps document/editor/operation policy and callbacks. Runtime editor-shell interaction and remaining coordinator/release slices remain open.
- **ARCH-19 — Editor document surface:** accepted-with-limits. `presentation/editor_document_surface.py` now owns per-document EditorWidget creation, presentation-safe settings/theme application, Save As language-hint refresh, and semantic modified/content/caret signal routing; MainWindow keeps `_DocumentTab`, document state, save/recovery/Replace All/session policy, and operation locking. Runtime editor interaction and remaining coordinator/release slices remain open.
- **ARCH-20 — Authored shell icon surface:** accepted-with-limits. `presentation/icons.py` owns semantic vector glyph rendering; `CommandSurface` and `WorkspaceSurface`/`WorkspacePanel` own icon projection and explicit theme refresh while MainWindow keeps only semantic metadata and application policy. Runtime painting and remaining coordinator/release slices remain open.
- **ARCH-21 — Semantic transient notification hierarchy:** accepted-with-limits. `StatusSurface` owns one localized transient-message state projection and centralized level styling while MainWindow retains notification and operation policy; runtime status rendering and remaining coordinator/release slices remain open.
- **ARCH-22 — Inline feedback state contract:** accepted-with-limits. `presentation/feedback.py` owns the shared property/repolish projection; WorkspacePanel and WorkspaceSearchDialog consume it without moving application policy or result ownership into widgets. FindBar mapping is intentionally deferred to D33; runtime rendering and remaining coordinator/release slices remain open.
- **ARCH-23 — FindBar semantic feedback projection:** accepted-with-limits. `FindSurface` forwards the shared `FeedbackLevel` contract; FindBar owns only status projection while MainWindow maps editor outcomes and retains operation/error policy. Runtime Find/Replace interaction and remaining coordinator/release slices remain open.
- **ARCH-24 — Session/Recovery notification severity closure:** accepted-with-limits. MainWindow remains the outcome owner and explicitly projects nineteen Session/Recovery levels through `StatusSurface`; no recovery/session state or application contract moves. Remaining coordinator/error/observability audits and runtime evidence remain open.
- **ARCH-25 — Plugin and extension notification severity closure:** accepted-with-limits. MainWindow remains the outcome owner and explicitly projects existing catalog, governance, runtime, and host outcomes through `StatusSurface`; no plugin trust, containment, execution, worker, stale-guard, or command-refresh policy moves. Remaining coordinator/error/observability audits and runtime evidence remain open.
- **ARCH-26 — Workspace and operation notification severity closure:** accepted-with-limits. MainWindow remains the outcome owner and explicitly projects existing workspace/search/file-boundary results and operation progress through `StatusSurface`; no generation, cancellation, containment, session barrier, or document-open policy moves. Remaining coordinator/error/observability audits and runtime evidence remain open.
- **ARCH-27 — MainWindow notification contract closure:** accepted-with-limits. MainWindow now supplies explicit legal transient levels at all 81 current notification call sites while StatusSurface remains the projection owner, permanent operation progress remains WORKING, and the external compatibility default remains available. Remaining coordinator extraction, observability, and runtime evidence remain open.
- **ARCH-28 — focus-state visibility:** accepted-with-limits. `presentation/theme.py` remains the single QSS owner and projects focus cues for command/tool buttons, document tabs, and checkboxes without moving keyboard, command, document, settings, locale, or motion policy. Runtime focus traversal and remaining coordinator/release slices remain open.
- **ARCH-29 — MainWindow operation-tracker boundary:** accepted-with-limits. `presentation/operation_tracker.py` owns only monotonic ID reservation and current-operation stale completion/cancellation guards; MainWindow retains `_busy`, TaskRunner, status phase, generation, session, workspace, service, and result policy. Runtime interleaving and remaining coordinator/release slices remain open.
- **ARCH-30 — plugin operation-state boundary:** accepted-with-limits. `presentation/plugin_operation_tracker.py` owns three closed, independent plugin operation slots and stale guards; MainWindow retains plugin services, TaskRunner, notifications, PluginSurface, command refresh, and security policy. Runtime interleaving and remaining coordinator/release slices remain open.
- **ARCH-31 — workspace-entry activation boundary:** accepted-with-limits. `WorkspacePanel` owns only item-kind-to-signal projection, including keyboard activation; MainWindow retains containment, document/workspace services, TaskRunner, async guards, and notifications. Native event ordering and remaining coordinator/release slices remain open.
- **ARCH-32 — workspace action hierarchy boundary:** accepted-with-limits. `presentation.theme` remains the single owner of the existing workspace action selectors; no signal, widget, application, locale, or workspace policy moves. Selector specificity, native rendering, and remaining coordinator/release slices remain open.
- **ARCH-33 — workspace-search lifecycle boundary:** accepted-with-limits. `presentation/workspace_search_operation_tracker.py` owns only active ID, generation, cancellation, and completion classification; MainWindow retains all service, worker, surface, containment, notification, startup, close, and result policy. Native callback interleaving and remaining coordinator/release slices remain open.
- **ARCH-34 — workspace-navigation lifecycle boundary:** accepted-with-limits. `presentation/workspace_operation_tracker.py` owns only workspace active ID, generation, invalidation, and completion classification; MainWindow retains generic operation/busy/status policy, TaskRunner, WorkspaceService/Surface, containment, session restore, notifications, and result handling. Native callback interleaving and remaining coordinator/release slices remain open.
- **ARCH-35 — workspace file activation boundary:** accepted-with-limits. `WorkspacePanel` and the existing `WorkspaceSurface` route file/folder activation while MainWindow retains busy gating, containment, existing-tab identity, asynchronous document dispatch, and notifications. Native event ordering and remaining coordinator/release slices remain open.
- **ARCH-36 — workspace tree visual-rhythm boundary:** accepted-with-limits. `WorkspacePanel` owns standard view interaction presentation and `presentation/theme.py` owns alternate/hover/selected QSS projection; semantic signals, services, filesystem policy, and MainWindow application policy remain unchanged. Native QSS rendering and remaining coordinator/release slices remain open.
- **ARCH-37 — session-restore state boundary:** accepted-with-limits. `presentation/session_restore_tracker.py` owns only typed ordered restore values and the pending document/open-operation binding; MainWindow retains session/recovery/workspace/document services, TaskRunner, tab projection, notifications, startup/close guards, and result policy. Native callback timing and remaining coordinator/release slices remain open.
- **ARCH-38 — document-tab state projection boundary:** accepted-with-limits. `presentation/icons.py` owns the authored modified glyph and `DocumentTabSurface` owns only palette-tinted marker projection/index alignment; MainWindow retains dirty interpretation, title projection, save/recovery/close policy, and tab lifecycle. Native rendering and remaining coordinator/release slices remain open.
- **ARCH-39 — session-save state boundary:** accepted-with-limits. `presentation/session_save_tracker.py` owns only the saved baseline, latest queued snapshot, in-flight operation identity, and stale/invalid/valid callback classification; MainWindow retains debounce, snapshot capture, SessionService, TaskRunner, notifications, startup, close, and result policy. Native callback timing and remaining coordinator/release slices remain open.
- **ARCH-40 — recovery-capture lifecycle boundary:** accepted-with-limits. `presentation/recovery_capture_tracker.py` owns opaque capture/job identity, document/snapshot lifecycle, discarded callbacks, worker-write state, and deferred deletion; MainWindow retains editor slicing, channel backpressure, RecoveryService, TaskRunner, tab, notification, and close policy. Native timing, durability, and remaining coordinator/release slices remain open.
- **ARCH-41 — settings-save callback boundary:** accepted-with-limits. `presentation/settings_save_tracker.py` owns one positive operation identity and stale/invalid/valid/failure classification; MainWindow retains SettingsService, TaskRunner, settings application, theme/locale/font/editor projection, transition, notifications, and close policy. Native settings interaction and remaining coordinator/release slices remain open.
- **ARCH-42 — Replace All lifecycle boundary:** accepted-with-limits. `presentation/replace_all_tracker.py` owns one typed active job, expected content-version guard, and stale finish protection; each queued callback carries its job identity while MainWindow retains ReplaceAllSession, QTimer, editor, cancellation, rollback, status, notification, and operation policy. Native callback timing, editor rollback, and remaining coordinator/release slices remain open.
- **ARCH-43 — Find Match snapshot boundary:** accepted-with-limits. `presentation/find_match_tracker.py` owns an immutable document/query/case/selection/content-version snapshot and exact-match/clear semantics; MainWindow retains EditorWidget find/selection, selected-text validation, busy gating, invalidation, replacement, and feedback policy. Native selection timing and remaining coordinator/release slices remain open.
- **ARCH-44 — Recovery Scan lifecycle boundary:** accepted-with-limits. `presentation/recovery_scan_tracker.py` owns one immutable operation/startup-context job and stale finish guard; MainWindow retains RecoveryService, TaskRunner, candidate validation, recovery prompts, session-restore continuation, notifications, and close policy. Native callback timing and remaining coordinator/release slices remain open.
- **ARCH-45 — document-tab path identity boundary:** accepted-with-limits. `DocumentTabSurface` now owns canonical full-registry path lookup with pathless/identity-exclusion semantics; MainWindow delegates ordinary lookup while retaining session-restore subset selection and document policy. Native tab ordering, filesystem case behavior, and remaining coordinator/release slices remain open.
- **ARCH-46 — session-restore tab projection boundary:** accepted-with-limits. `SessionRestoreTracker[TabT]` now owns ordered opaque restored-tab references and canonical active-path/first-tab selection input through a path callback; MainWindow retains all session, service, async, tab, notification, startup, initial-document, close, and result policy. Native callback timing and remaining coordinator/release slices remain open.
- **ARCH-47 — session-load state simplification:** accepted-with-limits. The write-only `_session_load_state` field and assignments are removed; `SessionLoadResult.state`, default normalization, save baseline, recovery-first startup sequencing, notifications, and close policy remain at the existing coordinator boundary. Runtime callback timing and remaining coordinator/release slices remain open.
- **ARCH-48 — status-phase forwarding simplification:** accepted-with-limits. The no-behavior `_sync_active_document_phase()` alias is removed; editor, workspace, tab, runner, and operation callers directly use the single `_sync_status_surface()` working/attention/ready precedence. Runtime event timing and remaining coordinator/release slices remain open.
- **UI-36 — document-tab selection hierarchy:** accepted-with-limits. Centralized theme QSS now makes selected/selected-hover tabs, the tab rail, and workspace dock titles easier to scan using existing contrast-safe tokens; tab behavior, warning/gold foregrounds, and remaining runtime/release gates remain open.
- **ARCH-49 — session-snapshot capture boundary:** accepted-with-limits. `presentation.session_snapshot_builder.build_session_snapshot[TabT]` owns only clean path-backed tab filtering, ordered domain snapshot assembly, cursor validation, and active-index derivation; MainWindow retains editor reads, session-save debounce, services, startup, restore, notifications, and close policy. Runtime callback timing and remaining coordinator/release slices remain open.
- **UI-37 — find-bar action hierarchy:** accepted-with-limits. FindBar now exposes presentation-only navigation/cancel/close selectors and centralized QSS gives query/replacement inputs, navigation, cancel, close, primary, and Replace All actions distinct visual roles using existing tokens; find behavior and warning semantics remain unchanged. Native QSS rendering and remaining runtime/release gates remain open.
- **UI-38 — toolbar context chip:** accepted-with-limits. The existing localized local/safe context label is now a compact token-driven chip with a leading accent boundary; CommandSurface, i18n, command callbacks, shortcuts, layout, and locale behavior remain unchanged. Native QSS rendering and remaining runtime/release gates remain open.
- **UI-39 — settings control hierarchy:** accepted-with-limits. Settings appearance/editor groups and theme/accent/font/size controls now have token-driven visual rails while titles remain contrast-safe; SettingsSnapshot, language, ranges, persistence, and application policy remain unchanged. Native QSS rendering and remaining runtime/release gates remain open.
- **ARCH-50 — document-tab forwarding simplification:** accepted-with-limits. MainWindow now calls `DocumentTabSurface` active/editor/path/containment lookup directly; the four no-policy aliases are removed while save/open duplicate exclusion, session restore subset selection, close/recovery/Replace All/Find, and status policy remain unchanged. Native callback timing and remaining runtime/release gates remain open.
- **UI-40 — dialog action hierarchy:** accepted-with-limits. Extension Catalog, Plugin Status, Workspace Search, and Settings now distinguish primary, warning, and quiet actions through presentation metadata; the shared dialog action rail and contrast-safe quiet states live in centralized QSS while signals, enablement, locale, persistence, and application policy remain unchanged. Native rendering, keyboard traversal, accessibility, and remaining runtime/release gates remain open.
- **ARCH-51 — MainWindow file-dialog/About forwarding simplification:** accepted-with-limits. Save As and dirty-tab close call `FileDialogSurface.choose_save_path` directly, and `help.about` binds directly to `MessageSurface.show_about`; the two no-policy aliases are removed while document, close, command, and locale policy remain unchanged. Native callback timing and remaining runtime/release gates remain open.
- **UI-41 — message-dialog visual hierarchy:** accepted-with-limits. About, error, unsaved-close, and recovery dialogs now use themed QMessageBox identities and primary/warning/quiet action roles while preserving localized content, button semantics, return mapping, and recovery policy. Native rendering, keyboard traversal, accessibility, and remaining runtime/release gates remain open.
- **UI-42 — notification localization closure:** accepted-with-limits. Duplicate-open, plugin-failure, known invalid-workspace, extension-catalog, and plugin-host notifications now project stable Chinese labels without losing dynamic diagnostics; Plugin Catalog summaries refresh with locale changes while application summaries remain locale-free. Native dialog rendering, runtime language switching, accessibility, and remaining runtime/release gates remain open.
- **UI-43 — plugin catalog locale projection:** accepted-with-limits. Plugin Catalog rows, tooltips, and empty state now localize stable field/enum labels in English/简体中文 with raw-value fallback while preserving names, IDs, versions, paths, hashes, permissions, errors, entry state, signals, and governance policy. Native list rendering and remaining runtime/release gates remain open.
- **UI-44 — plugin status boolean locale projection:** accepted-with-limits. Plugin Status tooltips now render enabled/active booleans as locale-aware `true/false` or `是/否` while preserving typed runtime state, lifecycle controls, trust/enablement predicates, and plugin policy. Native tooltip rendering and remaining runtime/release gates remain open.
- **UI-45 — plugin-dialog visual hierarchy:** accepted-with-limits. Plugin Catalog and Plugin Status now have a token-driven accent boundary, summary card, list content panel, focus cue, and selected-row edge/weight through centralized QSS; dialog behavior, data, locale, signals, and plugin policy remain unchanged. Native rendering and remaining runtime/release gates remain open.
- **UI-46 — settings appearance preview:** accepted-with-limits. Settings now shows a live presentation-only preview of pending theme, accent, interface font, and interface size choices using the canonical token stylesheet; Save/Cancel, persistence, global application timing, locale, editor settings, and motion policy remain unchanged. Native rendering, installed-font fallback, accessibility, and remaining runtime/release gates remain open.
- **UI-47 — settings preview surface boundary:** accepted-with-limits. The preview widget tree is isolated in `SettingsPreviewSurface` with a one-way `project(...)` contract; `SettingsDialog` retains form values, snapshot assembly, Save/Cancel, locale flow, and policy. Native Qt ownership/rendering and remaining runtime/release gates remain open.
- **UI-48 — workspace-search visual hierarchy:** accepted-with-limits. Find in Files now distinguishes workspace scope, query, results, diagnostics, focus, hover, selected, checked, and disabled states through scoped centralized QSS plus one semantic diagnostic-toggle identity; search behavior and async policy remain unchanged. Native rendering, accessibility, and remaining runtime/release gates remain open.
- **UI-49 — recovery notification dynamic localization:** accepted-with-limits. Recovery success notifications now localize the static “content remains unsaved” suffix through the existing presentation boundary while preserving the dynamic document name and `en-US` identity; native runtime language switching and remaining release gates remain open.
- **UI-50 — shell visual rhythm:** accepted-with-limits. Ordinary command-rail, document-tab, status-rail, and common-control surfaces now use a quieter centralized QSS baseline, while selected/focus/feedback/warning/error states remain explicit and the primary action uses a flat contrast-aware accent; native rendering, accessibility, and remaining runtime/release gates remain open.
- **ARCH-52 — plugin catalog coordinator boundary:** accepted-with-limits. Catalog scan and descriptor approval/revocation sequencing now lives behind a Qt-free `PluginCatalogCoordinator` with narrow task/view/notification contracts; MainWindow retains plugin operation close gates, host/runtime controls, locale/theme projection, and trust/security policy. Native callback timing and remaining runtime/release gates remain open.
- **ARCH-53 — plugin-host probe coordinator boundary:** accepted-with-limits. Diagnostic host probe sequencing now lives behind a Qt-free `PluginHostProbeCoordinator` with shared task/notification contracts; MainWindow retains plugin operation close gates, composition, notification projection, and host execution/security boundaries. Native callback timing and remaining runtime/release gates remain open.
- **ARCH-54 — plugin runtime coordinator boundary:** accepted-with-limits. Runtime failure/status/enablement sequencing now lives behind a Qt-free `PluginRuntimeCoordinator` with typed runtime/view/notification contracts; MainWindow retains busy/close/composition policy while `PluginRuntime` remains the application security boundary. Native event timing and remaining runtime/release gates remain open.
- **ARCH-55 — session-load coordinator boundary:** accepted-with-limits. `SessionLoadResult` classification, baseline projection, fixed error notification, and recovery-first continuation now live behind a Qt-free `SessionLoadCoordinator`; MainWindow retains session service/TaskRunner/startup/workspace/tab/close policy. Native callback timing and remaining runtime/release gates remain open.
- **ARCH-56 — recovery-scan coordinator boundary:** accepted-with-limits. Recovery inventory validation, stale finish, empty/failure notifications, and startup continuation now live behind a Qt-free `RecoveryScanCoordinator`; MainWindow retains RecoveryService, prompt decisions, restore policy, TaskRunner, and close gates. Native callback timing and remaining runtime/release gates remain open.
- **ARCH-57 — session-save coordinator boundary:** accepted-with-limits. Session-save completion classification, stale suppression, invalid/failure notifications, and latest-request draining now live behind a Qt-free `SessionSaveCoordinator`; MainWindow retains SessionService, snapshot capture, debounce, TaskRunner, startup, and close policy. Native callback timing and remaining runtime/release gates remain open.
- **ARCH-58 — settings-save coordinator boundary:** accepted-with-limits. Settings-save completion/failure classification now lives behind a Qt-free `SettingsSaveCoordinator`; MainWindow retains SettingsService, theme/locale/font/editor application, transition, notifications, and close policy. Native settings interaction and remaining runtime/release gates remain open.
- **ARCH-59 — workspace-search coordinator boundary:** accepted-with-limits. Workspace-search callback classification, typed result validation, invalidated cancellation, surface projection, and summary severity now live behind a Qt-free `WorkspaceSearchCoordinator`; MainWindow retains query/service/cancel/containment/result-open/close policy. Native search interaction and remaining runtime/release gates remain open.
- **ARCH-60 — workspace-navigation coordinator boundary:** accepted-with-limits. Workspace open/directory completion classification, loading/error projection, invalid-result handling, and failure/session-restore lifecycle now live behind a Qt-free `WorkspaceNavigationCoordinator`; MainWindow retains WorkspaceService, activation, search-root/directory-root policy, containment, document opening, session persistence, and close policy. Native navigation interaction and remaining runtime/release gates remain open.
- **ARCH-61 — document-open coordinator boundary:** accepted-with-limits. Ordinary and session-restore document-open completion classification, stale suppression, result validation, failure projection, and restore continuation now live behind a Qt-free `DocumentOpenCoordinator`; MainWindow retains DocumentService, duplicate-tab/editor/line/cursor/event policy, persistence, and close policy. Native editor/session interaction and remaining runtime/release gates remain open.
- **ARCH-62 — document-save coordinator boundary:** accepted-with-limits. Document-save stale/liveness classification, read-only release, `DocumentState` validation, and invalid/failure projection now live behind a Qt-free generic `DocumentSaveCoordinator[TabT]`; MainWindow retains state/language/title/recovery/event/notification/session-save/continuation policy and close behavior. Native editor/save interaction and remaining runtime/release gates remain open.
- **ARCH-63 — recovery-delete coordinator boundary:** accepted-with-limits. Recovery delete tracker release, owner identity clearing, success/error notification, and pending-delete drain now live behind a Qt-free generic `RecoveryDeleteCoordinator[JobT, OwnerT]`; MainWindow retains request admission, RecoveryService dispatch, operation/capture/write/recovery policy, persistence, and close behavior. Native recovery/delete interaction and remaining runtime/release gates remain open.
- **ARCH-64 — Replace All completion coordinator boundary:** accepted-with-limits. Replace All current-job release, live-tab editor unlock, tab-bar/Find operation-state release, and generic operation completion now live behind a Qt-free generic `ReplaceAllCompletionCoordinator[TabT, SessionT, ProgressT]`; MainWindow retains editor slicing/cancel/rollback/clean-state/limit/result policy and close behavior. Native Replace All interaction and remaining runtime/release gates remain open.
- **ARCH-65 — recovery-write coordinator boundary:** accepted-with-limits. Recovery writer discarded/capture/document/write lifecycle release and saved/failed ordering now live behind a Qt-free generic `RecoveryWriteCoordinator[JobT, OwnerT]`; MainWindow retains tab/content/snapshot/delete/notification/persistence policy and close behavior. Native capture/write interaction and remaining runtime/release gates remain open.
- **ARCH-66 — recovery-capture abort coordinator boundary:** accepted-with-limits. Recovery capture identity protection, worker-started discard versus pre-worker release, channel/session cleanup, document lifecycle release, and optional live-owner failure notification now live behind a Qt-free generic `RecoveryCaptureAbortCoordinator[JobT, OwnerT]`; MainWindow retains Qt scheduling, editor capture stepping, stale/cancel decisions, RecoveryService dispatch, retry eligibility, and close behavior. Native capture/session/channel interaction and remaining runtime/release gates remain open.
- **ARCH-67 — session-restore coordinator boundary:** accepted-with-limits. Ordered session restore startup/workspace guards, deferred and duplicate path progression, pending-open binding, active/first-tab completion selection, initial-document fallback, and finish/save ordering now live behind a Qt-free generic `SessionRestoreCoordinator[TabT]`; MainWindow retains workspace/document services, TaskRunner/open callbacks, tab/editor projection, startup/close state, and notification policy. Native session/open/workspace interaction and remaining runtime/release gates remain open.
- **ARCH-68 — recovery-write finish boundary:** accepted-with-limits. The existing Qt-free `RecoveryWriteCoordinator[JobT, OwnerT]` now directly owns `RecoveryCaptureTracker.finish_write()` and forwards pending-delete data, while MainWindow retains delete admission, RecoveryService dispatch, delete callbacks, and tab/content/snapshot policy. Native writer/capture/delete interaction and remaining runtime/release gates remain open.
- **ARCH-69 — document-tab removal coordinator boundary:** accepted-with-limits. Approved tab removal finalization now lives behind a Qt-free generic `DocumentTabRemovalCoordinator[TabT, CaptureT]` for recovery cleanup, tab/editor/event/session finalization, and empty-tab fallback; MainWindow retains busy/startup/dirty/save-confirm close policy and service/notification decisions. Native close/save/recovery timing and remaining runtime/release gates remain open.
- **ARCH-70 — document-tab creation coordinator boundary:** accepted-with-limits. Document-tab editor/record assembly, surface add, title/modified projection, session-save request, and status synchronization now live behind a Qt-free generic `DocumentTabCreationCoordinator[OpenedT, TabT, EditorT]`; MainWindow retains document/settings/editor policy, concrete tab/recovery identity construction, open/recovery outcomes, and close behavior. Native editor/currentChanged interaction and remaining runtime/release gates remain open.
- **ARCH-71 — document-open projection coordinator boundary:** accepted-with-limits. Valid ordinary/session document-open projection now lives behind a Qt-free generic `DocumentOpenProjectionCoordinator[TabT]`; MainWindow retains composition, concrete editor/tab callbacks, D86 stale/invalid/failure classification, services, and close policy. Native editor/tab/session timing and remaining runtime/release gates remain open.
- **ARCH-72 — document-save projection coordinator boundary:** accepted-with-limits. Valid document-save completion projection now lives behind a Qt-free generic `DocumentSaveProjectionCoordinator[TabT]`; D87 retains stale/invalid/failure classification, tab liveness, and read-only release, while MainWindow retains concrete editor/recovery/event/notification/session/close policy. Native editor/save timing and remaining runtime/release gates remain open.
- **ARCH-73 — workspace-navigation projection coordinator boundary:** accepted-with-limits. Valid workspace-open/directory projection now lives behind a Qt-free `WorkspaceNavigationProjectionCoordinator`; D85 retains stale/invalidated/loading/invalid/failure classification while MainWindow retains admission, WorkspaceService, concrete surface/search, session, and close policy. Native workspace/search timing and remaining runtime/release gates remain open.
- **ARCH-74 — settings-save projection coordinator boundary:** accepted-with-limits. Valid settings-save snapshot/theme/locale/editor/motion/success projection now lives behind a Qt-free `SettingsSaveProjectionCoordinator`; D83 retains tracker/stale/invalid/failure classification while MainWindow retains QApplication/theme, concrete surfaces, motion, notifications, and close policy. Native settings/theme/font/animation timing and remaining runtime/release gates remain open.
- **ARCH-75 — recovery-write projection coordinator boundary:** accepted-with-limits. Valid recovery saved/failed tab, snapshot identity, dirty/content-version, deletion, and feedback projection now lives behind a Qt-free `RecoveryProjectionCoordinator`; D90/D93 retain discarded/capture/document/write lifecycle and pending-delete release while MainWindow retains concrete recovery, tab, notification, and close policy. Native recovery timing and remaining runtime/release gates remain open.
- **ARCH-76 — close-readiness coordinator boundary:** accepted-with-limits. Ordered close admission, workspace-search cancellation, dirty/background protection, immediate session-save/pending-work recheck, and timer-stop policy now live behind a Qt-free `CloseGuardCoordinator`; MainWindow retains QCloseEvent, error-dialog projection, TaskRunner, timers, session persistence, and concrete operation ownership. Native close timing and remaining runtime/release gates remain open.
- **UI-51 — document-tab rail state clarity:** accepted-with-limits. The native document rail now has stable semantic identity, middle elision, non-expanding layout, explicit selected/hover/focus/disabled/close-button states, and a Paper/Sand muted-text contrast correction through centralized QSS; tab signals, dirty behavior, and close policy remain unchanged. Native rendering and remaining runtime/release gates remain open.
- **UI-52 — workspace resource-manager hierarchy:** accepted-with-limits. The workspace panel now exposes explicit localized unselected/loading/empty-directory states, readable path/tree semantics, calmer navigation copy, and centralized empty-state QSS while preserving file-first-click, folder-double-click, Enter/Return activation, loading/error ownership, and workspace service policy. Native Qt rendering and remaining runtime/release gates remain open.
- **ARCH-77 — session-save request/dispatch boundary:** accepted-with-limits. The existing Qt-free `SessionSaveCoordinator` now owns latest snapshot request, single-flight operation binding, concrete submission callback, completion classification, and queued-request draining; MainWindow retains QTimer, SessionService, TaskRunner, snapshot capture, startup, notifications, and close policy. Native worker timing and remaining runtime/release gates remain open.
- **UI-53 — editor canvas token hierarchy:** accepted-with-limits. The existing presentation theme resolver now produces explicit editor canvas, gutter, selection, caret, current-line, and Python syntax tokens with contrast-safe fallbacks; `EditorWidget` only projects them to QScintilla while editor behavior, settings, language, and document policy remain unchanged. Native rendering, DPI/font fallback, and remaining runtime/release gates remain open.
- **ARCH-78 — recovery-write dispatch callback boundary:** accepted-with-limits. The existing Qt-free `RecoveryWriteCoordinator` now binds typed success/failure callbacks for both recovery-write payload paths while MainWindow retains operation IDs, RecoveryService, channel/payload selection, TaskRunner, capture, notification, persistence, and close policy. Native worker timing, backpressure, durability, and remaining runtime/release gates remain open.
- **UI-54 — scrollbar chrome:** accepted-with-limits. Centralized theme QSS now gives vertical and horizontal scrollbars matched track/handle/hover states and no longer collapses `QScrollBar:horizontal` through the hidden subcontrol selector; scroll behavior and editor settings remain unchanged. Native style-engine/DPI rendering and remaining runtime/release gates remain open.
- **ARCH-79 — recovery-delete dispatch callback boundary:** accepted-with-limits. The existing Qt-free `RecoveryDeleteCoordinator` now binds typed success/failure callbacks for recovery deletion while MainWindow retains delete admission, operation IDs, RecoveryService, TaskRunner, notification, persistence, and close policy. Native worker timing, filesystem durability, and remaining runtime/release gates remain open.
- **ARCH-80 — presentation contract and observability audit:** accepted-with-limits. `scripts/audit_presentation_contracts.py` now statically guards Qt-free coordinator dependencies, explicit MainWindow notification levels, and TaskRunner pending-work projection through the WORKING phase; no runtime behavior or error policy moved. Native event timing, runtime error completeness, and remaining release gates remain open.
- **ARCH-81 — recovery-scan dispatch callback boundary:** accepted-with-limits. The existing Qt-free `RecoveryScanCoordinator` now binds typed success/failure callbacks for recovery inventory scans while MainWindow retains scan admission, startup/manual context, RecoveryService, TaskRunner, session continuation, and close policy. Native worker timing, recovery durability, and remaining release gates remain open.
- **ARCH-82 — document-save dispatch callback boundary:** accepted-with-limits. The existing Qt-free `DocumentSaveCoordinator` now binds typed success/failure callbacks for ordinary document saves while MainWindow retains snapshots, read-only policy, DocumentService, TaskRunner, persistence, and close behavior. Native save timing, filesystem durability, and remaining release gates remain open.

- **ARCH-83 — document-open dispatch callback boundary:** accepted-with-limits. The existing Qt-free `DocumentOpenCoordinator` now binds typed success/failure callbacks and optional line navigation for ordinary and session-restore opens while MainWindow retains path selection, restore binding, DocumentService, TaskRunner, persistence, status/notification, and close behavior. Native open timing, filesystem decoding, and remaining runtime/release gates remain open.
- **ARCH-84 — settings-save dispatch callback boundary:** accepted-with-limits. The existing Qt-free `SettingsSaveCoordinator` now binds typed success/failure callbacks through the existing tracker while MainWindow retains settings editing, SettingsService, TaskRunner, theme/font/locale/editor/motion projection, persistence, notifications, and close policy. Native settings rendering, worker timing, and remaining runtime/release gates remain open.
- **ARCH-85 — session-load dispatch callback boundary:** accepted-with-limits. The existing Qt-free `SessionLoadCoordinator` now binds typed success/failure callbacks for startup session loading while preserving baseline-before-recovery ordering and invalid-manifest retention; MainWindow retains startup admission, SessionService, TaskRunner, session state, notifications, persistence, and close policy. Native startup/session timing and remaining runtime/release gates remain open.
- **UI-55 — settings field hierarchy:** accepted-with-limits. Settings controls now expose stable semantic identities for language, theme, accent, interface/editor fonts and sizes, wrapping, line numbers, and motion; scoped centralized QSS adds readable field-label and compact option-row hover/checked/disabled states while settings values, locale, preview, persistence, and motion behavior remain unchanged. Native Qt rendering, formal contrast, installed fonts, DPI, and remaining runtime/release gates remain open.
- **D114 / ARCH-86 — workspace-navigation dispatch callback boundary:** accepted-with-limits. Workspace open and directory-load submissions now bind through typed methods on the existing Qt-free coordinator while generation invalidation, stale suppression, loading/error projection, session restore, WorkspaceService, TaskRunner, and close policy remain unchanged. Native callback timing, filesystem behavior, and remaining runtime/release gates remain open.
- **UI-56 / ARCH-87 — command-rail visual role hierarchy:** accepted-with-limits. The top command rail now gives Save a primary role, replacement/command-palette actions quiet roles, and workspace a context role through closed presentation metadata and scoped token QSS; callbacks, shortcuts, locale, icons, layout, command policy, and remaining runtime/release gates remain unchanged/open.
- **UI-57 / ARCH-88 — workspace-dock chrome hierarchy:** accepted-with-limits. The existing WorkspaceDock now has scoped frame/title/close/float styling with readable hover/pressed/disabled states while docking, workspace signals, navigation, locale, and close policy remain unchanged. Native QDockWidget rendering and remaining runtime/release gates remain open.
- **D115 / ARCH-89 — workspace-search dispatch callback boundary:** accepted-with-limits. Search callback binding now lives in the existing Qt-free coordinator while query/cancellation, WorkspaceSearchService, TaskRunner, surface, containment, locale, and close policy remain unchanged. Native search timing, filesystem behavior, and remaining runtime/release gates remain open.
- **UI-58 — dialog-shell edge hierarchy:** accepted-with-limits. Existing Settings and Command Palette dialogs now have token-driven outer frames and distinct top accents through object-scoped centralized QSS; child controls, signals, locale, focus order, layout, settings persistence, command selection, and remaining runtime/release gates remain unchanged/open.
- **D116 / ARCH-90 — session-save dispatch callback boundary:** accepted-with-limits. Latest-wins session persistence now binds the existing coordinator's operation callbacks through an injected dispatcher while snapshot capture/debounce, SessionService, notifications, startup restore, persistence, close policy, and remaining runtime/release gates remain unchanged/open.
- **UI-59 — control-affordance chrome:** accepted-with-limits. Existing ComboBox dropdowns and SpinBox up/down subcontrols now project readable token-driven base/hover/pressed/disabled states while value ranges, signals, focus, locale, persistence, native semantics, and remaining runtime/release gates remain unchanged/open.
- **UI-60 — dialog action-rail hierarchy:** accepted-with-limits. Plugin Catalog and Plugin Status now expose a shared token-separated action rail while button objects, order, enablement, signals, locale, plugin governance, and remaining runtime/release gates remain unchanged/open.
- **UI-61 — accent palette swatch hierarchy:** accepted-with-limits. Settings Theme and Accent choices now expose token-derived vector swatches while localized labels, ThemeId/AccentId values, preview, persistence, and remaining runtime/release gates remain unchanged/open.
- **UI-62 — document tab close-affordance hierarchy:** accepted-with-limits. Document tabs now expose a scoped 18px close target with explicit focus and disabled states while close signals, tab lifecycle, document policy, and remaining runtime/release gates remain unchanged/open.
- **D117 / ARCH-91 — operation-reserve facade simplification:** accepted-with-limits. MainWindow now uses the canonical OperationTracker.reserve contract directly and retains busy/status policy in its strategy methods while operation sequencing, dispatch, persistence, and remaining runtime/release gates remain unchanged/open.
- **D118 / ARCH-92 — session-restore ports contract:** accepted-with-limits. The existing Qt-free SessionRestoreCoordinator now consumes an explicit frozen/slotted typed ports contract instead of ten positional callbacks; MainWindow retains restore services, state, tab projection, startup/close, notification, TaskRunner, and persistence policy. Runtime callback timing and remaining runtime/release gates remain open.
- **UI-63 — Find close-affordance stability:** accepted-with-limits. The existing Find close control now has a stable 30px minimum height and scoped token-driven pressed/disabled states while FindBar identity, signals, locale, layout, and editor policy remain unchanged. Native rendering and remaining runtime/release gates remain open.
- **D119 / ARCH-93 — workspace file activation boundary:** accepted-with-limits. The existing WorkspaceSurface file intent now routes through a Qt-free typed activation coordinator that preserves file first-click, directory/keyboard semantics, containment, duplicate-tab focus, notifications, and the single asynchronous document-open boundary. Native tree interaction and remaining runtime/release gates remain open.
- **D120 / ARCH-94 — workspace-navigation ports contract:** accepted-with-limits. The existing Qt-free WorkspaceNavigationCoordinator now consumes a frozen/slotted named ports contract for tracker completion, surface access, projection, restore continuation, and notification while preserving generation/stale/invalidation/error behavior and MainWindow policy ownership. Native callback timing and remaining runtime/release gates remain open.
- **UI-64 / ARCH-95 — shell elevation and visual rhythm:** accepted-with-limits. The centralized stylesheet now gives the command rail and document-tab rail rounded grouped surfaces, clearer spacing, larger command targets, and preserved focus/pressed/selected/disabled hierarchy across all existing themes and accents. Native Qt rendering and remaining runtime/release gates remain open.
- **UI-65 / ARCH-96 — semantic message and tooltip chrome:** accepted-with-limits. Existing common/about/error/recovery message boxes and tooltips now use token-driven semantic accents, readable informative text, and consistent targets without changing MessageSurface, recovery decisions, locale, or application policy. Native dialog rendering and remaining runtime/release gates remain open.
- **D121 / ARCH-97 — close-guard ports contract:** accepted-with-limits. The existing Qt-free CloseGuardCoordinator now consumes a frozen/slotted named ports contract while preserving operation/search/dirty/background/pending precedence, cancellation, immediate-save, timer-stop, QCloseEvent, and MainWindow policy ownership. Native event timing and remaining runtime/release gates remain open.
- **D122 / ARCH-98 — document-open ports contract:** accepted-with-limits. The existing Qt-free DocumentOpenCoordinator now consumes a frozen/slotted named ports contract while preserving ordinary/session-restore completion, invalid/failure, notification, projection, line-number, and continuation behavior. Native editor timing and remaining runtime/release gates remain open.
- **D123 / ARCH-99 — document-save ports contract:** accepted-with-limits. The existing Qt-free generic DocumentSaveCoordinator now consumes a frozen/slotted named ports contract while preserving stale/liveness guards, read-only release, DocumentState validation, valid-save projection, failure errors, and MainWindow policy ownership. Native editor timing, filesystem durability, and remaining runtime/release gates remain open.
- **D124 / ARCH-100 — document-save projection ports contract:** accepted-with-limits. The existing Qt-free generic DocumentSaveProjectionCoordinator now consumes a frozen/slotted named ports contract while preserving state/language/title/recovery/event/notification/session-save/continuation order and MainWindow policy ownership. Native editor timing, filesystem durability, and remaining runtime/release gates remain open.
- **D125 / ARCH-101 — document-open projection ports contract:** accepted-with-limits. The existing Qt-free generic DocumentOpenProjectionCoordinator now consumes a frozen/slotted named ports contract while preserving duplicate/restored branches, line/cursor projection, event/notification, continuation order, and MainWindow policy ownership. Native editor timing, filesystem decoding, and remaining runtime/release gates remain open.
- **D126 / ARCH-102 — document-tab creation ports contract:** accepted-with-limits. The existing Qt-free generic DocumentTabCreationCoordinator now consumes a frozen/slotted named ports contract while preserving editor/tab construction, recovery identity, title/modified projection, session-save request, status sync, and MainWindow policy ownership. Native editor timing, recovery durability, and remaining runtime/release gates remain open.
- **D127 / ARCH-103 — document-tab removal ports contract:** accepted-with-limits. The existing Qt-free generic DocumentTabRemovalCoordinator now consumes a frozen/slotted named ports contract while preserving liveness, recovery-capture cancellation, snapshot cleanup, tab/editor/event/session finalization, empty-tab fallback, bool semantics, and MainWindow close policy. Native tab/editor timing, recovery durability, and remaining runtime/release gates remain open.
- **UI-66 / ARCH-104 — editor-stage visual rhythm:** accepted-with-limits. The existing EditorShellSurface now owns 10/8px stage margins and 8px inter-surface spacing, while the centralized `editorShell` selector uses `surface_1` around the existing `surface_0` tab/editor canvas. Tab/Find order, signals, visibility, locale, fonts, motion, document policy, and remaining runtime/release gates remain open.
- **D128 / ARCH-105 — core command registration boundary:** accepted-with-limits. The existing Qt-free CoreCommandCoordinator now owns deterministic construction/registration of the 23 built-in commands through named ports; MainWindow retains callback/application policy and CommandSurface retains Qt projection/plugin refresh. Command order, metadata, callback identity, and remaining runtime/release gates remain open.
- **D129 / ARCH-106 — core toolbar composition boundary:** accepted-with-limits. The existing presentation-only CoreToolbarCoordinator now owns deterministic composition of the six core toolbar actions plus the optional workspace action through named ports; MainWindow retains callback/application policy and CommandSurface retains QToolBar/QAction projection and locale-aware presentation. Action order, metadata, callback identity, and remaining runtime/release gates remain open.
- **D130 / ARCH-107 — pure toolbar contract extraction:** accepted-with-limits. IconKey, ToolbarActionRole, and ToolbarActionSpec now live in pure-Python contract modules; compatibility imports remain at the existing icon/projection seams and CoreToolbarCoordinator no longer imports a Qt-bearing module. Native rendering, runtime, clean-machine, and release gates remain open.
- **D131 / ARCH-108 — Replace All admission ports boundary:** accepted-with-limits. The existing Qt-free ReplaceAllAdmissionCoordinator now owns busy/active-tab/query admission, session creation, operation/tracker binding, and starter handoff through named ports; MainWindow retains editor/FindSurface/tab-bar/QTimer projection, cooperative slicing, completion/rollback, and application policy. Native Replace All timing and remaining runtime/release gates remain open.
- **D132 / ARCH-109 — recovery capture admission ports boundary:** accepted-with-limits. The existing Qt-free RecoveryCaptureAdmissionCoordinator now owns the recovery/busy gate, dirty-tab candidate filtering, inflight/delete-pending exclusion, snapshot identity, dirty-state normalization, and content-version capture through named ports; MainWindow retains channel/backpressure, editor capture, tracker/writer/QTimer lifecycle, abort/write, close policy, and remaining runtime/release gates.
- **UI-67 / ARCH-110 — shell surface depth and primary-work-area hierarchy:** accepted-with-limits. The centralized QSS now separates the main canvas, editor stage, command rail, status rail, document tab rail/items, workspace dock, and workspace panel through the existing surface ladder while preserving semantic interaction states, locale/font/motion settings, signals, and application policy. Native rendering, DPI/font metrics, accessibility, and remaining runtime/release gates remain open.
- **D133 / ARCH-111 — document-open admission boundary:** accepted-with-limits. The existing Qt-free DocumentOpenAdmissionCoordinator now owns busy/startup-restore admission, operation begin, session-restore binding, and asynchronous submission while FileDialogSurface, WorkspacePanel file/folder activation, DocumentOpenCoordinator result classification, line navigation, notifications, session restore, close policy, and remaining runtime/release gates remain unchanged/open.
- **D134 / ARCH-112 — document-save admission boundary:** accepted-with-limits. The existing Qt-free DocumentSaveAdmissionCoordinator now owns busy/startup-restore admission, duplicate-target rejection, state/text snapshot, read-only protection, operation begin, and asynchronous submission while save-as selection, FileDialogSurface, DocumentSaveCoordinator result classification, persistence, recovery/session, notifications, close policy, and remaining runtime/release gates remain unchanged/open.
- **D135 / ARCH-113 — workspace-navigation admission boundary:** accepted-with-limits. The existing Qt-free WorkspaceNavigationAdmissionCoordinator now owns workspace/surface availability and busy/startup-restore admission, loading projection, operation/generation binding, and open/directory submission while WorkspaceService, WorkspaceSurface, file/folder activation, containment, WorkspaceNavigationCoordinator result classification, projection, cancellation, session restore, notifications, close policy, and remaining runtime/release gates remain unchanged/open.
- **D136 / ARCH-114 — settings-save admission boundary:** accepted-with-limits. The existing Qt-free SettingsSaveAdmissionCoordinator now owns settings-service availability and in-flight admission, candidate editing, operation reservation, tracker binding, and asynchronous submission while SettingsSurface, settings persistence/validation, result classification, theme/locale/font/editor/motion application, notifications, close policy, and remaining runtime/release gates remain unchanged/open.
- **D137 / ARCH-115 — document-creation admission boundary:** accepted-with-limits. The existing Qt-free DocumentCreationAdmissionCoordinator now owns busy/startup-restore admission, the explicit initial-restore exception, document-service creation, tab projection handoff, DocumentOpened publication, and success-notification order while DocumentService, DocumentTabCreationCoordinator, editor/tab projection, EventBus, startup restore, notifications, close policy, and remaining runtime/release gates remain unchanged/open.
- **UI-68 / ARCH-116 — semantic state contrast closure:** accepted-with-limits. The centralized theme stylesheet now derives readable success/working/error foregrounds from success_bg, pressed, and error_bg across all supported themes/accents and reuses them for status, workspace/search, and FindBar feedback while warning/gold, accent endpoints, widget/signal/locale/motion behavior, and remaining runtime/release gates remain unchanged/open.
- **UI-69 / ARCH-117 — settings-dialog card hierarchy:** accepted-with-limits. The centralized theme stylesheet now gives the settings dialog a surface_0 canvas, distinct appearance/editor cards, a surface_3 preview card, and a scoped action rail while controls, form layout, signals, settings persistence, locale/font/motion behavior, and remaining runtime/release gates remain unchanged/open.
- **D138 / ARCH-118 — document-picker admission boundary:** accepted-with-limits. The existing Qt-free DocumentPickerAdmissionCoordinator now owns busy/startup-restore admission, native file selection, cancellation, and handoff to the existing asynchronous document-open boundary while FileDialogSurface, DocumentService, document-open classification, workspace directory selection, notifications, close policy, and remaining runtime/release gates remain unchanged/open.
- **D139 / ARCH-119 — workspace-picker admission boundary:** accepted-with-limits. The existing Qt-free WorkspacePickerAdmissionCoordinator now owns workspace availability, startup-restore/busy admission, native directory selection, cancellation, and handoff to the existing workspace navigation boundary while FileDialogSurface, document file selection/opening, WorkspaceService, file activation, containment, notifications, close policy, and remaining runtime/release gates remain unchanged/open.
- **UI-70 / ARCH-120 — modern shell elevation:** accepted-with-limits. The centralized theme stylesheet now separates command-bar hover, editor shell, document tab rail/tabs, status rail, workspace dock, and workspace empty-state surfaces through existing tokens while widget IDs, signals, layout, locale, fonts, motion, semantic states, theme/accent behavior, and remaining runtime/release gates remain unchanged/open.
- **D140 / ARCH-121 — workspace-search surface admission boundary:** accepted-with-limits. The existing Qt-free WorkspaceSearchSurfaceAdmissionCoordinator now owns startup-restore/service/root admission, search surface creation/reuse, root-change invalidation/reset, and presentation while query validation, operation tracking, cancellation, dispatch, result classification, containment, locale, notifications, close policy, and remaining runtime/release gates remain unchanged/open.
- **D141 / ARCH-122 — document-save picker admission boundary:** accepted-with-limits. The existing Qt-free generic DocumentSavePickerAdmissionCoordinator now owns active-tab/busy admission, ordinary Save versus Save As routing, native save-path selection, cancellation, and handoff to the existing save boundary while validation, conflict/read-only/persistence, result classification, recovery/session, notifications, close policy, and remaining runtime/release gates remain unchanged/open.
- **UI-71 / ARCH-123 — workspace-search query card:** accepted-with-limits. The workspace-search dialog now groups its existing query controls in a token-driven card with distinct field hover/focus and case-checkbox states while signals, keyboard behavior, results/diagnostics/status, locale, fonts, motion, theme/accent behavior, and remaining runtime/release gates remain unchanged/open.
- **D142 / ARCH-124 — presentation contract audit closure:** accepted-with-limits. The existing AST audit now enforces frozen/slotted coordinator Ports contracts and explicit `self._ports.notify(..., level=...)` calls while preserving coordinator behavior, MainWindow policy, Qt-free dependency checks, and TaskRunner pending-work observability. Native runtime and remaining release gates remain open.
- **D143 / ARCH-125 — presentation locale coordinator:** accepted-with-limits. The established locale refresh order now lives behind a Qt-free frozen/slotted callback contract while MainWindow retains translation, concrete surface, optional-surface, settings, and policy ownership. Native rendering, event timing, and remaining release gates remain open.
- **D144 / ARCH-126 — close-guard feedback coordinator:** accepted-with-limits. Existing close-block reason/message projection now lives behind a Qt-free frozen/slotted callback contract while MainWindow retains close classification, QCloseEvent, MessageSurface, error projection, and policy ownership. Native dialog/event timing and remaining release gates remain open.
- **UI-72 / ARCH-127 — status notification pill:** accepted-with-limits. The centralized `statusMessage` QSS now gives transient notifications a rounded token-driven surface and semantic left rail across 3 themes × 4 accents while preserving notification behavior, levels, locale, motion, and application policy. Native rendering and remaining release gates remain open.
- **UI-73 / ARCH-128 — document-stage edge:** accepted-with-limits. The centralized `QTabWidget#documentTabs::pane` rule no longer paints a redundant border, radius, or top overlap; the authored tab rail, editor canvas edge, focus states, and application policy remain unchanged. Native QSS specificity/rendering, runtime visual review, and remaining release gates remain open.
- **D145 / ARCH-129 — status-phase policy:** accepted-with-limits. A Qt-free coordinator now preserves the existing `working` > `attention` > `ready` priority from busy/pending/dirty facts while MainWindow retains concrete queries and StatusSurface retains Qt rendering. Native event timing, runtime visual review, and remaining release gates remain open.
- **UI-74 / ARCH-130 — status-rail context divider:** accepted-with-limits. The existing `statusContext` label now has a token-driven right divider and compact spacing, making it scan separately from the phase pill while preserving all four phase states and application policy. Native QSS rendering and remaining release gates remain open.
- **D146 / ARCH-131 — editor-change projection:** accepted-with-limits. The existing editor modified/content callback ordering now lives behind a Qt-free typed coordinator while MainWindow retains DocumentService, concrete tab/editor/surface/session ownership, Qt signals, and application policy. Native signal timing, runtime visual review, and remaining release gates remain open.
- **D147 / ARCH-132 — current-document transition projection:** accepted-with-limits. The existing current-tab transition ordering now lives behind a Qt-free typed coordinator while MainWindow retains tab/Find/status/notification/session ownership and application policy. Native signal timing, runtime visual review, and remaining release gates remain open.
- **UI-75 / ARCH-133 — active-document tab emphasis:** accepted-with-limits. The specific selected document-tab selectors now restore the token-driven left accent edge without changing tab geometry or behavior. Native QSS rendering, runtime visual review, and remaining release gates remain open.
- **D148 / ARCH-134 — editor-action admission:** accepted-with-limits. The existing active-tab/busy guard, editor-action invocation, and focus restoration now live behind a Qt-free typed coordinator while MainWindow retains tab/editor, busy, Qt focus, command composition, and document policy ownership. Native focus/event timing, runtime visual review, and remaining release gates remain open.
- **D149 / ARCH-135 — settings-save projection Ports contract:** accepted-with-limits. The existing valid-settings projection order now uses a frozen/slotted named Qt-free contract while MainWindow retains settings, editor, theme, animation, notification, and persistence ownership. Native event/rendering timing, runtime visual review, and remaining release gates remain open.
- **D150 / ARCH-136 — recovery projection Ports contract:** accepted-with-limits. Recovery-writer saved/failed outcome branches now use a frozen/slotted named Qt-free contract while MainWindow retains recovery state, tab identity, deletion/clear, notifications, writer lifecycle, and policy ownership. Native recovery timing/durability, runtime visual review, and remaining release gates remain open.
- **UI-76 / ARCH-137 — inactive-selection contrast closure:** accepted-with-limits. Inactive selected workspace/list rows now use the readable primary text token on the existing pressed surface across all supported theme/accent combinations; selection semantics, geometry, and policy remain unchanged. Native QSS rendering, runtime visual review, and remaining release gates remain open.
- **D151 / ARCH-138 — session-load Ports contract:** accepted-with-limits. Session-load result projection now uses a frozen/slotted named Qt-free contract while preserving default-baseline, invalid-feedback, baseline ordering, and recovery-first scheduling; MainWindow retains session service, startup, restore, notification, and policy ownership. Native startup timing, filesystem behavior, runtime visual review, and remaining release gates remain open.
 - **D152 / ARCH-139 — session-save Ports contract:** accepted-with-limits. Latest-wins session-save admission now uses a frozen/slotted named Qt-free contract while preserving tracker state, stale suppression, invalid/failure feedback, operation identity, and drain ordering; MainWindow retains timer, SessionService, snapshot, TaskRunner, notification, startup, and close ownership. Native timer/worker timing, filesystem durability, runtime visual review, and remaining release gates remain open.
 - **D153 / ARCH-140 — Replace All completion Ports contract:** accepted-with-limits. Replace All stale-guarded completion cleanup now uses a frozen/slotted generic named Qt-free contract while preserving tracker finish, tab unlock, tab-bar enablement, operation completion, and outcome projection order; MainWindow retains editor, FindSurface, tab, rollback, cancellation, and policy ownership. Native event/rollback timing, runtime visual review, and remaining release gates remain open.
 - **D154 / ARCH-141 — settings-save Ports contract:** accepted-with-limits. Settings-save result projection now uses a frozen/slotted named Qt-free contract while preserving stale suppression, invalid feedback, valid snapshot application, and matching failure projection; MainWindow retains settings persistence, validation, theme/locale/font/editor refresh, transition, notification, and close ownership. Native worker/timer timing, settings filesystem durability, runtime visual review, and remaining release gates remain open.
 - **D155 / ARCH-142 — workspace-search Ports contract:** accepted-with-limits. Workspace-search result projection now uses a frozen/slotted named Qt-free contract while preserving stale suppression, invalidation cancellation, invalid-result feedback, valid-result summary severity, and matching failure projection; MainWindow retains search service, query/generation cancellation, surface, containment, notification, and policy ownership. Native worker/event timing, filesystem traversal, runtime visual review, and remaining release gates remain open.
 - **D156 / ARCH-143 — plugin-runtime Ports contract:** accepted-with-limits. Plugin runtime status/control projection now uses a frozen/slotted named Qt-free contract while preserving failure notification/command refresh, unavailable/busy guards, runtime exception handling, and success refresh/notification/status order; MainWindow retains runtime and plugin trust/permission/enablement policy ownership. Native plugin lifecycle timing, runtime visual review, and remaining release gates remain open.
 - **D157 / ARCH-144 — recovery-delete Ports contract:** accepted-with-limits. Recovery-delete result projection now uses a frozen/slotted generic named Qt-free contract while preserving tracker release, owner cleanup, optional success notification, pending-delete scheduling, and failure notification order; MainWindow retains recovery persistence, filesystem, capture/write, tab, worker, and close ownership. Native delete timing, filesystem durability, runtime visual review, and remaining release gates remain open.
 - **D158 / ARCH-145 — recovery-scan Ports contract:** accepted-with-limits. Recovery inventory result projection now uses a frozen/slotted named Qt-free contract while preserving stale guard, invalid-inventory failure, empty non-startup info, candidate prompt order, startup continuation, and failure continuation; MainWindow retains RecoveryService, scan worker, restore state, filesystem, notification, and close ownership. Native scan/startup timing, runtime visual review, and remaining release gates remain open.
 - **D159 / ARCH-146 — recovery-write Ports contract:** accepted-with-limits. Recovery-write callback projection now uses a frozen/slotted generic named Qt-free contract while preserving discarded classification, capture abort, document release, pending-delete drain, and saved/failed projection order; MainWindow retains recovery persistence, capture/worker concurrency, filesystem, tab, notification, and close ownership. Native callback timing, durability, runtime visual review, and remaining release gates remain open.
 - **D160 / ARCH-147 — workspace-navigation projection Ports contract:** accepted-with-limits. Workspace open/directory result projection now uses a frozen/slotted named Qt-free contract while preserving search invalidation, workspace activation, root/surface guards, directory projection, opened notification, session save, and restore continuation order; MainWindow retains WorkspaceService, containment, file activation, TaskRunner, surface, session, startup, and close ownership. Native event/file-activation timing, runtime visual review, and remaining release gates remain open.
 - **D161 / ARCH-148 — plugin-host probe Ports contract:** accepted-with-limits. The diagnostic coordinator now uses a frozen/slotted named Qt-free dependency contract while preserving unavailable/busy guards, start/dispatch order, stale suppression, typed-result severity mapping, and failure projection; MainWindow retains host, tracker, TaskRunner, notification, containment, and external-execution policy ownership. Process timing, containment, runtime visual review, and remaining release gates remain open.
 - **D162 / ARCH-149 — plugin-catalog Ports contract:** accepted-with-limits. Catalog scan and descriptor governance now use a frozen/slotted named Qt-free dependency contract while preserving metadata-only validation, mutual exclusion, result severity, action enablement, stale suppression, governance success/rescan order, and security-policy ownership. Metadata filesystem timing, plugin security evidence, runtime visual review, and remaining release gates remain open.
 - **D163 / ARCH-150 — recovery-capture abort Ports contract:** accepted-with-limits. Recovery capture cancellation/failure now uses a frozen/slotted generic named Qt-free contract while preserving identity guard, discarded/released snapshot classification, channel/session cancellation, document completion, live-owner notification, and recovery policy ownership. Sol final adversarial source/order review passed; Terra windows had no conclusion, and native callback interleavings, durability, runtime visual review, and remaining release gates remain open.
 - **D164 / UI-77 — status-message visual weight:** accepted-with-limits. The centralized status-message base rule now uses semibold weight to strengthen transient feedback hierarchy while preserving localized text, info/success/warning/error state selectors, timer/visibility, layout, and notification policy. Native font metrics, screenshot/runtime visual review, and remaining release gates remain open.
 - **D165 / ARCH-152 — support handoff packet check:** accepted-with-limits. The local support packet is now required by static/release handoff checks and recorded in dossier evidence without changing support status, no-go semantics, or any external gate. Owner acceptance, clean-machine, and remaining release gates remain open.
 - **D166 / UI-78 / ARCH-153 — font-choice preview:** accepted-with-limits. Existing Settings font selectors now preview each allowlisted family through Qt `FontRole` data while preserving `currentText()` values, settings persistence, font fallback, locale, size, theme, motion, and application ownership. Native popup rendering, installed-font metrics, and remaining release gates remain open.
 - **D167 / UI-79 / ARCH-154 — Settings typography preview:** accepted-with-limits. Settings now previews both interface and editor font/size choices through `QFont`; preview QSS retains only named visual tokens, while persistence, application, locale, theme, motion, and release boundaries remain unchanged. Native rendering, fallback metrics, accessibility, and remaining release gates remain open.
 - **D168 / UI-80 / ARCH-155 — workspace entry visual semantics:** accepted-with-limits. Workspace file, folder, and unavailable entries now retain distinct authored icon semantics, use localized scan guidance, and use a light-canvas text fallback for file-icon contrast while item data, activation signals, open behavior, and application policy remain unchanged. Native icon/tooltip/accessibility rendering and remaining release gates remain open.
 - **D169 / UI-81 / ARCH-156 — FindBar authored action icons:** accepted-with-limits. Find/Replace navigation, replace, cancel, and close actions now use the authored icon provider with palette-aware detail contrast while labels, signals, shortcuts, operation state, and application policy remain unchanged. Native icon metrics/accessibility and remaining release gates remain open.
 - **D170 / UI-82 / ARCH-157 — command-rail visual hierarchy:** accepted-with-limits. The centralized command rail now uses a lighter card surface, restrained brand edge, more breathable spacing, consistent button hit height/radius, quieter quiet actions, and a compact context anchor while command construction, roles, shortcuts, callbacks, and palette-derived contrast remain unchanged. Native toolbar metrics/accessibility and remaining release gates remain open.
 - **D171 / UI-83 / ARCH-158 — document-tab visual hierarchy:** accepted-with-limits. The document rail now uses one restrained container, flat inactive tabs, clear hover feedback, a raised selected tab, and explicit focus/disabled/close states while tab creation, titles, modified markers, signals, and document policy remain unchanged. Native tab metrics/accessibility and remaining release gates remain open.
 - **D172 / UI-84 / ARCH-159 — editor-stage visual depth:** accepted-with-limits. The central editor now uses a softer stage surface and a single primary canvas hierarchy with preserved focus/selection token projection while editor fonts, lexer, syntax, caret, line numbers, wrapping, signals, and document policy remain unchanged. Native QScintilla metrics/accessibility and remaining release gates remain open.
 - **D173 / UI-85 / ARCH-160 — status-rail visual hierarchy:** accepted-with-limits. The bottom status area now uses a calmer shell surface, distinct message/status capsules, and more breathable phase pills while message severity, phase semantics, locale, timers, accessible names, and lifecycle policy remain unchanged. Native status-bar metrics/accessibility and remaining release gates remain open.
 - **D174 / UI-86 / ARCH-161 — Workspace-dock visual hierarchy:** accepted-with-limits. The left Workspace dock now uses a lighter frame, thinner title emphasis, more breathable title spacing, and 20px close/float controls aligned with the document rail while docking, file/folder activation, search, locale, selection, signals, and policy remain unchanged. Native docking metrics/accessibility and remaining release gates remain open.
 - **D175 / UI-87 / ARCH-162 — Command Palette visual hierarchy:** accepted-with-limits. The Command Palette result list now exposes a focus boundary, readable hover/selected-hover states, and a compact keyboard-hint capsule while filtering, ordering, stable IDs, activation, return-key acceptance, locale, and modal policy remain unchanged. Native list metrics/accessibility and remaining release gates remain open.
 - **D176 / UI-88 / ARCH-163 — FindBar visual rhythm:** accepted-with-limits. The editor Find/Replace bar now uses a calmer utility-card radius/spacing, secondary semibold labels, a visible case-option hover/focus surface, and a more breathable status capsule while search/replacement signals, keyboard behavior, action roles, locale, and operation state remain unchanged. Native FindBar metrics/accessibility and remaining release gates remain open.
 - **D177 / UI-89 / ARCH-164 — Settings guidance hierarchy:** accepted-with-limits. Settings now renders the apply-after-save and font-fallback notes as compact supporting capsules with distinct accent edges while locale, preview, SettingsSnapshot, Save/Cancel, font fallback, persistence, and motion policy remain unchanged. Native dialog metrics/accessibility and remaining release gates remain open.
 - **D178 / UI-90 / ARCH-165 — message-dialog action hierarchy:** accepted-with-limits. Common message boxes now expose Save as primary, Discard as warning, Cancel as quiet, and About/Error OK as quiet through existing theme roles while message decisions, localization, close/recovery policy, and application ownership remain unchanged. Native message-box metrics/accessibility and remaining release gates remain open.
 - **D179 / UI-91 / ARCH-166 — plugin-catalog guidance capsule:** accepted-with-limits. The existing plugin catalog governance hint now uses a token-driven supporting capsule with a scoped surface, border, pink accent edge, secondary text, radius, and spacing while catalog content, locale, selection, approve/revoke signals, trust/approval policy, and application ownership remain unchanged. Native dialog metrics/accessibility and remaining release gates remain open.
- **D180 / UI-92 / ARCH-167 — font-style settings contract:** accepted-with-limits. Settings now exposes localized regular, semibold, bold, and italic choices for both interface and editor fonts, previews those choices, persists them through schema v3 with v1/v2 regular fallbacks, applies interface QSS and live editor styles after save, and preserves settings/application ownership. Native font fallback, metrics, accessibility, DPI, runtime visual review, and remaining release gates remain open.
- **D181 / UI-93 / ARCH-168 — status accessibility localization:** accepted-with-limits. StatusRail and StatusSurface now localize shell status, workspace context, phase, notification names, and dynamic phase descriptions through the existing locale refresh path while notification payloads, severity, timers, lifecycle policy, theme, and application ownership remain unchanged. Native screen-reader behavior, accessibility-tree output, DPI, runtime visual review, and remaining release gates remain open.
- **D182 / UI-94 / ARCH-169 — theme token boundary:** accepted-with-limits. Immutable theme/editor tokens, bounded theme/accent resolution, and contrast helpers now live in Qt-free `presentation.theme_tokens`; `presentation.theme` remains the sole Qt palette/QSS/editor projection owner and preserves its public/private compatibility façade. Runtime Qt painting, DPI, fonts, and remaining release gates remain open.
- **D183 / UI-95 / ARCH-170 — surface-gradient hierarchy:** accepted-with-limits. The main window, editor shell, and command rail now use three restrained ThemeColors-driven gradients to add depth while the editor canvas, controls, semantic states, and behavior remain unchanged. Native QSS painting, DPI, screenshots, and remaining release gates remain open.
- **D184 / ARCH-171 — local distribution scripts:** accepted-with-limits. The portable source now includes hash-gated, user-local install/update/uninstall entry points with retained rollback state and explicit opt-in HKCU associations; scripts are statically parsed but not executed, so installer/update/association and release-owner gates remain open.
- **D185 / UI-96 / ARCH-172 — form-control highlight closure:** accepted-with-limits. Existing form controls now expose a stronger focus surface and ComboBox popup hover/selected highlight rail through centralized ThemeColors QSS; values, signals, layout, locale, fonts, motion, and settings ownership remain unchanged. Native popup rendering and remaining release gates remain open.
- **D186 / UI-97 / ARCH-173 — workspace file-entry closure:** accepted-with-limits. Workspace now exposes an explicit localized Open file action that reuses the existing file picker and asynchronous document-open boundary; folder navigation, tree file activation, loading policy, and application ownership remain unchanged. Native dialog/runtime and remaining release gates remain open.
- **D187 / UI-98 / ARCH-174 — responsive Settings scroll boundary:** accepted-with-limits. Existing Settings content now sits in one resizable vertical viewport while Save/Cancel remain outside and visible, improving short-height and larger-font usability without changing settings values, locale, persistence, theme application, motion, or ownership. Native scroll metrics, runtime, and remaining release gates remain open.
- **D189 / UI-99 / ARCH-175 — workspace-search empty-state boundary:** accepted-with-limits. The existing workspace-search result viewport now explicitly projects localized initial/loading/no-match/cancelled/error states through one stacked presentation stage, while populated result rows, path/line roles, double-click activation, diagnostics, cancellation, and search ownership remain unchanged. Native stack/list rendering, accessibility, DPI, runtime, and remaining release gates remain open.
- **D190 / UI-100 / ARCH-176 — Command Palette empty-state boundary:** accepted-with-limits. The existing Command Palette result viewport now explicitly projects localized empty-registry and filtered-no-match states through one stacked presentation stage, while populated command rows, stable IDs, row selection, Enter/item activation, Esc close, and command execution ownership remain unchanged. Native stack/list rendering, accessibility, DPI, runtime, and remaining release gates remain open.
- **D191 / UI-101 / ARCH-177 — dynamic Find/Replace status localization closure:** accepted-with-limits. Count-bearing plural and Replace All limit feedback now use strict presentation-layer matchers and the existing English/Simplified Chinese catalog, removing mixed-language tails without changing editor operations, cancellation, rollback, signals, or status severity. Native text metrics, accessibility, DPI, runtime, and remaining release gates remain open.
- **D192 / UI-102 / ARCH-178 — plugin failure phase localization closure:** accepted-with-limits. Stable plugin failure phases now render in Chinese through the existing presentation adapter while plugin IDs, error details, runtime lifecycle, trust, containment, command refresh, and notification severity remain unchanged. Native text metrics, accessibility, DPI, runtime, and remaining release gates remain open.
- **D193 / UI-103 / ARCH-179 — close-guard pending feedback localization:** accepted-with-limits. The count-bearing close-block message for retained background work now uses the existing English/Simplified Chinese catalog without changing close readiness, TaskRunner, session save, worker draining, or shutdown policy. Native text metrics, accessibility, DPI, runtime, and remaining release gates remain open.
- **D194 / UI-104 / ARCH-180 — native control affordance cohesion:** accepted-with-limits. ComboBox and SpinBox arrows now use explicit token-driven geometry and disabled/open states, while checked checkbox focus retains an accent boundary; widget behavior, settings values, signals, locale, motion, and application ownership remain unchanged. Native QSS parsing/painting, accessibility, DPI, runtime, and remaining release gates remain open.
- **D195 / ARCH-181 — Windows PowerShell packaging compatibility:** accepted-with-limits. `scripts/package.ps1` now writes the same source/hash/manifest contract under Windows PowerShell 5.1 and PowerShell 7 by using .NET Framework-compatible path and hashing APIs; application behavior, installer/update policy, and external release gates remain unchanged.
- **D196 / ARCH-182 — release verifier PowerShell compatibility:** accepted-with-limits. `scripts/verify_release_handoff.ps1` now decodes UTF-8 reports and preserves its existing gate predicates under Windows PowerShell 5.1 and PowerShell 7; both shells produce the same expected no-go result, with stale runtime reports and external release gates still open.
- **D197 / ARCH-183 — packaged measurement PowerShell compatibility:** accepted-with-limits. `scripts/measure_packaged.ps1` now has the same compatible JSON/UTF-8 read boundary while artifact binding, Qt offscreen process policy, capture validation, cleanup, and runtime evidence limits remain unchanged; no capture was run.
- **D198 / UI-105 / ARCH-184 — menu affordance cohesion:** accepted-with-limits. Centralized menu QSS now exposes explicit normal, hover, checked, checked-hover, disabled, and selected-and-checked indicator states across all 12 theme/accent projections without changing QAction or command behavior; native menu rendering, accessibility, DPI, runtime visual review, and remaining release gates remain open.
- **D199 / ARCH-185 — command menu contract closure:** accepted-with-limits. The application registry now rejects unsupported menu IDs and the presentation projection reuses the same four-value ordered contract, preventing silently invisible commands without adding dynamic menus or changing command behavior; runtime plugin integration, native rendering, and release gates remain open.
- **D200 / ARCH-186 — package source-revision determinism:** accepted-with-limits. `scripts/package.ps1` now sorts canonical source lines with ordinal comparison so Windows PowerShell 5.1 and PowerShell 7 produce the same source revision; separate artifact bytes remain individually bound and runtime/external release gates remain open.
- **D201 / UI-106 / ARCH-187 — disabled menu state hierarchy:** accepted-with-limits. Selected-disabled and checked-disabled menu items now use a readable subdued surface and authored left boundary instead of inheriting an accent background; QAction behavior, locale, native rendering evidence, and release gates remain unchanged/open.
- **D202 / ARCH-188 — workspace provider directory capability:** accepted-with-limits. Workspace directory classification now crosses the typed `WorkspaceProvider.is_directory` capability, keeping concrete filesystem predicates in `FileWorkspaceProvider` while preserving path normalization, containment, limits, error text, UI behavior, and runtime/release limits.
- **D203 / ARCH-189 — shared directory capability for workspace search:** accepted-with-limits. Workspace navigation and Find in Files reuse one `DirectoryCapability` contract, keeping concrete classification in adapters while preserving search limits, cancellation, containment, errors, composition, UI behavior, and runtime/release limits.
- **D204 / UI-107 / ARCH-190 — workspace-search diagnostics disabled-state hierarchy:** accepted-with-limits. Find in Files now projects a subdued token-driven disabled state for the diagnostics disclosure control, including checked-and-disabled ordering, while search busy/enablement behavior, warning presentation, locale, cancellation, signals, and runtime/release limits remain unchanged.
- **D205 / UI-108 / ARCH-191 — Find/Replace navigation disabled-state hierarchy:** accepted-with-limits. Previous/next navigation now project a subdued token-driven disabled state after their higher-specificity normal/hover/focus rules, while active-operation enablement, query/cancellation behavior, locale, icons, signals, and runtime/release limits remain unchanged.
- **D207 / UI-109 / ARCH-192 — Workspace tree disabled-state hierarchy:** accepted-with-limits. The visible populated Workspace tree now projects a subdued token-driven disabled container during provider refresh, while tree data, row selection, activation, navigation, cancellation, locale, signals, and runtime/release limits remain unchanged.
- **D208 / UI-110 / ARCH-193 — editor locked disabled-state hierarchy:** accepted-with-limits. The locked QScintilla editor now projects a subdued canvas, neutral boundary, and pressed selection surface without muting text/lexer colors, while Replace All locking, editor operations, selection, caret, locale, signals, and runtime/release limits remain unchanged.
- **D209 / UI-111 / ARCH-194 — plugin disabled action hierarchy:** accepted-with-limits. Plugin Catalog approve/revoke and Plugin Status enable/disable actions now project a scoped subdued disabled surface, neutral left boundary, and readable muted text while plugin enablement predicates, signals, trust/approval/lifecycle policy, locale, and runtime/release limits remain unchanged.
- **D210 / UI-112 / ARCH-195 — default document filter coverage:** accepted-with-limits. The existing localized native open/save filter now lists 18 common text/source extensions and retains `All files (*.*)` while file/folder selection, async document flow, encoding, persistence, and runtime/release limits remain unchanged.
- **D211 / UI-113 / ARCH-196 — workspace disabled-item hierarchy:** accepted-with-limits. Inaccessible workspace entries and the disabled truncation marker now use a subdued token-driven row surface, neutral left boundary, and readable muted text while selected-disabled specificity, loading, file/folder activation, locale, icons, and runtime/release limits remain unchanged.
- **D212 / UI-114 / ARCH-197 — localized untitled tab title:** accepted-with-limits. Pathless document tabs now use the active English or Simplified Chinese catalog label and reproject existing titles on locale changes while saved-file basenames, dirty markers, document state, and runtime/release limits remain unchanged.
- **D213 / UI-115 / ARCH-198 — editor-shell brand edge:** accepted-with-limits. The central editor stage now exposes one token-driven top brand edge aligned with the command rail and workspace hierarchy while child widgets, editor behavior, locale, motion, and runtime/release limits remain unchanged.
- **D214 / ARCH-199 — application error taxonomy:** accepted-with-limits. Document and workspace use cases now classify application-owned validation/state failures through a Qt-free taxonomy while preserving ValueError/RuntimeError compatibility, exact messages, domain conflicts, provider behavior, and runtime/release limits.
- **D215 / UI-116 / ARCH-200 — semantic accent endpoint foregrounds:** accepted-with-limits. The existing primary-action hover state now resolves its foreground from a dedicated `on_accent_pink` token across all 12 theme/accent combinations while action behavior, settings, locale, motion, and release/runtime limits remain unchanged.
- **D216 / ARCH-201 — editor policy error taxonomy:** accepted-with-limits. The eight application-owned `EditorOperationPolicy` limit invariants now use the shared Qt-free validation category while preserving exact messages, defaults, `ValueError` compatibility, and runtime/release limits.
- **D217 / UI-117 — toolbar brand anchor:** accepted-with-limits. The existing top command rail now begins with one non-interactive localized `✦ QuillForge` identity chip styled through centralized QSS while QAction order, callbacks, shortcuts, context, locale ownership, and runtime/release limits remain unchanged.
- **D218 / ARCH-202 — workspace-search application error taxonomy:** accepted-with-limits. The Qt-free workspace-search boundary now classifies 39 application/provider validation and type failures through built-in-compatible categories while preserving exact messages, validation order, search policy, provider contracts, and runtime/release limits.
- **D219 / ARCH-203 — remaining application error taxonomy:** accepted-with-limits. The remaining Qt-free command, plugin, session, recovery, release, and event boundaries now reuse stable validation/state categories while preserving built-in compatibility, exact messages, protocol/dataclass shapes, layer ownership, and runtime/release limits.
- **D220 / UI-118 — main-shell low-noise visual hierarchy:** accepted-with-limits. The main window, editor stage, command rail, and document-tab rail now use a calmer flat token ladder with fewer gradients and nested rounded surfaces while preserving all interaction states, theme/accent settings, locale, motion, and runtime/release limits.
- **D221 / ARCH-204 — i18n literal-key static gate:** accepted-with-limits. The existing presentation AST audit now compares literal `tr()` keys with the canonical English catalog while preserving dynamic keys, runtime fallback, and translation content. Native/runtime and remaining release gates remain open.
 - **D222 / ARCH-205 — windowed startup-failure boundary:** accepted-with-limits. The console-disabled portable entry point now records an actionable UTF-8 startup traceback in user-local app data and attempts a native Windows error message when initialization fails before Qt can show, without moving application behavior across layers. GUI/EXE runtime, clean-machine, and remaining release gates remain open.
 - **D223 / ARCH-206 — no-window packaged startup diagnostic:** accepted-with-limits. The existing entry dispatcher now exposes `--diagnose-startup --report <path>` before `QApplication` construction, reporting runtime, Qt/QScintilla, composition, and frozen-resource checks with deterministic JSON/exit codes. Frozen execution, GUI/runtime, clean-machine, and remaining release gates remain open.
 - **D225 / ARCH-207 — command-surface locale accessor startup fix:** accepted-with-limits. The captured startup `AttributeError` from the missing `CommandSurface._locale` accessor is fixed by delegating to the existing locale provider; command/menu behavior remains unchanged. Native EXE startup, clean-machine, and remaining release gates remain open.
 - **D226 / ARCH-208 — private self-call contract audit:** accepted-with-limits. The existing presentation AST audit now catches direct private self calls that have neither a class method nor an assigned provider attribute, preventing another D225-shaped missing accessor before shell startup. Dynamic callability, native runtime, clean-machine, and remaining release gates remain open.
 - **D227 / ARCH-209 — private attribute declaration-aware audit:** accepted-with-limits. The D226 presentation AST guard now recognizes direct class-level assignments and annotated fields as declared private attributes while preserving its narrow direct-call scope; runtime callability, native startup, clean-machine, and remaining release gates remain open.
 - **D228 / ARCH-210 — startup failure context record:** accepted-with-limits. The existing early startup log now records fixed executable, working-directory, frozen/runtime, and bundle context through a fail-open entry-boundary helper; native startup, privacy review beyond the documented path tradeoff, clean-machine, and remaining release gates remain open.
 - **D229 / ARCH-211 — startup diagnostic path fail-open:** accepted-with-limits. The existing early failure reporter now protects path resolution, context fallback, payload construction, and log writes so diagnostic failures cannot mask the original startup exception; native startup, clean-machine, and remaining release gates remain open.
 - **D230 / ARCH-212 — startup fallback display fail-open:** accepted-with-limits. The native fallback now treats exception stringification, MessageBox display, and stderr output as best-effort while preserving ordinary failure text and exit behavior; native startup, clean-machine, and remaining release gates remain open.
 - **D231 / ARCH-213 — MessageBox zero-return fallback:** accepted-with-limits. The startup fallback now treats only nonzero `MessageBoxW` results as successful and continues stderr when Win32 returns zero; native rendering, clean-machine, and remaining release gates remain open.
  - **D255 / ARCH-233 — application-error localization boundary:** accepted-with-limits. Common user-visible application and coordinator-wrapper errors are localized through the existing presentation i18n boundary while paths, names, IDs, unknown details, application policy, and en-US behavior remain unchanged; native startup, native dialogs, clean-machine, and remaining release gates remain open.
  - **D253 / ARCH-231 — workspace-entry error locale refresh:** accepted-with-limits. Known inaccessible-entry provider reasons are localized in disabled-row tooltips and refreshed on locale changes while unknown text, file/folder activation, row state, and provider policy remain unchanged; native startup, native dialogs, clean-machine, and remaining release gates remain open.
  - **D252 / ARCH-230 — plugin diagnostic locale refresh:** accepted-with-limits. Known plugin runtime-status and catalog-entry reasons/prefixes are localized through the existing i18n boundary, unknown provider text remains visible, and trust/approval/enablement/runtime policy remains unchanged; native startup, native dialogs, clean-machine, and remaining release gates remain open.
  - **D251 / ARCH-229 — workspace-search diagnostic locale refresh:** accepted-with-limits. Known provider diagnostic reasons/prefixes are localized through the existing i18n boundary, with relative paths/detail suffixes preserved and existing rows reprojected on locale changes without changing search policy or expansion state; native startup, native dialogs, clean-machine, and remaining release gates remain open.
  - **D250 / ARCH-228 — workspace search status locale refresh:** accepted-with-limits. WorkspaceSearchDialog now retains source data for catalog/error/summary statuses and rebuilds the current text after locale changes while search execution, results, cancellation, and diagnostics remain unchanged; native startup, native dialogs, clean-machine, and remaining release gates remain open.
  - **D249 / ARCH-227 — workspace error locale refresh:** accepted-with-limits. WorkspacePanel now retains the current typed/string error source and reprojects it after locale changes while successful directory results and new loads clear obsolete errors; navigation, service, session, notification, and last-good-page policy remain unchanged; native startup, native dialogs, clean-machine, and remaining release gates remain open.
  - **D248 / ARCH-226 — workspace typed-error projection:** accepted-with-limits. Workspace navigation now preserves typed failures through the existing panel surface so Chinese permission/path/codec feedback remains actionable while invalid-result strings, English behavior, navigation policy, and session restoration remain unchanged; native startup, native dialogs, clean-machine, and remaining release gates remain open.
  - **D247 / ARCH-225 — typed secondary error localization:** accepted-with-limits. Settings-save and Replace All handlers now preserve typed exceptions through the shared presentation error surface so Chinese filesystem/codec messages retain actionable metadata while English behavior and application policy remain unchanged; native startup, native dialogs, clean-machine, and remaining release gates remain open.
  - **D246 / ARCH-224 — startup path stat routing:** accepted-with-limits. Explicit desktop launch paths now use one stat-based directory/regular-file classifier so missing/invalid paths retain concrete errors while existing async admission and special-file rejection remain unchanged; native startup, shell association, clean-machine, and remaining release gates remain open.
  - **D245 / UI-27 — typed file-error localization:** accepted-with-limits. The existing document open/save error surface now projects typed filesystem and codec failures through one presentation-only localizer; English remains unchanged and Chinese retains path/encoding/position diagnostics, while native file I/O, dialogs, startup, and remaining release gates remain open.
  - **D244 / UI-26 — startup-path error localization:** accepted-with-limits. Explicit desktop-file launch failures now localize their two missing path-error prefixes in the existing message mapper while preserving English output, path details, startup parsing, document admission, native startup, and remaining release gates.
  - **D243 / UI-25 — readable accent text endpoints:** accepted-with-limits. Surface text that previously used raw `accent_alt` now routes through one conservative readable foreground resolver for links, the workspace eyebrow, and generic checkbox hover states; filled accent endpoint pairs remain unchanged, while native rendering, startup, clean-machine, and remaining release gates remain open.
  - **D242 / ARCH-223 — file dialog all-files default:** accepted-with-limits. The existing localized open/save filter now defaults to `All files (*)` while retaining the text/source filter, so extensionless and uncommon source/configuration files are visible without adding a second document-open path; native dialog/startup, clean-machine, and remaining release gates remain open.
  - **D241 / ARCH-222 — startup provenance diagnostic:** accepted-with-limits. The no-window startup diagnostic now records application import provenance and frozen Qt plugin/platform presence while leaving normal GUI startup unchanged; native startup, clean-machine, and remaining release gates remain open.
  - **D240 / ARCH-221 — frozen entrypoint path hardening:** accepted-with-limits. Direct source execution retains its diagnostic import path while frozen PyInstaller execution skips source-style `__file__` path surgery; native importer/startup, clean-machine, and remaining release gates remain open.
  - **D239 / ARCH-220 — target identity preflight:** accepted-with-limits. Update and rollback now validate current target/backup SHA-256 identity after confirmation and before any file move, refusing externally changed state; updater execution, race/permission, clean-machine, and remaining release gates remain open.
  - **D238 / ARCH-219 — update transaction safety:** accepted-with-limits. Local update and explicit rollback now track file exchange and state commit boundaries, restore prior target/backup layout under exact hash/path guards, and preserve uncertain external changes; updater execution, permissions, clean-machine, and remaining release gates remain open.
  - **D237 / ARCH-217 — multi-association safety:** accepted-with-limits. The opt-in local distribution boundary now handles multiple extensions through one operation-owned ProgId, scans for unknown references before shared-key deletion, and preserves the executable when cleanup is incomplete; native installer/registry execution, shell associations, clean-machine, and remaining release gates remain open.
  - **D235 / ARCH-216 — startup-path edge routing:** accepted-with-limits. Qt value options with one or two leading dashes remain with Qt, and non-temporary startup-path admission failures now warn instead of silently dropping requests; native startup, shell drag-and-drop, file associations, clean-machine, and remaining release gates remain open.
  - **D233 / ARCH-215 — desktop file-launch routing:** accepted-with-limits. Explicit file and directory arguments are now separated from Qt arguments and queued through the existing asynchronous document/workspace boundaries after recovery and session restoration; native EXE/shell drag-and-drop, file associations, clean-machine, and remaining release gates remain open.
  - **D232 / ARCH-214 — guarded entry-point application import:** accepted-with-limits. The public `main(argv)` wrapper now resolves `app.main` inside the existing startup exception boundary, so early application import failures reach diagnostics; native startup, clean-machine, and remaining release gates remain open.

## Current D6 slice

- **D6.1 — read-only extension catalog:** accepted with limits. Tools → Extension Catalog scans only the explicit user-local top-level JSON directory, validates a strict versioned manifest, classifies incompatible and duplicate IDs, and keeps every external entry untrusted and unloaded. Signatures, persisted trust, dynamic loading, sandboxing, installation, and updates remain separate security/release work.
- **D6.2 — unified plugin manifest policy:** accepted with limits. In-process plugin registration and external catalog classification now share one public identity/API/permission validator, including the common `plugin_id` length bound, before the next trust or isolation boundary is introduced.
- **D6.3 — descriptor approval ledger:** accepted with limits. Persist explicit digest-bound approvals and revocations as metadata governance while keeping external entries untrusted and unloaded.
- **D6.4 — plugin runtime control plane:** accepted with limits. Expose immutable lifecycle diagnostics and explicit enable/disable for registered in-process plugins; enablement cannot promote an untrusted registration, and external catalog entries remain non-executable.
- **D6.5 — persistent plugin enablement policy:** accepted with limits. Persist bounded local choices for explicitly registered plugins, restore them on startup, and fail closed on corrupt policy without overwriting it. Acceptance scenarios S15–S18 now project the accepted D6.2–D6.5 evidence consistently; S19 now projects D6.6's accepted-with-limits source boundary.
- **D6.6 — process-isolated external plugin host:** accepted with limits. Define a versioned, probe-only host protocol with typed failures, explicit launcher-versus-reported-host PID provenance, a strict `argv[0] --plugin-host --probe` entry boundary, and a fail-closed execution-capability invariant before any approved external code can execute; signatures, installation, platform sandboxing, update policy, and fresh packaged/runtime evidence remain separate release work.
- **D6.7 — plugin-host lifecycle and resource containment:** accepted with limits. The diagnostic host is created suspended, bound to a Windows Job Object before resuming the initial thread, and governed by kill-on-close, a two-process launcher/host allowance, and a 256 MiB per-process committed-memory limit; unsupported platforms and containment/resume failures are explicit and fail closed. Historical Windows/source evidence and the addressed Terra review close the implementation boundary, while fresh packaged/runtime, cross-machine, and complete-sandbox evidence remain open.
- **D6.8 — deny-by-default external plugin execution gate:** accepted with limits. Evaluate catalog/external-host evidence through one immutable application policy, reject malformed evidence fail-closed, project deterministic reasons in Extension Catalog, and keep the global external execution policy disabled until signatures, code identity, capability authorization, installation/update, and security review exist. Scenario S21 now projects this accepted-with-limits evidence; it does not authorize execution.

## Completion rule

The long-term goal remains active while any required delivery item is not `completed` with evidence. Known limitations are tracked as explicit acceptance or release risks; they are never converted into unverified success claims.

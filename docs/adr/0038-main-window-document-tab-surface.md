# ADR-0038: Extract the MainWindow document-tab surface

- **Status:** accepted-with-limits; Phase 2 bounded slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

`MainWindow` still owns the Qt tab widget, the record-to-index mapping, the
active-tab lookup, editor callback lookup, tab-title projection, and tab-bar
enablement. Those concerns are presentation projection and collection
coordination; save confirmation, recovery/session policy, document lifecycle
events, and operation callbacks are application-facing shell behavior.

Keeping both groups in one class makes tab layout changes unnecessarily touch
document workflows and leaves the index/list invariant implicit.

## Decision

Add `quillforge.presentation.document_tab_surface.DocumentTabSurface` as a
generic presentation coordinator. It owns:

- the `QTabWidget` object and close/current signal projection;
- the identity-based record collection and current-index lookup;
- editor-to-record lookup and membership checks;
- add/remove/current selection, title synchronization, and tab-bar enablement.

The coordinator depends only on a small structural `DocumentTabLike` contract
with an `EditorWidget`. `MainWindow` retains its existing private document-tab
record and owns document state transitions, path uniqueness, save-before-close
confirmation, recovery/session bookkeeping, lifecycle events, editor signals,
and all business callbacks. The surface receives close/current callbacks from
the shell but does not decide what those callbacks mean.

## Invariants

1. A record is appended before `QTabWidget` selection can notify the shell, so
   current-change callbacks always see a consistent collection/index mapping.
2. Removal updates the record collection before Qt removes the visual tab, so
   callbacks observe the remaining projection rather than a stale record.
3. Record membership and editor lookup use object identity; equal-looking
   document state cannot alias a different open tab.
4. MainWindow remains the source of document behavior and path identity policy;
   the surface does not import application or infrastructure modules.
5. The surface exposes the widget only for composition into the shell; it does
   not leak Qt objects into application services or plugin contracts.
6. The extraction does not introduce a service locator, global singleton,
   dependency-injection framework, or second document source of truth.

## Consequences

### Positive

- Tab projection and index invariants have one focused, inspectable owner.
- MainWindow no longer assembles or directly mutates `QTabWidget` state.
- Future tab presentation changes can stay within presentation without moving
  save/recovery/session policy across layers.
- The generic structural contract keeps the coordinator reusable for a future
  tab record without coupling it to document services.

### Limits

- MainWindow remains a large lifecycle coordinator; workspace, recovery,
  session, and plugin slices remain open.
- Qt runtime interaction, visual screenshots, accessibility output, and
  clean-machine/release evidence remain unrun under project policy.
- This is an incremental project architecture decision, not a claim of
  ByteDance private-standard compliance, certification, or release readiness.

## Alternatives rejected

- Moving save/close/recovery behavior into the surface: this would mix
  presentation projection with application policy.
- Keeping a second tab index in MainWindow: this would recreate the invariant
  the slice is intended to centralize.
- A generic event bus or dependency-injection container: unnecessary for the
  explicit two-callback boundary and would obscure ownership.
- A wholesale MainWindow rewrite: broader than the evidence-backed slice.

## Verification

- Static boundary probe confirms MainWindow no longer owns `QTabWidget`,
  `_tabs_widget`, or its tab-record collection; the new surface owns the
  projection and lookup methods.
- Compile, lint, format, handoff, package provenance, and release no-go checks
  are recorded in the D13 review and handoff.

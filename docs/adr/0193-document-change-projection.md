# ADR-0193: editor-change projection boundary

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D146 / ARCH-131

## Context

`MainWindow` received editor `modified` and `contentChanged` callbacks and
performed several ordered side effects inline: Find-match invalidation, tab
lookup, stale/unchanged guards, dirty-state mutation, tab-title/status/session
projection, and content-version advancement. The service, tab surface, and
Qt shell are valid owners of their concrete state, but the event ordering is a
small framework-neutral policy that should not be duplicated in a monolithic
window handler.

## Decision

Add the Qt-free generic `DocumentChangeProjectionCoordinator[EditorT, TabT]`
with a frozen/slotted `DocumentChangeProjectionPorts[EditorT, TabT]` contract.
It owns only the established editor-event ordering:

1. A modified callback invalidates Find, resolves the tab, ignores stale or
   unchanged state, then applies dirty state, title, status, and debounced
   session-save projection in that order.
2. A content callback resolves the tab, ignores stale editors, increments its
   content identity, then invalidates Find.

`MainWindow` keeps `DocumentService.mark_dirty`, tab/editor lookup, concrete
tab identity, FindSurface, StatusSurface, session timer/coordinator, Qt
signals, and application policy. No current-tab transition, editor behavior,
document persistence, async worker, close policy, locale, or stylesheet owner
moves.

## Alternatives rejected

- Keeping both callback sequences inline would preserve a high-coupling event
  handler and make the ordering contract harder to inspect independently.
- Moving tab records or `DocumentService` into the coordinator would couple a
  framework-neutral projection to application state ownership.
- Combining current-tab transition with editor mutation would create a broad
  document-lifecycle object with unrelated selection and mutation policy.

## Review and evidence

Pasteur the 5th / Luna max was assigned the architecture assessment and
returned no conclusion in the bounded window. Plato the 5th / Luna max was
assigned the independent read-only review and also returned no conclusion.
No child PASS is claimed. Parent review is `PASS`; simplification assessment
is `PASS` because the coordinator contains only the two existing event
sequences and named callbacks, without a new state machine or Qt dependency.

Authorized non-destructive evidence:

- `D146-PROJECTION-PROBE=PASS`
- `D146-SOURCE-WIRING-PROBE=PASS`
- `D146-PRESENTATION-AUDIT=PASS`
- `D146-COMPILEALL=PASS`
- `D146-RUFF=PASS`
- `D146-FORMAT=PASS`
- `D146-PACKAGE-BUILD=PASS`
- `D146-PACKAGE-IDENTITY-PROBE=PASS`
- `D146-CHECK=PASS`
- `D146-VERIFY-HANDOFF=PASS`
- expected release `NO-GO` dossier and no-launch checks.

The packaged candidate is `dist/QuillForge.exe` and the root test copy:

- SHA-256: `A9CF905D11573A891A7562B98BA571922BFC67922326336E4F48757671D2179C`
- bytes: `38539992`
- source revision: `tree-sha256:3ea9b2f76f24dfcafb512979e1495e164eb7f28184f5d8e0141593d6764da9a1`

Public-source applicability is Python 3.12/PyQt6 centralized presentation
contracts; embedded C/C++, MCU, RTOS, and manufacturer requirements do not
apply. Public CloudWeGo material remains an engineering reference only, not a
private ByteDance standard or a certification/compliance claim.

## Limits

Static and inline source probes do not prove native Qt signal timing, editor
event-loop behavior, font/DPI metrics, accessibility, runtime startup,
clean-machine or cross-machine behavior, signing, installer/update, legal
clearance, support ownership, or release readiness. Those gates remain open
under the active no-launch/no-release authorization boundary.

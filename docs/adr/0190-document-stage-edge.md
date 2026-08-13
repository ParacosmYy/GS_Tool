# ADR-0190: document-stage edge

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: UI-73 / ARCH-128

## Context

The document surface had three nested visual boundaries: the authored tab
rail, the `QTabWidget` pane outline, and the editor canvas outline. The middle
outline duplicated the hierarchy and made the central work area feel heavier
and older than the surrounding shell.

## Decision

Keep `presentation/theme.py` as the single visual owner and scope the pane
rule to `QTabWidget#documentTabs::pane`. Preserve the canvas surface, but set
the pane border and radius to zero and remove its one-pixel top offset. The
authored `QTabBar#documentTabBar` rail, selected/hover/focus/disabled states,
and `QsciScintilla#editor` border/focus cue remain unchanged. No widget,
signal, layout owner, editor behavior, locale, font, motion, or application
policy changes.

## Alternatives rejected

- Extracting a new shell-composition coordinator was deferred: the current
  visual issue is fully local to centralized QSS and a new indirection would
  not improve the user outcome.
- Removing the tab-rail or editor border would erase useful navigation/canvas
  hierarchy rather than remove only the redundant middle edge.
- A widget-local stylesheet would create a second styling owner and bypass the
  token-bound theme contract.

## Review and evidence

James the 5th / Luna max was assigned the architecture assessment and returned
no conclusion in the bounded window. Godel the 5th / Luna max was assigned the
independent read-only review and also returned no conclusion. No child PASS is
claimed. Parent review is `PASS`; simplification assessment is `PASS` because
the existing centralized selector is the smallest complete visual change.

Authorized non-destructive evidence:

- `UI73-DOCUMENT-STAGE-PROBE=PASS`
- `UI73-QSS-PROJECTION-PROBE=PASS:12 theme/accent projections`
- `UI73-COMPILEALL=PASS`
- `UI73-RUFF=PASS`
- `UI73-FORMAT=PASS`
- `UI73-PACKAGE-BUILD=PASS`
- `UI73-PACKAGE-IDENTITY-PROBE=PASS`
- `UI73-NO-PROCESS-PROBE=PASS`
- `UI73-CHECK=PASS`
- `UI73-VERIFY-HANDOFF=PASS`
- expected release `NO-GO` dossier and traceability checks.

The packaged candidate is `dist/QuillForge.exe` and the root test copy:

- SHA-256: `709FFAB43528F82819A4E2565EE02CC3FF5B7A72E701EC2AED3671DFCEECC9F6`
- bytes: `38536315`
- source revision: `tree-sha256:f5de41605177c15c4964b43628036883df419843a60ceca127d13aeabaaef662`

Public-source applicability is Python 3.12/PyQt6 centralized QSS; embedded
C/C++, MCU, RTOS, and manufacturer requirements do not apply. Public
CloudWeGo material remains an engineering reference only, not a private
ByteDance standard or a certification/compliance claim.

## Limits

Static stylesheet/token probes do not prove native QSS specificity or
rendering, font/DPI metrics, keyboard traversal, accessibility, runtime
startup, clean-machine or cross-machine behavior, signing, installer/update,
legal clearance, support ownership, or release readiness. Those gates remain
open under the active no-launch/no-release authorization boundary.


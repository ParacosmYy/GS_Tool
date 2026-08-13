# ADR-0237: Workspace search empty-state boundary

- Status: accepted-with-limits
- Date: 2026-08-11
- Delivery: D189 / UI-99 / ARCH-175

## Context

`WorkspaceSearchDialog` previously placed the result `QListWidget` directly in
the dialog layout. The initial state and a completed query with zero matches
therefore rendered as an unanchored blank panel; the status label carried the
only semantic cue. This was especially weak beside the existing query card and
diagnostic disclosure surface.

## Decision

Keep `WorkspaceSearchDialog` as the sole presentation owner and place the
existing result list plus one localized `QLabel#workspaceSearchEmpty` in a
single `QStackedLayout`. The dialog switches the stage from the list to the
empty state only when there are no result rows. The existing feedback-state
contract supplies `info`, `working`, `warning`, and `error` styling, while
`presentation.theme` remains the only QSS owner.

The empty-state message is retained as a small presentation key and is
refreshed through the existing locale path. Initial, loading, no-match,
cancelled, and error branches receive explicit copy; populated results keep
the existing item text, path/line roles, tooltip, and double-click signal.

## Preserved invariants

- Search validation and `search_requested` emission are unchanged.
- Cancellation and close continue to use the existing semantic signals.
- `WorkspaceSearchResult` remains the only result input; no service or worker
  dependency is introduced into the surface.
- Existing results remain visible during busy/error/cancel paths when the
  current surface already has rows.
- Diagnostics, root containment, locale refresh, and MainWindow ownership are
  unchanged.

## Review and applicability

The architecture consultation (`Descartes the 6th / Luna max`) and independent
review (`Einstein the 6th / Luna max`) both timed out within the bounded window;
both are recorded as `NO_CONCLUSION`. Parent review is `PASS`, and the
behavior-preserving simplification assessment is `PASS`.

This is a Python 3.12/PyQt6 presentation change. Embedded C/C++, MCU, RTOS,
and manufacturer requirements are not applicable. Public CloudWeGo material
is an engineering reference only; this ADR makes no private ByteDance
standard, certification, or compliance claim.

## Evidence and limits

- `D189-EMPTY-STATE-CONTRACT-PROBE=PASS`
- `D189-LOCALE-AND-QSS-PROBE=PASS`
- `D189-STATE-BRANCH-PROBE=PASS`
- `D189-SIGNAL-ROLE-PRESERVATION-PROBE=PASS`
- `D189-COMPILE-RUFF-FORMAT=PASS`
- `D189-PRESENTATION-AUDIT=PASS`
- `D189-PACKAGE-BUILD=PASS`
- `D189-PACKAGE-IDENTITY-PROBE=PASS`

Native Qt list/stack metrics, accessibility-tree output, DPI, screenshot
review, GUI/EXE startup, clean-machine, cross-machine, legal, signing,
installer, updater, support, and release-owner evidence remain open. Release
verification remains `no-go` under the current no-launch and external-gate
policy.


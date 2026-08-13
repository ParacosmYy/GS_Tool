# ADR-0100: Workspace search visual hierarchy

- **Status:** accepted-with-limits; D75 / UI-48 bounded slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

Find in Files already had bounded results, diagnostics, cancellation, and
status semantics, but its window mostly inherited generic list styling. Scope,
query, result rows, diagnostic records, and selected/focused states did not
form a strong scan order, making the surface feel dated and making important
warnings easy to miss.

## Decision

Add object-name-scoped QSS in `theme.py` for the existing
`workspaceSearchDialog`: emphasize the selected workspace scope and query,
give result and diagnostic lists distinct panels, and make hover, focus,
selected, disabled, and diagnostic-toggle states explicit. Add the existing
diagnostic toggle's presentation identity
`workspaceSearchDiagnosticsToggle` in `WorkspaceSearchDialog`.

The slice does not change result data, signals, locale strings, search policy,
TaskRunner ownership, cancellation, diagnostics bounds, or file activation.
Warning text uses the existing derived warning foreground and accent endpoints
are checked across every supported theme/accent combination.

## Invariants

1. `theme.py` remains the sole QSS/token owner; the dialog only supplies the
   semantic object name.
2. Search service, operation tracker, TaskRunner, result projection, locale,
   diagnostics, cancellation, and file-opening policy remain unchanged.
3. Result, diagnostic, focus, selected, disabled, hover, and checked states are
   distinguishable without relying on a color change alone.
4. Warning, selection, and amber hover foreground pairs meet the recorded
   4.5:1 static contrast target.
5. Native QSS specificity and actual layout/rendering remain runtime-owned
   evidence, not claims from the source probe.

## Alternatives considered

- **Style the whole application list globally:** rejected; it would leak
  diagnostic semantics into unrelated lists.
- **Add local `setStyleSheet()` calls in the dialog:** rejected; it would split
  the theme owner and make future theme/accent updates inconsistent.
- **Change result data into custom row widgets:** rejected; the existing item
  data and list behavior are sufficient for this visual hierarchy slice.

## Public-source applicability and embedded gate

This is Python/PyQt6 desktop presentation code. Embedded C/C++, MCU, BSP/HAL,
CMSIS, RTOS, ISR/DMA, driver, boot, Flash/NVM, power, motor-control, and
manufacturer requirements are not applicable. Public CloudWeGo material
remains an engineering reference only; no private ByteDance standard,
certification, or compliance claim is made.

## Verification target and limits

- `D75-SEARCH-HIERARCHY-CONTRAST-PROBE=PASS` covers all 3 themes × 4 accents,
  workspace-search selectors, selection, warning, and gold-hover contrast.
- Compileall, Ruff, format, package identity, handoff, and repository checks
  are recorded in the D75 handoff.
- The Architect and independent review windows returned no conclusion; no
  child PASS is claimed.
- Native QSS rendering, DPI, fonts, keyboard traversal, accessibility,
  startup, clean-machine, cross-machine, signing, legal, and release-owner
  evidence remain unrun or open.
- No unit tests, mocks, fixtures, harnesses, or test-only assets were created.

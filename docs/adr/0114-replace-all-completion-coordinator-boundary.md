# ADR-0114: Replace All completion coordinator boundary

- **Status:** accepted-with-limits; D89 / ARCH-64 bounded slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

`MainWindow._finish_replace_all` combined ReplaceAllTracker identity release,
editor operation unlock, tab-bar and Find operation-state release, generic
operation completion, and product-specific result handling. The product policy
includes rollback/clean-state restoration, limit/cancel/success/error status,
and user-facing messages.

## Decision

Extract the common lifecycle release into the Qt-free generic
`ReplaceAllCompletionCoordinator[TabT, SessionT, ProgressT]`. It receives the
existing `ReplaceAllTracker`, opaque-tab liveness/lock seams, tab-bar and Find
operation-state projections, generic operation completion, and an outcome
policy callback.

The coordinator owns only current-job release and common UI lifecycle cleanup.
MainWindow retains `ReplaceAllSession` stepping/cancellation, rollback,
clean-state restoration, limit/cancel/success/error messages, status policy,
and editor/document consequences.

## Invariants

1. `presentation/replace_all_completion_coordinator.py` imports no PyQt6 and
   does not import the QScintilla-backed `editor_widget` implementation.
2. A stale job returns before unlocking a tab, enabling the tab bar, changing
   Find operation state, completing the generic operation, or projecting an
   outcome.
3. A current job unlocks only a live tab, then releases tab-bar/Find/common
   operation state in the existing order.
4. The opaque `ProgressT` is passed to MainWindow unchanged; the coordinator
   does not interpret rollback, limit, cancel, or success semantics.
5. MainWindow retains all Replace All session, editor, document, status,
   notification, and close policy.

## Alternatives considered

- **Leave lifecycle cleanup in MainWindow:** rejected; tracker release and
  common UI cleanup are one focused callback boundary.
- **Move rollback or result messages into the coordinator:** rejected; these
  require editor/document semantics and product policy.
- **Import `ReplaceAllProgress` or `ReplaceAllSession` into the coordinator:**
  rejected; the generic progress/session parameters keep the boundary free of
  Qt/editor implementation coupling.
- **Move Replace All stepping/timer scheduling:** rejected; bounded UI slicing
  and cancellation remain MainWindow/editor policy.

## Public-source applicability and embedded gate

This is Python/PyQt6 desktop presentation/application-boundary code. Embedded
C/C++, MCU, BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot, Flash/NVM, power,
motor-control, and manufacturer requirements are not applicable. Public
CloudWeGo material remains an engineering reference only; no private
ByteDance standard, certification, or compliance claim is made.

## Review and simplification

- Architect role: Galileo the 3rd / Luna max; bounded window returned
  `NO_CONCLUSION`, so no child architecture PASS is claimed.
- Independent review: Mencius the 3rd / Luna max; bounded read-only window
  returned `NO_CONCLUSION` and was closed, so no independent PASS is claimed.
- Parent source review: PASS for stale tracker protection, live-tab unlock,
  common lifecycle ordering, generic progress forwarding, and retention of
  rollback/result policy.
- Simplification assessment: one common lifecycle coordinator removes
  duplicated release plumbing while preserving an explicit outcome seam.
  Generic progress avoids importing editor implementation. No further safe
  behavior-preserving reduction was identified.

## Verification target and limits

- `D89-REPLACE-ALL-QT-FREE-BOUNDARY-PROBE=PASS` covers the Qt-free/editor-free
  import boundary, callback wiring, tracker release ownership, old cleanup
  removal, and retained Replace All policy.
- Targeted compileall, Ruff, format, package identity, traceability, handoff,
  repository checks, no-process, and expected release no-go evidence are
  recorded in the D89 handoff.
- Native Replace All slicing, cancellation/rollback timing, accessibility,
  DPI, font metrics, runtime startup, clean-machine, cross-machine, signing,
  installer, updater, legal, support, and release owner evidence remain
  unrun or open.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created.

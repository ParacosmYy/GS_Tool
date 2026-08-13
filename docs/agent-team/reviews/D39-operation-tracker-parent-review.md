# D39 parent review — operation-tracker boundary

- **Delivery:** D39 / ARCH-29 / UI-25
- **Date:** 2026-08-10
- **Reviewer:** Architect (parent integration review)
- **Decision:** accepted-with-limits for the bounded static/source slice

## Scope and architecture decision

D39 extracts only the monotonic operation-ID and one-current-operation
stale-guard invariant into the Qt-free
`presentation.operation_tracker.OperationTracker`. `MainWindow` keeps the
existing `_next_operation_id()` facade for independent worker paths and uses
`begin()`/`complete()` for the current UI operation lifecycle. Workspace
cancellation delegates to `cancel()` before preserving the existing generation
invalidation, busy reset, document phase sync, surface feedback, and session
restore release.

The tracker does not own `TaskRunner`, `_busy`, status phase, notifications,
generation/session state, service policy, or result projection. This is the
smallest useful boundary for the current coordinator and avoids creating a
second state machine.

The required architect consultation was attempted with Lagrange the 2nd /
Terra max for a cross-module operation-lifecycle trace. Two bounded waits
returned no conclusion and the agent was closed; no architecture PASS is
claimed. An independent read-only review was attempted with Kant the 2nd /
Luna max; two bounded waits also returned no conclusion and it was closed. No
child review PASS is claimed.

## Static review findings

- `OperationTracker.reserve()` remains the only allocator for the preserved
  process-local sequence.
- `begin()` reserves before marking active, so every active ID remains part of
  the same monotonic sequence.
- `complete()` and `cancel()` reject any ID that is not current, preserving the
  stale-completion guard.
- MainWindow contains no direct `_operation_counter` or
  `_active_operation_id` ownership after the extraction.
- Independent session, recovery, settings, search, and plugin operation IDs
  continue through `_next_operation_id()` or their existing domain-local
  counters; their service and callback policies are unchanged.
- Workspace completion still calls `_complete_operation()` and workspace
  cancellation still clears the workspace ID, invalidates its generation,
  releases `_busy`, and synchronizes the document/status projection.

## Simplification assessment

The extraction is intentionally one small dataclass with no Qt dependency,
signal, callback, service locator, or new policy abstraction. Keeping `_busy`
and status synchronization in MainWindow avoids duplicating state or hiding
close/TaskRunner semantics in a generic tracker. No further behavior-preserving
simplification is required for D39.

## Public-source applicability and embedded gate

This is Python/PyQt6 code; embedded C/C++, MCU, BSP/HAL, RTOS, ISR/DMA,
driver, boot, Flash/NVM, power, and motor-control requirements are not
applicable. Public CloudWeGo material remains an engineering reference only;
no private ByteDance standard, certification, or compliance claim is made.
The applicability record is carried in ADR-0064.

## Authorized non-destructive validation

- D39 source boundary/lifecycle probe — PASS: tracker methods, old-field
  removal, delegation, and retained MainWindow policy owners are present.
- `uv run python -m compileall -q src/quillforge/presentation/operation_tracker.py src/quillforge/presentation/main_window.py` — PASS.
- `uv run ruff check src/quillforge/presentation/operation_tracker.py src/quillforge/presentation/main_window.py` — PASS.
- `uv run ruff format --check src/quillforge/presentation/operation_tracker.py src/quillforge/presentation/main_window.py` — PASS.
- JSON parse of acceptance, delivery, and handoff indexes — PASS.
- No unit tests, mocks, fixtures, harnesses, or test-only assets were created
  or run. No QApplication/Qt startup, screenshots, deployment, or hardware
  operation was authorized.

## Handoff and limits

The handoff will record the rebuilt package identity after the synchronized
package build. Release verification remains expected NO-GO while historical
runtime reports, clean-machine evidence, signing, installer, update, and
release-owner gates remain open.

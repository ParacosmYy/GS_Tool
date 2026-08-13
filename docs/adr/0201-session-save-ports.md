# ADR-0201: session-save Ports contract

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D152 / ARCH-139

## Context

`SessionSaveCoordinator` already isolated latest-wins session persistence
sequencing, but its constructor accepted six independent callbacks. The
callback meanings were stable while their positional/keyword mix made the
operation admission and dispatch boundary harder to review during the
enterprise architecture migration.

## Decision

Introduce the frozen/slotted Qt-free `SessionSavePorts` contract with named
callbacks for `can_save`, `capture_snapshot`, `next_operation_id`,
`save_snapshot`, `dispatch`, and `notify`. The coordinator keeps the existing
behavior:

- `request_latest()` admits only when saving is allowed, captures the newest
  snapshot, and drains when the tracker accepts it;
- `drain()` preserves the can-save, pending-request, and in-flight guards,
  allocates one operation ID, begins one tracker operation, and dispatches the
  captured snapshot;
- stale callbacks are ignored without notification or drain;
- invalid results notify the existing error and drain the newest queued
  request;
- matching failures notify the existing error and drain the newest queued
  request, while non-matching failures are ignored.

MainWindow retains session service, snapshot builder, debounce timer,
operation IDs, TaskRunner, notifications, and close/startup policy. No Qt type
enters the coordinator.

## Alternatives rejected

- Keeping six positional/independent callbacks would preserve avoidable
  operation-boundary wiring risk.
- Moving tracker state, persistence, debounce, or worker behavior into the
  contract would cross existing ownership boundaries.
- Adding a generic event bus or persistence state machine would broaden a
  stable latest-wins coordinator without changing behavior.

## Review and evidence

Planck the 5th / Luna max was assigned the architecture assessment and
returned no conclusion in the bounded window. Einstein the 5th / Luna max was
assigned the independent read-only review and also returned no conclusion.
No child PASS is claimed. Parent review is `PASS`; simplification assessment
is `PASS` because the immutable named contract removes callback coupling
without adding behavior or policy.

Authorized non-destructive evidence:

- `D152-LATEST-WINS-PROBE=PASS`
- `D152-STALE-INVALID-FAILURE-PROBE=PASS`
- `D152-PORTS-IMMUTABILITY-PROBE=PASS`
- `D152-SOURCE-WIRING-PROBE=PASS`
- `D152-QT-FREE-CONTRACT-PROBE=PASS`
- `D152-PRESENTATION-AUDIT=PASS`
- `D152-COMPILEALL=PASS`
- `D152-RUFF=PASS`
- `D152-FORMAT=PASS`
- `D152-PACKAGE-BUILD=PASS`
- `D152-PACKAGE-IDENTITY-PROBE=PASS`
- expected release `NO-GO` dossier and no-launch checks.

The packaged candidate is `dist/QuillForge.exe` and the root test copy:

- SHA-256: `4A146E8F0CD9074ACC524F58E3664F588D830790EE1EA5D86C6A74AA8B3C79DA`
- bytes: `38545547`
- source revision: `tree-sha256:e4b6d574ea362873eb776cb06cf99fcb59e2f7c2734918ef4ee305cba219fa57`

Public-source applicability is Python 3.12/PyQt6 presentation architecture;
embedded C/C++, MCU, RTOS, and manufacturer requirements do not apply.
Public CloudWeGo material remains an engineering reference only, not a
private ByteDance standard or certification/compliance claim.

## Limits

The latest-wins/order probes and static/package checks do not prove native Qt
timer/callback timing, filesystem durability, runtime startup,
clean-machine or cross-machine behavior, signing, installer, update, legal
clearance, support ownership, or release readiness. Those gates remain open
under the active no-launch/no-release authorization boundary.

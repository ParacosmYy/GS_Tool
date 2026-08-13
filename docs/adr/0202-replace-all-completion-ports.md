# ADR-0202: Replace All completion Ports contract

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D153 / ARCH-140

## Context

`ReplaceAllCompletionCoordinator` already isolated the cleanup boundary for a
cooperative Replace All job, but its constructor accepted six independent
callbacks. The lifecycle order was stable while positional wiring made the
editor-lock and operation-release safety path harder to review.

## Decision

Introduce the frozen/slotted Qt-free
`ReplaceAllCompletionPorts[TabT, SessionT, ProgressT]` contract with named
callbacks for tab containment/unlock, tab-bar enablement, operation-active
projection, operation completion, and product-specific outcome projection.
The coordinator retains the exact current-job sequence:

`tracker.finish -> contains/unlock -> tab-bar enabled -> operation inactive ->
complete operation -> project outcome`.

Stale jobs remain silent because `tracker.finish` is the first guard. MainWindow
retains the concrete editor lock, tab surface, Find surface, operation
tracker, ReplaceAllSession, rollback, cancellation, status, and policy
ownership. No Qt type enters the coordinator.

## Alternatives rejected

- Keeping positional callbacks would preserve avoidable safety-path wiring
  risk.
- Moving editor mutation, rollback, timer, or status policy into the contract
  would violate existing presentation/application ownership.
- Adding a generic event bus or Replace All state machine would broaden a
  stable lifecycle release boundary without changing behavior.

## Review and evidence

Carver the 5th / Luna max was assigned the architecture assessment and
returned no conclusion in the bounded window. Zeno the 5th / Luna max was
assigned the independent read-only review and also returned no conclusion.
No child PASS is claimed. Parent review is `PASS`; simplification assessment
is `PASS` because the immutable named contract removes callback coupling
without adding behavior or policy.

Authorized non-destructive evidence:

- `D153-COMPLETION-ORDER-PROBE=PASS`
- `D153-STALE-JOB-PROBE=PASS`
- `D153-PORTS-IMMUTABILITY-PROBE=PASS`
- `D153-SOURCE-WIRING-PROBE=PASS`
- `D153-QT-FREE-CONTRACT-PROBE=PASS`
- `D153-PRESENTATION-AUDIT=PASS`
- `D153-COMPILEALL=PASS`
- `D153-RUFF=PASS`
- `D153-FORMAT=PASS`
- `D153-PACKAGE-BUILD=PASS`
- `D153-PACKAGE-IDENTITY-PROBE=PASS`
- expected release `NO-GO` dossier and no-launch checks.

The packaged candidate is `dist/QuillForge.exe` and the root test copy:

- SHA-256: `9E85DA8F96FE9CABAE5D37A0FB623697A398315135D3676776C90D932BF5BA5E`
- bytes: `38544791`
- source revision: `tree-sha256:d4532e3adc81ad32372765da2fedf9996d74eee368f98e207f821d087755f8fb`

Public-source applicability is Python 3.12/PyQt6 presentation architecture;
embedded C/C++, MCU, RTOS, and manufacturer requirements do not apply.
Public CloudWeGo material remains an engineering reference only, not a
private ByteDance standard or certification/compliance claim.

## Limits

The completion-order/stale probes and static/package checks do not prove
native Qt timer/event timing, editor rollback, filesystem durability, runtime
startup, clean-machine or cross-machine behavior, signing, installer, update,
legal clearance, support ownership, or release readiness. Those gates remain
open under the active no-launch/no-release authorization boundary.

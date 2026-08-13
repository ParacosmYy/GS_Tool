# ADR-0207: recovery-scan Ports contract

- Status: accepted-with-limits
- Date: 2026-08-13
- Delivery: D158 / ARCH-145

## Context

`RecoveryScanCoordinator` already isolated recovery inventory callback
classification, but its constructor accepted four independent callbacks.
Tracker stale protection, inventory validation, candidate prompting, empty
feedback, and startup continuation were stable while positional wiring made
the recovery boundary harder to review.

## Decision

Introduce the frozen/slotted Qt-free `RecoveryScanPorts` contract with named
callbacks for current session snapshot lookup, recovery-candidate prompting,
startup continuation, and notification. Preserve the existing behavior:

- stale job: no projection;
- invalid inventory: error notification and startup continuation when needed;
- empty non-startup inventory: info notification;
- non-empty inventory: prompt each candidate in result order;
- startup success/failure: continue restore with the current session snapshot.

MainWindow retains RecoveryService, scan worker, restore state, filesystem,
notification, startup, close, and concrete Qt ownership. No Qt type enters the
coordinator.

## Alternatives rejected

- Keeping positional callbacks would retain avoidable recovery-path wiring
  risk.
- Moving scan validation, candidate policy, or startup restore into Ports would
  broaden application ownership.
- Introducing an event bus or recovery state machine would add lifecycle
  complexity without changing behavior.

## Review and evidence

Kant the 5th / Luna max was assigned the architecture assessment and returned
no conclusion in the bounded window. Lorentz the 5th / Luna max was assigned
the independent read-only review and also returned no conclusion. No child
PASS is claimed. Parent review is `PASS`; simplification assessment is `PASS`
because the immutable named contract removes positional coupling without
adding behavior or policy.

Authorized non-destructive evidence:

- `D158-RECOVERY-SCAN-BRANCH-PROBE=PASS`
- `D158-STALE-INVALID-EMPTY-CANDIDATE-STARTUP-FAILURE-PROBE=PASS`
- `D158-PORTS-IMMUTABILITY-PROBE=PASS`
- `D158-SOURCE-WIRING-PROBE=PASS`
- `D158-QT-FREE-CONTRACT-PROBE=PASS`
- `D158-PRESENTATION-AUDIT=PASS`
- `D158-COMPILEALL=PASS`
- `D158-RUFF=PASS`
- `D158-FORMAT=PASS`
- `D158-PACKAGE-BUILD=PASS`
- `D158-PACKAGE-IDENTITY-PROBE=PASS`
- expected release `NO-GO` dossier and no-launch checks.

The packaged candidate is `dist/QuillForge.exe` and the root test copy:

- SHA-256: `1C01FB2E5AE1CBE295D34E37CB3C8357C10D7839B1B1AF0CFDD762BD8E4DE56B`
- bytes: `38546847`
- source revision: `tree-sha256:72a3c626065ca00ab6aa40bb173092561e5347e45da70a327585f73c191230b5`

Public-source applicability is Python 3.12/PyQt6 presentation architecture;
embedded C/C++, MCU, RTOS, and manufacturer requirements do not apply.
Public CloudWeGo material remains an engineering reference only, not a
private ByteDance standard or certification/compliance claim.

## Limits

The inline and static/package checks do not prove native scan timing, startup
scheduling, filesystem behavior, runtime startup, clean-machine or
cross-machine behavior, signing, installer, update, legal clearance, support
ownership, or release readiness. Those gates remain open under the active
no-launch/no-release authorization boundary.

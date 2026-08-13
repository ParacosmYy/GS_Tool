# ADR-0206: recovery-delete Ports contract

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D157 / ARCH-144

## Context

`RecoveryDeleteCoordinator` already isolated recovery snapshot delete callback
projection, but its constructor accepted three independent callbacks. Tracker
release, owner cleanup, deferred-delete draining, and notification order were
stable while positional wiring made the persistence boundary harder to review.

## Decision

Introduce the frozen/slotted generic Qt-free `RecoveryDeletePorts[OwnerT]`
contract with named callbacks for live-owner cleanup, pending-delete
scheduling, and notification. Preserve the existing order:

- successful completion: tracker release, owner cleanup when present, optional
  success notification, then pending-delete scheduling;
- failure: tracker failure release, then error notification.

MainWindow retains recovery persistence, filesystem operations, capture/write
state, tab identity, worker dispatch, notification, close, and concrete Qt
ownership. No Qt type enters the coordinator.

## Alternatives rejected

- Keeping positional callbacks would retain avoidable delete-path wiring risk.
- Moving recovery durability, pending state, or file deletion into Ports would
  broaden the composition boundary and alter ownership.
- Introducing a generic event bus or delete state machine would add lifecycle
  complexity without changing behavior.

## Review and evidence

Bohr the 5th / Luna max was assigned the architecture assessment and returned
no conclusion in the bounded window. Gauss the 5th / Luna max was assigned the
independent read-only review and also returned no conclusion. No child PASS is
claimed. Parent review is `PASS`; simplification assessment is `PASS` because
the immutable named contract removes positional coupling without adding
behavior, persistence, or policy.

Authorized non-destructive evidence:

- `D157-RECOVERY-DELETE-BRANCH-PROBE=PASS`
- `D157-COMPLETE-PENDING-FAILURE-ORDER-PROBE=PASS`
- `D157-PORTS-IMMUTABILITY-PROBE=PASS`
- `D157-SOURCE-WIRING-PROBE=PASS`
- `D157-QT-FREE-CONTRACT-PROBE=PASS`
- `D157-PRESENTATION-AUDIT=PASS`
- `D157-COMPILEALL=PASS`
- `D157-RUFF=PASS`
- `D157-FORMAT=PASS`
- `D157-PACKAGE-BUILD=PASS`
- `D157-PACKAGE-IDENTITY-PROBE=PASS`
- expected release `NO-GO` dossier and no-launch checks.

The packaged candidate is `dist/QuillForge.exe` and the root test copy:

- SHA-256: `09F6C828634FE26315F8C6EDDFFF5AF6013776D9E002E7A27B46C8BDAA2E4BAC`
- bytes: `38546866`
- source revision: `tree-sha256:49a98461f5aa2cd4bb19eb497a30173e81ad4bb590fb568f78e3028d6d0a48eb`

Public-source applicability is Python 3.12/PyQt6 presentation architecture;
embedded C/C++, MCU, RTOS, and manufacturer requirements do not apply.
Public CloudWeGo material remains an engineering reference only, not a
private ByteDance standard or certification/compliance claim.

## Limits

The inline and static/package checks do not prove native delete timing,
filesystem durability, runtime startup, clean-machine or cross-machine
behavior, signing, installer, update, legal clearance, support ownership, or
release readiness. Those gates remain open under the active no-launch/no-
release authorization boundary.

# ADR-0212: Recovery-capture abort Ports contract

- Status: accepted-with-limits
- Date: 2026-08-11
- Delivery: D163 / ARCH-150

## Context

`RecoveryCaptureAbortCoordinator` had a positional constructor carrying seven
callbacks for owner/job identity, snapshot/channel lookup, session cancellation,
failure notification, and live-owner lookup. The coordinator already keeps the
tracker as a separate lifecycle state owner, but positional assembly obscured a
high-risk release boundary.

## Decision

Introduce frozen/slotted generic `RecoveryCaptureAbortPorts[JobT, OwnerT]` and
construct the coordinator with `(tracker, ports)`. Preserve the exact existing
release sequence:

1. derive owner and document identity and reject a nonmatching job silently;
2. finish the matching capture;
3. if a channel exists, mark the snapshot discarded when the worker started or
   release the snapshot when it did not, then abort the channel;
4. cancel the session;
5. complete the document lifecycle;
6. only when requested and live, notify the owner of failure.

`MainWindow` retains tracker, channel, session, tab, worker, filesystem, and
close-policy ownership. The Ports contract does not change tracker state
semantics, channel implementation, worker scheduling, or recovery durability.

## Alternatives considered

- Keep positional callbacks: rejected because a high-risk lifecycle boundary
  remains order-sensitive and hard to review at the composition root.
- Move tracker/channel policy into the coordinator: rejected because it would
  expand lifecycle ownership and couple presentation to recovery internals.
- Replace all callbacks with a generic service object: rejected because it
  would hide the explicit identity/release seam and increase coupling.

## Review and simplification

- Architect role: Parfit the 5th / Terra max; bounded waits ended without a
  conclusion, recorded as `NO_CONCLUSION`; no concurrency assurance is claimed.
- Independent high-risk review: Ampere the 5th / Terra max; bounded window
  returned `NO_CONCLUSION`.
- Final adversarial review: Helmholtz the 5th / Sol medium; `PASS`, with no
  must-fix behavior change or release-order error found. This is still not a
  formal runtime concurrency or durability approval.
- Parent review: PASS for the minimal source change and exact release order;
  no formal concurrency or durability approval is claimed.
- Simplification assessment: PASS; one immutable generic contract removes
  positional assembly without adapters, duplicate state, or policy movement.

## Authorized evidence

- Inline production-class lifecycle/branch/order/guard/immutability probes:
  `D163-RECOVERY-CAPTURE-ABORT-BRANCH-PROBE=PASS`,
  `D163-RECOVERY-CAPTURE-ABORT-ORDER-GUARD-PROBE=PASS`,
  `D163-PORTS-IMMUTABILITY-PROBE=PASS`.
- Source and Qt-free AST probes: `D163-SOURCE-WIRING-PROBE=PASS`,
  `D163-QT-FREE-CONTRACT-PROBE=PASS`.
- `scripts/check.ps1`: `D163-PRESENTATION-AUDIT=PASS`,
  `D163-COMPILEALL=PASS`, `D163-RUFF=PASS`, `D163-FORMAT=PASS`.
- Package and identity evidence are recorded after the D163 package build.
- Final adversarial review: `D163-SOL-ADVERSARIAL-REVIEW=PASS`.
- Release handoff remains expected `NO-GO`; no application launch, clean-machine
  run, flashing, deployment, or target operation was authorized.

## Public-source applicability

This is Python 3.12/PyQt6 presentation architecture. Embedded C/C++,
MCU/RTOS/BSP/HAL, and manufacturer requirements are not applicable. Public
CloudWeGo material is an engineering reference only; no private ByteDance
standard or compliance claim is made.

## Limits

This decision does not prove callback interleavings, thread scheduling,
channel cancellation timing, filesystem durability, cross-machine behavior,
packaged startup, signing, installer/update behavior, or enterprise release
readiness. No formal concurrency, safety, or certification claim is made.

# ADR-0198: recovery projection Ports contract

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D150 / ARCH-136

## Context

`RecoveryProjectionCoordinator` kept eight positional callbacks for projecting
recovery-writer outcomes. The decision branches were already isolated and
Qt-free, but positional construction obscured ownership and made future
changes to recovery safety paths harder to review.

## Decision

Introduce the frozen/slotted generic
`RecoveryProjectionPorts[OwnerT]` contract with named callbacks for liveness,
dirty state, current snapshot/content identity, deletion, clearing, newer-edit
feedback, and failure feedback. The coordinator retains the existing branch
order:

- saved/discarded: schedule deletion;
- saved/not-live: schedule deletion;
- saved/live: inspect dirty, snapshot identity, and content version in order;
- failed/discarded: no-op;
- failed/live: notify failure.

MainWindow retains recovery state, tab identity, deletion scheduling, clear
policy, notification text/levels, writer lifecycle, and all concrete Qt and
application ownership. The coordinator remains framework-neutral.

## Alternatives rejected

- Keeping positional callbacks would preserve an avoidable safety-path wiring
  hazard.
- Moving recovery state or snapshot deletion into this contract would violate
  the existing lifecycle/persistence boundary.
- Adding a state machine or event bus would broaden a stable branch projection
  without improving behavior.

## Review and evidence

Anscombe the 5th / Luna max was assigned the architecture assessment and
returned no conclusion in the bounded window. Banach the 5th / Luna max was
assigned the independent read-only review and also returned no conclusion. No
child PASS is claimed. Parent review is `PASS`; simplification assessment is
`PASS` because the named contract removes positional coupling while leaving
all branch behavior and ownership in place.

Authorized non-destructive evidence:

- `D150-RECOVERY-BRANCH-PROBE=PASS`
- `D150-SOURCE-WIRING-PROBE=PASS`
- `D150-PRESENTATION-AUDIT=PASS`
- `D150-COMPILEALL=PASS`
- `D150-RUFF=PASS`
- `D150-FORMAT=PASS`
- `D150-PACKAGE-BUILD=PASS`
- `D150-PACKAGE-IDENTITY-PROBE=PASS`
- expected release `NO-GO` dossier and no-launch checks.

The packaged candidate is `dist/QuillForge.exe` and the root test copy:

- SHA-256: `28B29A9906539388762D33749BA18A2FFF3142710AA4C5B20482DEB0D734EAD5`
- bytes: `38543581`
- source revision: `tree-sha256:b76f3bb9fea61c77ba0960bc9faf7bab61123d7fc092faeb78d4f42f76ce1ed8`

Public-source applicability is Python 3.12/PyQt6 presentation architecture;
embedded C/C++, MCU, RTOS, and manufacturer requirements do not apply.
Public CloudWeGo material remains an engineering reference only, not a
private ByteDance standard or certification/compliance claim.

## Limits

Branch/contract probes and static/package checks do not prove native Qt event
timing, recovery durability, filesystem behavior, font/DPI metrics,
accessibility, runtime startup, clean-machine or cross-machine behavior,
signing, installer, update, legal clearance, support ownership, or release
readiness. Those gates remain open under the active no-launch/no-release
authorization boundary.

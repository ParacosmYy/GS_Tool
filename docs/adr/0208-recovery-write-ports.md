# ADR-0208: recovery-write Ports contract

- Status: accepted-with-limits
- Date: 2026-08-13
- Delivery: D159 / ARCH-146

## Context

`RecoveryWriteCoordinator` already isolated the callback boundary around one
recovery snapshot write, but its constructor accepted five independent
callbacks. The boundary is high risk because discarded snapshots, capture
abort, document lifecycle, deferred deletion, and saved/failed projections
interleave around asynchronous worker delivery.

## Decision

Introduce the frozen/slotted generic Qt-free `RecoveryWritePorts[JobT, OwnerT]`
contract with named callbacks for document identity, capture abort, pending
delete scheduling, saved projection, and failed projection. Preserve the exact
existing lifecycle order:

- completion: consume discarded -> complete document -> finish write/pending
  delete -> project saved;
- failure: consume discarded -> abort matching capture when present -> complete
  document -> finish write/pending delete -> project failed.

`submit()` continues to bind owner, content version, snapshot ID, and worker
callbacks. MainWindow retains RecoveryService, capture/channel concurrency,
filesystem, tab identity, worker dispatch, notification, close, and policy
ownership. No Qt type enters the coordinator.

## Alternatives rejected

- Keeping positional callbacks would retain avoidable concurrency-path wiring
  risk.
- Moving tracker state, capture abort, durability, or deletion semantics into
  Ports would alter ownership and the recovery lifecycle.
- Introducing a generic event bus or recovery state machine would add a second
  lifecycle abstraction without changing behavior.

## Review and evidence

Goodall the 5th / Terra max was assigned the high-risk architecture assessment
and returned no conclusion in the bounded window. Gibbs the 5th / Sol medium
was assigned the final adversarial architecture review and also returned no
conclusion. Volta the 5th / Luna max was assigned the independent read-only
review and returned no conclusion. No child PASS is claimed. Parent review is
`PASS`; simplification assessment is `PASS` because the immutable named
contract removes positional coupling without adding behavior, concurrency, or
persistence policy.

Authorized non-destructive evidence:

- `D159-RECOVERY-WRITE-BRANCH-PROBE=PASS`
- `D159-COMPLETE-FAIL-DISCARDED-PENDING-ORDER-PROBE=PASS`
- `D159-SUBMIT-BINDING-PROBE=PASS`
- `D159-PORTS-IMMUTABILITY-PROBE=PASS`
- `D159-SOURCE-WIRING-PROBE=PASS`
- `D159-QT-FREE-CONTRACT-PROBE=PASS`
- `D159-PRESENTATION-AUDIT=PASS`
- `D159-COMPILEALL=PASS`
- `D159-RUFF=PASS`
- `D159-FORMAT=PASS`
- `D159-PACKAGE-BUILD=PASS`
- `D159-PACKAGE-IDENTITY-PROBE=PASS`
- expected release `NO-GO` dossier and no-launch checks.

The packaged candidate is `dist/QuillForge.exe` and the root test copy:

- SHA-256: `A77479C01564FC0963BFCC8DF6BC10AE49AF37E5CD4BD9AA7B2D01E3AACB45D4`
- bytes: `38547436`
- source revision: `tree-sha256:f16dd9f996ac90e083c2ad6b8afd4ae612667a5a2b08db483d08186ee9a64677`

Public-source applicability is Python 3.12/PyQt6 presentation architecture;
embedded C/C++, MCU, RTOS, and manufacturer requirements do not apply.
Public CloudWeGo material remains an engineering reference only, not a
private ByteDance standard or certification/compliance claim.

## Limits

Terra, Sol, and independent review windows returned no conclusion; no child
PASS or formal concurrency/durability assurance is claimed. Inline and
static/package checks do not prove native callback timing, capture/write
interleavings, filesystem durability, runtime startup, clean-machine or
cross-machine behavior, signing, installer, update, legal clearance, support
ownership, or release readiness. Those gates remain open under the active
no-launch/no-release authorization boundary.

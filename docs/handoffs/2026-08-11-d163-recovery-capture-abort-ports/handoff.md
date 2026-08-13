# Handoff: 2026-08-11-d163-recovery-capture-abort-ports

| Field | Value |
|---|---|
| ID | `2026-08-11-d163-recovery-capture-abort-ports` |
| Delivery / slice | `D163 / ARCH-150 recovery-capture abort Ports contract` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | `Current local checkout only` |
| Created | `2026-08-11T03:10:00+08:00` |

## User outcome

Recovery capture cancellation and failure now use a named immutable Ports
contract. Existing matching-capture guards, snapshot discarded/released
classification, channel/session cancellation, document completion, and
live-owner notification order remain explicit without changing recovery policy.

## Scope and boundaries

### In scope

- Frozen/slotted generic Qt-free `RecoveryCaptureAbortPorts[JobT, OwnerT]`.
- MainWindow named wiring and preservation of capture release ordering.
- Source, inline, static, compile, package, and traceability evidence.

### Out of scope

- No RecoveryCaptureTracker state model, RecoveryService, channel
  implementation, write/delete coordinator, worker dispatch, filesystem
  durability, tab lifecycle, close policy, notification wording, Qt surface
  behavior, locale/theme/motion projection, or runtime startup change.
- No QApplication/EXE launch, native interleaving, clean-machine,
  cross-machine, signing, installer, updater, legal, support, or release-owner
  evidence.

## Team roles and ownership

| Role | Owner / agent | Result |
|---|---|---|
| Architect | Parfit the 5th / Terra max | `NO_CONCLUSION` after bounded windows; no concurrency assurance |
| Independent review | Ampere the 5th / Terra max | `NO_CONCLUSION` after bounded window; no independent high-risk PASS |
| Final adversarial review | Helmholtz the 5th / Sol medium | `PASS`; no must-fix source behavior or release-order error |
| Parent | Architect | `PASS`; sole writer, integration, review, simplification, and verification |

## Changed files and modules

- `src/quillforge/presentation/recovery_capture_abort_coordinator.py` —
  frozen/slotted generic Ports contract and `_ports` lifecycle callbacks.
- `src/quillforge/presentation/main_window.py` — named Ports construction only.
- `tasks/plan.md` and `tasks/todo.md` — bounded D163 scope and status.
- `docs/adr/0212-recovery-capture-abort-ports.md`.
- `docs/agent-team/reviews/D163-recovery-capture-abort-ports-parent-review.md`.
- `docs/agent-team/reviews/D163-recovery-capture-abort-ports-independent-review.md`.

## Decisions and constraints

- The coordinator owns classification sequencing only; MainWindow retains
  tracker, channel, session, tab, worker, filesystem, notification, and close
  policy ownership.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, inline, package, and static
  evidence are the authorized validation boundary.

## Verification commands and results

- `D163-RECOVERY-CAPTURE-ABORT-BRANCH-PROBE=PASS`.
- `D163-RECOVERY-CAPTURE-ABORT-ORDER-GUARD-PROBE=PASS`.
- `D163-PORTS-IMMUTABILITY-PROBE=PASS`.
- `D163-SOURCE-WIRING-PROBE=PASS`.
- `D163-QT-FREE-CONTRACT-PROBE=PASS`.
- `D163-GLOBAL-PRESENTATION-PORTS-INVENTORY=PASS` — 43 frozen/slotted Ports
  contracts; zero legacy positional coordinator constructors.
- `D163-PRESENTATION-AUDIT=PASS`.
- `D163-COMPILEALL=PASS`.
- `D163-RUFF=PASS`.
- `D163-FORMAT=PASS`.
- `D163-PACKAGE-BUILD=PASS`.
- `D163-PACKAGE-IDENTITY-PROBE=PASS`.
- `D163-SOL-ADVERSARIAL-REVIEW=PASS`.
- Expected release `NO-GO`; handoff/index/register and no-launch checks are
  recorded.

## Unrun checks and reason

- Terra architecture and independent high-risk conclusions — bounded windows
  timed out; recorded as `NO_CONCLUSION`, not PASS. Sol final adversarial
  review passed the bounded source/order audit but is not runtime evidence.
- QApplication/native callback interleavings, thread scheduling, channel
  timing, filesystem durability, runtime startup, clean-machine, cross-machine,
  signing, installer, updater, legal, support, and release-owner checks —
  prohibited or outside current authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static/inline evidence cannot prove callback interleavings, thread timing,
  channel cancellation, filesystem durability, or actual packaged startup.
- No formal concurrency, safety, certification, or release-readiness claim is
  made; the candidate remains unsigned and release remains NO-GO.

## Acceptance and evidence IDs

- Acceptance: `S216`, `D163-AC01`.
- Evidence: ADR-0212, parent/independent/adversarial review records, D163
  probes, static checks, package manifest, handoff/index/register checks,
  expected release NO-GO, and explicit concurrency/runtime limits.

## Next owner and next action

- Owner: Architect.
- Action: re-evaluate remaining presentation boundaries only after this
  high-risk slice receives an independent conclusion or explicit escalation;
  complete authorized runtime/release gates when authority permits.

## Artifact information

The candidate was rebuilt after the Ports-contract change without launching
QuillForge:

- Artifact: `dist/QuillForge.exe` and `QuillForge.exe`.
- SHA-256: `4C6188F655EDB31C58E835FEBBC04DA21302F480E2516D916E9D140D94C43BD5`.
- Size: `38547913` bytes.
- Source revision: `tree-sha256:89f8e174586c4a4e5d2460668cc5bf23eef5327c8f32ab743e3c3acf121a39cf`.
- Manifest: `dist/QuillForge.release.json`.

## Disposition

`accepted-with-limits`: recovery capture abort now has a named immutable
contract with unchanged identity, snapshot, channel, session, document, and
notification sequence; native callback interleavings, concurrency, durability,
runtime, release, and external evidence gates remain open.

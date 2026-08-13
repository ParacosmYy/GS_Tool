# Handoff: 2026-08-13-d159-recovery-write-ports

| Field | Value |
|---|---|
| ID | 2026-08-13-d159-recovery-write-ports |
| Delivery / slice | D159 / ARCH-146 recovery-write Ports contract |
| Status | `accepted-with-limits` |
| Owner | architect |
| Checkout | Current local checkout only |
| Created | 2026-08-13T00:45:00+08:00 |

## User outcome

Recovery-write callback projection now exposes its existing high-risk lifecycle
through a generic named immutable Ports contract. Discarded classification,
capture abort, document release, deferred deletion, and saved/failed projection
order remain explicit and unchanged.

## Scope and boundaries

### In scope

- Frozen/slotted generic Qt-free `RecoveryWritePorts[JobT, OwnerT]` contract.
- Submit identity binding and complete/fail/discarded/pending order
  preservation.
- MainWindow named wiring, source, inline, static, compile, package, and
  traceability evidence.

### Out of scope

- No RecoveryService, capture channel, worker dispatch, filesystem durability,
  tracker state model, recovery projection policy, delete service, tab
  lifecycle, close policy, notification wording, Qt surface,
  locale/theme/motion projection, or runtime-startup change.
- No QApplication/EXE launch, native rendering, clean-machine, cross-machine,
  signing, installer, updater, legal, support, or release-owner evidence.

## Team roles and ownership

| Role | Owner / agent | Result |
|---|---|---|
| Architect | Goodall the 5th / Terra max | `NO_CONCLUSION` after bounded window; no architecture PASS |
| Final adversarial architecture | Gibbs the 5th / Sol medium | `NO_CONCLUSION` after bounded window; no final PASS |
| Independent review | Volta the 5th / Luna max | `NO_CONCLUSION` after bounded window; no independent PASS |
| Parent | Architect | `PASS`; sole writer, integration, review, simplification, and verification |

## Changed files and modules

- `src/quillforge/presentation/recovery_write_coordinator.py` — generic
  frozen/slotted named Ports and preserved high-risk lifecycle order.
- `src/quillforge/presentation/main_window.py` — named Ports construction only.
- `tasks/plan.md` and `tasks/todo.md` — bounded D159 scope and status.
- `docs/adr/0208-recovery-write-ports.md`
- `docs/agent-team/reviews/D159-recovery-write-ports-parent-review.md`
- `docs/agent-team/reviews/D159-recovery-write-ports-independent-review.md`

## Decisions and constraints

- The coordinator owns only callback binding and lifecycle projection;
  recovery persistence, capture/channel concurrency, filesystem, tracker,
  delete, tab, notification, worker, and close policy remain outside it.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, inline, package, and static
  evidence are the authorized validation boundary.

## Verification commands and results

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
- expected release `NO-GO` and no-launch/traceability checks.

## Unrun checks and reason

- Terra, Sol, and independent conclusions — bounded child windows timed out;
  recorded as `NO_CONCLUSION`, not PASS.
- QApplication/native callback timing, capture/write interleavings,
  filesystem durability, runtime startup, clean-machine, cross-machine,
  signing, installer, updater, legal, support, and release owner checks —
  prohibited or outside current authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- This is the highest-risk slice in the current sequence: static/inline
  evidence cannot prove queued callback timing, capture/write interleavings,
  or filesystem durability.
- The candidate remains unsigned and release remains NO-GO while report,
  clean-machine, legal, installer/update, and release-owner gates are open.

## Acceptance and evidence IDs

- Acceptance: `S212`, `D159-AC01`.
- Evidence: ADR-0208, parent/independent/Terra/Sol review records, D159
  probes, static checks, package manifest, handoff/index/register checks,
  expected release NO-GO, and explicit runtime/concurrency limits.

## Next owner and next action

- Owner: Architect.
- Action: continue only with a separately bounded architecture slice; obtain
  authorized runtime/durability evidence before claiming recovery release
  readiness.

## Artifact information

The candidate was rebuilt after the Ports-contract change without launching
QuillForge:

- Artifact: `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `A77479C01564FC0963BFCC8DF6BC10AE49AF37E5CD4BD9AA7B2D01E3AACB45D4`
- Size: `38547436` bytes
- Source revision: `tree-sha256:f16dd9f996ac90e083c2ad6b8afd4ae612667a5a2b08db483d08186ee9a64677`
- Manifest: `dist/QuillForge.release.json`

## Disposition

`accepted-with-limits`: recovery-write projection now has a generic named
immutable contract with unchanged completion/failure/discarded/pending order;
native concurrency, durability, runtime, release, and external evidence gates
remain open and no formal concurrency assurance is claimed.

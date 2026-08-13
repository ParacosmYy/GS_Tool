# Handoff: 2026-08-10-d109-recovery-scan-dispatch

| Field | Value |
|---|---|
| ID | `2026-08-10-d109-recovery-scan-dispatch` |
| Delivery / slice | `D109 / ARCH-81 recovery-scan dispatch callback boundary` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-10T23:45:00+08:00` |

## User outcome

Recovery inventory scanning now has one typed callback-binding path. The
existing Qt-free `RecoveryScanCoordinator` binds the scan job to its established
result/failure lifecycle while MainWindow retains RecoveryService, operation
IDs, TaskRunner, startup/manual context, session continuation, notification,
and close policy.

## Scope and boundaries

### In scope

- Add typed scan operation/success/failure/dispatcher contracts.
- Add `RecoveryScanCoordinator.submit(...)` for callback binding and generic
  dispatch.
- Route MainWindow recovery scan through the existing lifecycle owner.
- Preserve stale suppression, invalid inventory handling, manual empty-result
  feedback, startup continuation, and synchronous dispatcher exception behavior.
- Record public-source applicability, parent review, independent review status,
  simplification, static, inline, package, and release-limit evidence.

### Out of scope

- No RecoveryService, recovery schema/store, scan algorithm, tracker policy,
  startup state, session restore, notification contract, TaskRunner
  implementation, persistence, or close policy changed.
- No new coordinator, state owner, signal, dependency, or test-only asset was
  introduced.
- No QApplication launch, worker timing, filesystem durability,
  crash/restart, screenshot, clean-machine, cross-machine, signing,
  installer, updater, legal, support, or release-owner evidence.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Nash the 3rd / Luna max | Read-only D109 boundary consultation; `NO_CONCLUSION` after bounded wait |
| Independent review | Raman the 3rd / Luna max | Read-only D109 review; `NO_CONCLUSION` after bounded wait |
| Parent | Architect | Sole writer, integration, source review, simplification, packaging, and verification |

No child PASS is claimed.

## Changed files and modules

- `src/quillforge/presentation/recovery_scan_coordinator.py` — typed
  dispatcher contract and callback-binding method.
- `src/quillforge/presentation/main_window.py` — recovery scan now uses the
  coordinator boundary and no longer owns duplicate scan closures.
- `docs/adr/0136-recovery-scan-dispatch-callback-boundary.md` — decision,
  invariants, alternatives, applicability, review, simplification, and limits.
- `docs/agent-team/reviews/D109-arch-81-recovery-scan-parent-review.md` — parent
  review.
- `docs/agent-team/reviews/D109-arch-81-recovery-scan-independent-review.md` —
  independent `NO_CONCLUSION` record.
- `docs/specs/enterprise-architecture-migration.md`, `docs/ARCHITECTURE.md`,
  `docs/ROADMAP.md`, `tasks/plan.md`, `tasks/todo.md`.

## Decisions and constraints

- The existing `RecoveryScanCoordinator` remains the sole inventory lifecycle
  owner; no parallel generic task abstraction was added.
- MainWindow continues to allocate operation IDs, choose RecoveryService
  operations, pass TaskRunner, distinguish startup/manual scan, and control
  close.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, inline, package, and
  non-launching static evidence are the permitted validation boundary.
- This is Python/PyQt6 desktop code. Embedded C/C++ assurance and vendor
  manufacturer requirements are `N/A`.
- Public CloudWeGo material remains an engineering reference only; no private
  ByteDance standard, certification, or compliance claim is made.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `D109-RECOVERY-SCAN-DISPATCH-PROBE=PASS` | `PASS` | Inline production-class probe covered success, failure, stale suppression, manual empty result, startup continuation, and synchronous dispatcher exception propagation. |
| `D109-SOURCE-DEPENDENCY-PROBE=PASS` | `PASS` | Typed submit is present, duplicate MainWindow scan closures are absent, and the coordinator has no Qt/TaskRunner/RecoveryService/widget imports. |
| `uv run python -m compileall -q src/quillforge` | `PASS` | Static compilation only; no QApplication launch. |
| `uv run ruff check src/quillforge` | `PASS` | All checks passed. |
| `uv run ruff format --check src/quillforge` | `PASS` | 119 files already formatted. |
| `D109-PACKAGE-IDENTITY-PROBE=PASS` | `PASS` | Root/dist SHA `1179D391EE3A5E9ABCC86BD3483CBAA4FFA8EA8BCE4EC85F339F9D6B1DE87F85`; 38,492,548 bytes; source `tree-sha256:6467b0de77e9dcc6e200727f7b489954dd9b28b4e01a9a8fe7b13d25e7dabc84`. |
| `D109-NO-LAUNCH-PROBE=PASS` | `PASS` | Package completed and no QuillForge process was running afterward. |
| `D109-JSON-TRACEABILITY-PROBE=PASS` | `PASS` | Acceptance, delivery register, handoff index, and release manifest point to D109/current identity. |
| `D109-RELEASE-DOSSIER-PROBE=PASS` | `PASS` | Current dossier is `no-go`, bound to the D109 artifact, with 10 open gates and the three known mechanical report-binding failures. |
| `D109-RELEASE-EXPECTED-NO-GO=PASS` | `PASS` | `verify_release_handoff.ps1` remains intentionally non-zero because authorized runtime/report refresh and release gates are still open. |
| `scripts\verify_handoff.ps1` | `PASS` | Handoff status/index contract passed. |
| `scripts\check.ps1` | `PASS` | Notice, workflow, acceptance, architecture-boundary, presentation-contract, lock, Ruff, and compile checks passed. |

## Unrun checks and reason

- Native TaskRunner queued timing, recovery filesystem durability,
  crash/restart recovery, startup, screen-reader output, DPI, screenshot,
  clean-machine, cross-machine, hardware, signing, installer, updater, legal,
  support, and release-owner checks — prohibited or outside current
  authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static/inline probes prove callback shape and coordinator lifecycle behavior,
  not native worker scheduling or durable recovery filesystem behavior.
- Nash and Raman review windows returned `NO_CONCLUSION`; no child PASS is
  claimed. Parent source review and simplification assessment are recorded.
- The portable candidate remains unsigned and release remains `NO-GO`; known
  report-binding failures and external release gates remain open.

## Acceptance and evidence IDs

- Acceptance: `S140`, `D109-AC01`.
- Evidence: ADR-0136, D109 source/dispatch probes, parent/independent review
  records, compile/lint/format checks, package identity, handoff/index/register
  checks, expected release NO-GO, and explicit runtime limits.

## Next owner and next action

- Owner: Architect.
- Action: continue the next bounded MainWindow/application contract slice and
  complete authorized runtime/release gates when authority and environment
  permit.

## Artifact information

The portable candidate was rebuilt without launching QuillForge:

- Artifact: `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `1179D391EE3A5E9ABCC86BD3483CBAA4FFA8EA8BCE4EC85F339F9D6B1DE87F85`
- Size: `38,492,548` bytes
- Source revision: `tree-sha256:6467b0de77e9dcc6e200727f7b489954dd9b28b4e01a9a8fe7b13d25e7dabc84`
- Manifest: `dist/QuillForge.release.json`

## Disposition

`accepted-with-limits`: duplicate recovery-scan callback ownership is
consolidated behind the existing Qt-free coordinator, while native timing,
durability, runtime, and enterprise release gates remain open.

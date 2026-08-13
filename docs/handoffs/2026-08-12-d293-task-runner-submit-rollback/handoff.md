# Handoff: 2026-08-12-d293-task-runner-submit-rollback

| Field | Value |
|---|---|
| ID | `2026-08-12-d293-task-runner-submit-rollback` |
| Delivery / slice | `D293 / ARCH-263 TaskRunner submission rollback boundary` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-12T23:30:00+08:00` |

## User outcome

The repaired candidate no longer retains a failed TaskRunner submission as
phantom pending work when Qt signal connection or pool startup raises. Existing
success/failure callback behavior remains intact, and a late completion after a
partial start cannot duplicate the pending-state transition.

## Scope and boundaries

### In scope

- TaskRunner submission failure rollback.
- Shared idempotent release and pending-state observability.
- Targeted source contract, package rebuild, and non-destructive diagnostics.

### Out of scope

- Forced worker termination, synchronous thread-pool waits, runtime reuse,
  native EXE/Qt launch, clean-machine startup, and release-owner gates.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | parent; Luna/max consultation | Integration, lifecycle boundary, final review, and handoff decision |
| Project Manager | parent record | Plan, dependencies, risks, and status |
| Product | parent record | User outcome and acceptance |
| Developer 1 | parent | TaskRunner implementation |
| Developer 2 | parent | Static contract and packaging integration |
| QA | parent | Source, package, and non-destructive verification |

## Changed files and modules

- `src/quillforge/presentation/task_runner.py` — rollback failed submission retention and centralize idempotent release.
- `scripts/audit_presentation_contracts.py` — guard the rollback/shared-release contract.
- `docs/adr/0329-task-runner-submit-rollback.md` — decision and limits.
- `docs/agent-team/reviews/D293-task-runner-submit-rollback-parent-review.md` — parent review and simplification.
- `docs/agent-team/reviews/D293-task-runner-submit-rollback-independent-review.md` — bounded independent-review result.

## Decisions and constraints

- Preserve task registration-before-start ordering and callback-before-release
  semantics.
- Do not wait for, terminate, or forcibly suppress worker threads/callbacks.
- Shared checkout writer: parent only; no worktree or parallel writer was used.
- Runtime launch policy: EXE/Qt startup was not allowed; source, static,
  package, archive, and no-window diagnostic evidence were authorized.

## Review and source applicability

Architecture consultation: `NO_CONCLUSION` after three bounded waits.
Independent review: `NO_CONCLUSION` after three bounded waits. Parent review:
`PASS`. Simplification assessment: `PASS`.

This is Python 3.12/PyQt6 desktop code. Public embedded-vendor source
applicability is N/A; no manufacturer, MCU, SDK, RTOS, MISRA, ISO 26262,
ASPICE, certification, or private ByteDance-standard claim is made.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `uv run python -m compileall -q src scripts` | PASS | Source compiles. |
| `uv run ruff check src scripts` | PASS | Ruff clean. |
| `uv run ruff format --check src scripts` | PASS | 149 files already formatted. |
| `uv run python scripts/audit_presentation_contracts.py` | PASS | Worker rollback and existing presentation contracts pass. |
| `scripts/check.ps1` | PASS | Handoff, audit, formatting, and project checks pass. |
| `uv run python -m quillforge --diagnose-startup --report .diagnostics-d293-startup.json` | PASS | Runtime composition/startup restore passed; no window/exec entered. |
| `uv run python -m quillforge --diagnose-file-open .\README.md --report .diagnostics-d293-file-open.json` | PASS | Regular file reached one startup-open tab; no window/exec entered. |
| `scripts/package.ps1` | PASS | Portable EXE and root copy rebuilt with matching identity. |
| PE/archive inspection | PASS | AMD64, PE32+, Windows GUI, required 9 archive entries present. |
| `scripts/verify_release_handoff.ps1` | EXPECTED NO-GO | Native artifact-bound reports and remaining release gates remain open. |

## Unrun checks and reason

- Native EXE/Qt startup, normal close, and real late queued-callback timing —
  prohibited by the active no-launch policy.
- Clean-machine startup, signing, installer, updater, shell integration, and
  release-owner acceptance — external evidence remains open.
- Unit tests or test-only assets — prohibited by project policy and not needed
  for this source/static delivery slice.

## Known risks and limits

- Independent review returned `NO_CONCLUSION`; no child approval is claimed.
- A pool start failure after the task has actually been accepted may still
  deliver a late callback; the shared release makes the observation idempotent,
  but this slice does not cancel or suppress that callback.
- `_Task.run()` exception policy is unchanged and remains a separate lifecycle
  decision.

## Acceptance and evidence IDs

- Acceptance: `S333`
- Evidence: `D293-TASK-ROLLBACK-CONTRACT=PASS`,
  `D293-SOURCE-DIAGNOSTIC=PASS startup=passed file_open=passed`,
  `D293-COMPILEALL=PASS`, `D293-RUFF=PASS`, `D293-FORMAT=PASS`,
  `D293-CHECK=PASS`, `D293-PACKAGE-IDENTITY=PASS`.

## Next owner and next action

- Owner: `Project Manager (QuillForge)`
- Action: retain the rebuilt D293 candidate and authorize native/clean-machine
  evidence before release consideration.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and `QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size: `8319A2D74A6D80044CBE2C3ED54AB1987E0350D18A9F443356882C02C2825589` / `38590010` bytes
- Source revision: `tree-sha256:b633ad84ea7256ec050f6b2aeaabdc1ca33142734766ac7924fb6d5c36677cdf`
- Packaging note: rebuilt portable one-file candidate; root copy matches.

## Disposition

Accepted with limits. D293 is source/package verified; independent review,
native startup, real worker teardown, and remaining release gates remain open.

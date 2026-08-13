# Handoff: 2026-08-12-d295-task-runner-abnormal-termination

| Field | Value |
|---|---|
| ID | `2026-08-12-d295-task-runner-abnormal-termination` |
| Delivery / slice | `D295 / ARCH-265 TaskRunner abnormal termination boundary` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-13T00:30:00+08:00` |

## User outcome

The worker boundary no longer reports a non-`Exception` abnormal termination
as a successful operation with a `None` result. Such termination now travels
through the existing recoverable failure callback while retained-task cleanup
and pending-state projection remain intact.

## Scope and boundaries

### In scope

- `_Task.run()` abnormal termination normalization.
- Existing `Exception` callback compatibility and pending release.
- Static contract, source diagnostics, package rebuild, and handoff evidence.

### Out of scope

- Widening coordinator APIs, global interrupt policy, forced worker
  termination, native EXE launch, clean-machine startup, and direct injected
  runtime verification.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | parent; Luna/max consultation | Integration, worker boundary, final review, and handoff decision |
| Project Manager | parent record | Plan, dependencies, risks, and status |
| Product | parent record | User outcome and acceptance |
| Developer 1 | parent | TaskRunner implementation |
| Developer 2 | parent | Static contract and packaging integration |
| QA | parent | Source, package, and non-destructive verification |

## Changed files and modules

- `src/quillforge/presentation/task_runner.py` — normalize non-`Exception` worker termination into the existing failure interface.
- `scripts/audit_presentation_contracts.py` — guard the abnormal-termination conversion contract.
- `docs/adr/0331-task-runner-abnormal-termination.md` — decision and limits.
- `docs/agent-team/reviews/D295-task-runner-abnormal-termination-parent-review.md` — parent review and simplification.
- `docs/agent-team/reviews/D295-task-runner-abnormal-termination-independent-review.md` — bounded independent-review result.

## Decisions and constraints

- Keep all existing coordinator failure types as `Exception`.
- Preserve completion signaling and release order; do not force-stop workers.
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
| `uv run python scripts/audit_presentation_contracts.py` | PASS | Abnormal termination and existing presentation contracts pass. |
| `scripts/check.ps1` | PASS | Handoff, audit, formatting, and project checks pass. |
| `uv run python -m quillforge --diagnose-startup --report .diagnostics-d295-startup.json` | PASS | Startup composition/restore passed; no window/exec entered. |
| `uv run python -m quillforge --diagnose-file-open .\README.md --report .diagnostics-d295-file-open.json` | PASS | Regular file reached one startup-open tab; no window/exec entered. |
| `scripts/package.ps1` | PASS | Portable EXE and root copy rebuilt with matching identity. |
| PE/archive inspection | PASS | AMD64, PE32+, Windows GUI, required 9 archive entries present. |
| `scripts/verify_release_handoff.ps1` | EXPECTED NO-GO | Native artifact-bound reports and remaining release gates remain open. |

## Unrun checks and reason

- Direct `BaseException` injection, unit tests, mocks, fixtures, and harnesses —
  excluded by project verification policy.
- Native EXE/Qt startup, normal close, and real thread timing — prohibited by
  the active no-launch policy.
- Clean-machine startup, signing, installer, updater, shell integration, and
  release-owner acceptance — external evidence remains open.

## Known risks and limits

- Independent review returned `NO_CONCLUSION`; no child approval is claimed.
- The wrapper makes abnormal worker termination recoverable but does not add
  structured logging or a dedicated localized message.
- Native thread scheduling and frozen-package behavior remain unverified.

## Acceptance and evidence IDs

- Acceptance: `S335`
- Evidence: `D295-ABNORMAL-TERMINATION-CONTRACT=PASS`,
  `D295-SOURCE-DIAGNOSTIC=PASS startup=passed file_open=passed`,
  `D295-COMPILEALL=PASS`, `D295-RUFF=PASS`, `D295-FORMAT=PASS`,
  `D295-CHECK=PASS`, `D295-PACKAGE-IDENTITY=PASS`.

## Next owner and next action

- Owner: `Project Manager (QuillForge)`
- Action: retain the rebuilt D295 candidate and authorize native/clean-machine
  evidence before release consideration.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and `QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size: `5654A9F92279E892004D8992AD4EDE90248CF92F25A60C28E7B809B72E973ECB` / `38593735` bytes
- Source revision: `tree-sha256:88e1f21be0877ef8ba66500c41cc5aa9102e241a0506bd8ccf2f4784fd2b4478`
- Packaging note: rebuilt portable one-file candidate; root copy matches.

## Disposition

Accepted with limits. D295 is source/package verified; independent review,
direct abnormal-termination injection, native startup, real worker timing,
and remaining release gates remain open.

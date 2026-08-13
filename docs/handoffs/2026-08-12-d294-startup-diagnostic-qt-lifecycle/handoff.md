# Handoff: 2026-08-12-d294-startup-diagnostic-qt-lifecycle

| Field | Value |
|---|---|
| ID | `2026-08-12-d294-startup-diagnostic-qt-lifecycle` |
| Delivery / slice | `D294 / ARCH-264 Startup diagnostic QApplication lifecycle` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-12T23:55:00+08:00` |

## User outcome

The source startup diagnostic no longer creates sequential Qt application
instances that trigger the repeated `Qt6111ThemeChangeObserverWindow` class
registration warning. Both runtime-composition and startup-restore probes now
share one diagnostic-owned QApplication lifecycle.

## Scope and boundaries

### In scope

- One QApplication lifetime for startup diagnostics.
- Ownership-sensitive cleanup and static contract coverage.
- Source diagnostics, project checks, package rebuild, and archive identity.

### Out of scope

- Native EXE launch, interactive window rendering, clean-machine startup,
  forced worker termination, and TaskRunner BaseException policy.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | parent; Luna/max consultation | Integration, lifecycle boundary, final review, and handoff decision |
| Project Manager | parent record | Plan, dependencies, risks, and status |
| Product | parent record | User outcome and acceptance |
| Developer 1 | parent | Diagnostic lifecycle implementation |
| Developer 2 | parent | Static contract and packaging integration |
| QA | parent | Source, package, and non-destructive verification |

## Changed files and modules

- `src/quillforge/app.py` — hold one diagnostic QApplication across Qt probes and clean it in `finally` when owned.
- `scripts/audit_presentation_contracts.py` — guard application ownership and probe/cleanup ordering.
- `docs/adr/0330-startup-diagnostic-qt-lifecycle.md` — decision and limits.
- `docs/agent-team/reviews/D294-startup-diagnostic-qt-lifecycle-parent-review.md` — parent review and simplification.
- `docs/agent-team/reviews/D294-startup-diagnostic-qt-lifecycle-independent-review.md` — bounded independent-review result.

## Decisions and constraints

- Keep the normal desktop entrypoint unchanged and keep diagnostics no-window.
- Do not mask the issue with an internal platform override or global singleton.
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
| `uv run python scripts/audit_presentation_contracts.py` | PASS | Diagnostic lifecycle and existing presentation contracts pass. |
| `scripts/check.ps1` | PASS | Handoff, audit, formatting, and project checks pass. |
| `uv run python -m quillforge --diagnose-startup --report .diagnostics-d294-after-scope-final.json` | PASS | All checks passed; no repeated window-class warning; no window/exec entered. |
| `uv run python -m quillforge --diagnose-file-open .\README.md --report .diagnostics-d294-file-final.json` | PASS | Regular file reached one startup-open tab; no window/exec entered. |
| `scripts/package.ps1` | PASS | Portable EXE and root copy rebuilt with matching identity. |
| PE/archive inspection | PASS | AMD64, PE32+, Windows GUI, required 9 archive entries present. |
| `scripts/verify_release_handoff.ps1` | EXPECTED NO-GO | Native artifact-bound reports and remaining release gates remain open. |

## Unrun checks and reason

- Native EXE/Qt startup, normal close, and interactive window rendering —
  prohibited by the active no-launch policy.
- Clean-machine startup, signing, installer, updater, shell integration, and
  release-owner acceptance — external evidence remains open.
- Unit tests or test-only assets — prohibited by project policy and not added.

## Known risks and limits

- Independent review returned `NO_CONCLUSION`; no child approval is claimed.
- This change proves the source diagnostic lifecycle only; it does not prove the
  frozen EXE can start on a clean machine.
- TaskRunner `BaseException` behavior remains an explicit future review surface.

## Acceptance and evidence IDs

- Acceptance: `S334`
- Evidence: `D294-QT-DIAGNOSTIC-SCOPE=PASS one_application=1 warning=0`,
  `D294-SOURCE-DIAGNOSTIC=PASS startup=passed file_open=passed`,
  `D294-COMPILEALL=PASS`, `D294-RUFF=PASS`, `D294-FORMAT=PASS`,
  `D294-CHECK=PASS`, `D294-PACKAGE-IDENTITY=PASS`.

## Next owner and next action

- Owner: `Project Manager (QuillForge)`
- Action: retain the rebuilt D294 candidate and authorize native/clean-machine
  evidence before release consideration.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and `QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size: `FD81CABD6D35326803AB63A5E5291770D09CD0FC05ED2ABC30E27FC046054686` / `38592323` bytes
- Source revision: `tree-sha256:99a32cddba6dbb384b92e294c2ff8cf7d018d69d874fb790dce55906d04738e3`
- Packaging note: rebuilt portable one-file candidate; root copy matches.

## Disposition

Accepted with limits. D294 is source/package verified; independent review,
native startup, clean-machine behavior, and remaining release gates remain
open.

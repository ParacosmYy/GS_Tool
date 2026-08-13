# Handoff: 2026-08-13-d297-frozen-qt-runtime-preflight

| Field | Value |
|---|---|
| ID | `2026-08-13-d297-frozen-qt-runtime-preflight` |
| Delivery / slice | `D297 / ARCH-267 Frozen Qt runtime preflight` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-13T02:30:00+08:00` |

## User outcome

The normal frozen GUI entry path now checks its own Qt platform plugin and
required runtime files before QApplication construction. If the one-file
bundle is incomplete, the existing startup fallback can show/log the exact
missing bundle paths instead of leaving the user with an unexplained failure.

## Scope and boundaries

### In scope

- Frozen-only normal GUI preflight before QApplication import.
- Reuse of existing Qt plugin/runtime dependency checks.
- Actionable `RuntimeError` projection through the existing startup boundary.
- Static audit, no-Qt simulation, source diagnostics, package rebuild, PE/
  archive identity, and handoff evidence.

### Out of scope

- Native EXE/Qt startup, clean-machine behavior, real platform-loader failure,
  visual rendering, installer, signing, updater, file associations, and
  release-gate closure.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | parent; Luna/max consultation | Integration, boundary decision, parent review, and handoff |
| Project Manager | parent record | Plan, dependencies, risks, and status |
| Product | parent record | Startup reliability outcome and acceptance |
| Developer 1 | parent | Application-entry implementation |
| Developer 2 | parent | Static contract and package integration |
| QA | parent | Source, simulation, archive, identity, and non-destructive verification |

## Changed files and modules

- `src/quillforge/app.py` — add the frozen-only pre-QApplication validation.
- `scripts/audit_presentation_contracts.py` — guard branch ordering, placement,
  and reuse of dependency checks.
- ADR, review, acceptance, delivery-register, roadmap, todo, plan, and handoff
  records for D297.

## Decisions and constraints

- Keep the validation at the application entry boundary and after explicit
  plugin-host/diagnostic dispatch; do not couple diagnostics to normal startup.
- Reuse the existing frozen dependency inventories; do not maintain a second
  packaging manifest in application code.
- Shared checkout writer: parent only; no worktree or parallel writer was used.
- Runtime launch policy: EXE/Qt startup was not allowed; source, static,
  package, archive, and no-Qt simulation evidence were authorized.

## Review and source applicability

Architecture consultation: `NO_CONCLUSION` after three bounded waits.
Independent review: `NO_CONCLUSION` after three bounded waits. Parent review:
`PASS`. Simplification assessment: `PASS`.

This is Python 3.12/PyQt6/PyInstaller desktop code. Public embedded-vendor
source applicability is N/A; no manufacturer, MCU, SDK, RTOS, MISRA, ISO 26262,
ASPICE, certification, or private ByteDance-standard claim is made.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `uv run python -m compileall -q src scripts` | PASS | Source compiles. |
| `uv run ruff check src scripts` | PASS | Ruff clean. |
| `uv run ruff format --check src scripts` | PASS | 149 files formatted. |
| `uv run python scripts/audit_presentation_contracts.py` | PASS | Pre-QApplication/branch contract passes. |
| `scripts/check.ps1` | PASS | Handoff, audit, formatting, and project checks pass before packaging. |
| source `--diagnose-startup` | PASS | Composition/restore passed; no window/exec entered. |
| source `--diagnose-file-open README.md` | PASS | One startup path opened; no window/exec entered. |
| no-Qt frozen helper simulation | PASS | Bundle success and missing-dependency branches passed. |
| `scripts/package.ps1` | PASS | Root/dist copies match. |
| PE/archive inspection | PASS | AMD64, PE32+, Windows GUI, Qt DLLs, qwindows, QScintilla, and modules present. |
| `scripts/verify_release_handoff.ps1` | EXPECTED NO-GO | Native artifact-bound reports and enterprise gates remain open. |

## Unrun checks and reason

- Native EXE/Qt startup, clean-machine startup, and real platform-loader
  conflict reproduction — prohibited by `software_start_allowed=false`.
- Unit tests, mocks, fixtures, and test harnesses — excluded by project policy.
- Signing, installer, updater, registry, cross-machine, and release-owner
  evidence — outside this authorized local slice.

## Known risks and limits

- Independent review returned `NO_CONCLUSION`; no child approval is claimed.
- Static/archive and no-Qt simulation evidence cannot prove Qt loader behavior
  on every Windows machine or under antivirus/temp-directory pressure.
- Release remains no-go until artifact-bound runtime and enterprise gates are
  refreshed by an authorized operator.

## Acceptance and evidence IDs

- Acceptance: `S337`
- Evidence: `D297-FROZEN-QT-RUNTIME-PREFLIGHT=PASS`, package identity, source
  diagnostics, no-Qt simulation, PE/archive checks, and review records.

## Next owner and next action

- Owner: `Project Manager (QuillForge)`
- Action: authorize native and clean-machine startup evidence for the exact
  D297 artifact before release consideration.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and `QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size: `51973E09AB443FB83E6180CEFC5D4EE3B6A5F596C4C4727A2C3DF817DC962123` / `38593850` bytes
- Source revision: `tree-sha256:44fe1672603d11b1d311ec4a110f428e88c34b686c23414b6bb21ead0c39a4bc`

## Disposition

Accepted with limits. D297 is source/package/archive/simulation verified;
independent review, native startup, clean-machine behavior, and remaining
release gates remain open.

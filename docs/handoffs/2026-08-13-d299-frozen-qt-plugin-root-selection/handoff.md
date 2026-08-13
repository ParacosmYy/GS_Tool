# Handoff: 2026-08-13-d299-frozen-qt-plugin-root-selection

| Field | Value |
|---|---|
| ID | `2026-08-13-d299-frozen-qt-plugin-root-selection` |
| Delivery / slice | `D299 / ARCH-269 Frozen Qt plugin root selection` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-13T04:30:00+08:00` |

## User outcome

Frozen startup now prefers the Qt plugin directory that actually contains
`platforms/qwindows.dll`. A partial Qt6 plugin directory can no longer mask a
complete supported fallback directory, while normal Qt6 bundles retain their
existing selection. Plugin configuration and startup diagnostics use the same
selection rule.

## Scope and boundaries

### In scope

- Shared qwindows-bearing selection across frozen plugin configuration and
  startup diagnostics.
- Qt6-first/legacy fallback ordering and deterministic incomplete-bundle
  behavior.
- Static contract, source diagnostics, selector simulation, package rebuild,
  PE/archive identity, and handoff evidence.

### Out of scope

- Generating a physical split-layout bundle, native EXE/Qt startup,
  clean-machine behavior, real Windows DLL loading, visual rendering,
  installer, signing, updater, file associations, and release-gate closure.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | parent; Luna/max consultation | Integration, boundary decision, parent review, and handoff |
| Project Manager | parent record | Plan, dependencies, risks, and status |
| Product | parent record | Startup compatibility outcome and acceptance |
| Developer 1 | parent | Frozen plugin selector implementation |
| Developer 2 | parent | Static contract and package integration |
| QA | parent | Source, report, archive, identity, and non-destructive verification |

## Changed files and modules

- `src/quillforge/app.py` — centralize qwindows-bearing plugin-root selection.
- `scripts/audit_presentation_contracts.py` — guard selector reuse and the
  platform-plugin predicate.
- ADR, review, acceptance, delivery-register, roadmap, todo, plan, and handoff
  records for D299.

## Decisions and constraints

- Keep plugin-root policy at the existing application-entry infrastructure
  boundary; do not duplicate it in presentation or domain code.
- Prefer an actually usable platform-plugin root, then preserve deterministic
  diagnostics when all candidates are incomplete.
- Shared checkout writer: parent only; no worktree or parallel writer was
  used.
- Runtime launch policy: EXE/Qt startup was not allowed; source, static,
  package, archive, and in-process simulation evidence were authorized.

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
| `uv run python scripts/audit_presentation_contracts.py` | PASS | Shared selector contract passes. |
| `scripts/check.ps1` | PASS | Project checks pass before packaging. |
| source `--diagnose-startup --report ...` | PASS | Composition/restore passed; no window/exec entered. |
| source `--diagnose-file-open README.md --report ...` | PASS | One startup path opened; no window/exec entered. |
| selector simulation | PASS | Current Qt6 selection and broken-Qt6/complete-legacy fallback selection passed. |
| root `QuillForge.exe` / `dist/QuillForge.exe` identity | PASS | Copies and manifest match. |
| PE/archive inspection | PASS | AMD64, PE32+, Windows GUI, Qt6 DLLs, qwindows, QScintilla, runtime hook, and modules present. |
| `scripts/verify_release_handoff.ps1` | EXPECTED NO-GO | Native reports and enterprise gates remain open. |

## Unrun checks and reason

- Physical split-layout bundle, native EXE/Qt startup, clean-machine startup,
  and real DLL-loader behavior — outside the no-launch policy and current
  source/package scope.
- Unit tests, mocks, fixtures, and test harnesses — excluded by project policy.
- Signing, installer, updater, registry, cross-machine, and release-owner
  evidence — outside this authorized local slice.

## Known risks and limits

- Independent review returned `NO_CONCLUSION`; no child approval is claimed.
- The split-layout fallback is exercised by an in-process selector simulation,
  not a physically rebuilt alternate bundle.
- Release remains no-go until artifact-bound runtime and enterprise gates are
  refreshed by an authorized operator.

## Acceptance and evidence IDs

- Acceptance: `S339`
- Evidence: `D299-FROZEN-QT-PLUGIN-ROOT=PASS`, source diagnostics, selector
  simulation, package identity, PE/archive checks, and review records.

## Next owner and next action

- Owner: `Project Manager (QuillForge)`
- Action: authorize split-layout/native startup evidence if compatibility is
  required for a supported distribution matrix.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and `QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size: `762A4BC8605A4D4561DA79CFD1F94E94C100867CA8047136DD7B0193B2EC76EC` / `38594314` bytes
- Source revision: `tree-sha256:83ddbd9ec410de312ede60bfac4efdfaeb89f150893e9b37c709cfec1399394b`

## Disposition

Accepted with limits. D299 is source/package/current-layout/simulation
verified; independent review, physical split-layout, native startup,
clean-machine behavior, and remaining release gates remain open.

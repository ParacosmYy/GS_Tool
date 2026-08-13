# Handoff: 2026-08-13-d298-frozen-qt-layout-fallback

| Field | Value |
|---|---|
| ID | `2026-08-13-d298-frozen-qt-layout-fallback` |
| Delivery / slice | `D298 / ARCH-268 Frozen Qt layout fallback` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-13T03:30:00+08:00` |

## User outcome

The frozen Qt dependency preflight now recognizes the same current and legacy
PyQt6 layouts as plugin discovery. A valid `PyQt6/Qt/bin` bundle is no longer
misclassified as missing Qt DLLs, while incomplete bundles still report
actionable missing paths before QApplication construction.

## Scope and boundaries

### In scope

- Qt6-first/Qt-fallback dependency inventory in the existing startup helper.
- D297 diagnostic and fail-fast compatibility.
- Static contract, source diagnostics, package rebuild, PE/archive identity,
  and handoff evidence.

### Out of scope

- Generating a legacy-layout bundle, native EXE/Qt startup, clean-machine
  behavior, real Windows DLL loading, visual rendering, installer, signing,
  updater, file associations, and release-gate closure.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | parent; Luna/max consultation | Integration, boundary decision, parent review, and handoff |
| Project Manager | parent record | Plan, dependencies, risks, and status |
| Product | parent record | Startup compatibility outcome and acceptance |
| Developer 1 | parent | Runtime inventory implementation |
| Developer 2 | parent | Static contract and package integration |
| QA | parent | Source, report, archive, identity, and non-destructive verification |

## Changed files and modules

- `src/quillforge/app.py` — evaluate both frozen PyQt6 DLL layouts.
- `scripts/audit_presentation_contracts.py` — guard both roots and fallback
  ordering.
- ADR, review, acceptance, delivery-register, roadmap, todo, plan, and handoff
  records for D298.

## Decisions and constraints

- Keep PyInstaller's Qt6-first/Qt fallback order at the existing infrastructure
  boundary; do not duplicate layout policy in `main()` or presentation code.
- Preserve the current diagnostic result shape and Qt6 success paths.
- Shared checkout writer: parent only; no worktree or parallel writer was used.
- Runtime launch policy: EXE/Qt startup was not allowed; source, static,
  package, archive, and current-layout evidence were authorized.

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
| `uv run python scripts/audit_presentation_contracts.py` | PASS | Qt6/Qt layout contract passes. |
| `scripts/check.ps1` | PASS | Project checks pass before packaging. |
| source `--diagnose-startup` | PASS | Composition/restore passed; no window/exec entered. |
| source `--diagnose-file-open README.md` | PASS | One startup path opened; no window/exec entered. |
| current-layout dependency report | PASS | Qt6 layout returns the unchanged required paths and no missing files. |
| root `QuillForge.exe` / `dist/QuillForge.exe` identity | PASS | Copies and manifest match. |
| PE/archive inspection | PASS | AMD64, PE32+, Windows GUI, Qt6 DLLs, qwindows, QScintilla, and modules present. |
| `scripts/verify_release_handoff.ps1` | EXPECTED NO-GO | Native reports and enterprise gates remain open. |

## Unrun checks and reason

- Physical legacy-layout bundle, native EXE/Qt startup, clean-machine startup,
  and real DLL-loader behavior — outside the no-launch policy and current
  source/package scope.
- Unit tests, mocks, fixtures, and test harnesses — excluded by project policy.
- Signing, installer, updater, registry, cross-machine, and release-owner
  evidence — outside this authorized local slice.

## Known risks and limits

- Independent review returned `NO_CONCLUSION`; no child approval is claimed.
- The fallback branch is statically verified but not exercised with a physical
  legacy-layout bundle.
- Release remains no-go until artifact-bound runtime and enterprise gates are
  refreshed by an authorized operator.

## Acceptance and evidence IDs

- Acceptance: `S338`
- Evidence: `D298-FROZEN-QT-LAYOUT-FALLBACK=PASS`, current-layout report,
  source diagnostics, package identity, PE/archive checks, and review records.

## Next owner and next action

- Owner: `Project Manager (QuillForge)`
- Action: authorize legacy-layout/native startup evidence if compatibility is
  required for a supported distribution matrix.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and `QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size: `149E1AC499344A2533CC6640CAEC5B50FFCEAECF8534AF816B62D3908A15EA85` / `38592959` bytes
- Source revision: `tree-sha256:b234931cf08d96c35b234da2f78cab401b72b01d73fd21b864983c7f0ee6c489`

## Disposition

Accepted with limits. D298 is source/package/current-layout verified;
independent review, physical legacy-layout, native startup, clean-machine
behavior, and remaining release gates remain open.

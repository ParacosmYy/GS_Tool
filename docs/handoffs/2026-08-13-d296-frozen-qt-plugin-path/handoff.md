# Handoff: 2026-08-13-d296-frozen-qt-plugin-path

| Field | Value |
|---|---|
| ID | `2026-08-13-d296-frozen-qt-plugin-path` |
| Delivery / slice | `D296 / ARCH-266 Frozen Qt plugin path binding` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-13T01:30:00+08:00` |

## User outcome

The frozen one-file candidate now binds Qt plugin discovery to its own
extracted PyQt6 bundle before QApplication construction. This removes a
machine-level Qt plugin path as a source of platform-plugin misselection while
preserving source and diagnostic behavior.

## Scope and boundaries

### In scope

- Frozen-only plugin-root selection in `quillforge.app.main()`.
- Explicit `QT_PLUGIN_PATH` and `QT_QPA_PLATFORM_PLUGIN_PATH` binding.
- Static contract, source diagnostics, package rebuild, PE/archive identity,
  and handoff evidence.

### Out of scope

- Native EXE/Qt startup, clean-machine behavior, visual rendering, installer,
  signing, updater, file associations, and release-gate closure.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | parent; Luna/max consultation | Integration, boundary decision, parent review, and handoff |
| Project Manager | parent record | Plan, dependencies, risks, and status |
| Product | parent record | Startup reliability outcome and acceptance |
| Developer 1 | parent | Entry-boundary implementation |
| Developer 2 | parent | Static contract and package integration |
| QA | parent | Source, archive, identity, and non-destructive verification |

## Changed files and modules

- `src/quillforge/app.py` — add the frozen-only Qt plugin binding helper and
  invoke it before QApplication imports.
- `scripts/audit_presentation_contracts.py` — guard ordering and environment
  assignments.
- ADR, review, acceptance, delivery-register, roadmap, todo, plan, and handoff
  records for D296.

## Decisions and constraints

- Keep the plugin-path binding at the application entry boundary and restrict
  it to frozen execution; do not duplicate it in composition or presentation.
- Prefer the current PyQt6 `Qt6/plugins` layout and retain the legacy `Qt/plugins`
  fallback for packaging compatibility.
- Shared checkout writer: parent only; no worktree or parallel writer was used.
- Runtime launch policy: EXE/Qt startup was not allowed; source, static,
  package, archive, and no-window diagnostic evidence were authorized.

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
| `uv run python scripts/audit_presentation_contracts.py` | PASS | Frozen plugin ordering/path contract passes. |
| `scripts/check.ps1` | PASS | Handoff, audit, formatting, and project checks pass. |
| source `--diagnose-startup` | PASS | Composition/restore passed; no window/exec entered. |
| source `--diagnose-file-open README.md` | PASS | One startup path opened; no window/exec entered. |
| `scripts/package.ps1` | PASS | Root/dist copies match. |
| PE/archive inspection | PASS | AMD64, PE32+, Windows GUI, qwindows/runtime hook/resources present. |
| `scripts/verify_release_handoff.ps1` | EXPECTED NO-GO | Native artifact-bound reports and enterprise gates remain open. |

## Unrun checks and reason

- Native EXE/Qt startup, clean-machine startup, and platform-loader conflict
  reproduction — prohibited by `software_start_allowed=false`.
- Unit tests, mocks, fixtures, and test harnesses — excluded by project policy.
- Signing, installer, updater, registry, cross-machine, and release-owner
  evidence — outside this authorized local slice.

## Known risks and limits

- Independent review returned `NO_CONCLUSION`; no child approval is claimed.
- Static archive evidence cannot prove Qt platform-loader behavior on every
  Windows machine or under antivirus/temp-directory pressure.
- Release remains no-go until artifact-bound runtime and enterprise gates are
  refreshed by an authorized operator.

## Acceptance and evidence IDs

- Acceptance: `S336`
- Evidence: `D296-FROZEN-QT-PLUGIN-CONTRACT=PASS`, package identity,
  source diagnostics, PE/archive checks, and review records.

## Next owner and next action

- Owner: `Project Manager (QuillForge)`
- Action: authorize native and clean-machine startup evidence for the exact
  D296 artifact before release consideration.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and `QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size: `6DAA35980E56F99B53443D61D77C106912FD15764C9A728FEAF051C96086B56C` / `38592937` bytes
- Source revision: `tree-sha256:8440d45d84334bd7056d91acade305320558bdc2360a08fd5997108acfc8ad63`

## Disposition

Accepted with limits. D296 is source/package/archive verified; independent
review, native startup, clean-machine behavior, and remaining release gates
remain open.

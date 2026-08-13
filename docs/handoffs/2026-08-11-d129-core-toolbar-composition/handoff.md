# Handoff: 2026-08-11-d129-core-toolbar-composition

| Field | Value |
|---|---|
| ID | 2026-08-11-d129-core-toolbar-composition |
| Delivery / slice | D129 / ARCH-106 core toolbar composition boundary |
| Status | `accepted-with-limits` |
| Owner | architect |
| Checkout | Current local checkout only |
| Created | 2026-08-11T21:00:00+08:00 |

## User outcome

The core command rail's action metadata now has an explicit presentation
composition boundary. MainWindow no longer assembles toolbar specs directly;
CommandSurface still owns the actual Qt toolbar, actions, icons, translation,
and state projection.

## Scope and boundaries

### In scope

- Frozen/slotted `CoreToolbarPorts`.
- `CoreToolbarCoordinator` composition of six base actions plus optional
  workspace context action.
- Named MainWindow callback mapping.
- Static, package, and traceability evidence for D129.

### Out of scope

- No QToolBar/QAction construction, signals, icons, locale, retranslation,
  shortcut, command registry, menu, plugin, document, workspace policy, or
  application behavior changed.
- No new async path, worker, singleton, service locator, EventBus, or test-only
  asset.
- No QApplication launch, native toolbar rendering, clean-machine,
  cross-machine, signing, installer, updater, legal, support, or release-owner
  evidence.

## Team roles and ownership

| Role | Owner / agent | Result |
|---|---|---|
| Architect | Averroes the 4th / Luna max | `NO_CONCLUSION` after two bounded waits; no child PASS |
| Independent review | Mencius the 4th / Luna max | `NO_CONCLUSION` after two bounded waits; no independent PASS |
| Parent | Architect | `PASS`; sole writer, integration, review, simplification, and verification |

## Changed files and modules

- `src/quillforge/presentation/core_toolbar_coordinator.py` — new named
  toolbar-spec composition boundary.
- `src/quillforge/presentation/main_window.py` — named callback mapping and
  removal of embedded toolbar assembly.
- `docs/adr/0168-core-toolbar-composition-boundary.md`
- `docs/agent-team/reviews/D129-core-toolbar-composition-parent-review.md`
- `docs/agent-team/reviews/D129-core-toolbar-composition-independent-review.md`

## Decisions and constraints

- CoreToolbarCoordinator owns only presentation-spec composition; MainWindow
  owns callback policy and CommandSurface owns Qt projection.
- Existing `ToolbarActionSpec`, `IconKey`, command catalog, plugin refresh, and
  locale paths remain the source of truth.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, inline, package, and static
  evidence are the authorized validation boundary.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `D129-CORE-TOOLBAR-BEHAVIOR-PROBE=PASS` | PASS | Exact no-workspace/workspace action count, order, metadata, and callback identity. |
| `D129-CORE-TOOLBAR-CONTRACT-PROBE=PASS` | PASS | Frozen ports, MainWindow mapping, and removed embedded assembly method. |
| `D129-COMPILEALL=PASS` | PASS | Static compilation; no QApplication launch. |
| `D129-RUFF=PASS` | PASS | Target source passed `uv run ruff check`. |
| `D129-FORMAT=PASS` | PASS | Target source passed `uv run ruff format --check`. |
| `D129-PACKAGE-IDENTITY-PROBE=PASS` | PASS | Root and `dist` candidates match: SHA-256 `E651B91174DF1B8521486E10E4FC7C34777598C17124CC288215E6580446FCD8`, 38,510,044 bytes, source `tree-sha256:993e78a4f02d4351aa070a4b44a3e430e92694fe9f8a31daf5efdeca1873b973`. |
| `D129-PACKAGE-NO-LAUNCH-PROBE=PASS` | PASS | Packaging completed without launching QuillForge; no process remained. |
| `D129-JSON-TRACEABILITY-PROBE=PASS` | PASS | Acceptance, delivery register, handoff index, manifest, and release handoff are synchronized after record update. |
| `D129-RELEASE-DOSSIER-PROBE=PASS` | PASS | Release dossier binds the current D129 artifact identity. |
| `D129-RELEASE-EXPECTED-NO-GO=PASS` | PASS | Expected NO-GO remains due open external gates and known report-binding failures. |

## Unrun checks and reason

- Architect and independent review conclusions — child windows timed out;
  recorded as `NO_CONCLUSION`, not PASS.
- Native QToolBar/QAction rendering, QApplication startup, runtime shortcut
  delivery, locale retranslation timing, DPI/font metrics, accessibility,
  clean-machine, cross-machine, signing, installer, updater, legal, support,
  and release-owner checks — prohibited or outside authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static action probes cannot prove native toolbar geometry, platform style
  rendering, shortcut dispatch, or plugin callback timing.
- Averroes architecture and Mencius independent review windows returned
  `NO_CONCLUSION`; no child PASS is claimed.
- The portable candidate remains unsigned and release remains NO-GO while
  external gates and report-binding gates are open.

## Acceptance and evidence IDs

- Acceptance: `S172`, `D129-AC01`.
- Evidence: ADR-0168, toolbar behavior/contract probes, parent and independent
  review records, simplification assessment, static checks, package identity,
  handoff/index/register checks, expected release NO-GO, and explicit runtime
  limits.

## Next owner and next action

- Owner: Architect.
- Action: continue the next bounded MainWindow/application or visual-quality
  slice and complete authorized runtime/release gates when authority and
  environment permit.

## Artifact information

The portable candidate was rebuilt without launching QuillForge after the
source change:

- Artifact: `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `E651B91174DF1B8521486E10E4FC7C34777598C17124CC288215E6580446FCD8`
- Size: `38510044` bytes
- Source revision: `tree-sha256:993e78a4f02d4351aa070a4b44a3e430e92694fe9f8a31daf5efdeca1873b973`
- Manifest: `dist/QuillForge.release.json`

## Disposition

`accepted-with-limits`: core toolbar action composition is explicit while
native toolbar/runtime/release evidence remains open.

# Handoff: 2026-08-11-d130-pure-toolbar-contract-extraction

| Field | Value |
|---|---|
| ID | 2026-08-11-d130-pure-toolbar-contract-extraction |
| Delivery / slice | D130 / ARCH-107 pure toolbar contract extraction |
| Status | `accepted-with-limits` |
| Owner | architect |
| Checkout | Current local checkout only |
| Created | 2026-08-11T22:00:00+08:00 |

## User outcome

The D129 core toolbar coordinator now has a genuine pure-contract dependency
boundary. Icon identifiers and toolbar action metadata no longer require
loading the Qt-bearing renderer/projection modules, while existing import
seams and visible toolbar semantics remain compatible.

## Scope and boundaries

### In scope

- Pure-Python `IconKey` contract.
- Pure-Python `ToolbarActionRole` and `ToolbarActionSpec` contracts.
- Compatibility imports from `icons.py` and `command_surface.py`.
- CoreToolbarCoordinator import-direction correction.
- Static, package, and traceability evidence for D130.

### Out of scope

- No QToolBar/QAction construction, signals, icon pixels, locale,
  retranslation, shortcut, command registry, menu, plugin, document,
  workspace policy, or application behavior changed.
- No new async path, worker, singleton, service locator, EventBus, or
  test-only asset.
- No QApplication launch, native toolbar rendering, clean-machine,
  cross-machine, signing, installer, updater, legal, support, or release-owner
  evidence.

## Team roles and ownership

| Role | Owner / agent | Result |
|---|---|---|
| Architect | Leibniz the 4th / Luna max; follow-up Lovelace the 4th / Luna max | Both bounded windows returned `NO_CONCLUSION`; no child PASS |
| Independent review | Poincare the 4th / Luna max; final Godel the 4th / Luna max | Both bounded windows returned `NO_CONCLUSION`; no independent PASS |
| Parent | Architect | `PASS`; sole writer, integration, review, simplification, and verification |

## Changed files and modules

- `src/quillforge/presentation/icon_contract.py` — pure icon identifier
  contract.
- `src/quillforge/presentation/toolbar_contract.py` — pure toolbar action
  role/spec contract.
- `src/quillforge/presentation/icons.py` — compatibility import for IconKey.
- `src/quillforge/presentation/command_surface.py` — compatibility imports
  for toolbar contracts; Qt projection remains local.
- `src/quillforge/presentation/core_toolbar_coordinator.py` — pure contract
  imports only.
- `docs/adr/0169-pure-toolbar-contract-extraction.md`
- `docs/agent-team/reviews/D130-pure-toolbar-contract-parent-review.md`
- `docs/agent-team/reviews/D130-pure-toolbar-contract-independent-review.md`

## Decisions and constraints

- Contract modules own values and metadata; the renderer/projection layer owns
  Qt, locale, and pixels.
- Existing `icons.py` and `command_surface.py` import seams remain valid.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, inline, package, and static
  evidence are the authorized validation boundary.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `D130-CORE-TOOLBAR-BEHAVIOR-PROBE=PASS` | PASS | Exact no-workspace/workspace count, order, metadata, and callback identity. |
| `D130-QT-FREE-CONTRACT-PROBE=PASS` | PASS | Coordinator/contract import path did not load PyQt6; contract module ownership is pure. |
| `D130-COMPILEALL=PASS` | PASS | Static compilation; no QApplication launch. |
| `D130-RUFF=PASS` | PASS | Changed source passed Ruff check. |
| `D130-FORMAT=PASS` | PASS | Changed source passed Ruff format check. |
| `D130-PACKAGE-IDENTITY-PROBE=PASS` | PASS | Root and `dist` candidates match: SHA-256 `7E70F3DC59DDC9C3289E96E1AF66CC0DE6701FC00F1EFB421145CB58DAB00B88`, 38,510,134 bytes, source `tree-sha256:c278cd0279f1841b7c3be331f7a27b7976e1d14516e440c546455979759bea62`. |
| `D130-PACKAGE-NO-LAUNCH-PROBE=PASS` | PASS | Packaging completed without launching QuillForge; no process remained. |
| `D130-JSON-TRACEABILITY-PROBE=PASS` | PASS | Acceptance, delivery register, handoff index, manifest, and release handoff synchronized after record update. |
| `D130-RELEASE-DOSSIER-PROBE=PASS` | PASS | Release dossier binds the current D130 artifact identity. |
| `D130-RELEASE-EXPECTED-NO-GO=PASS` | PASS | Expected NO-GO remains due open external gates and known report-binding failures. |

## Unrun checks and reason

- Architect and independent review conclusions — initial and final child
  windows timed out; recorded as `NO_CONCLUSION`, not PASS.
- Native QToolBar/QAction rendering, QApplication startup, runtime shortcut
  delivery, locale retranslation timing, DPI/font metrics, accessibility,
  clean-machine, cross-machine, signing, installer, updater, legal, support,
  and release-owner checks — prohibited or outside authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static import probes cannot prove native toolbar geometry, platform style
  rendering, shortcut dispatch, or plugin callback timing.
- Leibniz/Lovelace architecture and Poincare/Godel independent review windows
  returned `NO_CONCLUSION`; no child PASS is claimed.
- The portable candidate remains unsigned and release remains NO-GO while
  external gates and report-binding gates are open.

## Acceptance and evidence IDs

- Acceptance: `S173`, `D130-AC01`.
- Evidence: ADR-0169, pure-contract import/toolbar behavior probes, parent and
  independent review records, simplification assessment, static checks,
  package identity, handoff/index/register checks, expected release NO-GO, and
  explicit runtime limits.

## Next owner and next action

- Owner: Architect.
- Action: continue the next bounded MainWindow/application or visual-quality
  slice and complete authorized runtime/release gates when authority and
  environment permit.

## Artifact information

The portable candidate was rebuilt without launching QuillForge after the
source change:

- Artifact: `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `7E70F3DC59DDC9C3289E96E1AF66CC0DE6701FC00F1EFB421145CB58DAB00B88`
- Size: `38510134` bytes
- Source revision: `tree-sha256:c278cd0279f1841b7c3be331f7a27b7976e1d14516e440c546455979759bea62`
- Manifest: `dist/QuillForge.release.json`

## Disposition

`accepted-with-limits`: toolbar contracts are pure and compatibility seams are
preserved while native toolbar/runtime/release evidence remains open.

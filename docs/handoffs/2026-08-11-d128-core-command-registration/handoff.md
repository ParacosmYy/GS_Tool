# Handoff: 2026-08-11-d128-core-command-registration

| Field | Value |
|---|---|
| ID | 2026-08-11-d128-core-command-registration |
| Delivery / slice | D128 / ARCH-105 core command registration boundary |
| Status | `accepted-with-limits` |
| Owner | architect |
| Checkout | Current local checkout only |
| Created | 2026-08-11T20:00:00+08:00 |

## User outcome

The built-in command catalog now has an explicit Qt-free composition boundary.
MainWindow no longer owns the 23-command construction tuple, while existing
menus, toolbar actions, shortcuts, plugin commands, and callback policy remain
unchanged.

## Scope and boundaries

### In scope

- Frozen/slotted `CoreCommandPorts`.
- Qt-free `CoreCommandCoordinator` and deterministic registration order.
- Named MainWindow callback composition.
- Static, package, and traceability evidence for D128.

### Out of scope

- No `CommandRegistry` semantics, `CommandSurface` QAction/menu/toolbar
  projection, plugin registration/refresh, locale, shortcut, document,
  workspace, settings, close, or application policy changed.
- No new async path, worker, singleton, service locator, EventBus, or test-only
  asset.
- No QApplication launch, native menu/toolbar rendering, clean-machine,
  cross-machine, signing, installer, updater, legal, support, or release-owner
  evidence.

## Team roles and ownership

| Role | Owner / agent | Result |
|---|---|---|
| Architect | Maxwell the 4th / Luna max | `NO_CONCLUSION` after two bounded waits; no child PASS |
| Independent review | Bacon the 4th / Luna max | `NO_CONCLUSION` after two bounded waits; no independent PASS |
| Parent | Architect | `PASS`; sole writer, integration, review, simplification, and verification |

## Changed files and modules

- `src/quillforge/presentation/core_command_coordinator.py` — new Qt-free
  named ports and command registration boundary.
- `src/quillforge/presentation/main_window.py` — named callback mapping and
  removal of the embedded command tuple.
- `docs/adr/0167-core-command-registration-boundary.md`
- `docs/agent-team/reviews/D128-core-command-registration-parent-review.md`
- `docs/agent-team/reviews/D128-core-command-registration-independent-review.md`

## Decisions and constraints

- The coordinator owns command catalog construction only; MainWindow owns
  behavior and callback composition; CommandSurface owns Qt projection.
- Existing `CommandRegistry` collision and plugin refresh semantics remain the
  source of truth.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, inline, package, and static
  evidence are the authorized validation boundary.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `D128-CORE-COMMAND-BEHAVIOR-PROBE=PASS` | PASS | Exact 23-command ID/order/metadata and callback identity coverage. |
| `D128-CORE-COMMAND-CONTRACT-PROBE=PASS` | PASS | Frozen ports, Qt-free module, MainWindow mapping, and removed embedded method. |
| `D128-COMPILEALL=PASS` | PASS | Static compilation; no QApplication launch. |
| `D128-RUFF=PASS` | PASS | Target source passed `uv run ruff check`. |
| `D128-FORMAT=PASS` | PASS | Target source passed `uv run ruff format --check`. |
| `D128-PACKAGE-IDENTITY-PROBE=PASS` | PASS | Root and `dist` candidates match: SHA-256 `F22251C83B26A64C13D8031A5E827F3D2FE3C50BA53483F2F6C869007674AC27`, 38,508,475 bytes, source `tree-sha256:0bbb823be350c97b2d58d586b93a3d5a55ffa58392a87869862fcd33f6841426`. |
| `D128-PACKAGE-NO-LAUNCH-PROBE=PASS` | PASS | Packaging completed without launching QuillForge; no process remained. |
| `D128-JSON-TRACEABILITY-PROBE=PASS` | PASS | Acceptance, delivery register, handoff index, manifest, and release handoff are synchronized after record update. |
| `D128-RELEASE-DOSSIER-PROBE=PASS` | PASS | Release dossier binds the current D128 artifact identity. |
| `D128-RELEASE-EXPECTED-NO-GO=PASS` | PASS | Expected NO-GO remains due open external gates and known report-binding failures. |

## Unrun checks and reason

- Architect and independent review conclusions — child windows timed out;
  recorded as `NO_CONCLUSION`, not PASS.
- Native QAction/menu/toolbar rendering, QApplication startup, runtime
  shortcut delivery, plugin interleaving, DPI/font metrics, clean-machine,
  cross-machine, signing, installer, updater, legal, support, and release-owner
  checks — prohibited or outside authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static command probes cannot prove native QAction lifecycle, platform menu
  rendering, shortcut dispatch, or plugin callback timing.
- Maxwell architecture and Bacon independent review windows returned
  `NO_CONCLUSION`; no child PASS is claimed.
- The portable candidate remains unsigned and release remains NO-GO while
  external gates and report-binding gates are open.

## Acceptance and evidence IDs

- Acceptance: `S171`, `D128-AC01`.
- Evidence: ADR-0167, core command behavior/contract probes, parent and
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
- SHA-256: `F22251C83B26A64C13D8031A5E827F3D2FE3C50BA53483F2F6C869007674AC27`
- Size: `38508475` bytes
- Source revision: `tree-sha256:0bbb823be350c97b2d58d586b93a3d5a55ffa58392a87869862fcd33f6841426`
- Manifest: `dist/QuillForge.release.json`

## Disposition

`accepted-with-limits`: core command registration is explicit while native
menu/toolbar/runtime/release evidence remains open.

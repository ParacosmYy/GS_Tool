# Handoff: 2026-08-10-d21-file-dialog-surface

| Field | Value |
|---|---|
| ID | `2026-08-10-d21-file-dialog-surface` |
| Delivery / slice | `D21 / ARCH-12 MainWindow file-dialog surface` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-10T15:30:00+08:00` |

## User outcome

Opening a document, choosing a workspace folder, and selecting a Save As
destination now have explicit native-dialog intents. The file-open path uses
the file picker, the workspace path uses the folder picker, and all three
selection results are handed back as `Path | None` without moving async or
path-policy behavior out of MainWindow.

## Scope and boundaries

### In scope

- `FileDialogSurface` native file/folder/save composition, localization, and
  `Path | None` conversion.
- MainWindow delegation for Open, Workspace, Save As, and locale refresh.
- D21 ADR, review, acceptance/register/index, architecture/spec/roadmap/task,
  package provenance, and release no-go synchronization.

### Out of scope

- No change to explicit local Open/Save As policy, workspace containment,
  search-result containment, session restore, symlink/reparse identity, or
  document-service I/O.
- No dialog redesign, filter-policy enforcement, new dependency, or UI
  framework change.
- No QApplication startup, screenshots, interactive visual acceptance,
  clean-machine, signing, installer/update, deployment, or hardware evidence.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | parent | Architecture decision, implementation, integration, verification, handoff |
| Project Manager | fixed six-role workflow; bounded audit did not return before timeout | Dependencies, risks, and open release gates; parent recorded equivalent evidence |
| Product | fixed six-role workflow; bounded audit did not return before timeout | File/folder/save user outcome and acceptance; parent resolved explicit path-policy scope |
| Developer 1 | fixed six-role workflow; bounded audit did not return before timeout | MainWindow/application policy boundary; parent retained existing ownership |
| Developer 2 | fixed six-role workflow; bounded audit did not return before timeout | FileDialogSurface presentation composition; parent implemented the minimal API |
| QA | fixed six-role workflow; bounded audit did not return before timeout | Static/package/no-launch verification; parent ran authorized checks |
| Independent reviewer | Luna bounded path-policy audit; Mencius / Luna max focused review | Registration/scope concern and startup-restore picker concern were resolved and recorded in parent review |

## Changed files and modules

- `src/quillforge/presentation/file_dialog_surface.py` — owns native file,
  folder, and save dialog calls plus locale/Path conversion.
- `src/quillforge/presentation/main_window.py` — composes the surface and
  retains guards, async starts, save policy, and application consequences.
- `docs/adr/0046-main-window-file-dialog-surface.md` — D21 decision,
  invariants, explicit path-policy scope, and limits.
- `docs/agent-team/reviews/D21-file-dialog-surface-parent-review.md` — parent
  review, independent-review result, simplification, and validation record.
- `docs/specs/enterprise-architecture-migration.md`, `docs/ARCHITECTURE.md`,
  `docs/ROADMAP.md` — architecture and migration projections.
- `tasks/plan.md`, `tasks/todo.md` — D21 tracking.
- `docs/agent-team/acceptance.json`, `docs/agent-team/delivery-register.json`,
  `docs/handoffs/index.json` — acceptance and delivery projections.

## Decisions and constraints

- `choose_document()` is the file-only intent and calls
  `QFileDialog.getOpenFileName`.
- `choose_workspace()` is the directory-only intent and calls
  `QFileDialog.getExistingDirectory`.
- `choose_save_path()` preserves the prior current-path/`Untitled.txt`
  default and returns `None` on cancel.
- `_open_document()` rejects picker entry while startup session restoration is
  in flight, so a selected path is not silently discarded after the dialog.
- Shared checkout writer: Architect. No worktree, unit-test-only asset, or Qt
  launch was used.

## Public-source applicability and embedded gate

This slice is Python/PyQt6, not embedded C/C++ or firmware. The embedded
enterprise workflow and embedded code-review simplifier are **N/A** for
MCU/vendor constraints because no firmware target, SDK, RTOS, ISR/DMA, driver,
protocol, boot, Flash/NVM, power, or hardware was changed. Public architecture
references are engineering references only:
[CloudWeGo About](https://www.cloudwego.io/about/),
[CloudWeGo open-source announcement](https://www.cloudwego.io/blog/2021/09/13/cloudwego-open-source-announcement/),
and [Kitex framework extension](https://www.cloudwego.io/docs/kitex/tutorials/framework-exten/).
No private ByteDance standard, certification, manufacturer requirement, or
release-readiness claim is made.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `uv run python -m compileall -q src\quillforge` | PASS | Non-destructive compile gate. |
| `uv run ruff check src\quillforge` | PASS | Import/lint gate. |
| `uv run ruff format --check src\quillforge` | PASS | Format gate. |
| D21 source boundary probe | PASS | MainWindow has no direct QFileDialog; three semantic methods and policy call sites are present. |
| `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\package.ps1` | PASS | Historical D21 portable candidate; root/dist SHA `4FE6A679B03A597009B8B2F310FF2493522EE4C142866F30A787A2648538BB53`, size `38,386,277` bytes; source `tree-sha256:08c9c1550303dc685ce29cdf62f16462cba870a70134f5a4a50d86669ab20431`; D22 is current. |
| JSON acceptance/register/index parse | PASS | D21 ledger files parse after synchronization. |
| `scripts/verify_handoff.ps1` | PASS | Handoff/index/register status and required sections are synchronized. |
| `scripts/check.ps1` | PASS | Static/source/format gates pass; NOTICE coverage reports 13 lockfile packages and 75 formatted files. |
| `scripts/verify_release_handoff.ps1` | EXPECTED NO-GO | Exit 1 with exact mechanical failures `packaged_report_artifact_match`, `interactive_startup_report_consistent`, `startup_preflight_report_consistent`; 10 release gates remain open. |

## Independent review

The bounded Luna path-policy audit returned **REVISE** because the D21 ledger
and explicit cross-workspace policy scope were missing at review time. The
parent added the ADR, D21-AC01/S50, register/index/handoff records, and the
explicit decision that Open/Save As/session/containment policy is unchanged.
Mencius / Luna max then returned **CONCERNS** because `_open_document()` could
open a picker during startup restoration and discard the selected path later.
The parent added the startup-restore guard, reran compile/Ruff/format and the
source probe, and rebuilt the package. Both review concerns are resolved at
source/documentation level; no runtime dialog PASS is claimed.

## Simplification assessment

The change removes direct native dialog blocks and repeated locale/filter/Path
conversion details from MainWindow. The new surface has only one locale field
and three semantic methods, with no duplicate state or speculative framework.
No additional behavior-preserving simplification is required.

## Unrun checks and reason

- QApplication/Qt startup, native dialog delivery, extension filters,
  screenshots, screen-reader output, DPI/font behavior, and visual acceptance
  — prohibited by the permanent no-launch policy.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — not created,
  modified, or run under project constraints.
- Clean-machine, cross-machine, signing, installer/update, deployment, and
  hardware checks — not authorized.

## Known risks and limits

- The native file filter is advisory; document-service validation remains the
  actual I/O boundary.
- Explicit Open/Save As may still target arbitrary local paths by decision;
  D21 does not claim workspace containment for those flows.
- Runtime visual/native behavior, release, and external gates remain open.

## Acceptance and evidence IDs

- Acceptance: `D21-AC01`, `S50`.
- Evidence: ADR-0046, source modules, architecture/spec/roadmap/task records,
  parent review, this handoff, static checks, handoff verifier, package
  manifest, and release no-go dossier.

## Next owner and next action

- Owner: Architect.
- Action: continue the next bounded MainWindow/application coordinator slice
  only after D21 verifier/check/release-no-go evidence is refreshed; keep
  native runtime and external release gates explicit.

## Artifact information

- Artifact path: `QuillForge.exe` and `dist/QuillForge.exe` (historical D21 capture).
- Version: `0.1.0` / package manifest version.
- SHA-256 / size: `4FE6A679B03A597009B8B2F310FF2493522EE4C142866F30A787A2648538BB53` /
  `38,386,277` bytes; source
  `tree-sha256:08c9c1550303dc685ce29cdf62f16462cba870a70134f5a4a50d86669ab20431`.
- Packaging note: D21 capture is historical after D22 source edits; it is not
  release approval.

## Disposition

`accepted-with-limits`: D21 file/folder/save dialog separation is source-level
verified, the pre-registration review concern is resolved, and runtime,
clean-machine, and external release gates remain open.

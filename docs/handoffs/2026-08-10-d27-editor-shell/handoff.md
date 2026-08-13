# Handoff: 2026-08-10-d27-editor-shell

| Field | Value |
|---|---|
| ID | `2026-08-10-d27-editor-shell` |
| Delivery / slice | `D27 / ARCH-18 central editor shell surface` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-10T18:30:00+08:00` |

## User outcome

The central editor shell now has one composition owner for its layout, document
tabs, and FindBar. This keeps future visual refinement of the editor canvas and
query rail localized while the main coordinator retains document and operation
behavior.

## Scope and boundaries

### In scope

- `EditorShellSurface` central widget/layout, child-surface parentage, initial
  FindBar state, and FindBar locale route.
- MainWindow delegation while retaining callbacks, document/editor state,
  Replace All, close/save/recovery/session, and command policy.
- D27 ADR, review, acceptance/register/index, architecture/spec/roadmap/task,
  package provenance, and release no-go synchronization.

### Out of scope

- No document, editor, Find/Replace, Replace All, close/save, recovery/session,
  command, or locale behavior changes.
- No Qt startup, tab/FindBar interaction, screenshots, interactive visual
  acceptance, clean-machine, signing, installer/update, deployment, or
  hardware evidence.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | parent | Architecture decision, implementation, integration, verification, handoff |
| Project Manager | fixed six-role workflow | Dependencies, risks, and open release gates |
| Product | fixed six-role workflow | Editor-shell composition user outcome and acceptance |
| Developer 1 | fixed six-role workflow | Document/editor/operation policy ownership |
| Developer 2 | fixed six-role workflow | Central layout and child-surface composition ownership |
| QA | fixed six-role workflow | Static/package/no-launch verification |
| Independent reviewer | Ampere the 2nd / Luna max | Bounded review returned no conclusion; no child PASS claimed |

## Changed files and modules

- `src/quillforge/presentation/editor_shell_surface.py` — owns central shell
  widget/layout, DocumentTabSurface, FindSurface, parentage, and locale route.
- `src/quillforge/presentation/main_window.py` — constructs the composite,
  installs its widget, and retains semantic child-surface/policy usage.
- `docs/adr/0052-editor-shell-surface.md` — D27 decision, invariants, scope,
  and limits.
- `docs/agent-team/reviews/D27-editor-shell-parent-review.md` — parent
  review, independent no-conclusion record, simplification, and validation.
- `docs/specs/enterprise-architecture-migration.md`, `docs/ARCHITECTURE.md`,
  `docs/ROADMAP.md` — architecture and migration projections.
- `tasks/plan.md`, `tasks/todo.md` — D27 tracking.
- `docs/agent-team/acceptance.json`, `docs/agent-team/delivery-register.json`,
  `docs/handoffs/index.json` — acceptance and delivery projections.

## Decisions and constraints

- `EditorShellSurface` owns `editorShell`, zero-margin vertical layout, tab
  rail ordering, FindBar ordering, and initial FindBar hidden state.
- MainWindow passes existing semantic callbacks and continues to use the
  existing `tabs`/`find` surface APIs for policy and operations.
- The composite introduces no application state, document record model,
  general layout framework, or second locale state model.
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
| `uv run ruff format --check src\quillforge` | PASS | Format gate; 78 source files were already formatted. |
| D27 source boundary probe | PASS | MainWindow no longer directly composes central QWidget/QVBoxLayout or child surfaces; EditorShellSurface owns composition and locale route. |
| `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\package.ps1` | PASS | Historical D27 candidate root/dist SHA `8EFBFD85EF378E0A23DC55E6E33468F52EBEF5F72AB4944DDFFB102110E53655`, size `38,392,493` bytes; source `tree-sha256:d648b1e3b3ebb9fdd461934fe94e7a64673ee227642221ee30c75fda3afff174`. |
| `scripts/verify_handoff.ps1` | PASS | D27 handoff/index/register/acceptance status and required sections are synchronized. |
| `scripts/check.ps1` | PASS | D27 source, formatting, inventory, and static project checks passed. |
| `scripts/verify_release_handoff.ps1` | EXPECTED NO-GO | Exit 1; exact mechanical failures `packaged_report_artifact_match`, `interactive_startup_report_consistent`, `startup_preflight_report_consistent`; 10 release gates remain open. |

## Independent review

Ampere the 2nd / Luna max was assigned a bounded read-only D27 architecture
review with no write access, no Qt launch, and no test creation/run. Two
bounded wait windows returned no conclusion; the reviewer was then closed. No
child PASS or FAIL is claimed. D27 is accepted only with parent static source
reasoning and deterministic checks.

## Simplification assessment

The extraction removes central widget/layout composition and concrete child
surface construction from MainWindow while preserving existing semantic APIs
and callback ownership. No further safe behavior-preserving simplification is
required for this slice.

## Unrun checks and reason

- QApplication/Qt startup, tab/FindBar interaction, focus, screenshots, visual
  hierarchy, screen-reader output, DPI/font behavior, and runtime acceptance —
  prohibited by the permanent no-launch policy.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — not created,
  modified, or run under project constraints.
- Clean-machine, cross-machine, signing, installer/update, deployment, and
  hardware checks — not authorized.

## Known risks and limits

- Independent review has no conclusion; no child PASS is represented as
  evidence.
- Runtime central-shell parentage, child interaction, visual/accessibility
  behavior, and cross-machine rendering remain unverified.
- Existing D7/D8 legal, clean-machine, signing/installer/update, and release
  gates remain open.

## Acceptance and evidence IDs

- Acceptance: `D27-AC01`, `S56`.
- Evidence: ADR-0052, source modules, architecture/spec/roadmap/task records,
  parent review, this handoff, static checks, handoff verifier, package
  manifest, and release no-go dossier.

## Next owner and next action

- Owner: Architect.
- Action: continue the next bounded MainWindow/application coordinator slice
  only after D27 verifier/check/release-no-go evidence is refreshed; obtain a
  fresh independent review window when available.

## Artifact information

- Artifact path: `QuillForge.exe` and `dist/QuillForge.exe`.
- Version: `0.1.0` / package manifest version.
- SHA-256 / size: `8EFBFD85EF378E0A23DC55E6E33468F52EBEF5F72AB4944DDFFB102110E53655` /
  `38,392,493` bytes; source
  `tree-sha256:d648b1e3b3ebb9fdd461934fe94e7a64673ee227642221ee30c75fda3afff174`.
- Packaging note: D27's portable candidate was rebuilt after D27 source edits;
  it is not the current release candidate and is retained as historical
  evidence. D26 and D27 package identities are historical.

## Disposition

`accepted-with-limits`: D27 editor-shell composition is source-level verified
by the parent with an explicit independent no-conclusion record; runtime,
clean-machine, and external release gates remain open.

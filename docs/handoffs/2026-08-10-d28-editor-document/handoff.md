# Handoff: 2026-08-10-d28-editor-document

| Field | Value |
|---|---|
| ID | `2026-08-10-d28-editor-document` |
| Delivery / slice | `D28 / ARCH-19 editor document surface` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-10T19:00:00+08:00` |

## User outcome

Per-document editor setup now has one presentation owner. Future editor font,
theme, language, or signal-routing refinements can stay in the adapter while
MainWindow continues to own document behavior and operation consequences.

## Scope and boundaries

### In scope

- `EditorDocumentSurface` editor creation, settings/theme application, Save As
  language-hint refresh, and semantic modified/content/caret signal routing.
- MainWindow delegation while retaining document state, save/recovery,
  Replace All, session, operation-lock, and notification policy.
- D28 ADR, review, acceptance/register/index, architecture/spec/roadmap/task,
  package provenance, and release no-go synchronization.

### Out of scope

- No document model, dirty-state, save, recovery, Replace All, session,
  operation, or notification behavior changes.
- No Qt startup, editor interaction, screenshots, interactive visual
  acceptance, clean-machine, signing, installer/update, deployment, or
  hardware evidence.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | parent | Architecture decision, implementation, integration, verification, handoff |
| Project Manager | fixed six-role workflow | Dependencies, risks, and open release gates |
| Product | fixed six-role workflow | Editor setup and future visual-refinement outcome |
| Developer 1 | fixed six-role workflow | Document/save/recovery/operation policy ownership |
| Developer 2 | fixed six-role workflow | Editor adapter setup and signal composition ownership |
| QA | fixed six-role workflow | Static/package/no-launch verification |
| Independent reviewer | Chandrasekhar the 2nd / Luna max | Bounded read-only review returned PASS; runtime remains unverified |

## Changed files and modules

- `src/quillforge/presentation/editor_document_surface.py` — owns editor
  adapter creation, settings/theme application, language refresh, and signal
  routing.
- `src/quillforge/presentation/main_window.py` — supplies callbacks and uses
  the surface while retaining document/operation policy.
- `docs/adr/0053-editor-document-surface.md` — D28 decision and invariants.
- `docs/agent-team/reviews/D28-editor-document-parent-review.md` — parent
  review, simplification, independent-review status, and validation.
- `docs/specs/enterprise-architecture-migration.md`, `docs/ARCHITECTURE.md`,
  `docs/ROADMAP.md` — architecture and migration projections.
- `tasks/plan.md`, `tasks/todo.md` — D28 tracking.
- `docs/agent-team/acceptance.json`, `docs/agent-team/delivery-register.json`,
  `docs/handoffs/index.json` — acceptance and delivery projections.

## Decisions and constraints

- `EditorDocumentSurface` owns only EditorWidget setup, appearance settings,
  language-hint refresh, and semantic signal routing.
- MainWindow supplies callbacks and retains `_DocumentTab`, document state,
  save/recovery/Replace All/session policy, operation locks, and notifications.
- The surface introduces no application state, document record model, generic
  UI factory, event bus, or second settings state model.
- Shared checkout writer: Architect. No worktree, unit-test-only asset, or Qt
  launch was used.

## Public-source applicability and embedded gate

This slice is Python/PyQt6, not embedded C/C++ or firmware. The embedded
enterprise workflow and embedded code-review simplifier are **N/A** for
MCU/vendor constraints; no firmware target, SDK, RTOS, ISR/DMA, driver,
protocol, boot, Flash/NVM, power, or hardware changed. Public CloudWeGo
references are engineering references only:
[CloudWeGo About](https://www.cloudwego.io/about/),
[CloudWeGo open-source announcement](https://www.cloudwego.io/blog/2021/09/13/cloudwego-open-source-announcement/),
and [Kitex framework extension](https://www.cloudwego.io/docs/kitex/tutorials/framework-exten/).
No private ByteDance standard, manufacturer requirement, certification, or
release-readiness claim is made.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `uv run python -m compileall -q src\quillforge` | PASS | Non-destructive compile gate. |
| `uv run ruff check src\quillforge` | PASS | Import/lint gate. |
| `uv run ruff format --check src\quillforge` | PASS | 79 source files already formatted. |
| D28 source boundary probe | PASS | MainWindow delegates editor adapter setup and signal wiring. |
| `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\package.ps1` | PASS | Historical D28 root/dist SHA `CDCB3F4ECB9800B44C28D34E92B1F2077D36BF8F0E26F0054EAF6C41746842FE`, size `38,394,286` bytes; source `tree-sha256:7a410bd083789eecf06c7a7e59c1a1d6ca3bf41cd94b85d0ea6249a51ed2e004`. |
| `scripts\verify_handoff.ps1` | PASS | D28 handoff/index/register/acceptance status and required sections are synchronized. |
| `scripts\check.ps1` | PASS | D28 source, formatting, inventory, and static project checks passed. |
| `scripts\verify_release_handoff.ps1` | EXPECTED NO-GO | Exit 1; exact mechanical failures `packaged_report_artifact_match`, `interactive_startup_report_consistent`, `startup_preflight_report_consistent`; 10 release gates remain open. |

## Independent review

Chandrasekhar the 2nd / Luna max was assigned a bounded read-only D28 review
with no write access, no Qt launch, and no test creation/run. The reviewer
returned PASS for creation order, callback semantics, Save As language
refresh, settings application, policy boundaries, and duplicate-wiring
absence. The reviewer also recorded that runtime Qt lifecycle remains
unverified; this is not a runtime PASS.

## Simplification assessment

The extraction removes repeated editor setup and direct QScintilla signal
connections from MainWindow while keeping callback ownership and policy in the
coordinator. It deliberately avoids a generic factory or second state model;
no further safe behavior-preserving simplification is required.

## Unrun checks and reason

- QApplication/Qt startup, editor interaction, focus, tab lifetime, visual
  hierarchy, screen-reader output, DPI/font behavior, and runtime acceptance —
  prohibited by the permanent no-launch policy.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — not created,
  modified, or run under project constraints.
- Clean-machine, cross-machine, signing, installer/update, deployment, and
  hardware checks — not authorized.

## Known risks and limits

- Independent Luna review passed the static/source scope but does not cover
  runtime Qt lifecycle or QScintilla signal delivery.
- Editor interaction, tab lifetime, visual/accessibility behavior, and
  cross-machine rendering remain unverified.
- Existing D7/D8 legal, clean-machine, signing/installer/update, and release
  gates remain open.

## Acceptance and evidence IDs

- Acceptance: `D28-AC01`, `S57`.
- Evidence: ADR-0053, source modules, architecture/spec/roadmap/task records,
  parent review, this handoff, static checks, handoff verifier, package
  manifest, and release no-go dossier.

## Next owner and next action

- Owner: Architect.
- Action: continue the next bounded MainWindow/application coordinator slice
  only after D28 verifier/check/release-no-go evidence is refreshed; keep
  runtime editor and release gates explicit.

## Artifact information

- Artifact path: `QuillForge.exe` and `dist/QuillForge.exe`.
- Version: `0.1.0` / package manifest version.
- SHA-256 / size: `CDCB3F4ECB9800B44C28D34E92B1F2077D36BF8F0E26F0054EAF6C41746842FE` /
  `38,394,286` bytes; source
  `tree-sha256:7a410bd083789eecf06c7a7e59c1a1d6ca3bf41cd94b85d0ea6249a51ed2e004`.
- Packaging note: D28's portable candidate was rebuilt after D28 source edits;
  it is not the current release candidate and is retained as historical
  evidence. Prior D26, D27, and D28 package identities are historical.

## Disposition

`accepted-with-limits`: D28 editor-document composition is source-level
verified by the parent and independent Luna review; runtime, clean-machine,
and external release gates remain open.

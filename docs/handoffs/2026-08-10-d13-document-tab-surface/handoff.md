# Handoff: 2026-08-10-d13-document-tab-surface

| Field | Value |
|---|---|
| ID | `2026-08-10-d13-document-tab-surface` |
| Delivery / slice | `D13 / ARCH-04 MainWindow document-tab surface coordinator` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-10T08:00:00+08:00` |

## User outcome

QuillForge's document-tab rail now has a focused presentation owner. Tab
creation, selection, record/index mapping, editor reverse lookup, title sync,
and temporary tab-bar locking are centralized while document behavior remains
under the existing MainWindow/application-facing callbacks.

## Scope and boundaries

### In scope

- `DocumentTabSurface` and its structural `DocumentTabLike` contract.
- QTabWidget composition, tab collection/index invariants, active/editor lookup,
  title projection, tab-bar enablement, and callback wiring.
- MainWindow delegation with save/close, path uniqueness, recovery/session,
  editor callback, and lifecycle ownership preserved.
- ADR, architecture/spec/roadmap/task, acceptance/register/index, review,
  package provenance, and release no-go synchronization.

### Out of scope

- No document service, save policy, recovery/session policy, workspace, plugin,
  or command behavior moved into the surface.
- No service locator, dependency-injection framework, singleton, event bus,
  microservice/RPC layer, or wholesale MainWindow rewrite.
- No QApplication startup, screenshots, interactive visual acceptance,
  clean-machine, signing, installer/update, deployment, or hardware evidence.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | parent | Architecture decision, implementation, integration, verification, handoff |
| Project Manager | fixed six-role workflow | Dependencies, risks, and open release gates |
| Product | fixed six-role workflow | User-visible tab behavior and continuity |
| Developer 1 | fixed six-role workflow | MainWindow/presentation ownership boundary |
| Developer 2 | fixed six-role workflow | Qt tab projection, index ordering, and lifetime reasoning |
| QA | fixed six-role workflow | Static/package/no-launch verification |
| Independent reviewer | Kierkegaard / Luna max | Read-only review assigned; bounded waits expired with no conclusion; reviewer closed |

## Changed files and modules

- `src/quillforge/presentation/document_tab_surface.py` — owns the tab rail
  Qt projection, identity/index mapping, active/editor lookup, title sync, and
  tab-bar enablement.
- `src/quillforge/presentation/main_window.py` — composes the surface and
  retains document state, callbacks, save/close, recovery/session, and events.
- `docs/adr/0038-main-window-document-tab-surface.md` — decision and invariants.
- `docs/specs/enterprise-architecture-migration.md` — Phase 2 document-tab
  acceptance.
- `docs/ARCHITECTURE.md`, `docs/ROADMAP.md` — architecture and roadmap projection.
- `tasks/plan.md`, `tasks/todo.md` — D13 tracking.
- `docs/agent-team/reviews/D13-document-tab-surface-parent-review.md` — parent
  review, independent-review limitation, simplification, and validation.
- `docs/agent-team/acceptance.json`, `delivery-register.json`,
  `docs/handoffs/index.json` — acceptance and delivery projections.

## Decisions and constraints

- `DocumentTabSurface` depends on only a structural editor-bearing tab contract;
  it does not import application services or infrastructure.
- MainWindow remains the source of document behavior and path identity policy.
- Records are appended before Qt selection and removed before Qt projection
  removal, keeping current-change callbacks aligned with the record list.
- Membership uses identity, not dataclass equality, for predictable tab lookup.
- Shared checkout writer: Architect. No worktree, unit-test-only asset, or Qt
  launch was used.

## Public-source applicability

This is a Python/PyQt6 desktop slice, not embedded C/C++ or firmware. The
enterprise architecture references are public engineering sources only:
[CloudWeGo About](https://www.cloudwego.io/about/),
[CloudWeGo open-source announcement](https://www.cloudwego.io/blog/2021/09/13/cloudwego-open-source-announcement/),
and [Kitex framework extension](https://www.cloudwego.io/docs/kitex/tutorials/framework-exten/).
They inform explicit boundaries, composable interfaces, and incremental
verification; no private ByteDance standard or certification claim is made.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `uv run python -m compileall -q src\quillforge` | PASS | Whole-source compile after D13 extraction. |
| `uv run ruff check src\quillforge` | PASS | Whole-source lint after D13 extraction. |
| `uv run ruff format --check src\quillforge` | PASS | Whole-source format after correction. |
| D13 source boundary/order probe | PASS | Surface owns projection/index; MainWindow retains behavior. |
| JSON parse for acceptance/register/index | PASS | D13 IDs and latest handoff resolve. |
| `scripts/verify_handoff.ps1` | PASS | Handoff/index status equality verified after this handoff was added. |
| `scripts/check.ps1` | PASS | NOTICE, handoff, source, and formatting gates passed after D13. |
| `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\package.ps1` | PASS | Portable candidate rebuilt after D13 source edit. |
| `scripts/verify_release_handoff.ps1` | EXPECTED NO-GO | Exit 1 with exactly three mechanical failures (`packaged_report_artifact_match`, `interactive_startup_report_consistent`, `startup_preflight_report_consistent`) and ten open gates. |

## Independent review

Kierkegaard / Luna max was assigned a bounded read-only review. Two bounded
waits expired without a conclusion; the reviewer was closed. No child result is
treated as PASS. The parent review records the limitation and supplies source
reasoning instead.

## Simplification assessment

The slice removes direct QTabWidget/list/index management from MainWindow and
keeps one explicit, small presentation boundary. It adds no duplicate document
source, registry, container, singleton, or speculative abstraction. No further
safe simplification is required for this bounded slice.

## Unrun checks and reason

- Qt startup, signal delivery, widget destruction, screenshots, keyboard/tab
  interaction, screen-reader output, and runtime visual acceptance — prohibited
  by the active no-launch policy.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — not created,
  modified, or run under project constraints.
- Clean-machine, cross-machine, signing, installer/update, deployment, and
  hardware checks — not authorized.

## Known risks and limits

- MainWindow remains a large coordinator; workspace/recovery/session/plugin
  slices and the broader contract audit remain open.
- Runtime-native tab metrics, callback timing, accessibility rendering, and
  cross-machine appearance remain unproven.
- D7/D8 legal/clean-machine/release gates remain open.

## Acceptance and evidence IDs

- Acceptance: `D13-AC01`, `S42`.
- Evidence: ADR-0038, source modules, architecture/spec/roadmap/task records,
  parent review, this handoff, static checks, and package manifest.

## Next owner and next action

- Owner: Architect.
- Action: continue the next bounded MainWindow coordinator slice only after
  D13 package and release-no-go evidence are refreshed; keep open runtime and
  release gates explicit.

## Artifact information

- Artifact path: `QuillForge.exe` and `dist/QuillForge.exe`.
- Version: `0.1.0` / package manifest version.
- SHA-256 / size: `C30117128BA55F3787B0849A8C88DAA86D0C6507E58098D2B07914BC2A7FF7BD` / `38,374,607` bytes; source `tree-sha256:0b443af09ede15d29d60b51e3fc5c34ab6b6f981a2888c1763bba08c7c87e576`.
- Packaging note: portable package evidence is not release approval.

## Disposition

`accepted-with-limits`: D13 document-tab surface extraction is source-level
behavior-preserving by reviewed invariants; the bounded independent review
returned no conclusion, runtime interaction remains unrun, and the remaining
enterprise/release gates remain limited or open as recorded above.

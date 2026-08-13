# Handoff: 2026-08-10-d11-enterprise-composition-root

| Field | Value |
|---|---|
| ID | `2026-08-10-d11-enterprise-composition-root` |
| Delivery / slice | `D11 / ARCH-01..02 enterprise architecture baseline and composition root` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-10T06:00:00+08:00` |

## User outcome

QuillForge has begun the enterprise-architecture migration with a documented,
public-source-grounded baseline and a concrete first refactor. Adapter wiring
and desktop lifecycle are now isolated in `composition.py`; Qt-free diagnostic
adapter selection is isolated in `diagnostic_composition.py`; `app.py` remains
a small entry-point/diagnostic dispatcher and Qt event-loop owner.

## Scope and boundaries

### In scope

- Public-source applicability record and explicit non-claims about ByteDance.
- ADR-0036 and enterprise migration specification.
- `DesktopRuntime` composition/lifecycle extraction.
- Architecture docs, project-local skill, plan, acceptance, register, roadmap,
  handoff, package, and release provenance synchronization.

### Out of scope

- No wholesale folder rewrite, microservice/RPC layer, service locator,
  dependency-injection framework, or public behavior change.
- No MainWindow decomposition yet; no domain/application/infrastructure
  contract rewrite.
- No Qt startup, screenshot, runtime lifecycle, clean-machine, signing,
  installer/update, deployment, or hardware evidence.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | parent | Architecture decision, implementation, integration, verification, handoff |
| Project Manager | fixed six-role workflow | Dependencies, risks, and D11 status |
| Product | fixed six-role workflow | Extensibility/user-outcome acceptance |
| Developer 1 | fixed six-role workflow | Dependency-boundary review |
| Developer 2 | fixed six-role workflow | Composition/lifecycle implementation review |
| QA | fixed six-role workflow | Static/package/no-launch verification |
| Independent reviewer | Hubble / Terra max | Cross-module architecture and lifecycle review; no conclusion in bounded wait, reviewer closed |

## Changed files and modules

- `src/quillforge/composition.py` — creates concrete services/adapters and owns
  `DesktopRuntime.start/stop`.
- `src/quillforge/diagnostic_composition.py` — assembles the Qt-free diagnostic
  search adapter boundary.
- `src/quillforge/app.py` — dispatches diagnostics, creates the Qt application,
  runs the event loop, and delegates desktop composition.
- `docs/specs/enterprise-architecture-migration.md` — baseline and phases.
- `docs/adr/0036-enterprise-composition-root.md` — architecture decision.
- `docs/ARCHITECTURE.md` — synchronized composition-root and lifecycle contract.
- `skills/quillforge-enterprise-architecture/*` — local architecture workflow.
- `tasks/plan.md`, `tasks/todo.md`, `docs/ROADMAP.md` — D11 tracking.
- `docs/agent-team/acceptance.json`, `delivery-register.json`,
  `docs/handoffs/index.json` — D11 acceptance/evidence projections.

## Decisions and constraints

- `app.py` does not import concrete infrastructure modules on either the
  desktop entry path or the diagnostic path. `composition.py` owns desktop
  selection and `diagnostic_composition.py` owns the Qt-free diagnostic choice.
- Qt-free diagnostic branches remain before the lazy QApplication/composition
  imports.
- `DesktopRuntime` preserves activation, restore, menu refresh, show, event
  loop, and deactivation ordering from the prior composition.
- Public CloudWeGo references inform interface-first, decoupled, incremental,
  observable extension principles; no private ByteDance standard is claimed.
- Shared checkout writer: Architect. No worktree, unit-test-only asset, or Qt
  launch was used.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `uv run python -m compileall -q src\\quillforge` | PASS | Whole-source compile. |
| `uv run ruff check src\\quillforge` | PASS | Whole-source lint. |
| `uv run ruff format --check src\\quillforge` | PASS | Whole-source format. |
| Source architecture/lifecycle probe | PASS | Composition ownership, lazy diagnostic boundary, and lifecycle order. |
| `scripts/verify_handoff.ps1` | PASS | Final handoff/index status equality. |
| `scripts/check.ps1` | PASS | Repository static gate. |
| `scripts/package.ps1` via PowerShell Core | PASS | Final identity recorded below. |
| `scripts/verify_release_handoff.ps1` | EXPECTED NO-GO (exit 1) | Existing three mechanical failures and ten external gates remain. |

## Unrun checks and reason

- QApplication/Qt startup, desktop runtime lifecycle, screenshots, and visual
  acceptance — prohibited by the active no-launch policy.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — not created,
  modified, or run.
- Clean-machine, cross-machine, signing, installer/update, deployment, and
  hardware checks — not authorized.

## Known risks and limits

- `MainWindow` is still a large presentation coordinator; future extraction
  must preserve existing application ownership and operation IDs.
- Source-level order review cannot prove runtime event-loop equivalence without
  authorized execution.
- Release remains `NO-GO`; D11 does not close D7/D8 or external gates.

## Acceptance and evidence IDs

- Acceptance: `D11-AC01`, `S40`.
- Evidence: specification, ADR-0036, composition/app source, architecture
  docs, parent review, this handoff, `tasks/plan.md`, `tasks/todo.md`,
  `scripts/verify_handoff.ps1`, `scripts/check.ps1`, package script, and
  `dist/QuillForge.release.json`.

## Next owner and next action

- Owner: Architect.
- Action: retain the explicit Terra no-conclusion record and plan ARCH-03
  MainWindow coordinator extraction with a separate contract and acceptance
  slice.

## Artifact information

- Artifact path: `QuillForge.exe` and `dist/QuillForge.exe`.
- Version: `0.1.0` / package manifest version.
- SHA-256 / size: `7C29D8E012DDD8F8EDFDD467671614D3ACB4CA273A4EDA883C86578EE569A8B1` / `38,369,904` bytes.
- Packaging note: portable package rebuild is required before D11 closure;
  this is not release approval.

## Disposition

`accepted-with-limits`: D11's public-source baseline and composition-root
extraction are package-closed with an explicit independent-review no-conclusion
record. Full architecture migration and runtime/release evidence remain open.

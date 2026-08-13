# D11 / ARCH-01..02 parent review: enterprise composition root

| Field | Value |
|---|---|
| Hook | `after-design` + `after-source-change` |
| Scope | Public-source architecture baseline and first desktop composition-root migration |
| Decision | `accepted-with-limits` |
| Owner | Architect |
| Checkout | Current local checkout only |

## User outcome

QuillForge now has an explicit enterprise-architecture baseline and a first
low-risk migration slice. The entry-point dispatcher is no longer responsible
for constructing every desktop concrete adapter: `composition.py` owns desktop
wiring and lifecycle, `diagnostic_composition.py` owns Qt-free diagnostic
adapter selection, and `app.py` owns mode dispatch and the Qt event loop.

This is an incremental architecture delivery, not a claim that every module has
already been rewritten. The next slices are MainWindow coordinator extraction,
contract/error/observability audit, and authorized runtime/release evidence.

## Public-source applicability

The architecture baseline cites public primary sources only:

- CloudWeGo's public About page and open-source announcement describe
  independently usable components, high extensibility/reliability, feedback
  loops, verified incremental releases, and separation from internal ecology.
- Kitex's public extension documentation describes interface-oriented
  middleware/options/suites rather than hard-coding one implementation.
- Hertz's public repository documents extension, logging, monitoring, tracing,
  and error handling as explicit framework concerns.
- Python 3.12 `typing.Protocol` documents the structural contract mechanism used
  by application ports.
- Qt 6 `QObject` and `QCoreApplication` documentation define object ownership,
  signals, and event-loop responsibilities relevant to the composition boundary.
- PyInstaller's public runtime documentation defines frozen identity and bundled
  path behavior relevant to the package/diagnostic boundary.

These are public engineering references. They are not ByteDance private rules,
manufacturer requirements, certification evidence, or a claim of internal
ByteDance implementation equivalence.

## Architecture decision

- Keep the dependency direction `presentation -> application -> domain`; keep
  infrastructure behind application ports and plugins behind public API
  contracts.
- Add `quillforge.composition` as the concrete desktop composition root, with
  `DesktopRuntime` as the named lifecycle owner.
- Keep `quillforge.app.main()` responsible for argument dispatch, Qt-free
  diagnostics before Qt import, `QApplication` creation, `application.exec()`,
  and final shutdown.
- Preserve the old construction/activation sequence exactly: settings/theme,
  services, built-in registration, MainWindow wiring, provider/notifier wiring,
  plugin activation, startup restore, menu refresh, show, event loop, and plugin
  deactivation.
- Do not add a service locator, singleton, dependency-injection framework,
  microservice/RPC layer, or speculative contract duplication.

## Changed files and modules

- `src/quillforge/composition.py` — new desktop composition root and
  `DesktopRuntime.start/stop` lifecycle.
- `src/quillforge/app.py` — keeps diagnostic dispatch and event-loop ownership;
  delegates concrete assembly to named composition boundaries.
- `src/quillforge/diagnostic_composition.py` — keeps packaged workspace-search
  diagnostics Qt-free while isolating the concrete provider choice.
- `docs/specs/enterprise-architecture-migration.md` — public-source baseline,
  invariants, migration phases, acceptance, and non-goals.
- `docs/adr/0036-enterprise-composition-root.md` — records the decision and
  alternatives.
- `docs/ARCHITECTURE.md` — synchronizes the composition-root description.
- `skills/quillforge-enterprise-architecture/SKILL.md` — local reusable
  workflow for future architecture slices.
- `tasks/plan.md`, `tasks/todo.md`, `docs/ROADMAP.md` — track D11 and remaining
  migration work.

## Fixed-role input and ownership

| Role | Owner / agent | Contribution | Disposition |
|---|---|---|---|
| Architect | parent | Integrated source audit, public-source baseline, ADR, code migration, and final evidence | accepted |
| Project Manager | fixed workflow | D11 dependencies and external gates remain tracked in the project plan | incorporated |
| Product | fixed workflow | User outcome is extensibility without a destabilizing rewrite | incorporated |
| Developer 1 | fixed workflow | Boundary ownership and single-writer constraints | incorporated |
| Developer 2 | fixed workflow | Composition/lifecycle implementation scope | incorporated |
| QA | fixed workflow | Static/package/no-launch verification matrix | incorporated |
| Independent architecture reviewer | Hubble / Terra max | Cross-module dependency, lifecycle, diagnostic-boundary, and coupling review | no conclusion; bounded wait expired and reviewer was closed |

## Simplification assessment

The extraction removes concrete-construction noise from `app.py` without
introducing a container or indirection framework. `DesktopRuntime` has only two
owned resources and two lifecycle methods; it does not duplicate application
services or hide them behind a dictionary. The diagnostic branches remain lazy,
so Qt-free packaged probes keep their existing import boundary. No further
behavior-preserving simplification was identified before the independent Terra
review.

## Independent review result

Hubble / Terra max was assigned a bounded read-only review of
`composition.py`/`app.py`. No conclusion was returned within the bounded wait
window; the reviewer was closed and no PASS is claimed. The parent review
therefore remains the controlling independent-review record with this explicit
no-conclusion disposition.

## Authorized verification

- `uv run python -m compileall -q src\\quillforge` — PASS.
- `uv run ruff check src\\quillforge` — PASS.
- `uv run ruff format --check src\\quillforge` — PASS.
- `scripts/check.ps1` — PASS after source/spec/ADR synchronization.
- Source architecture probe — PASS: app has no concrete infrastructure imports,
  named composition modules own concrete adapter imports, diagnostics precede
  Qt imports, and lifecycle calls remain ordered.
- `scripts/verify_handoff.ps1` — PASS after final D11 handoff/index sync.
- `scripts/package.ps1` — PASS after the D11 source slice.
- `scripts/verify_release_handoff.ps1` — expected existing NO-GO after package;
  it must retain exactly the known three mechanical failures and ten open gates.

## Unrun checks and reason

- QApplication/Qt startup, MainWindow lifecycle execution, screenshots, visual
  behavior, and runtime coordinator proof — prohibited by the project no-launch
  policy.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — not created,
  modified, or run under project constraints.
- Clean-machine, cross-machine, signing, installer/update, deployment, and
  hardware evidence — not authorized and outside this local architecture slice.

## Known risks and limits

- `MainWindow` remains a large presentation coordinator; this handoff does not
  claim full enterprise migration completion.
- `composition.py` is intentionally a composition root and may remain long;
  business logic must not be moved into it.
- Runtime lifecycle equivalence is source-reviewed, not executed under the
  no-launch policy.
- D7/D8 and release gates remain open; architecture evidence cannot close them.

## Acceptance and evidence IDs

- Acceptance: `D11-AC01`, `S40`.
- Evidence: `docs/specs/enterprise-architecture-migration.md`,
  `docs/adr/0036-enterprise-composition-root.md`,
  `src/quillforge/composition.py`, `src/quillforge/app.py`,
  `docs/ARCHITECTURE.md`, this review,
  `docs/handoffs/2026-08-10-d11-enterprise-composition-root/handoff.md`,
  `tasks/plan.md`, `tasks/todo.md`, `scripts/check.ps1`, and package manifest.

## Next owner and next action

- Owner: Architect.
- Action: integrate the independent Terra conclusion, refresh the package and
  live artifact ledgers, then plan MainWindow coordinator extraction as ARCH-03.

## Artifact information

- Artifact path: `QuillForge.exe` and `dist/QuillForge.exe`.
- Version: `0.1.0` / package manifest version.
- SHA-256 / size: `7C29D8E012DDD8F8EDFDD467671614D3ACB4CA273A4EDA883C86578EE569A8B1` / `38,369,904` bytes.
- Packaging note: portable package must be rebuilt after the composition-root
  source change; package evidence is not release approval.

## Disposition

`accepted-with-limits`: the public-source baseline and composition-root slice
are implemented with preserved source-level lifecycle order. Full migration,
runtime proof, and external release gates remain open.

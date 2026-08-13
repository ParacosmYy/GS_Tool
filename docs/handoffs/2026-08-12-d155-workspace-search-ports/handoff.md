# Handoff: 2026-08-12-d155-workspace-search-ports

| Field | Value |
|---|---|
| ID | 2026-08-12-d155-workspace-search-ports |
| Delivery / slice | D155 / ARCH-142 workspace-search Ports contract |
| Status | `accepted-with-limits` |
| Owner | architect |
| Checkout | Current local checkout only |
| Created | 2026-08-12T23:45:00+08:00 |

## User outcome

Workspace-search result projection now exposes its existing stale,
invalidated, invalid, valid, and failure boundary through named immutable
Ports. The callback replacement also closed an immediately detected stale
member-reference regression before the slice was accepted.

## Scope and boundaries

### In scope

- Frozen/slotted Qt-free `WorkspaceSearchPorts` contract.
- Tracker stale/invalidation classification and result projection preservation.
- MainWindow named wiring, source, inline, static, compile, package, and
  traceability evidence.

### Out of scope

- No workspace search service, query/cancellation generation, result model,
  surface behavior, containment policy, notification wording, Qt surface,
  locale/theme/motion projection, or runtime-startup change.
- No QApplication/EXE launch, native rendering, clean-machine, cross-machine,
  signing, installer, updater, legal, support, or release-owner evidence.

## Team roles and ownership

| Role | Owner / agent | Result |
|---|---|---|
| Architect | Aquinas the 5th / Luna max | `NO_CONCLUSION` after bounded window; no architecture PASS |
| Independent review | Boole the 5th / Luna max | `NO_CONCLUSION` after bounded window; no independent PASS |
| Parent | Architect | `PASS`; sole writer, integration, review, simplification, and verification |

## Changed files and modules

- `src/quillforge/presentation/workspace_search_coordinator.py` — frozen/slotted
  named Ports, preserved branch projection, and root-cause fix for the failure
  path's stale surface accessor.
- `src/quillforge/presentation/main_window.py` — named Ports construction only.
- `tasks/plan.md` and `tasks/todo.md` — bounded D155 scope and status.
- `docs/adr/0204-workspace-search-ports.md`
- `docs/agent-team/reviews/D155-workspace-search-ports-parent-review.md`
- `docs/agent-team/reviews/D155-workspace-search-ports-independent-review.md`

## Decisions and constraints

- The coordinator owns only callback classification and result projection;
  search service, query/generation cancellation, result surface, containment,
  notification, worker, and close policy remain outside it.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, inline, package, and static
  evidence are the authorized validation boundary.

## Verification commands and results

- `D155-WORKSPACE-SEARCH-BRANCH-PROBE=PASS`
- `D155-STALE-INVALIDATED-INVALID-VALID-FAILURE-PROBE=PASS`
- `D155-PORTS-IMMUTABILITY-PROBE=PASS`
- `D155-SOURCE-WIRING-PROBE=PASS`
- `D155-QT-FREE-CONTRACT-PROBE=PASS`
- `D155-PRESENTATION-AUDIT=PASS`
- `D155-COMPILEALL=PASS`
- `D155-RUFF=PASS`
- `D155-FORMAT=PASS`
- `D155-PACKAGE-BUILD=PASS`
- `D155-PACKAGE-IDENTITY-PROBE=PASS`
- expected release `NO-GO` and no-launch/traceability checks.

## Unrun checks and reason

- Architect and independent conclusions — bounded child windows timed out;
  recorded as `NO_CONCLUSION`, not PASS.
- QApplication/native worker/event timing, filesystem traversal, runtime
  startup, clean-machine, cross-machine, signing, installer, updater, legal,
  support, and release owner checks — prohibited or outside current
  authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static/inline evidence cannot prove callback timing relative to queued Qt
  delivery or filesystem traversal behavior.
- The candidate remains unsigned and release remains NO-GO while report,
  clean-machine, legal, installer/update, and release-owner gates are open.

## Acceptance and evidence IDs

- Acceptance: `S208`, `D155-AC01`.
- Evidence: ADR-0204, parent/independent review records, D155 probes, static
  checks, package manifest, handoff/index/register checks, expected release
  NO-GO, and explicit runtime limits.

## Next owner and next action

- Owner: Architect.
- Action: continue the next bounded MainWindow/application or visual-quality
  slice and complete authorized runtime/release gates when authority and
  environment permit.

## Artifact information

The candidate was rebuilt after the Ports-contract change without launching
QuillForge:

- Artifact: `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `54B05D3C49B6D36FBBBFD6574F7DDA1794457896D145C72FF3CEFD685FF1556E`
- Size: `38545481` bytes
- Source revision: `tree-sha256:bb8d9f11d89541898d2564e5646e7e01596d8b3a8f643a5cca5c7a1d23de14b9`
- Manifest: `dist/QuillForge.release.json`

## Disposition

`accepted-with-limits`: workspace-search result projection now has a named
immutable contract with unchanged stale/invalidation/invalid/valid/failure
behavior; native timing, traversal, runtime, release, and external evidence
gates remain open.

# Handoff: 2026-08-10-d85-workspace-navigation-coordinator

| Field | Value |
|---|---|
| ID | `2026-08-10-d85-workspace-navigation-coordinator` |
| Delivery / slice | `D85 / ARCH-60 workspace-navigation coordinator boundary` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-10T23:59:00+08:00` |

## User outcome

Workspace open and directory-navigation completion handling now has one
focused Qt-free coordinator. The shell still owns WorkspaceService,
TaskRunner, workspace activation, root/search policy, file activation,
containment, session persistence, and close behavior.

## Scope and boundaries

### In scope

- Qt-free `WorkspaceNavigationCoordinator` for open, directory, and failure
  completion classification.
- Existing tracker stale/invalidated/current lifecycle and loading/error
  projection through a minimal surface Protocol.
- MainWindow callback wiring and removal of the three old completion methods.
- Preservation of valid workspace activation, directory-root projection,
  search-root invalidation, session continuation, and operation policy.
- Source, static, package, handoff, and release evidence.

### Out of scope

- No WorkspaceService, filesystem traversal, root containment, document
  opening, workspace panel/surface construction, or close policy change.
- No new asynchronous framework, worker, retry, cache, or notification model.
- No native runtime, screenshot, accessibility, clean-machine, signing,
  installer, updater, legal, support, or release-owner change.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Carver the 3rd / Luna max | Read-only boundary consultation; `NO_CONCLUSION` after bounded window |
| Correction architect | Franklin the 3rd / Luna max | Read-only review of local surface/Path correction; `NO_CONCLUSION` |
| Independent review | Bernoulli the 3rd / Luna max | Read-only source review; `NO_CONCLUSION` after bounded window |
| Parent | Architect | Sole writer, integration, source review, and verification |

No child PASS is claimed.

## Changed files and modules

- `src/quillforge/presentation/workspace_navigation_coordinator.py` — Qt-free
  lifecycle, validation, surface, and notification orchestration.
- `src/quillforge/presentation/main_window.py` — coordinator composition and
  callback wiring; workspace/application policy remains local.
- D85 ADR/reviews/handoff and synchronized delivery records.

## Decisions and constraints

- MainWindow retains WorkspaceService, TaskRunner, generic operation/busy
  policy, workspace activation, search invalidation/root projection,
  containment, document opening, session save/restore, and close behavior.
- The coordinator ignores stale callbacks, releases loading for non-stale
  callbacks, preserves invalid-result messages, and never owns a Qt object or
  filesystem policy.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, static, packaging, and release
  handoff checks are the permitted validation boundary.
- This is Python/PyQt6 application/presentation code. Embedded C/C++
  assurance and vendor manufacturer requirements are `N/A`.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| D85 workspace-navigation Qt-free boundary probe | `PASS` | Callback wiring, tracker ownership, stale/invalidated/error boundary, old callback removal, and policy retention. |
| D85 targeted compileall | `PASS` | Changed presentation source. |
| D85 Ruff / format | `PASS` | `uv run ruff check` and `uv run ruff format --check` on changed source. |
| D85 package identity | `PASS` | Root/dist candidate identity recorded below. |
| D85 JSON/traceability/release/no-process probes | `PASS` | `D85-JSON-PARSE-PROBE`, `D85-TRACEABILITY-PROBE`, `D85-RELEASE-DOSSIER-PROBE`, and `D85-NO-PROCESS-PROBE` passed; release remains expected NO-GO. |
| `scripts\verify_handoff.ps1` | `PASS` | Final D85 records and indexed handoff are synchronized. |
| `scripts\check.ps1` | `PASS` | Repository checks passed; 106 files already formatted. |
| `scripts\verify_release_handoff.ps1` | `EXPECTED NO-GO` | Existing artifact-binding and external release gates remain open. |

## Unrun checks and reason

- Native Qt workspace navigation, callback/cancellation timing, accessibility,
  DPI, fonts, runtime startup, clean-machine, cross-machine, signing,
  installer, updater, legal, support, and release-owner checks — prohibited or
  outside authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project R&D policy and not created.
- Hardware/firmware evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static source probes do not prove native callback interleaving, Qt signal
  delivery, or workspace rendering.
- All delegated D85 review windows returned no conclusion; no child PASS is
  claimed.
- The portable candidate remains unsigned and release remains `NO-GO`.

## Acceptance and evidence IDs

- Acceptance: `D85-AC01`, `S114`.
- Evidence: ADR-0110, D85 boundary probe, parent/independent review records,
  static checks, package identity, and expected release NO-GO.

## Next owner and next action

- Owner: Architect.
- Action: synchronize D85 delivery records, run handoff/repository/release
  checks, then continue the next bounded MainWindow/application coordinator
  slice or obtain authorized runtime/release evidence.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and root `QuillForge.exe`.
- Version: `0.1.0`.
- SHA-256 / size: `1DC7A5959B0BA659648B2E34D6C3AB8ECC07995534DD301D14D93FA725AB6398` /
  `38,463,557` bytes.
- Source revision: `tree-sha256:951084c305d30104f483de7b04144081ddc476601e71f22603d812bfceb3d270`.

## Disposition

`accepted-with-limits`: workspace-navigation completion handling is isolated
behind a Qt-free typed boundary and the package identity is recorded, while
native runtime and release gates remain open.

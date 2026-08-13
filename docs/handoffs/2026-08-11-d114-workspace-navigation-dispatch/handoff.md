# Handoff: 2026-08-11-d114-workspace-navigation-dispatch

| Field | Value |
|---|---|
| ID | 2026-08-11-d114-workspace-navigation-dispatch |
| Delivery / slice | D114 / ARCH-86 workspace-navigation dispatch callback boundary |
| Status | `accepted-with-limits` |
| Owner | architect |
| Checkout | Current local checkout only |
| Created | 2026-08-11T01:15:00+08:00 |

## User outcome

Workspace opening and directory navigation now share one typed callback-binding
boundary. MainWindow still owns the service, worker, operation admission,
generation allocation, surface, containment, session, persistence,
notification, and close policy.

## Scope and boundaries

### In scope

- Add typed workspace-navigation operation/success/failure/dispatcher
  contracts.
- Add submit_open(...) and submit_directory(...) to the existing Qt-free
  coordinator.
- Route MainWindow workspace-open and directory-load dispatch through those
  methods.
- Preserve valid/invalid result handling, stale and generation invalidation,
  loading/error projection, session-restore continuation, and synchronous
  dispatcher exception behavior.
- Record public-source applicability, independent review, parent review,
  simplification, static, package, and release-limit evidence.

### Out of scope

- No WorkspaceService, filesystem provider, WorkspaceOperationTracker policy,
  TaskRunner implementation, surface composition, file activation,
  session-schema, persistence, or close-policy change.
- No new asynchronous framework, retry, cache, event bus, service locator, or
  generic runner adapter was introduced.
- No QApplication launch, native rendering, screenshot, clean-machine,
  cross-machine, signing, installer, updater, legal, support, or
  release-owner evidence.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Boyle the 4th / Luna max | Read-only D114 boundary consultation; NO_CONCLUSION after bounded wait |
| Independent review | Pascal the 4th / Luna max | Read-only D114 review; NO_CONCLUSION after two bounded waits |
| Parent | Architect | Sole writer, integration, source review, simplification, packaging, and verification |

No child PASS is claimed.

## Changed files and modules

- src/quillforge/presentation/workspace_navigation_coordinator.py — typed
  dispatcher contracts and callback-binding methods.
- src/quillforge/presentation/main_window.py — open/directory dispatch now
  uses the coordinator boundary.
- docs/adr/0142-workspace-navigation-dispatch-callback-boundary.md —
  decision, invariants, alternatives, applicability, review, simplification,
  and limits.
- docs/agent-team/reviews/D114-arch-86-workspace-navigation-parent-review.md —
  parent review.
- docs/agent-team/reviews/D114-arch-86-workspace-navigation-independent-review.md
  — independent NO_CONCLUSION record.
- docs/specs/enterprise-architecture-migration.md, docs/ARCHITECTURE.md,
  docs/ROADMAP.md, tasks/plan.md, tasks/todo.md.

## Decisions and constraints

- The existing WorkspaceNavigationCoordinator remains the sole
  workspace-navigation callback lifecycle owner; no parallel dispatcher
  abstraction was added.
- MainWindow retains WorkspaceService, TaskRunner, generic operation/busy
  policy, generation admission, activation/containment, session ordering,
  persistence, notification, and close behavior.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, inline, package, and
  non-launching static evidence are the permitted validation boundary.
- This is Python/PyQt6 application/presentation code. Embedded C/C++
  assurance and vendor manufacturer requirements are N/A.
- Public CloudWeGo material remains an engineering reference only; no private
  ByteDance standard, certification, or compliance claim is made.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| D114-WORKSPACE-NAVIGATION-DISPATCH-PROBE=PASS | PASS | Production-class inline probe covered valid open/directory, invalid result, current failure, generation-mismatch invalidation, cancelled/stale callbacks, and dispatcher exception propagation. |
| D114-SOURCE-DEPENDENCY-PROBE=PASS | PASS | Typed contracts/methods and MainWindow open/directory routes are present; coordinator remains a Qt-free boundary. |
| uv run python -m compileall -q src scripts | PASS | Static compilation only; no QApplication launch. |
| uv run ruff check src scripts | PASS | All checks passed. |
| uv run ruff format --check src scripts | PASS | All files are formatted. |
| uv run python scripts/audit_presentation_contracts.py | PASS | Existing presentation contract/error/observability gate. |
| D114-PACKAGE-IDENTITY-PROBE=PASS | PASS | EE17704F3765B4ED271998B543ECADB08C4643F5A9985404D932573F519B36CE; 38,498,193 bytes; source tree-sha256:041e6887e060c1a00482f30872f5dd7f234c9737f028277ea066d99a99910bb6. |
| D114-PACKAGE-NO-LAUNCH-PROBE=PASS | PASS | Package completed without launching QuillForge; no process was started by the package command. |
| D114-JSON-TRACEABILITY-PROBE=PASS | PASS | Acceptance, register, index, and release manifest are synchronized to D114 identity. |
| D114-RELEASE-DOSSIER-PROBE=PASS | PASS | Current dossier is D114-bound and intentionally no-go. |
| D114-RELEASE-EXPECTED-NO-GO=PASS | PASS | Existing open runtime/release gates keep the release verifier intentionally non-zero. |
| scripts\verify_handoff.ps1 | PASS | Final handoff/index contract. |
| scripts\check.ps1 | PASS | Repository/static checks after traceability updates. |

## Unrun checks and reason

- Native TaskRunner timing, QApplication startup, actual workspace filesystem
  navigation/durability, rendering, screen-reader output, DPI, fonts,
  clean-machine, cross-machine, signing, installer, updater, legal, support,
  and release-owner checks — prohibited or outside current authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static/inline probes prove callback shape and lifecycle ordering, not native
  queued timing, filesystem behavior, or human visual perception.
- Boyle architecture returned NO_CONCLUSION; Pascal independent review also
  returned NO_CONCLUSION. No child PASS is claimed.
- The portable candidate remains unsigned and release remains NO-GO; known
  report-binding failures and external release gates remain open.

## Acceptance and evidence IDs

- Acceptance: S146, D114-AC01.
- Evidence: ADR-0142, D114 source/dispatch probes, parent/independent review
  records, compile/lint/format checks, package identity, handoff/index/register
  checks, expected release NO-GO, and explicit runtime limits.

## Next owner and next action

- Owner: Architect.
- Action: continue the next bounded MainWindow/application contract or
  distinct user-visible visual-quality slice and complete authorized
  runtime/release gates when authority and environment permit.

## Artifact information

The portable candidate was rebuilt without launching QuillForge:

- Artifact: dist/QuillForge.exe and root QuillForge.exe
- SHA-256: EE17704F3765B4ED271998B543ECADB08C4643F5A9985404D932573F519B36CE
- Size: 38,498,193 bytes
- Source revision: tree-sha256:041e6887e060c1a00482f30872f5dd7f234c9737f028277ea066d99a99910bb6
- Manifest: dist/QuillForge.release.json

## Disposition

accepted-with-limits: workspace open/directory callback binding is consolidated
behind the existing Qt-free typed coordinator, while native runtime,
filesystem, and enterprise release gates remain open.

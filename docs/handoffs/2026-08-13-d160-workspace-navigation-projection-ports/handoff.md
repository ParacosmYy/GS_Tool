# Handoff: 2026-08-13-d160-workspace-navigation-projection-ports

| Field | Value |
|---|---|
| ID | 2026-08-13-d160-workspace-navigation-projection-ports |
| Delivery / slice | D160 / ARCH-147 workspace-navigation projection Ports contract |
| Status | `accepted-with-limits` |
| Owner | architect |
| Checkout | Current local checkout only |
| Created | 2026-08-13T01:05:00+08:00 |

## User outcome

Workspace open and directory projection now expose their existing lifecycle
through named immutable Ports. The opened-result order, surface guards,
directory root projection, session save, and restore continuation remain
explicit, preserving the file/folder navigation boundary.

## Scope and boundaries

### In scope

- Frozen/slotted Qt-free `WorkspaceNavigationProjectionPorts` contract.
- Opened/directory order and missing-surface/root guard preservation.
- MainWindow named wiring, source, inline, static, compile, package, and
  traceability evidence.

### Out of scope

- No WorkspaceService, document open policy, workspace admission/tracker,
  file activation, TaskRunner, Qt surface behavior, notification wording,
  locale/theme/motion projection, close policy, or runtime-startup change.
- No QApplication/EXE launch, native rendering, clean-machine, cross-machine,
  signing, installer, updater, legal, support, or release-owner evidence.

## Team roles and ownership

| Role | Owner / agent | Result |
|---|---|---|
| Architect | Sartre the 5th / Luna max | `NO_CONCLUSION` after bounded window; no architecture PASS |
| Independent review | Kepler the 5th / Luna max | `NO_CONCLUSION` after bounded window; no independent PASS |
| Parent | Architect | `PASS`; sole writer, integration, review, simplification, and verification |

## Changed files and modules

- `src/quillforge/presentation/workspace_navigation_projection_coordinator.py`
  — frozen/slotted named Ports and preserved open/directory projection.
- `src/quillforge/presentation/main_window.py` — named Ports construction only.
- `tasks/plan.md` and `tasks/todo.md` — bounded D160 scope and status.
- `docs/adr/0209-workspace-navigation-projection-ports.md`
- `docs/agent-team/reviews/D160-workspace-navigation-projection-ports-parent-review.md`
- `docs/agent-team/reviews/D160-workspace-navigation-projection-ports-independent-review.md`

## Decisions and constraints

- The coordinator owns only validated result projection; WorkspaceService,
  containment, file activation, TaskRunner, surface, session, startup, close,
  and policy ownership remain outside it.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, inline, package, and static
  evidence are the authorized validation boundary.

## Verification commands and results

- `D160-WORKSPACE-NAVIGATION-BRANCH-PROBE=PASS`
- `D160-OPENED-DIRECTORY-ORDER-SHORT-CIRCUIT-PROBE=PASS`
- `D160-PORTS-IMMUTABILITY-PROBE=PASS`
- `D160-SOURCE-WIRING-PROBE=PASS`
- `D160-QT-FREE-CONTRACT-PROBE=PASS`
- `D160-PRESENTATION-AUDIT=PASS`
- `D160-COMPILEALL=PASS`
- `D160-RUFF=PASS`
- `D160-FORMAT=PASS`
- `D160-PACKAGE-BUILD=PASS`
- `D160-PACKAGE-IDENTITY-PROBE=PASS`
- expected release `NO-GO` and no-launch/traceability checks.

## Unrun checks and reason

- Architect and independent conclusions — bounded child windows timed out;
  recorded as `NO_CONCLUSION`, not PASS.
- QApplication/native event timing, file/folder activation, filesystem
  behavior, runtime startup, clean-machine, cross-machine, signing,
  installer, updater, legal, support, and release owner checks — prohibited or
  outside current authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static/inline evidence cannot prove event timing relative to native workspace
  signals or actual file/folder activation.
- The candidate remains unsigned and release remains NO-GO while report,
  clean-machine, legal, installer/update, and release-owner gates are open.

## Acceptance and evidence IDs

- Acceptance: `S213`, `D160-AC01`.
- Evidence: ADR-0209, parent/independent review records, D160 probes, static
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
- SHA-256: `177B037CB066DC976E5DA4F22F59AD59971646475A176F7DAE035C5D10ED62FB`
- Size: `38547989` bytes
- Source revision: `tree-sha256:b72e02761e3b409da9393f8a5119f4e563c68d5667e9ca44a34758cec5d39c22`
- Manifest: `dist/QuillForge.release.json`

## Disposition

`accepted-with-limits`: workspace navigation projection now has a named
immutable contract with unchanged opened/directory order and guards; native
timing, activation, runtime, release, and external evidence gates remain
open.

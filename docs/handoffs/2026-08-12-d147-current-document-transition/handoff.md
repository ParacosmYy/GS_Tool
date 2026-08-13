# Handoff: 2026-08-12-d147-current-document-transition

| Field | Value |
|---|---|
| ID | 2026-08-12-d147-current-document-transition |
| Delivery / slice | D147 / ARCH-132 current-document transition projection |
| Status | `accepted-with-limits` |
| Owner | architect |
| Checkout | Current local checkout only |
| Created | 2026-08-12T15:00:00+08:00 |

## User outcome

Switching tabs now has one explicit, inspectable presentation boundary. Find
state resets first, the active document context remains readable, status and
session metadata refresh in the established order, and empty-tab transitions
retain their existing no-notification behavior.

## Scope and boundaries

### In scope

- `CurrentDocumentTransitionCoordinator[TabT]` and its frozen/slotted Ports.
- MainWindow current-tab callback wiring.
- Exact transition-order and empty-tab probes, package identity, and records.

### Out of scope

- No editor mutation, document service, tab identity, persistence
  implementation, async worker, close policy, locale refresh, QSS, or runtime
  startup change.
- No QApplication/EXE launch, native signal/rendering, clean-machine,
  cross-machine, signing, installer, updater, legal, support, or release-owner
  evidence.

## Team roles and ownership

| Role | Owner / agent | Result |
|---|---|---|
| Architect | Linnaeus the 5th / Luna max | `NO_CONCLUSION` after bounded window; no child PASS |
| Independent review | Ptolemy the 5th / Luna max | `NO_CONCLUSION` after bounded window; no independent PASS |
| Parent | Architect | `PASS`; sole writer, integration, review, simplification, and verification |

## Changed files and modules

- `src/quillforge/presentation/current_document_transition_coordinator.py` —
  Qt-free generic coordinator and Ports contract.
- `src/quillforge/presentation/main_window.py` — composition/wiring and thin
  current-tab callback adapter.
- `tasks/plan.md` and `tasks/todo.md` — bounded D147 scope and status.
- `docs/adr/0194-current-document-transition.md`
- `docs/agent-team/reviews/D147-current-document-transition-parent-review.md`
- `docs/agent-team/reviews/D147-current-document-transition-independent-review.md`

## Decisions and constraints

- MainWindow remains the sole owner of Qt, tab identity, FindSurface,
  StatusSurface, notification policy, session timers, and application policy.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, inline, package, and static
  evidence are the authorized validation boundary.

## Verification commands and results

- `D147-TRANSITION-PROBE=PASS`
- `D147-SOURCE-WIRING-PROBE=PASS`
- `D147-PRESENTATION-AUDIT=PASS`
- `D147-COMPILEALL=PASS`
- `D147-RUFF=PASS`
- `D147-FORMAT=PASS`
- `D147-PACKAGE-BUILD=PASS`
- `D147-PACKAGE-IDENTITY-PROBE=PASS`
- `D147-CHECK=PASS`
- `D147-VERIFY-HANDOFF=PASS`
- expected release `NO-GO` and no-launch/traceability checks.

## Unrun checks and reason

- Architect and independent conclusions — bounded child windows timed out;
  recorded as `NO_CONCLUSION`, not PASS.
- QApplication/native signal timing, focus/rendering, font/DPI,
  accessibility, runtime startup, clean-machine, cross-machine, signing,
  installer, updater, legal, support, and release-owner checks — prohibited or
  outside current authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static and inline ordering probes cannot prove queued Qt signal interleavings
  or focus behavior on every Windows font/DPI environment.
- The candidate remains unsigned and release remains NO-GO while report,
  clean-machine, legal, installer/update, and release-owner gates are open.

## Acceptance and evidence IDs

- Acceptance: `S198`, `D147-AC01`.
- Evidence: ADR-0194, parent/independent review records, D147 probes, static
  checks, package manifest, handoff/index/register checks, expected release
  NO-GO, and explicit runtime limits.

## Next owner and next action

- Owner: Architect.
- Action: continue the next bounded MainWindow/application or visual-quality
  slice and complete authorized runtime/release gates when authority and
  environment permit.

## Artifact information

The candidate was rebuilt after the source change without launching QuillForge:

- Artifact: `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `4B942B56320B727BF0022B71EAD55E7D95B278467E200D1AA3BBF5B2F3698660`
- Size: `38542225` bytes
- Source revision: `tree-sha256:1cbff80f9605e40c6ea5667f8a97355627d1d41c22cd82dd4cc7f3dbe78badff`
- Manifest: `dist/QuillForge.release.json`

## Disposition

`accepted-with-limits`: current-document transition projection is separated
behind a typed Qt-free boundary; native runtime, release, and external
evidence gates remain open.

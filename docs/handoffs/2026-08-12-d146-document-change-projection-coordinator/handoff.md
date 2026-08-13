# Handoff: 2026-08-12-d146-document-change-projection-coordinator

| Field | Value |
|---|---|
| ID | 2026-08-12-d146-document-change-projection-coordinator |
| Delivery / slice | D146 / ARCH-131 editor-change projection |
| Status | `accepted-with-limits` |
| Owner | architect |
| Checkout | Current local checkout only |
| Created | 2026-08-12T14:00:00+08:00 |

## User outcome

Editor changes now pass through one explicit presentation boundary. Dirty
state, tab-title/status refresh, session-save request, content identity, and
Find invalidation retain their established order while MainWindow remains a
composition root rather than the owner of every editor-event sequence.

## Scope and boundaries

### In scope

- `DocumentChangeProjectionCoordinator[EditorT, TabT]` and its
  frozen/slotted Ports contract.
- MainWindow wiring for modified/content callbacks.
- Qt-free/source-order probes, package identity, and architecture traceability.

### Out of scope

- No current-tab transition, editor engine, DocumentService implementation,
  persistence, tab identity, locale, QSS, async worker, close policy, or
  runtime startup change.
- No QApplication/EXE launch, native signal/rendering, clean-machine,
  cross-machine, signing, installer, updater, legal, support, or release-owner
  evidence.

## Team roles and ownership

| Role | Owner / agent | Result |
|---|---|---|
| Architect | Pasteur the 5th / Luna max | `NO_CONCLUSION` after bounded window; no child PASS |
| Independent review | Plato the 5th / Luna max | `NO_CONCLUSION` after bounded window; no independent PASS |
| Parent | Architect | `PASS`; sole writer, integration, review, simplification, and verification |

## Changed files and modules

- `src/quillforge/presentation/document_change_projection_coordinator.py` —
  Qt-free generic coordinator and Ports contract.
- `src/quillforge/presentation/main_window.py` — composition/wiring and thin
  editor callback adapters.
- `tasks/plan.md` and `tasks/todo.md` — bounded D146 scope and status.
- `docs/adr/0193-document-change-projection.md`
- `docs/agent-team/reviews/D146-document-change-projection-parent-review.md`
- `docs/agent-team/reviews/D146-document-change-projection-independent-review.md`

## Decisions and constraints

- MainWindow remains the sole owner of Qt, DocumentService, concrete tabs,
  FindSurface, StatusSurface, session timers, and application policy.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, inline, package, and static
  evidence are the authorized validation boundary.

## Verification commands and results

- `D146-PROJECTION-PROBE=PASS`
- `D146-SOURCE-WIRING-PROBE=PASS`
- `D146-PRESENTATION-AUDIT=PASS`
- `D146-COMPILEALL=PASS`
- `D146-RUFF=PASS`
- `D146-FORMAT=PASS`
- `D146-PACKAGE-BUILD=PASS`
- `D146-PACKAGE-IDENTITY-PROBE=PASS`
- `D146-CHECK=PASS`
- `D146-VERIFY-HANDOFF=PASS`
- expected release `NO-GO` and no-launch/traceability checks.

## Unrun checks and reason

- Architect and independent conclusions — bounded child windows timed out;
  recorded as `NO_CONCLUSION`, not PASS.
- QApplication/native signal timing, editor rendering, font/DPI,
  accessibility, runtime startup, clean-machine, cross-machine, signing,
  installer, updater, legal, support, and release-owner checks — prohibited or
  outside current authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static and inline ordering probes cannot prove queued Qt signal interleavings
  or editor-engine callback timing on every Windows environment.
- The candidate remains unsigned and release remains NO-GO while report,
  clean-machine, legal, installer/update, and release-owner gates are open.

## Acceptance and evidence IDs

- Acceptance: `S197`, `D146-AC01`.
- Evidence: ADR-0193, parent/independent review records, D146 probes, static
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
- SHA-256: `A9CF905D11573A891A7562B98BA571922BFC67922326336E4F48757671D2179C`
- Size: `38539992` bytes
- Source revision: `tree-sha256:3ea9b2f76f24dfcafb512979e1495e164eb7f28184f5d8e0141593d6764da9a1`
- Manifest: `dist/QuillForge.release.json`

## Disposition

`accepted-with-limits`: editor-change projection is separated behind a typed
Qt-free boundary; native runtime, release, and external evidence gates remain
open.

# Handoff: 2026-08-11-d131-replace-all-admission-boundary

| Field | Value |
|---|---|
| ID | 2026-08-11-d131-replace-all-admission-boundary |
| Delivery / slice | D131 / ARCH-108 Replace All admission ports boundary |
| Status | `accepted-with-limits` |
| Owner | architect |
| Checkout | Current local checkout only |
| Created | 2026-08-11T23:00:00+08:00 |

## User outcome

Replace All admission is now explicit and extensible. MainWindow no longer
embeds busy/active-tab/query validation, session construction, operation/job
binding, and duplicate-race cleanup; it continues to own the visible Find/
tab-bar projection, QTimer slices, completion, rollback, and application
policy.

## Scope and boundaries

### In scope

- Frozen/slotted `ReplaceAllAdmissionPorts`.
- `ReplaceAllAdmissionCoordinator` admission and job-binding behavior.
- Named MainWindow mapping for editor, FindSurface, tracker, operation, and
  starter callbacks.
- Static, package, and traceability evidence for D131.

### Out of scope

- No ReplaceAllTracker, ReplaceAllCompletionCoordinator, editor session
  algorithm, QTimer loop, progress, cancellation, rollback, FindSurface,
  tab-bar, locale, command, plugin, document, workspace, or application policy
  behavior changed.
- No new async path, worker, singleton, service locator, EventBus, or
  test-only asset.
- No QApplication launch, native Find/Replace rendering, QTimer timing,
  clean-machine, cross-machine, signing, installer, updater, legal, support,
  or release-owner evidence.

## Team roles and ownership

| Role | Owner / agent | Result |
|---|---|---|
| Architect | Arendt the 4th / Luna max | `NO_CONCLUSION` after two bounded waits; no child PASS |
| Independent review | James the 4th / Luna max | `NO_CONCLUSION` after two bounded waits; no independent PASS |
| Parent | Architect | `PASS`; sole writer, integration, review, simplification, and verification |

## Changed files and modules

- `src/quillforge/presentation/replace_all_admission_coordinator.py` — new
  Qt-free admission and binding boundary.
- `src/quillforge/presentation/main_window.py` — named port wiring and starter
  projection.
- `docs/adr/0170-replace-all-admission-boundary.md`
- `docs/agent-team/reviews/D131-replace-all-admission-parent-review.md`
- `docs/agent-team/reviews/D131-replace-all-admission-independent-review.md`

## Decisions and constraints

- The coordinator owns admission and tracker/operation binding only; the
  existing completion coordinator remains the lifecycle-release owner.
- MainWindow remains the only owner of Qt widgets, QTimer scheduling, editor
  policy, visible status, progress, cancellation, rollback, and close policy.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, inline, package, and static
  evidence are the authorized validation boundary.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `D131-REPLACE-ALL-ADMISSION-BEHAVIOR-PROBE=PASS` | PASS | Exact inflight/busy/empty/value-error/success behavior and branch order. |
| `D131-REPLACE-ALL-ADMISSION-CONTRACT-PROBE=PASS` | PASS | Frozen ports, session/operation/job identity, dirty/content-version, and starter handoff. |
| `D131-QT-FREE-ADMISSION-PROBE=PASS` | PASS | Coordinator import path did not load PyQt6. |
| `D131-COMPILEALL=PASS` | PASS | Static compilation; no QApplication launch. |
| `D131-RUFF=PASS` | PASS | Changed source passed Ruff check. |
| `D131-FORMAT=PASS` | PASS | Changed source passed Ruff format check. |
| `D131-PACKAGE-IDENTITY-PROBE=PASS` | PASS | Root and `dist` candidates match: SHA-256 `311E7599644BDD43E7F50E48917A543AA854E91C7369612924F1B145E0F81E64`, 38,514,094 bytes, source `tree-sha256:f078ee3c5ee76815ddee8b38d58f472a6af214b949c34ff805de3b16d92cf75e`. |
| `D131-PACKAGE-NO-LAUNCH-PROBE=PASS` | PASS | Packaging completed without launching QuillForge; no process remained. |
| `D131-JSON-TRACEABILITY-PROBE=PASS` | PASS | Acceptance, delivery register, handoff index, manifest, and release handoff synchronized after record update. |
| `D131-RELEASE-DOSSIER-PROBE=PASS` | PASS | Release dossier binds the current D131 artifact identity. |
| `D131-RELEASE-EXPECTED-NO-GO=PASS` | PASS | Expected NO-GO remains due open external gates and known report-binding failures. |

## Unrun checks and reason

- Architect and independent review conclusions — child windows timed out;
  recorded as `NO_CONCLUSION`, not PASS.
- Native Find/Replace rendering, QApplication startup, QTimer callback timing,
  editor slicing, accessibility, clean-machine, cross-machine, signing,
  installer, updater, legal, support, and release-owner checks — prohibited or
  outside authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static admission probes cannot prove native Find/Replace rendering, QTimer
  scheduling, editor rollback timing, or filesystem durability.
- Arendt architecture and James independent review windows returned
  `NO_CONCLUSION`; no child PASS is claimed.
- The portable candidate remains unsigned and release remains NO-GO while
  external gates and report-binding gates are open.

## Acceptance and evidence IDs

- Acceptance: `S174`, `D131-AC01`.
- Evidence: ADR-0170, admission behavior/contract probes, parent and
  independent review records, simplification assessment, static checks,
  package identity, handoff/index/register checks, expected release NO-GO, and
  explicit runtime limits.

## Next owner and next action

- Owner: Architect.
- Action: continue the next bounded MainWindow/recovery or visual-quality
  slice and complete authorized runtime/release gates when authority and
  environment permit.

## Artifact information

The portable candidate was rebuilt without launching QuillForge after the
source change:

- Artifact: `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `311E7599644BDD43E7F50E48917A543AA854E91C7369612924F1B145E0F81E64`
- Size: `38514094` bytes
- Source revision: `tree-sha256:f078ee3c5ee76815ddee8b38d58f472a6af214b949c34ff805de3b16d92cf75e`
- Manifest: `dist/QuillForge.release.json`

## Disposition

`accepted-with-limits`: Replace All admission is explicit while native
Find/Replace/runtime/release evidence remains open.

# Handoff: 2026-08-12-d132-recovery-capture-admission-boundary

| Field | Value |
|---|---|
| ID | 2026-08-12-d132-recovery-capture-admission-boundary |
| Delivery / slice | D132 / ARCH-109 recovery capture admission ports boundary |
| Status | `accepted-with-limits` |
| Owner | architect |
| Checkout | Current local checkout only |
| Created | 2026-08-12T01:00:00+08:00 |

## User outcome

Recovery autosave candidate selection is now explicit and extensible.
MainWindow no longer embeds recovery/busy gates, dirty-tab filtering,
inflight/delete-pending exclusion, snapshot identity, dirty-state
normalization, or content-version capture; it still owns the channel/editor
capture and every downstream lifecycle boundary.

## Scope and boundaries

### In scope

- Frozen/slotted `RecoveryCaptureAdmissionPorts`.
- Immutable `RecoveryCaptureAdmission` candidate.
- `RecoveryCaptureAdmissionCoordinator` with per-tab starter sequencing.
- Named MainWindow mapping for recovery state, documents, tabs, and policy.
- Static, package, and traceability evidence for D132.

### Out of scope

- No channel/backpressure, EditorWidget capture, `_RecoveryCaptureJob`,
  RecoveryCaptureTracker registration, RecoveryService, TaskRunner/writer,
  QTimer, progress, abort, delete, notification, close, locale, document,
  workspace, or application policy behavior changed.
- No new async path, worker, singleton, service locator, EventBus, or
  test-only asset.
- No QApplication launch, native recovery rendering, channel/editor timing,
  clean-machine, cross-machine, signing, installer, updater, legal, support,
  or release-owner evidence.

## Team roles and ownership

| Role | Owner / agent | Result |
|---|---|---|
| Architect | Schrodinger the 4th / Luna max | `NO_CONCLUSION` after two bounded waits; no child PASS |
| Independent review | Pasteur the 4th / Luna max | `NO_CONCLUSION` after two bounded waits; no independent PASS |
| Parent | Architect | `PASS`; sole writer, integration, review, simplification, and verification |

## Changed files and modules

- `src/quillforge/presentation/recovery_capture_admission_coordinator.py` —
  new Qt-free candidate-selection boundary.
- `src/quillforge/presentation/main_window.py` — named port wiring and
  existing capture starter projection.
- `docs/adr/0171-recovery-capture-admission-boundary.md`
- `docs/agent-team/reviews/D132-recovery-capture-admission-parent-review.md`
- `docs/agent-team/reviews/D132-recovery-capture-admission-independent-review.md`

## Decisions and constraints

- The coordinator owns admission and immutable candidate construction only;
  MainWindow remains the lifecycle owner after admission.
- Candidate callbacks occur in existing tab order, before the next tab is
  inspected, preserving snapshot and starter timing.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, inline, package, and static
  evidence are the authorized validation boundary.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `D132-RECOVERY-ADMISSION-BEHAVIOR-PROBE=PASS` | PASS | Exact gate/filter/snapshot/state/version behavior and per-tab order. |
| `D132-RECOVERY-ADMISSION-CONTRACT-PROBE=PASS` | PASS | Frozen ports and immutable candidate contract. |
| `D132-QT-FREE-ADMISSION-PROBE=PASS` | PASS | Coordinator import path did not load PyQt6. |
| `D132-COMPILEALL=PASS` | PASS | Static compilation; no QApplication launch. |
| `D132-RUFF=PASS` | PASS | Changed source passed Ruff check. |
| `D132-FORMAT=PASS` | PASS | Changed source passed Ruff format check. |
| `D132-PACKAGE-IDENTITY-PROBE=PASS` | PASS | Root and `dist` candidates match: SHA-256 `446B9AB57EE958224C19D3150DF125A82E6050B90DA60819EA5C2F489FE1177C`, 38,516,374 bytes, source `tree-sha256:837b3de3ba08a8801150ac0d9d577c62e738b97342064e18fbe8b7976e99da57`. |
| `D132-PACKAGE-NO-LAUNCH-PROBE=PASS` | PASS | Packaging completed without launching QuillForge; no process remained. |
| `D132-JSON-TRACEABILITY-PROBE=PASS` | PASS | Acceptance, delivery register, handoff index, manifest, and release handoff synchronized after record update. |
| `D132-RELEASE-DOSSIER-PROBE=PASS` | PASS | Release dossier binds the current D132 artifact identity. |
| `D132-RELEASE-EXPECTED-NO-GO=PASS` | PASS | Expected NO-GO remains due open external gates and known report-binding failures. |

## Unrun checks and reason

- Architect and independent review conclusions — child windows timed out;
  recorded as `NO_CONCLUSION`, not PASS.
- Native recovery rendering, QApplication startup, channel/backpressure timing,
  editor capture timing, QTimer slices, filesystem durability, accessibility,
  clean-machine, cross-machine, signing, installer, updater, legal, support,
  and release-owner checks — prohibited or outside authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static admission probes cannot prove native recovery UI, channel timing,
  editor cursor safety, QTimer scheduling, or persistence durability.
- Schrodinger architecture and Pasteur independent review windows returned
  `NO_CONCLUSION`; no child PASS is claimed.
- The portable candidate remains unsigned and release remains NO-GO while
  external gates and report-binding gates are open.

## Acceptance and evidence IDs

- Acceptance: `S175`, `D132-AC01`.
- Evidence: ADR-0171, recovery admission behavior/contract probes, parent and
  independent review records, simplification assessment, static checks,
  package identity, handoff/index/register checks, expected release NO-GO, and
  explicit runtime limits.

## Next owner and next action

- Owner: Architect.
- Action: continue the next bounded recovery/MainWindow or visual-quality
  slice and complete authorized runtime/release gates when authority and
  environment permit.

## Artifact information

The portable candidate was rebuilt without launching QuillForge after the
source change:

- Artifact: `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `446B9AB57EE958224C19D3150DF125A82E6050B90DA60819EA5C2F489FE1177C`
- Size: `38516374` bytes
- Source revision: `tree-sha256:837b3de3ba08a8801150ac0d9d577c62e738b97342064e18fbe8b7976e99da57`
- Manifest: `dist/QuillForge.release.json`

## Disposition

`accepted-with-limits`: recovery capture admission is explicit while native
recovery/channel/editor/runtime/release evidence remains open.

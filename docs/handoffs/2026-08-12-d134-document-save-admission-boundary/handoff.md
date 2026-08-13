# Handoff: 2026-08-12-d134-document-save-admission-boundary

| Field | Value |
|---|---|
| ID | 2026-08-12-d134-document-save-admission-boundary |
| Delivery / slice | D134 / ARCH-112 document-save admission boundary |
| Status | `accepted-with-limits` |
| Owner | architect |
| Checkout | Current local checkout only |
| Created | 2026-08-12T02:30:00+08:00 |

## User outcome

Document saves now have an explicit typed admission boundary. Busy/startup
restore gates, duplicate target rejection, state/text snapshots, read-only
protection, operation identity, and asynchronous dispatch remain ordered while
save-as selection, result classification, and close/recovery policy retain
their existing behavior.

## Scope and boundaries

### In scope

- `DocumentSaveAdmissionCoordinator` and `DocumentSaveAdmissionPorts`.
- `MainWindow` composition wiring and `_start_save` delegation.
- Static behavior/contract/Qt-free probes, package, and traceability records.

### Out of scope

- No `FileDialogSurface`, `DocumentService`, `DocumentSaveCoordinator`,
  result projection, recovery/session persistence, notification, close, or
  application-policy rewrite.
- No QApplication launch, native dialog, filesystem durability, clean-machine,
  cross-machine, signing, installer, updater, legal, support, or release-owner
  evidence.

## Team roles and ownership

| Role | Owner / agent | Result |
|---|---|---|
| Architect | Cicero the 4th / Luna max | `NO_CONCLUSION` after bounded window; no child PASS |
| Independent review | Epicurus the 4th / Luna max | `NO_CONCLUSION` after bounded window; no independent PASS |
| Parent | Architect | `PASS`; sole writer, integration, review, simplification, and verification |

## Changed files and modules

- `src/quillforge/presentation/document_save_admission_coordinator.py` — new
  Qt-free admission contract and sequencing.
- `src/quillforge/presentation/main_window.py` — named ports wiring and
  `_start_save` delegation only.
- `docs/adr/0174-document-save-admission-boundary.md`
- `docs/agent-team/reviews/D134-document-save-admission-parent-review.md`
- `docs/agent-team/reviews/D134-document-save-admission-independent-review.md`

## Decisions and constraints

- Admission and completion remain separate contracts; result classification
  stays in `DocumentSaveCoordinator`.
- Duplicate target identity remains a tab-surface policy port; filesystem
  persistence remains application/infrastructure-owned.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, inline, package, and static
  evidence are the authorized validation boundary.

## Verification commands and results

- `D134-SAVE-ADMISSION-BEHAVIOR-PROBE=PASS`
- `D134-SAVE-ADMISSION-CONTRACT-PROBE=PASS`
- `D134-QT-FREE-SAVE-PROBE=PASS`
- `D134-COMPILEALL=PASS`
- `D134-RUFF=PASS`
- `D134-FORMAT=PASS`
- `D134-CHECK=PASS`
- `D134-VERIFY-HANDOFF=PASS`
- `D134-PACKAGE-BUILD=PASS`

## Unrun checks and reason

- Architect and independent review conclusions — child windows timed out;
  recorded as `NO_CONCLUSION`, not PASS.
- QApplication startup, native save dialog behavior, worker callback timing,
  filesystem durability, keyboard/accessibility traversal, DPI/font fallback,
  clean-machine, cross-machine, signing, installer, updater, legal, support,
  and release-owner checks — prohibited or outside authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static admission evidence cannot prove native QFileDialog or queued worker
  behavior on every Windows environment.
- Cicero architecture and Epicurus independent review windows returned
  `NO_CONCLUSION`; no child PASS is claimed.
- The portable candidate remains unsigned and release remains NO-GO while
  external and report-binding gates are open.

## Acceptance and evidence IDs

- Acceptance: `S178`, `D134-AC01`.
- Evidence: ADR-0174, save-admission behavior/contract/Qt-free probes, parent
  and independent review records, simplification assessment, static checks,
  package identity, handoff/index/register checks, expected release NO-GO, and
  explicit runtime limits.

## Next owner and next action

- Owner: Architect.
- Action: continue the next bounded MainWindow/application or distinct visual
  gap slice and complete authorized runtime/release gates when authority and
  environment permit.

## Artifact information

The portable candidate was rebuilt without launching QuillForge after the
source change:

- Artifact: `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `235D8A4418D757E99CE7E9888DA635EDC256A445A75A8E261BDAE6FEA157A6BC`
- Size: `38520997` bytes
- Source revision: `tree-sha256:f0f3b2a775f5d27b189eeb7f32b2d4598bc7578dbafbb8c3bb6e6fc07907e4d4`
- Manifest: `dist/QuillForge.release.json`

## Disposition

`accepted-with-limits`: document-save admission is centralized at a typed
presentation boundary while native/runtime/release evidence remains open.

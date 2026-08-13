# Handoff: 2026-08-12-d133-document-open-admission-boundary

| Field | Value |
|---|---|
| ID | 2026-08-12-d133-document-open-admission-boundary |
| Delivery / slice | D133 / ARCH-111 document-open admission boundary |
| Status | `accepted-with-limits` |
| Owner | architect |
| Checkout | Current local checkout only |
| Created | 2026-08-12T02:00:00+08:00 |

## User outcome

Document-open admission is now explicit and auditable: busy/startup-restore
gates, operation identity, session-restore binding, and asynchronous dispatch
are behind a typed Qt-free coordinator. Existing file dialogs, workspace file
activation, folder navigation, result classification, and tab projection keep
their behavior and ownership.

## Scope and boundaries

### In scope

- `DocumentOpenAdmissionCoordinator` and `DocumentOpenAdmissionPorts`.
- `MainWindow` composition wiring and `_start_open` delegation.
- Static behavior/contract/Qt-free probes, package, and traceability records.

### Out of scope

- No `FileDialogSurface`, `WorkspacePanel`, `DocumentService`,
  `DocumentOpenCoordinator`, tab/editor projection, line navigation,
  session-restore result, notification, close, or application-policy rewrite.
- No QApplication launch, native dialog, filesystem durability, clean-machine,
  cross-machine, signing, installer, updater, legal, support, or release-owner
  evidence.

## Team roles and ownership

| Role | Owner / agent | Result |
|---|---|---|
| Architect | Laplace the 4th / Luna max | `NO_CONCLUSION` after bounded window; no child PASS |
| Independent review | Sartre the 4th / Luna max | `NO_CONCLUSION` after bounded window; no independent PASS |
| Parent | Architect | `PASS`; sole writer, integration, review, simplification, and verification |

## Changed files and modules

- `src/quillforge/presentation/document_open_admission_coordinator.py` — new
  Qt-free admission contract and sequencing.
- `src/quillforge/presentation/main_window.py` — named ports wiring and
  `_start_open` delegation only.
- `docs/adr/0173-document-open-admission-boundary.md`
- `docs/agent-team/reviews/D133-document-open-admission-parent-review.md`
- `docs/agent-team/reviews/D133-document-open-admission-independent-review.md`

## Decisions and constraints

- Admission and completion remain separate contracts; result classification
  stays in `DocumentOpenCoordinator`.
- Existing file/folder activation and native dialog policies remain outside
  the coordinator.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, inline, package, and static
  evidence are the authorized validation boundary.

## Verification commands and results

- `D133-OPEN-ADMISSION-BEHAVIOR-PROBE=PASS`
- `D133-OPEN-ADMISSION-CONTRACT-PROBE=PASS`
- `D133-QT-FREE-OPEN-PROBE=PASS`
- `D133-COMPILEALL=PASS`
- `D133-RUFF=PASS`
- `D133-FORMAT=PASS`
- `D133-CHECK=PASS`
- `D133-VERIFY-HANDOFF=PASS`
- `D133-PACKAGE-BUILD=PASS`

## Unrun checks and reason

- Architect and independent review conclusions — child windows timed out;
  recorded as `NO_CONCLUSION`, not PASS.
- QApplication startup, native file dialog behavior, worker callback timing,
  filesystem durability, keyboard/accessibility traversal, DPI/font fallback,
  clean-machine, cross-machine, signing, installer, updater, legal, support,
  and release-owner checks — prohibited or outside authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static admission evidence cannot prove native QFileDialog or queued worker
  behavior on every Windows environment.
- Laplace architecture and Sartre independent review windows returned
  `NO_CONCLUSION`; no child PASS is claimed.
- The portable candidate remains unsigned and release remains NO-GO while
  external and report-binding gates are open.

## Acceptance and evidence IDs

- Acceptance: `S177`, `D133-AC01`.
- Evidence: ADR-0173, open-admission behavior/contract/Qt-free probes, parent
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
- SHA-256: `BB5C5F53C8E3EB0ABF05A5FAC8AF77DD531D68DF289F6600F1BE8DFE20F8D0FE`
- Size: `38518124` bytes
- Source revision: `tree-sha256:36802eb3104b0050a2c8ca5390897a400eb191f1ef856bc4e53376ef4fd0547f`
- Manifest: `dist/QuillForge.release.json`

## Disposition

`accepted-with-limits`: document-open admission is centralized at a typed
presentation boundary while native/runtime/release evidence remains open.

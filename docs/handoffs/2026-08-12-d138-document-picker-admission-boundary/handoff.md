# Handoff: 2026-08-12-d138-document-picker-admission-boundary

| Field | Value |
|---|---|
| ID | 2026-08-12-d138-document-picker-admission-boundary |
| Delivery / slice | D138 / ARCH-118 document-picker admission boundary |
| Status | `accepted-with-limits` |
| Owner | architect |
| Checkout | Current local checkout only |
| Created | 2026-08-12T05:30:00+08:00 |

## User outcome

Opening a document from the native file picker now has one explicit typed
admission boundary. The file path is visibly handed into the existing
asynchronous document-open path, while workspace folder selection remains a
separate flow.

## Scope and boundaries

### In scope

- `DocumentPickerAdmissionCoordinator` and
  `DocumentPickerAdmissionPorts`.
- MainWindow `_open_document` delegation and composition wiring.
- Static behavior/contract/Qt-free/wiring probes, package, and records.

### Out of scope

- No `FileDialogSurface`, `DocumentService`, `DocumentOpenAdmissionCoordinator`,
  document result classification, workspace directory picker, notifications,
  settings, or close-policy rewrite.
- No QApplication/native QFileDialog launch, file-system durability,
  clean-machine, cross-machine, signing, installer, updater, legal, support,
  or release-owner evidence.

## Team roles and ownership

| Role | Owner / agent | Result |
|---|---|---|
| Architect | Hegel the 4th / Luna max | `NO_CONCLUSION` after bounded window; no child PASS |
| Independent review | Archimedes the 4th / Luna max | `NO_CONCLUSION` after bounded window; no independent PASS |
| Parent | Architect | `PASS`; sole writer, integration, review, simplification, and verification |

## Changed files and modules

- `src/quillforge/presentation/document_picker_admission_coordinator.py` —
  new Qt-free picker admission contract and sequence.
- `src/quillforge/presentation/main_window.py` — named ports wiring and
  `_open_document` delegation only.
- `docs/adr/0180-document-picker-admission-boundary.md`
- `docs/agent-team/reviews/D138-document-picker-admission-parent-review.md`
- `docs/agent-team/reviews/D138-document-picker-admission-independent-review.md`

## Decisions and constraints

- Native file selection and asynchronous document opening remain separate
  presentation/application boundaries.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, inline, package, and static
  evidence are the authorized validation boundary.

## Verification commands and results

- `D138-DOCUMENT-PICKER-BEHAVIOR-PROBE=PASS`
- `D138-DOCUMENT-PICKER-CONTRACT-PROBE=PASS`
- `D138-QT-FREE-PICKER-PROBE=PASS`
- `D138-MAINWINDOW-WIRING-PROBE=PASS`
- `D138-COMPILEALL=PASS`
- `D138-RUFF=PASS`
- `D138-FORMAT=PASS`
- `D138-CHECK=PASS`
- `D138-VERIFY-HANDOFF=PASS`
- `D138-PACKAGE-BUILD=PASS`

## Unrun checks and reason

- Architect conclusion — child window timed out; recorded as `NO_CONCLUSION`.
- Independent review conclusion — child window timed out; recorded as
  `NO_CONCLUSION`, not PASS.
- QApplication/native QFileDialog, file-system behavior, queued worker timing,
  runtime startup, clean-machine, cross-machine, signing, installer, updater,
  legal, support, and release-owner checks — prohibited or outside
  authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static source probes cannot prove native QFileDialog return values or
  document-open worker timing on every Windows environment.
- Hegel architecture and Archimedes independent review windows returned
  `NO_CONCLUSION`; no child PASS is claimed.
- The portable candidate remains unsigned and release remains NO-GO while
  external and report-binding gates are open.

## Acceptance and evidence IDs

- Acceptance: `S184`, `D138-AC01`.
- Evidence: ADR-0180, picker behavior/contract/Qt-free/wiring probes, parent
  and independent review records, simplification assessment, static checks,
  package identity, handoff/index/register checks, expected release NO-GO, and
  explicit runtime limits.

## Next owner and next action

- Owner: Architect.
- Action: continue the next bounded MainWindow/application or visual-quality
  slice and complete authorized runtime/release gates when authority and
  environment permit.

## Artifact information

The candidate was rebuilt after the source change without launching
QuillForge:

- Artifact: `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `9BA984B543022D655FF3A1C962CF4BFA144C8757ADC8E585C3F2BEC7D5CB652E`
- Size: `38528084` bytes
- Source revision: `tree-sha256:40ad6520a2fe70257fc141bf3ffc9221042b3c12be09bca891ce8e775916d22c`
- Manifest: `dist/QuillForge.release.json`

## Disposition

`accepted-with-limits`: document picker admission is centralized and typed;
native dialog/runtime/release evidence remains open.

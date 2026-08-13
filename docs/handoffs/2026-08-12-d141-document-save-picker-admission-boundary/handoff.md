# Handoff: 2026-08-12-d141-document-save-picker-admission-boundary

| Field | Value |
|---|---|
| ID | 2026-08-12-d141-document-save-picker-admission-boundary |
| Delivery / slice | D141 / ARCH-122 document-save picker admission boundary |
| Status | `accepted-with-limits` |
| Owner | architect |
| Checkout | Current local checkout only |
| Created | 2026-08-12T07:30:00+08:00 |

## User outcome

Save and Save As now have one explicit typed picker boundary. Ordinary Save
uses an existing document path, untitled documents choose a destination, Save
As always opens the native save picker, cancellation is a no-op, and the
selected path enters the existing asynchronous save policy.

## Scope and boundaries

### In scope

- `DocumentSavePickerAdmissionCoordinator[TabT]` and its ports contract.
- MainWindow save/save-as composition wiring and delegation.
- Static behavior/contract/Qt-free/wiring probes, package, and records.

### Out of scope

- No `FileDialogSurface`, `DocumentSaveAdmissionCoordinator`, document
  validation/conflict/read-only/persistence, result classification,
  recovery/session projection, notifications, or close-policy rewrite.
- No QApplication/native save dialog launch, file-system durability,
  clean-machine, cross-machine, signing, installer, updater, legal, support,
  or release-owner evidence.

## Team roles and ownership

| Role | Owner / agent | Result |
|---|---|---|
| Architect | Fermat the 4th / Luna max | `NO_CONCLUSION` after bounded window; no child PASS |
| Independent review | Dirac the 4th / Luna max | `NO_CONCLUSION` after bounded window; no independent PASS |
| Parent | Architect | `PASS`; sole writer, integration, review, simplification, and verification |

## Changed files and modules

- `src/quillforge/presentation/document_save_picker_admission_coordinator.py` —
  new Qt-free Save/Save As picker admission contract and sequence.
- `src/quillforge/presentation/main_window.py` — named ports wiring and
  `_save_document`/`_save_as_document` delegation only.
- `docs/adr/0184-document-save-picker-admission-boundary.md`
- `docs/agent-team/reviews/D141-document-save-picker-admission-parent-review.md`
- `docs/agent-team/reviews/D141-document-save-picker-admission-independent-review.md`

## Decisions and constraints

- Save target selection and asynchronous document saving remain separate
  presentation/application boundaries.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, inline, package, and static
  evidence are the authorized validation boundary.

## Verification commands and results

- `D141-DOCUMENT-SAVE-PICKER-BEHAVIOR-PROBE=PASS`
- `D141-DOCUMENT-SAVE-PICKER-CONTRACT-PROBE=PASS`
- `D141-QT-FREE-DOCUMENT-SAVE-PICKER-PROBE=PASS`
- `D141-MAINWINDOW-DOCUMENT-SAVE-PICKER-WIRING-PROBE=PASS`
- `D141-COMPILEALL=PASS`
- `D141-RUFF=PASS`
- `D141-FORMAT=PASS`
- `D141-CHECK=PASS`
- `D141-VERIFY-HANDOFF=PASS`
- `D141-PACKAGE-BUILD=PASS`

## Unrun checks and reason

- Architect conclusion — child window timed out; recorded as `NO_CONCLUSION`.
- Independent review conclusion — child window timed out; recorded as
  `NO_CONCLUSION`, not PASS.
- QApplication/native save dialog, file-system behavior, queued worker timing,
  runtime startup, clean-machine, cross-machine, signing, installer, updater,
  legal, support, and release-owner checks — prohibited or outside
  authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static source probes cannot prove native save-dialog return values or worker
  timing on every Windows environment.
- Fermat architecture and Dirac independent review windows returned
  `NO_CONCLUSION`; no child PASS is claimed.
- The portable candidate remains unsigned and release remains NO-GO while
  external and report-binding gates are open.

## Acceptance and evidence IDs

- Acceptance: `S188`, `D141-AC01`.
- Evidence: ADR-0184, Save/Save As picker behavior/contract/Qt-free/wiring
  probes, parent and independent review records, simplification assessment,
  static checks, package identity, handoff/index/register checks, expected
  release NO-GO, and explicit runtime limits.

## Next owner and next action

- Owner: Architect.
- Action: continue the next bounded MainWindow/application or visual-quality
  slice and complete authorized runtime/release gates when authority and
  environment permit.

## Artifact information

The candidate was rebuilt after the source change without launching
QuillForge:

- Artifact: `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `AECE3EA108FC3079B3E6D3B0CBA1439334BCDB2178BC5DAB6384E2C276E5D083`
- Size: `38533584` bytes
- Source revision: `tree-sha256:c85a83c0c62190b27e59f6b7e3b97fec922c2659b26a0dceed9d60d3ad78ffbf`
- Manifest: `dist/QuillForge.release.json`

## Disposition

`accepted-with-limits`: Save/Save As picker admission is centralized and
typed; native dialog/filesystem/runtime/release evidence remains open.

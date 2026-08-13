# Handoff: 2026-08-12-d137-document-creation-admission-boundary

| Field | Value |
|---|---|
| ID | 2026-08-12-d137-document-creation-admission-boundary |
| Delivery / slice | D137 / ARCH-115 document-creation admission boundary |
| Status | `accepted-with-limits` |
| Owner | architect |
| Checkout | Current local checkout only |
| Created | 2026-08-12T04:00:00+08:00 |

## User outcome

New documents now pass through one explicit typed admission boundary. Busy and
startup-restore gates, creation, tab projection, lifecycle publication, and
success notification retain their existing behavior while application,
editor, tab, event, and notification policies remain separately owned.

## Scope and boundaries

### In scope

- `DocumentCreationAdmissionCoordinator` and
  `DocumentCreationAdmissionPorts`.
- `MainWindow` composition wiring and `_new_document` delegation.
- Static behavior/contract/Qt-free probes, package, and traceability records.

### Out of scope

- No `DocumentService`, `DocumentTabCreationCoordinator`, `EventBus`, editor,
  tab title/status/session-save, notification, or startup-restore policy
  rewrite.
- No QApplication launch, native editor interaction, visual rendering,
  clean-machine, cross-machine, signing, installer, updater, legal, support,
  or release-owner evidence.

## Team roles and ownership

| Role | Owner / agent | Result |
|---|---|---|
| Architect | Socrates the 4th / Luna max | `NO_CONCLUSION` after bounded window; no child PASS |
| Independent review | Helmholtz the 4th / Luna max | `NO_CONCLUSION` after bounded window; no independent PASS |
| Parent | Architect | `PASS`; sole writer, integration, review, simplification, and verification |

## Changed files and modules

- `src/quillforge/presentation/document_creation_admission_coordinator.py` —
  new Qt-free admission contract and sequencing.
- `src/quillforge/presentation/main_window.py` — named ports wiring and
  `_new_document` delegation only.
- `docs/adr/0177-document-creation-admission-boundary.md`
- `docs/agent-team/reviews/D137-document-creation-admission-parent-review.md`
- `docs/agent-team/reviews/D137-document-creation-admission-independent-review.md`

## Decisions and constraints

- Admission, tab assembly, application creation, lifecycle publication, and
  notification remain separate contracts and owners.
- `allow_during_startup` remains explicit so initial-document restore keeps its
  existing exception without weakening ordinary user admission.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, inline, package, and static
  evidence are the authorized validation boundary.

## Verification commands and results

- `D137-DOCUMENT-CREATION-BEHAVIOR-PROBE=PASS`
- `D137-DOCUMENT-CREATION-CONTRACT-PROBE=PASS`
- `D137-QT-FREE-CREATION-PROBE=PASS`
- `D137-COMPILEALL=PASS`
- `D137-RUFF=PASS`
- `D137-FORMAT=PASS`
- `D137-CHECK=PASS`
- `D137-VERIFY-HANDOFF=PASS`
- `D137-PACKAGE-BUILD=PASS`

## Unrun checks and reason

- Architect and independent review conclusions — child windows timed out;
  recorded as `NO_CONCLUSION`, not PASS.
- QApplication startup, initial restore timing, native editor/tab behavior,
  visual rendering, clean-machine, cross-machine, signing, installer,
  updater, legal, support, and release-owner checks — prohibited or outside
  authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static admission evidence cannot prove native startup restore timing or
  editor/tab rendering on every Windows environment.
- Socrates architecture and Helmholtz independent review windows returned
  `NO_CONCLUSION`; no child PASS is claimed.
- The portable candidate remains unsigned and release remains NO-GO while
  external and report-binding gates are open.

## Acceptance and evidence IDs

- Acceptance: `S181`, `D137-AC01`.
- Evidence: ADR-0177, creation behavior/contract/Qt-free probes, parent and
  independent review records, simplification assessment, static checks,
  package identity, handoff/index/register checks, expected release NO-GO,
  and explicit runtime limits.

## Next owner and next action

- Owner: Architect.
- Action: continue the next bounded MainWindow/application or distinct visual
  gap slice and complete authorized runtime/release gates when authority and
  environment permit.

## Artifact information

The portable candidate was rebuilt without launching QuillForge after the
source change:

- Artifact: `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `90F6D78CEA651D31605069A71E609E18819B8ECC194992A8B98FFBF56F27A307`
- Size: `38525572` bytes
- Source revision: `tree-sha256:d149c6afc9cc4049b11ff61a12a679c172bcce43fde551d2b759aef6de17a4f4`
- Manifest: `dist/QuillForge.release.json`

## Disposition

`accepted-with-limits`: document creation is centralized at a typed
presentation boundary while native/runtime/release evidence remains open.

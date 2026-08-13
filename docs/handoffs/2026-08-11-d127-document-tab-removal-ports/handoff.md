# Handoff: 2026-08-11-d127-document-tab-removal-ports

| Field | Value |
|---|---|
| ID | 2026-08-11-d127-document-tab-removal-ports |
| Delivery / slice | D127 / ARCH-103 document-tab removal ports contract |
| Status | `accepted-with-limits` |
| Owner | architect |
| Checkout | Current local checkout only |
| Created | 2026-08-11T18:00:00+08:00 |

## User outcome

Post-admission document-tab removal now has an explicit typed composition
boundary. Recovery capture cancellation, tab/editor cleanup, close events,
session persistence, and empty-tab fallback remain ordered and independently
replaceable.

## Scope and boundaries

### In scope

- Add frozen/slotted generic `DocumentTabRemovalPorts[TabT, CaptureT]`.
- Replace the eleven positional MainWindow callbacks with named mapping.
- Preserve missing/live/capture/empty-tab behavior, bool return semantics, and
  close/recovery finalization order.

### Out of scope

- No CloseGuardCoordinator, EditorDocumentSurface, DocumentTabSurface,
  recovery tracker, DocumentService, TaskRunner, persistence, startup, locale,
  or application policy semantics changed.
- No new async path, worker, singleton, or test-only asset.
- No QApplication launch, native tab/editor rendering, clean-machine,
  cross-machine, signing, installer, updater, legal, support, or release-owner
  evidence.

## Team roles and ownership

| Role | Owner / agent | Result |
|---|---|---|
| Architect | Dewey the 4th / Luna max | `NO_CONCLUSION` after two bounded waits; no child PASS |
| Independent review | Franklin the 4th / Luna max | `NO_CONCLUSION` after two bounded waits; no independent PASS |
| Parent | Architect | `PASS`; sole writer, integration, review, simplification, and verification |

## Changed files and modules

- `src/quillforge/presentation/document_tab_removal_coordinator.py` — typed
  ports and named callback use.
- `src/quillforge/presentation/main_window.py` — named composition.
- `tasks/plan.md`, `tasks/todo.md`, and
  `docs/specs/enterprise-architecture-migration.md` — bounded slice scope.
- `docs/adr/0165-document-tab-removal-ports-contract.md`
- `docs/agent-team/reviews/D127-document-tab-removal-ports-parent-review.md`
- `docs/agent-team/reviews/D127-document-tab-removal-ports-independent-review.md`

## Decisions and constraints

- `DocumentTabRemovalPorts[TabT, CaptureT]` is callback-only and frozen/slotted;
  it is not a second close, recovery, tab, editor, or persistence state owner.
- MainWindow retains close admission and concrete recovery/tab/editor policy;
  the coordinator only sequences the existing callbacks.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, inline, package, and static
  evidence are the authorized validation boundary.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `D127-DOCUMENT-TAB-REMOVAL-BEHAVIOR-PROBE=PASS` | PASS | Missing/live/capture/empty-tab paths and exact side-effect order. |
| `D127-DOCUMENT-TAB-REMOVAL-CONTRACT-PROBE=PASS` | PASS | Frozen/slotted ports, generic contract, named composition, and export. |
| `D127-COMPILEALL=PASS` | PASS | Static compilation; no QApplication launch. |
| `D127-RUFF=PASS` | PASS | Target source passed `uv run ruff check`. |
| `D127-FORMAT=PASS` | PASS | Target files passed `uv run ruff format --check`. |
| `D127-CHECK-SCRIPT=PASS` | PASS | Project static/check gate passed. |
| `D127-HANDOFF-PRE-RECORD=PASS` | PASS | Existing handoff checks passed before record synchronization. |
| `D127-PACKAGE-IDENTITY-PROBE=PASS` | PASS | Root and `dist` candidates match: SHA-256 `FD7FCD2D5ECD3B61B24DEC5FEBF8D6F63659E7CDAE4BF787B4EF04BDE14A4320`, 38,506,982 bytes, source `tree-sha256:5aace1b1e595c13febbf84c15ab99366cc233064c0fd7034fa09a96525d0d306`. |
| `D127-PACKAGE-NO-LAUNCH-PROBE=PASS` | PASS | Packaging completed without launching QuillForge; no process remained. |
| `D127-JSON-MANIFEST-PROBE=PASS` | PASS | Manifest path, artifact identity, and source provenance are explicit. |
| `D127-RECORD-IDENTITY-PROBE=PASS` | PASS | Acceptance, delivery register, handoff index, manifest, and release handoff are synchronized. |
| `D127-RELEASE-DOSSIER-PROBE=PASS` | PASS | Release dossier binds the current D127 artifact identity. |
| `D127-RELEASE-EXPECTED-NO-GO=PASS` | PASS | Expected NO-GO remains due open external gates and three known mechanical report-binding failures. |

## Unrun checks and reason

- Architect and independent review conclusions — child windows timed out twice;
  recorded as `NO_CONCLUSION`, not PASS.
- Native tab/editor rendering, QApplication startup, worker interleaving,
  recovery/filesystem durability, clean-machine, cross-machine, signing,
  installer, updater, legal, support, and release-owner checks — prohibited or
  outside authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Inline/static probes do not prove native tab/editor rendering, real callback
  timing, or recovery durability.
- Dewey architecture and Franklin independent review windows returned
  `NO_CONCLUSION`; no child PASS is claimed.
- The portable candidate remains unsigned and release remains NO-GO while
  external gates and report-binding gates are open.

## Acceptance and evidence IDs

- Acceptance: `S169`, `D127-AC01`.
- Evidence: ADR-0165, source/behavior probes, parent and independent review
  records, simplification assessment, static checks, package identity,
  handoff/index/register checks, expected release NO-GO, and explicit runtime
  limits.

## Next owner and next action

- Owner: Architect.
- Action: continue the next bounded MainWindow/application or visual-quality
  slice and complete authorized runtime/release gates when authority and
  environment permit.

## Artifact information

The portable candidate was rebuilt without launching QuillForge after source
and record synchronization:

- Artifact: `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `FD7FCD2D5ECD3B61B24DEC5FEBF8D6F63659E7CDAE4BF787B4EF04BDE14A4320`
- Size: `38506982` bytes
- Source revision: `tree-sha256:5aace1b1e595c13febbf84c15ab99366cc233064c0fd7034fa09a96525d0d306`
- Manifest: `dist/QuillForge.release.json`

## Disposition

`accepted-with-limits`: document-tab removal composition is explicit while
native/runtime/release evidence remains open.

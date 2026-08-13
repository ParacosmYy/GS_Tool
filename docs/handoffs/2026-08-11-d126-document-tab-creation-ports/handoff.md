# Handoff: 2026-08-11-d126-document-tab-creation-ports

| Field | Value |
|---|---|
| ID | 2026-08-11-d126-document-tab-creation-ports |
| Delivery / slice | D126 / ARCH-102 document-tab creation ports contract |
| Status | `accepted-with-limits` |
| Owner | architect |
| Checkout | Current local checkout only |
| Created | 2026-08-11T17:00:00+08:00 |

## User outcome

Document-tab assembly now has an explicit typed composition boundary. Editor
creation, recovery identity, tab insertion, metadata, persistence, and status
seams are named and independently replaceable without changing their order.

## Scope and boundaries

### In scope

- Add frozen/slotted generic `DocumentTabCreationPorts[OpenedT, TabT, EditorT]`.
- Replace the eight positional MainWindow callbacks with named mapping.
- Preserve normal/recovery tab assembly, title/modified projection, session-save
  request, status synchronization, and return order.

### Out of scope

- No EditorDocumentSurface, DocumentTabSurface, DocumentService, TaskRunner,
  recovery, persistence, startup, close, locale, or application policy
  semantics changed.
- No new async path, worker, singleton, or test-only asset.
- No QApplication launch, native editor rendering, clean-machine,
  cross-machine, signing, installer, updater, legal, support, or release-owner
  evidence.

## Team roles and ownership

| Role | Owner / agent | Result |
|---|---|---|
| Architect | Raman the 4th / Luna max | `NO_CONCLUSION` after two bounded waits; no child PASS |
| Independent review | Dalton the 4th / Luna max | `NO_CONCLUSION` after two bounded waits; no independent PASS |
| Parent | Architect | `PASS`; sole writer, integration, review, simplification, and verification |

## Changed files and modules

- `src/quillforge/presentation/document_tab_creation_coordinator.py` — typed
  ports and named callback use.
- `src/quillforge/presentation/main_window.py` — named composition.
- `tasks/plan.md`, `tasks/todo.md`, and
  `docs/specs/enterprise-architecture-migration.md` — bounded slice scope.
- `docs/adr/0164-document-tab-creation-ports-contract.md`
- `docs/agent-team/reviews/D126-document-tab-creation-ports-parent-review.md`
- `docs/agent-team/reviews/D126-document-tab-creation-ports-independent-review.md`

## Decisions and constraints

- `DocumentTabCreationPorts[OpenedT, TabT, EditorT]` is callback-only and
  frozen/slotted; it is not a second editor, tab, recovery, or persistence state
  owner.
- MainWindow retains concrete editor/tab creation and application policy; the
  coordinator only sequences the existing callbacks.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, inline, package, and static
  evidence are the authorized validation boundary.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `D126-DOCUMENT-TAB-CREATION-BEHAVIOR-PROBE=PASS` | PASS | Normal/recovery identity and exact assembly order. |
| `D126-DOCUMENT-TAB-CREATION-CONTRACT-PROBE=PASS` | PASS | Frozen/slotted ports, generic contract, named composition, and export. |
| `D126-COMPILEALL=PASS` | PASS | Static compilation; no QApplication launch. |
| `D126-RUFF=PASS` | PASS | Target source passed `uv run ruff check`. |
| `D126-FORMAT=PASS` | PASS | Target files passed `uv run ruff format --check`. |
| `D126-CHECK-SCRIPT=PASS` | PASS | Project static/check gate passed. |
| `D126-HANDOFF-PRE-RECORD=PASS` | PASS | Existing handoff checks passed before record synchronization. |
| `D126-PACKAGE-IDENTITY-PROBE=PASS` | PASS | Root and `dist` candidates match: SHA-256 `8B5CB773234B51BCB4C67BA6CC9CFD81A5F98AFEBBA0CBCC43AB0F3194C8E552`, 38,503,420 bytes, source `tree-sha256:f8332b77ca092bf14d4de57c648ad3b1fed140f75b4a972eff3dbb7e66da628a`. |
| `D126-PACKAGE-NO-LAUNCH-PROBE=PASS` | PASS | Packaging completed without launching QuillForge; no process remained. |
| `D126-JSON-MANIFEST-PROBE=PASS` | PASS | Manifest path, artifact identity, and source provenance are explicit. |
| `D126-RECORD-IDENTITY-PROBE=PASS` | PASS | Acceptance, delivery register, handoff index, manifest, and release handoff are synchronized. |
| `D126-RELEASE-DOSSIER-PROBE=PASS` | PASS | Release dossier binds the current D126 artifact identity. |
| `D126-RELEASE-EXPECTED-NO-GO=PASS` | PASS | Expected NO-GO remains due open external gates and three known mechanical report-binding failures. |

## Unrun checks and reason

- Architect and independent review conclusions — child windows timed out twice;
  recorded as `NO_CONCLUSION`, not PASS.
- Native editor rendering, QApplication startup, worker interleaving,
  filesystem/recovery durability, clean-machine, cross-machine, signing,
  installer, updater, legal, support, and release-owner checks — prohibited or
  outside authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Inline/static probes do not prove native editor rendering, real callback
  timing, or recovery durability.
- Raman architecture and Dalton independent review windows returned
  `NO_CONCLUSION`; no child PASS is claimed.
- The portable candidate remains unsigned and release remains NO-GO while
  external gates and report-binding gates are open.

## Acceptance and evidence IDs

- Acceptance: `S168`, `D126-AC01`.
- Evidence: ADR-0164, source/behavior probes, parent and independent review
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
- SHA-256: `8B5CB773234B51BCB4C67BA6CC9CFD81A5F98AFEBBA0CBCC43AB0F3194C8E552`
- Size: `38503420` bytes
- Source revision: `tree-sha256:f8332b77ca092bf14d4de57c648ad3b1fed140f75b4a972eff3dbb7e66da628a`
- Manifest: `dist/QuillForge.release.json`

## Disposition

`accepted-with-limits`: document-tab assembly composition is explicit while
native/runtime/release evidence remains open.

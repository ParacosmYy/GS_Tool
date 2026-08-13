# Handoff: 2026-08-11-d124-document-save-projection-ports

| Field | Value |
|---|---|
| ID | 2026-08-11-d124-document-save-projection-ports |
| Delivery / slice | D124 / ARCH-100 document-save projection ports contract |
| Status | `accepted-with-limits` |
| Owner | architect |
| Checkout | Current local checkout only |
| Created | 2026-08-11T15:00:00+08:00 |

## User outcome

Validated save projection now has an explicit typed composition boundary. The
existing observable sequence is documented by named ports, making future save
feedback changes easier to extend without positional callback coupling.

## Scope and boundaries

### In scope

- Add frozen/slotted generic `DocumentSaveProjectionPorts[TabT]`.
- Replace the seven positional MainWindow projection callbacks with named
  mapping.
- Preserve valid-save state/language/title/recovery/event/notification/
  session-save/continuation order.

### Out of scope

- No DocumentService, TaskRunner, DocumentSaveCoordinator, tab/editor,
  persistence, startup, close, locale, or application policy semantics changed.
- No new async path, worker, singleton, or test-only asset.
- No QApplication launch, native editor rendering, clean-machine,
  cross-machine, signing, installer, updater, legal, support, or release-owner
  evidence.

## Team roles and ownership

| Role | Owner / agent | Result |
|---|---|---|
| Architect | Faraday the 4th / Luna max | `NO_CONCLUSION` after two bounded waits; no child PASS |
| Independent review | Halley the 4th / Luna max | `NO_CONCLUSION` after two bounded waits; no independent PASS |
| Parent | Architect | `PASS`; sole writer, integration, review, simplification, and verification |

## Changed files and modules

- `src/quillforge/presentation/document_save_projection_coordinator.py` — typed
  ports and named callback use.
- `src/quillforge/presentation/main_window.py` — named composition.
- `tasks/plan.md`, `tasks/todo.md`, and
  `docs/specs/enterprise-architecture-migration.md` — bounded slice scope.
- `docs/adr/0162-document-save-projection-ports-contract.md`
- `docs/agent-team/reviews/D124-document-save-projection-ports-parent-review.md`
- `docs/agent-team/reviews/D124-document-save-projection-ports-independent-review.md`

## Decisions and constraints

- `DocumentSaveProjectionPorts[TabT]` is callback-only and frozen/slotted; it
  is not a second document or save state owner.
- `DocumentSaveCoordinator` remains the classification owner; MainWindow
  retains services, persistence, startup, and close policy.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, inline, package, and static
  evidence are the authorized validation boundary.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `D124-DOCUMENT-SAVE-PROJECTION-BEHAVIOR-PROBE=PASS` | PASS | Exact projection order with and without optional continuation. |
| `D124-DOCUMENT-SAVE-PROJECTION-CONTRACT-PROBE=PASS` | PASS | Frozen/slotted ports, generic contract, named composition, and export. |
| `D124-COMPILEALL=PASS` | PASS | Static compilation; no QApplication launch. |
| `D124-RUFF=PASS` | PASS | Target source passed `uv run ruff check`. |
| `D124-FORMAT=PASS` | PASS | Target files passed `uv run ruff format --check`. |
| `D124-CHECK-SCRIPT=PASS` | PASS | Project static/check gate passed. |
| `D124-HANDOFF-PRE-RECORD=PASS` | PASS | Existing handoff checks passed before record synchronization. |
| `D124-PACKAGE-IDENTITY-PROBE=PASS` | PASS | Root and `dist` candidates match: SHA-256 `32215F407582DDDBE7215C3F8C245A8A10BAA35FB3BF61A569F13C4E15E4F71F`, 38,505,391 bytes, source `tree-sha256:6febcbd2b2ffc5d114dfabf5c66a09e89e6c8e67f088ec54dd72a4d578575be6`. |
| `D124-PACKAGE-NO-LAUNCH-PROBE=PASS` | PASS | Packaging completed without launching QuillForge; no process remained. |
| `D124-JSON-MANIFEST-PROBE=PASS` | PASS | Manifest path, artifact identity, and source provenance are explicit. |
| `D124-RECORD-IDENTITY-PROBE=PASS` | PASS | Acceptance, delivery register, handoff index, manifest, and release handoff are synchronized. |
| `D124-RELEASE-DOSSIER-PROBE=PASS` | PASS | Release dossier binds the current D124 artifact identity. |
| `D124-RELEASE-EXPECTED-NO-GO=PASS` | PASS | Expected NO-GO remains due open external gates and three known mechanical report-binding failures. |

## Unrun checks and reason

- Architect and independent review conclusions — child windows timed out twice;
  recorded as `NO_CONCLUSION`, not PASS.
- Native editor rendering, QApplication startup, worker interleaving,
  filesystem durability/timing, clean-machine, cross-machine, signing,
  installer, updater, legal, support, and release-owner checks — prohibited or
  outside authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Inline/static probes do not prove native editor rendering, real callback
  timing, or filesystem durability.
- Faraday architecture and Halley independent review windows returned
  `NO_CONCLUSION`; no child PASS is claimed.
- The portable candidate remains unsigned and release remains NO-GO while
  external gates and report-binding gates are open.

## Acceptance and evidence IDs

- Acceptance: `S166`, `D124-AC01`.
- Evidence: ADR-0162, source/behavior probes, parent and independent review
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
- SHA-256: `32215F407582DDDBE7215C3F8C245A8A10BAA35FB3BF61A569F13C4E15E4F71F`
- Size: `38505391` bytes
- Source revision: `tree-sha256:6febcbd2b2ffc5d114dfabf5c66a09e89e6c8e67f088ec54dd72a4d578575be6`
- Manifest: `dist/QuillForge.release.json`

## Disposition

`accepted-with-limits`: save projection composition is explicit while native/
runtime/release evidence remains open.

# Handoff: 2026-08-11-d123-document-save-ports

| Field | Value |
|---|---|
| ID | 2026-08-11-d123-document-save-ports |
| Delivery / slice | D123 / ARCH-99 document-save ports contract |
| Status | `accepted-with-limits` |
| Owner | architect |
| Checkout | Current local checkout only |
| Created | 2026-08-11T14:00:00+08:00 |

## User outcome

Document saving now has an explicit typed composition boundary. Future save
completion policy can evolve through named ports without obscuring stale
operation, tab liveness, read-only, projection, or error wiring.

## Scope and boundaries

### In scope

- Add frozen/slotted generic `DocumentSavePorts[TabT]`.
- Replace the five positional MainWindow constructor callbacks with named
  mapping.
- Preserve stale/liveness guards, read-only release, result validation, save
  success/failure projection, continuation, and dispatcher callback binding.

### Out of scope

- No DocumentService, TaskRunner, DocumentSaveProjectionCoordinator, tab/editor,
  persistence, startup, close, locale, or application policy semantics changed.
- No new async path, worker, singleton, or test-only asset.
- No QApplication launch, native editor rendering, clean-machine,
  cross-machine, signing, installer, updater, legal, support, or release-owner
  evidence.

## Team roles and ownership

| Role | Owner / agent | Result |
|---|---|---|
| Architect | Kepler the 4th / Luna max | `NO_CONCLUSION` after two bounded waits; no child PASS |
| Independent review | Volta the 4th / Luna max | `NO_CONCLUSION` after two bounded waits; no independent PASS |
| Parent | Architect | `PASS`; sole writer, integration, review, simplification, and verification |

## Changed files and modules

- `src/quillforge/presentation/document_save_coordinator.py` — typed ports and
  named callback use.
- `src/quillforge/presentation/main_window.py` — named composition.
- `tasks/plan.md`, `tasks/todo.md`, and
  `docs/specs/enterprise-architecture-migration.md` — bounded slice scope.
- `docs/adr/0161-document-save-ports-contract.md`
- `docs/agent-team/reviews/D123-document-save-ports-parent-review.md`
- `docs/agent-team/reviews/D123-document-save-ports-independent-review.md`

## Decisions and constraints

- `DocumentSavePorts[TabT]` is callback-only and frozen/slotted; it is not a
  second document, tab, or save state owner.
- `DocumentSaveProjectionCoordinator` remains the valid-result projection
  owner; MainWindow retains services, persistence, startup, and close policy.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, inline, package, and static
  evidence are the authorized validation boundary.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `D123-DOCUMENT-SAVE-BEHAVIOR-PROBE=PASS` | PASS | Stale, missing-tab, valid, invalid, failure, and submit callback paths. |
| `D123-DOCUMENT-SAVE-CONTRACT-PROBE=PASS` | PASS | Frozen/slotted ports, named composition, Qt-free import, and export. |
| `D123-COMPILEALL=PASS` | PASS | Static compilation; no QApplication launch. |
| `D123-RUFF=PASS` | PASS | Target source passed `uv run ruff check`. |
| `D123-FORMAT=PASS` | PASS | Target files passed `uv run ruff format --check`. |
| `D123-CHECK-SCRIPT=PASS` | PASS | Project static/check gate passed. |
| `D123-HANDOFF-SCRIPT=PASS` | PASS | Existing handoff checks passed. |
| `D123-PACKAGE-IDENTITY-PROBE=PASS` | PASS | Root and `dist` candidates match: SHA-256 `6214CD4F14FD3749A2F44951ACC9A55922AF30A5A81C43116D03D903957F23BF`, 38,505,460 bytes, source `tree-sha256:4474d706539165a5cc9628e6c59a15ea2b6a4f147fee6e75a59147a2500b7a85`. |
| `D123-PACKAGE-NO-LAUNCH-PROBE=PASS` | PASS | Packaging completed without launching QuillForge; no process remained. |
| `D123-JSON-MANIFEST-PROBE=PASS` | PASS | Manifest path, artifact identity, and source provenance are explicit. |
| `D123-RECORD-IDENTITY-PROBE=PASS` | PASS | Acceptance, delivery register, handoff index, manifest, and release handoff are synchronized. |
| `D123-RELEASE-DOSSIER-PROBE=PASS` | PASS | Release dossier binds the current D123 artifact identity. |
| `D123-RELEASE-EXPECTED-NO-GO=PASS` | PASS | Expected NO-GO remains due open external gates and three known mechanical report-binding failures. |

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

- Inline/static probes do not prove native editor rendering or real callback
  timing or filesystem durability.
- Kepler architecture and Volta independent review windows returned
  `NO_CONCLUSION`; no child PASS is claimed.
- The portable candidate remains unsigned and release remains NO-GO while
  external gates and report-binding gates are open.

## Acceptance and evidence IDs

- Acceptance: `S165`, `D123-AC01`.
- Evidence: ADR-0161, source/behavior probes, parent and independent review
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
- SHA-256: `6214CD4F14FD3749A2F44951ACC9A55922AF30A5A81C43116D03D903957F23BF`
- Size: `38505460` bytes
- Source revision: `tree-sha256:4474d706539165a5cc9628e6c59a15ea2b6a4f147fee6e75a59147a2500b7a85`
- Manifest: `dist/QuillForge.release.json`

## Disposition

`accepted-with-limits`: document-save composition is explicit while native/
runtime/release evidence remains open.

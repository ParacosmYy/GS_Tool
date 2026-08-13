# Handoff: 2026-08-11-d125-document-open-projection-ports

| Field | Value |
|---|---|
| ID | 2026-08-11-d125-document-open-projection-ports |
| Delivery / slice | D125 / ARCH-101 document-open projection ports contract |
| Status | `accepted-with-limits` |
| Owner | architect |
| Checkout | Current local checkout only |
| Created | 2026-08-11T16:00:00+08:00 |

## User outcome

Valid document-open projection now has an explicit typed composition boundary.
Duplicate, restore, line, cursor, event, notification, and continuation
decisions are represented by named ports without changing their behavior.

## Scope and boundaries

### In scope

- Add frozen/slotted generic `DocumentOpenProjectionPorts[TabT]`.
- Replace the nine positional MainWindow projection callbacks with named
  mapping.
- Preserve duplicate/restored branches and new-tab line/cursor/event/
  notification/continuation order.

### Out of scope

- No DocumentService, DocumentOpenCoordinator, TaskRunner, tab/editor,
  persistence, startup, close, locale, or application policy semantics changed.
- No new async path, worker, singleton, or test-only asset.
- No QApplication launch, native editor rendering, clean-machine,
  cross-machine, signing, installer, updater, legal, support, or release-owner
  evidence.

## Team roles and ownership

| Role | Owner / agent | Result |
|---|---|---|
| Architect | Hubble the 4th / Luna max | `NO_CONCLUSION` after two bounded waits; no child PASS |
| Independent review | Parfit the 4th / Luna max | `NO_CONCLUSION` after two bounded waits; no independent PASS |
| Parent | Architect | `PASS`; sole writer, integration, review, simplification, and verification |

## Changed files and modules

- `src/quillforge/presentation/document_open_projection_coordinator.py` — typed
  ports and named callback use.
- `src/quillforge/presentation/main_window.py` — named composition.
- `tasks/plan.md`, `tasks/todo.md`, and
  `docs/specs/enterprise-architecture-migration.md` — bounded slice scope.
- `docs/adr/0163-document-open-projection-ports-contract.md`
- `docs/agent-team/reviews/D125-document-open-projection-ports-parent-review.md`
- `docs/agent-team/reviews/D125-document-open-projection-ports-independent-review.md`

## Decisions and constraints

- `DocumentOpenProjectionPorts[TabT]` is callback-only and frozen/slotted; it
  is not a second document, tab, or restore state owner.
- `DocumentOpenCoordinator` remains open classification owner; MainWindow
  retains services, persistence, startup, and close policy.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, inline, package, and static
  evidence are the authorized validation boundary.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `D125-DOCUMENT-OPEN-PROJECTION-BEHAVIOR-PROBE=PASS` | PASS | Ordinary/restored duplicate and new-tab line/cursor/order paths. |
| `D125-DOCUMENT-OPEN-PROJECTION-CONTRACT-PROBE=PASS` | PASS | Frozen/slotted ports, generic contract, named composition, and export. |
| `D125-COMPILEALL=PASS` | PASS | Static compilation; no QApplication launch. |
| `D125-RUFF=PASS` | PASS | Target source passed `uv run ruff check`. |
| `D125-FORMAT=PASS` | PASS | Target files passed `uv run ruff format --check`. |
| `D125-CHECK-SCRIPT=PASS` | PASS | Project static/check gate passed. |
| `D125-HANDOFF-PRE-RECORD=PASS` | PASS | Existing handoff checks passed before record synchronization. |
| `D125-PACKAGE-IDENTITY-PROBE=PASS` | PASS | Root and `dist` candidates match: SHA-256 `6257029C06B602A461B4C09B620F6EB95740520366C3584C5D78F167EE903D24`, 38,505,926 bytes, source `tree-sha256:90c30c14a05656b5c330343410e20a9c6fee042caac60a4212a9b04664c861f8`. |
| `D125-PACKAGE-NO-LAUNCH-PROBE=PASS` | PASS | Packaging completed without launching QuillForge; no process remained. |
| `D125-JSON-MANIFEST-PROBE=PASS` | PASS | Manifest path, artifact identity, and source provenance are explicit. |
| `D125-RECORD-IDENTITY-PROBE=PASS` | PASS | Acceptance, delivery register, handoff index, manifest, and release handoff are synchronized. |
| `D125-RELEASE-DOSSIER-PROBE=PASS` | PASS | Release dossier binds the current D125 artifact identity. |
| `D125-RELEASE-EXPECTED-NO-GO=PASS` | PASS | Expected NO-GO remains due open external gates and three known mechanical report-binding failures. |

## Unrun checks and reason

- Architect and independent review conclusions — child windows timed out twice;
  recorded as `NO_CONCLUSION`, not PASS.
- Native editor rendering, QApplication startup, worker interleaving,
  filesystem decoding timing, clean-machine, cross-machine, signing,
  installer, updater, legal, support, and release-owner checks — prohibited or
  outside authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Inline/static probes do not prove native editor rendering or real callback
  timing or filesystem decoding.
- Hubble architecture and Parfit independent review windows returned
  `NO_CONCLUSION`; no child PASS is claimed.
- The portable candidate remains unsigned and release remains NO-GO while
  external gates and report-binding gates are open.

## Acceptance and evidence IDs

- Acceptance: `S167`, `D125-AC01`.
- Evidence: ADR-0163, source/behavior probes, parent and independent review
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
- SHA-256: `6257029C06B602A461B4C09B620F6EB95740520366C3584C5D78F167EE903D24`
- Size: `38505926` bytes
- Source revision: `tree-sha256:90c30c14a05656b5c330343410e20a9c6fee042caac60a4212a9b04664c861f8`
- Manifest: `dist/QuillForge.release.json`

## Disposition

`accepted-with-limits`: document-open projection composition is explicit while
native/runtime/release evidence remains open.

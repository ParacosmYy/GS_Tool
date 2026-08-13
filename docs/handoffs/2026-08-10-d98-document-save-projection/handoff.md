# Handoff: 2026-08-10-d98-document-save-projection

| Field | Value |
|---|---|
| ID | `2026-08-10-d98-document-save-projection` |
| Delivery / slice | `D98 / ARCH-72 document-save projection coordinator boundary` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-10T23:59:00+08:00` |

## User outcome

Valid asynchronous document-save completion now has one focused Qt-free
projection coordinator. The existing save behavior remains intact: a validated
live result updates clean state, language, title, recovery, event and success
feedback, session persistence, and the optional continuation in the same order.

## Scope and boundaries

### In scope

- `DocumentSaveProjectionCoordinator[TabT]` valid-result contract and ordering.
- Direct D87 `DocumentSaveCoordinator` wiring and removal of the old valid-save
  method from MainWindow.
- Source, contract, static, package, handoff, and release evidence.

### Out of scope

- No DocumentService/Store, target/path/encoding/conflict policy, editor
  implementation, tab surface, recovery policy, session schema, event contract,
  locale contract, plugin API, or close policy changed.
- No new worker, retry, cache, mutable shared state, or generic
  document-operation framework.
- No Qt launch, screenshot, native editor/save review, accessibility/DPI/font,
  clean-machine, cross-machine, signing, installer, updater, legal, support,
  or release-owner evidence.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Popper the 3rd / Luna max | Read-only boundary consultation; `NO_CONCLUSION` after bounded windows |
| Independent review | Helmholtz the 3rd / Luna max | Read-only source review; `NO_CONCLUSION` after bounded windows |
| Parent | Architect | Sole writer, integration, source review, simplification, and verification |

No child PASS is claimed.

## Changed files and modules

- `src/quillforge/presentation/document_save_projection_coordinator.py` —
  Qt-free valid-save projection sequence.
- `src/quillforge/presentation/main_window.py` — direct coordinator wiring and
  removal of `_apply_saved_document`; concrete `_apply_saved_state` retained.
- D98 ADR, parent/independent review records, handoff, and synchronized
  acceptance/delivery/roadmap/spec/task/release records.

## Decisions and constraints

- `DocumentSaveCoordinator` remains responsible for stale suppression, tab
  liveness, read-only release, `DocumentState` validation, and invalid/failure
  classification.
- MainWindow remains the composition root and supplies concrete editor,
  recovery, EventBus, notification, session-save, and close-policy callbacks.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, static, packaging, and release
  handoff checks are the permitted validation boundary.
- This is Python/PyQt6 application/presentation code. Embedded C/C++ assurance
  and vendor manufacturer requirements are `N/A` for this slice.
- Public CloudWeGo material remains an engineering reference only; no private
  ByteDance standard, certification, or compliance claim is made.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `D98-SAVE-PROJECTION-SOURCE-PROBE=PASS` | `PASS` | Qt-free source boundary, direct wiring, and old method removal. |
| `D98-SAVE-PROJECTION-ORDER-PROBE=PASS` | `PASS` | State/language/title/recovery/event/notification/session/continuation order and path pass-through. |
| `python -m compileall -q src/quillforge` | `PASS` | No launch or QApplication instantiation. |
| `uv run ruff check src/quillforge` | `PASS` | No lint errors. |
| `uv run ruff format --check src/quillforge` | `PASS` | 115 files already formatted. |
| `scripts\package.ps1` | `PASS` | Portable candidate rebuilt; root/dist identities match. |
| D98 package identity | `PASS` | SHA-256 `F489AED828573C8629B361D52C817A3DCD1B7283CC108E3B106B8A26B0303834`, 38,481,583 bytes. |
| `D98-RELEASE-EXPECTED-NO-GO=PASS` | `PASS` | Release verifier reports the expected three mechanical report-binding failures and ten open gates. |

## Unrun checks and reason

- Native editor/save callback timing, accessibility, DPI, fonts, runtime
  startup, clean-machine, cross-machine, signing, installer, updater, legal,
  support, and release-owner checks — prohibited or outside current
  authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project R&D policy and not created.
- Hardware/firmware evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static contract probes do not prove Qt signal timing, native editor/save
  behavior, or close-time interleavings.
- Both delegated D98 review windows returned no conclusion; no child PASS is
  claimed.
- The portable candidate remains unsigned and release remains `NO-GO`; three
  mechanical report-binding failures and ten external release gates remain.

## Acceptance and evidence IDs

- Acceptance: `D98-AC01`, `S127`.
- Evidence: ADR-0123, D98 source/order probes, parent/independent review
  records, compile/lint/format checks, package identity, handoff/index/register
  checks, and expected release NO-GO.

## Next owner and next action

- Owner: Architect.
- Action: run synchronized handoff/repository/release checks, then continue the
  next smallest MainWindow/application boundary or obtain authorized
  runtime/release evidence.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and root `QuillForge.exe`.
- Version: `0.1.0`.
- SHA-256 / size: `F489AED828573C8629B361D52C817A3DCD1B7283CC108E3B106B8A26B0303834` /
  `38,481,583` bytes.
- Source revision: `tree-sha256:620b6484cc6c084f427ed1da2ab550b20dd84db1ef4b8bf4e2f213ccac175ca6`.
- Packaging note: portable one-file candidate rebuilt; signing and installer
  remain open.

## Disposition

`accepted-with-limits`: valid document-save projection is isolated behind a
Qt-free typed boundary, while native runtime and enterprise release gates
remain open.

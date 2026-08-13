# Handoff: 2026-08-10-d97-document-open-projection

| Field | Value |
|---|---|
| ID | `2026-08-10-d97-document-open-projection` |
| Delivery / slice | `D97 / ARCH-71 document-open projection coordinator boundary` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-10T23:59:00+08:00` |

## User outcome

Valid document-open projection now has one focused Qt-free coordinator. The
existing file-open behavior remains intact: duplicate files are rejected or
reused during session restore, new tabs open with optional line/cursor
positioning, and events, notifications, and restore continuation keep their
previous order.

## Scope and boundaries

### In scope

- `DocumentOpenProjectionCoordinator[TabT]` valid-result contract.
- Ordinary/session duplicate handling, tab projection, line/cursor placement,
  restored-tab recording, event/notification projection, and restore
  continuation.
- Direct D86 `DocumentOpenCoordinator` wiring and removal of the old
  MainWindow valid-open method.
- Source, contract, static, package, handoff, and release evidence.

### Out of scope

- No DocumentService/Store, D86 stale/invalid/failure classification,
  DocumentTabSurface, editor implementation, persistence schema, recovery
  policy, plugin API, locale contract, or close policy changed.
- No new worker, retry, cache, event bus, document state model, or service
  locator.
- No Qt launch, screenshot, native editor/tab/session review,
  accessibility/DPI/font, clean-machine, cross-machine, signing, installer,
  updater, legal, support, or release-owner evidence.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Leibniz the 3rd / Luna max | Read-only boundary consultation; `NO_CONCLUSION` after bounded window |
| Independent review | Dewey the 3rd / Luna max | Read-only source review; `NO_CONCLUSION` after bounded window |
| Parent | Architect | Sole writer, integration, source review, simplification, and verification |

No child PASS is claimed.

## Changed files and modules

- `src/quillforge/presentation/document_open_projection_coordinator.py` —
  Qt-free valid-open projection sequence.
- `src/quillforge/presentation/main_window.py` — direct coordinator wiring and
  removal of `_apply_opened_document`.
- D97 ADR, parent/independent review records, handoff, and synchronized
  acceptance/delivery/roadmap/spec/task records.

## Decisions and constraints

- `DocumentOpenCoordinator` remains responsible for stale suppression,
  `OpenedDocument` validation, restore binding consumption, and ordinary/
  session failure classification.
- MainWindow remains the composition root and supplies concrete tab/editor,
  EventBus, notification, session, and error callbacks.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, static, packaging, and release
  handoff checks are the permitted validation boundary.
- This is Python/PyQt6 application/presentation code. Embedded C/C++
  assurance and vendor manufacturer requirements are `N/A`.
- Public CloudWeGo material remains an engineering reference only; no private
  ByteDance standard, certification, or compliance claim is made.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `D97-DOCUMENT-OPEN-PROJECTION-PROBE=PASS` | `PASS` | Ordinary/session new and duplicate ordering, line/cursor, recording, event, notification, and continuation. |
| `D97-QT-FREE-WIRING-PROBE=PASS` | `PASS` | Forbidden dependency strings absent; old method removed; D86 wires directly to `.project`. |
| `python -m compileall -q src/quillforge` | `PASS` | No launch or QApplication instantiation. |
| `uv run ruff check src/quillforge` | `PASS` | No lint errors. |
| `uv run ruff format --check src/quillforge` | `PASS` | 114 files already formatted. |
| `scripts\package.ps1` | `PASS` | Portable candidate rebuilt; root/dist identities match. |
| D97 package identity | `PASS` | SHA-256 `AAB904661E563A476E287F50C427354B813B11181D7E9997ED1D60BC7C93471A`, 38,481,204 bytes. |

## Unrun checks and reason

- Native editor/tab/session callback timing, accessibility, DPI, fonts,
  runtime startup, clean-machine, cross-machine, signing, installer,
  updater, legal, support, and release-owner checks — prohibited or outside
  current authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project R&D policy and not created.
- Hardware/firmware evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static contract probes do not prove Qt signal timing, native tab/editor
  behavior, or session-restore interleavings.
- Both delegated D97 review windows returned no conclusion; no child PASS is
  claimed.
- The portable candidate remains unsigned and release remains `NO-GO`; three
  mechanical report-binding failures and ten external release gates remain.

## Acceptance and evidence IDs

- Acceptance: `D97-AC01`, `S126`.
- Evidence: ADR-0122, D97 source probes, parent/independent review records,
  compile/lint/format checks, package identity, handoff/index/register checks,
  and expected release NO-GO.

## Next owner and next action

- Owner: Architect.
- Action: run synchronized handoff/repository/release checks, then continue
  the next smallest MainWindow/application boundary or obtain authorized
  runtime/release evidence.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and root `QuillForge.exe`.
- Version: `0.1.0`.
- SHA-256 / size: `AAB904661E563A476E287F50C427354B813B11181D7E9997ED1D60BC7C93471A` /
  `38,481,204` bytes.
- Source revision: `tree-sha256:fafc85d6ae6f71353327be7ad090f5c62b9a5fa508fe287eda46916933e69282`.
- Packaging note: portable one-file candidate rebuilt; signing and installer
  remain open.

## Disposition

`accepted-with-limits`: valid document-open projection is isolated behind a
Qt-free typed boundary, while native runtime and enterprise release gates
remain open.

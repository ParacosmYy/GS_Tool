# Handoff: 2026-08-10-d87-document-save-coordinator

| Field | Value |
|---|---|
| ID | `2026-08-10-d87-document-save-coordinator` |
| Delivery / slice | `D87 / ARCH-62 document-save coordinator boundary` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-10T23:59:00+08:00` |

## User outcome

Asynchronous document-save completion and failure handling now has one focused
Qt-free coordinator with an opaque tab contract. MainWindow still owns save
dispatch, document state consequences, editor presentation, recovery cleanup,
events, notifications, session persistence, and close behavior.

## Scope and boundaries

### In scope

- Qt-free `DocumentSaveCoordinator[TabT]` for stale, live, invalid, valid,
  and failed save completion classification.
- Existing tab-liveness and read-only lifecycle projection through narrow
  callbacks.
- MainWindow callback wiring and removal of the two old save completion methods.
- Preservation of state/language/title/recovery/event/notification/session-save
  and `after` policy.
- Source, static, package, handoff, and release evidence.

### Out of scope

- No DocumentService, DocumentStore, encoding, conflict, target-path, tab
  identity, editor widget, recovery store, event bus, or close policy change.
- No new asynchronous framework, worker, retry, cache, or persistence model.
- No native runtime, screenshot, accessibility, clean-machine, signing,
  installer, updater, legal, support, or release-owner change.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Kuhn the 3rd / Luna max | Read-only boundary consultation; `NO_CONCLUSION` after bounded window |
| Independent review | Averroes the 3rd / Luna max | Read-only source review; `NO_CONCLUSION` after bounded window |
| Parent | Architect | Sole writer, integration, source review, and verification |

No child PASS is claimed.

## Changed files and modules

- `src/quillforge/presentation/document_save_coordinator.py` — Qt-free
  save-result/failure and tab lifecycle classification.
- `src/quillforge/presentation/main_window.py` — coordinator composition and
  callback wiring; valid save policy remains local.
- D87 ADR/reviews/handoff and synchronized delivery records.

## Decisions and constraints

- MainWindow retains DocumentService, target/duplicate policy, state/text
  snapshots, TaskRunner, editor state consequences, recovery cleanup, events,
  notifications, session save, optional `after`, and close behavior.
- The coordinator ignores stale callbacks, protects removed tabs, restores
  read-only state for live callbacks, validates `DocumentState`, and never
  owns a Qt object or filesystem policy.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, static, packaging, and release
  handoff checks are the permitted validation boundary.
- This is Python/PyQt6 application/presentation code. Embedded C/C++
  assurance and vendor manufacturer requirements are `N/A`.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| D87 document-save Qt-free boundary probe | `PASS` | Callback wiring, tab/liveness/read-only boundary, old callback removal, and retained policy. |
| D87 targeted compileall | `PASS` | Changed presentation source. |
| D87 Ruff / format | `PASS` | `uv run ruff check` and `uv run ruff format --check` on changed source. |
| D87 package identity | `PASS` | Root/dist candidate identity recorded below. |
| D87 JSON/traceability/release/no-process probes | `PASS` | Synchronized records, package manifest, and expected release dossier checks passed. |
| `scripts\verify_handoff.ps1` | `PASS` | Final D87 records and indexed handoff are synchronized. |
| `scripts\check.ps1` | `PASS` | Repository checks passed; 108 files already formatted. |
| `scripts\verify_release_handoff.ps1` | `EXPECTED NO-GO` | Existing artifact-binding and external release gates remain open. |

## Unrun checks and reason

- Native editor mutability, tab-close/save timing, accessibility, DPI, fonts,
  runtime startup, clean-machine, cross-machine, signing, installer, updater,
  legal, support, and release-owner checks — prohibited or outside
  authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project R&D policy and not created.
- Hardware/firmware evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static source probes do not prove native callback interleaving, editor
  mutability rendering, or save timing.
- Both delegated D87 review windows returned no conclusion; no child PASS is
  claimed.
- The portable candidate remains unsigned and release remains `NO-GO`.

## Acceptance and evidence IDs

- Acceptance: `D87-AC01`, `S116`.
- Evidence: ADR-0112, D87 boundary probe, parent/independent review records,
  static checks, package identity, and expected release NO-GO.

## Next owner and next action

- Owner: Architect.
- Action: synchronize D87 delivery records, run handoff/repository/release
  checks, then continue the next bounded MainWindow/application coordinator
  slice or obtain authorized runtime/release evidence.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and root `QuillForge.exe`.
- Version: `0.1.0`.
- SHA-256 / size: `CC6C729837C22D6103E180C78C81DB30F2C49AEC19AA6F1BF51B59B27FC8379D` /
  `38,466,941` bytes.
- Source revision: `tree-sha256:27dd8ee70cc3603084889d20f837e60a22bdcf8aef280cc66cd8352c28aabf8c`.

## Disposition

`accepted-with-limits`: document-save completion handling is isolated behind a
Qt-free typed boundary and the package identity is recorded, while native
runtime and release gates remain open.

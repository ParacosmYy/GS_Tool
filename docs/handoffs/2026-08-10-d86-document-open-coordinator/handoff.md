# Handoff: 2026-08-10-d86-document-open-coordinator

| Field | Value |
|---|---|
| ID | `2026-08-10-d86-document-open-coordinator` |
| Delivery / slice | `D86 / ARCH-61 document-open coordinator boundary` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-10T23:59:00+08:00` |

## User outcome

Asynchronous ordinary document opens and ordered session-restore opens now
share one focused Qt-free completion coordinator. MainWindow still owns the
document service dispatch, tab/editor consequences, duplicate identity policy,
cursor/line projection, events, notifications, persistence, and close policy.

## Scope and boundaries

### In scope

- Qt-free `DocumentOpenCoordinator` for stale, invalid, valid, and failure
  completion classification.
- Existing session-restore operation binding consumption and continuation.
- MainWindow callback wiring and removal of the three old open callback
  methods.
- Preservation of line-number forwarding, duplicate-tab handling, editor/tab
  creation, cursor restoration, events, and success policy.
- Source, static, package, handoff, and release evidence.

### Out of scope

- No DocumentService, DocumentStore, file decoding, path resolution, tab
  surface, editor widget, save callback, recovery policy, or close policy
  change.
- No new asynchronous framework, worker, retry, cache, or event bus.
- No native runtime, screenshot, accessibility, clean-machine, signing,
  installer, updater, legal, support, or release-owner change.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Copernicus the 3rd / Luna max | Read-only boundary consultation; `NO_CONCLUSION` after bounded window |
| Independent review | Hubble the 3rd / Luna max | Read-only source review; `NO_CONCLUSION` after bounded window |
| Parent | Architect | Sole writer, integration, source review, and verification |

No child PASS is claimed.

## Changed files and modules

- `src/quillforge/presentation/document_open_coordinator.py` — Qt-free
  open-result/failure and session-restore lifecycle classification.
- `src/quillforge/presentation/main_window.py` — coordinator composition and
  callback wiring; valid tab/editor policy remains local.
- D86 ADR/reviews/handoff and synchronized delivery records.

## Decisions and constraints

- MainWindow retains DocumentService, TaskRunner, generic operation/busy
  policy, duplicate path/tab policy, editor creation, line/cursor projection,
  events, notifications, session save/restore sequencing, and close behavior.
- The coordinator ignores stale callbacks, consumes only matching restore
  bindings, preserves existing ordinary/session invalid/failure messages, and
  never owns a Qt object or filesystem policy.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, static, packaging, and release
  handoff checks are the permitted validation boundary.
- This is Python/PyQt6 application/presentation code. Embedded C/C++
  assurance and vendor manufacturer requirements are `N/A`.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| D86 document-open Qt-free boundary probe | `PASS` | Callback wiring, session-binding ownership, old callback removal, and retained tab/save policy. |
| D86 targeted compileall | `PASS` | Changed presentation source. |
| D86 Ruff / format | `PASS` | `uv run ruff check` and `uv run ruff format --check` on changed source. |
| D86 package identity | `PASS` | Root/dist candidate identity recorded below. |
| D86 JSON/traceability/release/no-process probes | `PASS` | Synchronized records, package manifest, and expected release dossier checks passed. |
| `scripts\verify_handoff.ps1` | `PASS` | Final D86 records and indexed handoff are synchronized. |
| `scripts\check.ps1` | `PASS` | Repository checks passed; 107 files already formatted. |
| `scripts\verify_release_handoff.ps1` | `EXPECTED NO-GO` | Existing artifact-binding and external release gates remain open. |

## Unrun checks and reason

- Native Qt tab/editor and session restore interaction, callback timing,
  accessibility, DPI, fonts, runtime startup, clean-machine, cross-machine,
  signing, installer, updater, legal, support, and release-owner checks —
  prohibited or outside authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project R&D policy and not created.
- Hardware/firmware evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static source probes do not prove native callback interleaving, Qt signal
  delivery, editor rendering, or session restore timing.
- Both delegated D86 review windows returned no conclusion; no child PASS is
  claimed.
- The portable candidate remains unsigned and release remains `NO-GO`.

## Acceptance and evidence IDs

- Acceptance: `D86-AC01`, `S115`.
- Evidence: ADR-0111, D86 boundary probe, parent/independent review records,
  static checks, package identity, and expected release NO-GO.

## Next owner and next action

- Owner: Architect.
- Action: synchronize D86 delivery records, run handoff/repository/release
  checks, then continue the next bounded MainWindow/application coordinator
  slice or obtain authorized runtime/release evidence.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and root `QuillForge.exe`.
- Version: `0.1.0`.
- SHA-256 / size: `CD04E0621C98BC2519335FF322F197416968C1CCF613AEB1FBDDAFF2467744CA` /
  `38,465,423` bytes.
- Source revision: `tree-sha256:1905665ca609e16bcd7074b622de0e65e2ece74b3ece31cf4ace14618e328b09`.

## Disposition

`accepted-with-limits`: document-open completion handling is isolated behind a
Qt-free typed boundary and the package identity is recorded, while native
runtime and release gates remain open.

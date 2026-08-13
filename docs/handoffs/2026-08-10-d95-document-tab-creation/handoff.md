# Handoff: 2026-08-10-d95-document-tab-creation

| Field | Value |
|---|---|
| ID | `2026-08-10-d95-document-tab-creation` |
| Delivery / slice | `D95 / ARCH-70 document-tab creation coordinator boundary` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-10T23:59:00+08:00` |

## User outcome

Document-tab assembly now has one focused Qt-free coordinator. MainWindow still
owns document/open/recovery consequences, editor/settings policy, concrete tab
creation, notifications, persistence, and close behavior.

## Scope and boundaries

### In scope

- Qt-free generic `DocumentTabCreationCoordinator[OpenedT, TabT, EditorT]`.
- Existing editor creation, tab construction, recovery snapshot binding,
  surface projection, title/modified refresh, session-save, and status order.
- MainWindow composition and `_add_tab` delegation.
- Source, static, package, handoff, and release evidence.

### Out of scope

- No DocumentService, RecoveryService, editor implementation, settings schema,
  DocumentTabSurface contract, event/notification, session restore, or close
  policy change.
- No new asynchronous framework, worker, retry, cache, or persistence model.
- No native runtime, screenshot, accessibility, clean-machine, signing,
  installer, updater, legal, support, or release-owner change.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Huygens the 3rd / Luna max | Read-only boundary consultation; `NO_CONCLUSION` after bounded window |
| Independent review | Fermat the 3rd / Luna max | Read-only source review; `NO_CONCLUSION` after bounded window |
| Parent | Architect | Sole writer, integration, source review, and verification |

No child PASS is claimed.

## Changed files and modules

- `src/quillforge/presentation/document_tab_creation_coordinator.py` — Qt-free
  tab assembly and projection orchestration.
- `src/quillforge/presentation/main_window.py` — coordinator composition and
  `_add_tab` delegation; concrete policy remains local.
- D95 ADR/reviews/handoff and synchronized delivery records.

## Decisions and constraints

- The coordinator preserves editor creation, recovery identity, tab surface
  add/currentChanged timing, title/modified projection, session-save request,
  and status synchronization order.
- MainWindow retains settings/theme/editor policy, document results, tab record
  factory, open/recovery events and notifications, persistence, and close
  behavior.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, static, packaging, and release
  handoff checks are the permitted validation boundary.
- This is Python/PyQt6 application/presentation code. Embedded C/C++
  assurance and vendor manufacturer requirements are `N/A`.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `D95-DOCUMENT-TAB-CREATION-QT-FREE-BOUNDARY-PROBE=PASS` | `PASS` | Import boundary, MainWindow wiring, delegation, and assembly removal. |
| D95 targeted compileall | `PASS` | Changed presentation source and package modules. |
| D95 Ruff / format | `PASS` | `uv run ruff check` and `uv run ruff format --check` on changed source. |
| D95 package identity | `PASS` | Root/dist candidate identity recorded below. |
| D95 JSON/traceability/release/no-process probes | `PASS` | Synchronized records, package manifest, and expected release dossier checks. |
| `scripts\verify_handoff.ps1` | `PASS` | Final D95 records and indexed handoff are synchronized. |
| `scripts\check.ps1` | `PASS` | Repository checks passed. |
| `scripts\verify_release_handoff.ps1` | `EXPECTED NO-GO` | Existing three stale runtime-report bindings and ten external release gates remain open. |

## Unrun checks and reason

- Native editor creation/currentChanged timing, accessibility, DPI, fonts,
  runtime startup, clean-machine, cross-machine, signing, installer, updater,
  legal, support, and release-owner checks — prohibited or outside
  authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project R&D policy and not created.
- Hardware/firmware evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static source probes do not prove native editor creation, currentChanged
  re-entry, or font/DPI behavior.
- Both delegated D95 review windows returned no conclusion; no child PASS is
  claimed.
- The portable candidate remains unsigned and release remains `NO-GO`.

## Acceptance and evidence IDs

- Acceptance: `D95-AC01`, `S124`.
- Evidence: ADR-0120, D95 boundary probe, parent/independent review records,
  static checks, package identity, and expected release NO-GO.

## Next owner and next action

- Owner: Architect.
- Action: synchronize D95 delivery records, run handoff/repository/release
  checks, then continue the next bounded MainWindow/application coordinator
  slice or obtain authorized runtime/release evidence.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and root `QuillForge.exe`.
- Version: `0.1.0`.
- SHA-256 / size: `47E87F24B5E7106C6600D64D9B5638E11B2AEF30555554B8BBB68C2D4751CE15` /
  `38,478,966` bytes.
- Source revision: `tree-sha256:45e4df79c51dde0bde122fa50162b0cf588a8bcf3331f230b2c087dddd656a07`.

## Disposition

`accepted-with-limits`: document-tab assembly is isolated behind a Qt-free typed
boundary while native runtime and release gates remain open.

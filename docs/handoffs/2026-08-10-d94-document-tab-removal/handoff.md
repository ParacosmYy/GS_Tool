# Handoff: 2026-08-10-d94-document-tab-removal

| Field | Value |
|---|---|
| ID | `2026-08-10-d94-document-tab-removal` |
| Delivery / slice | `D94 / ARCH-69 document-tab removal coordinator boundary` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-10T23:59:00+08:00` |

## User outcome

After close eligibility is approved, document-tab removal now runs through one
focused Qt-free coordinator. MainWindow still owns close guards, dirty/save
confirmation, recovery service policy, notifications, persistence, and close
behavior.

## Scope and boundaries

### In scope

- Qt-free generic `DocumentTabRemovalCoordinator[TabT, CaptureT]`.
- Existing recovery capture cancellation, snapshot cleanup, tab/editor/event/
  session finalization, and empty-tab fallback order.
- MainWindow composition and `_remove_tab` delegation while `_close_tab` retains
  close policy.
- Source, static, package, handoff, and release evidence.

### Out of scope

- No dirty/save/cancel policy, DocumentTabSurface contract, recovery service,
  snapshot store, editor implementation, event bus, session schema, or close
  guard change.
- No new asynchronous framework, worker, retry, cache, or persistence model.
- No native runtime, screenshot, accessibility, clean-machine, signing,
  installer, updater, legal, support, or release-owner change.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Lagrange the 3rd / Luna max | Read-only boundary consultation; `NO_CONCLUSION` after bounded window |
| Independent review | Harvey the 3rd / Luna max | Read-only source review; `NO_CONCLUSION` after bounded window |
| Parent | Architect | Sole writer, integration, source review, and verification |

No child PASS is claimed.

## Changed files and modules

- `src/quillforge/presentation/document_tab_removal_coordinator.py` — Qt-free
  tab finalization orchestration.
- `src/quillforge/presentation/main_window.py` — coordinator composition and
  `_remove_tab` delegation; close policy remains local.
- D94 ADR/reviews/handoff and synchronized delivery records.

## Decisions and constraints

- The coordinator invokes recovery cancel/snapshot cleanup and preserves the
  old tab projection, editor teardown, event, session-save, and empty-tab order.
- MainWindow retains the choice to close, save-before-close, cancel, and all
  service/notification/close policy.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, static, packaging, and release
  handoff checks are the permitted validation boundary.
- This is Python/PyQt6 application/presentation code. Embedded C/C++
  assurance and vendor manufacturer requirements are `N/A`.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `D94-DOCUMENT-TAB-REMOVAL-QT-FREE-BOUNDARY-PROBE=PASS` | `PASS` | Import boundary, MainWindow delegation, and finalization removal. |
| D94 targeted compileall | `PASS` | Changed presentation source and package modules. |
| D94 Ruff / format | `PASS` | `uv run ruff check` and `uv run ruff format --check` on changed source. |
| D94 package identity | `PASS` | Root/dist candidate identity recorded below. |
| D94 JSON/traceability/release/no-process probes | `PASS` | Synchronized records, package manifest, and expected release dossier checks. |
| `scripts\verify_handoff.ps1` | `PASS` | Final D94 records and indexed handoff are synchronized. |
| `scripts\check.ps1` | `PASS` | Repository checks passed. |
| `scripts\verify_release_handoff.ps1` | `EXPECTED NO-GO` | Existing three stale runtime-report bindings and ten external release gates remain open. |

## Unrun checks and reason

- Native close/save/recovery/delete timing, editor teardown, accessibility,
  DPI, fonts, runtime startup, clean-machine, cross-machine, signing,
  installer, updater, legal, support, and release-owner checks — prohibited or
  outside authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project R&D policy and not created.
- Hardware/firmware evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static source probes do not prove native close/save/recovery timing or editor
  destruction behavior.
- Both delegated D94 review windows returned no conclusion; no child PASS is
  claimed.
- The portable candidate remains unsigned and release remains `NO-GO`.

## Acceptance and evidence IDs

- Acceptance: `D94-AC01`, `S123`.
- Evidence: ADR-0119, D94 boundary probe, parent/independent review records,
  static checks, package identity, and expected release NO-GO.

## Next owner and next action

- Owner: Architect.
- Action: synchronize D94 delivery records, run handoff/repository/release
  checks, then continue the next bounded MainWindow/application coordinator
  slice or obtain authorized runtime/release evidence.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and root `QuillForge.exe`.
- Version: `0.1.0`.
- SHA-256 / size: `AB24C523DBAF5D418D3436A1A5920290674FDAC6B787F7CB643056F9865CB757` /
  `38,478,658` bytes.
- Source revision: `tree-sha256:2c6a260102ada2e7543b991438c5060025f212f2363a1cf054d74d8334ff6eba`.

## Disposition

`accepted-with-limits`: approved document-tab removal finalization is isolated
behind a Qt-free typed boundary while native runtime and release gates remain
open.

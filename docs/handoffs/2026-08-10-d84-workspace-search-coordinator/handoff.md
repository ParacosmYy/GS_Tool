# Handoff: 2026-08-10-d84-workspace-search-coordinator

| Field | Value |
|---|---|
| ID | `2026-08-10-d84-workspace-search-coordinator` |
| Delivery / slice | `D84 / ARCH-59 workspace-search coordinator boundary` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-10T23:59:00+08:00` |

## User outcome

Find in Files completion handling now has a focused Qt-free coordinator. The
shell still owns workspace search service/query construction, cooperative
cancellation, result activation, root containment, surface construction, and
close behavior.

## Scope and boundaries

### In scope

- Qt-free `WorkspaceSearchCoordinator` for current/stale/invalidated result
  and failure classification.
- Minimal surface Protocol, locale-aware summary seam, and notification
  projection.
- MainWindow callback wiring and removal of the two old completion callbacks.
- Preservation of service/query/cancel/containment/open/close ownership.
- Source, static, package, handoff, and release evidence.

### Out of scope

- No WorkspaceSearchService, provider, search policy, filesystem traversal,
  root containment, document opening, dialog construction, or close policy
  change.
- No MainWindow wholesale rewrite or new asynchronous framework.
- No native runtime, screenshot, accessibility, clean-machine, signing,
  installer, updater, legal, support, or release-owner change.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Faraday the 3rd / Luna max | Read-only boundary consultation; `NO_CONCLUSION` after bounded window |
| Independent review | Bohr the 3rd / Luna max | Read-only source review; `NO_CONCLUSION` after bounded window |
| Parent | Architect | Sole writer, integration, source review, and verification |

No child PASS is claimed.

## Changed files and modules

- `src/quillforge/presentation/workspace_search_coordinator.py` — Qt-free
  callback classification and surface/summary orchestration.
- `src/quillforge/presentation/main_window.py` — coordinator composition and
  callback wiring; search policy remains local.
- D84 ADR/reviews/handoff and synchronized delivery records.

## Decisions and constraints

- MainWindow retains WorkspaceSearchService, query validation, TaskRunner,
  generation/cancellation inputs, workspace root/containment, result opening,
  surface construction, and close behavior.
- The coordinator ignores stale callbacks and never owns a Qt object,
  filesystem path policy, or editor activation.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, static, packaging, and release
  handoff checks are the permitted validation boundary.
- This is Python/PyQt6 application/presentation code. Embedded C/C++
  assurance and vendor manufacturer requirements are `N/A`.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| D84 workspace-search boundary probe | `PASS` | Callback wiring, tracker ownership, stale/invalidated/result/failure projection, and policy retention. |
| D84 Qt-free coordinator probe | `PASS` | Bare import does not load PyQt6. |
| D84 compileall / Ruff / format | `PASS` | Changed presentation source. |
| D84 package identity probe | `PASS` | Root/dist candidate identity recorded below. |
| D84 JSON/traceability/release/no-process probes | `PASS` | Machine records, current dossier, and no running QuillForge process. |
| `scripts\verify_handoff.ps1` | `PASS` | Final D84 records and indexed handoff are synchronized. |
| `scripts\check.ps1` | `PASS` | Final repository checks passed after synchronization. |
| `scripts\verify_release_handoff.ps1` | `EXPECTED NO-GO` | Existing report bindings and external gates remain open. |

## Unrun checks and reason

- Native Qt search dialog rendering, callback/cancellation timing,
  accessibility, DPI, fonts, runtime startup, clean-machine, cross-machine,
  signing, installer, updater, legal, support, and release-owner checks —
  prohibited or outside authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project R&D policy and not created.
- Hardware/firmware evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static source probes do not prove native cancellation interleaving or search
  dialog rendering.
- Both delegated review windows returned no conclusion; no child PASS is
  claimed.
- The portable candidate remains unsigned and release remains `NO-GO`.

## Acceptance and evidence IDs

- Acceptance: `D84-AC01`, `S113`.
- Evidence: ADR-0109, D84 boundary/Qt-free probes, parent/independent review
  records, static checks, package identity, and expected release NO-GO.

## Next owner and next action

- Owner: Architect.
- Action: synchronize D84 delivery records, run handoff/repository checks, then
  continue the next bounded MainWindow/application coordinator slice or obtain
  authorized runtime/release evidence.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and root `QuillForge.exe`.
- Version: `0.1.0`.
- SHA-256 / size: `35B670A77033ADC4F9B9A7143CB6E4BEE19AE7B4AA99F0652D94BEBB76832336` / `38,461,955` bytes.
- Source revision: `tree-sha256:d454eb3e5044173747fda07a4f93bc206bc739a188311711f2c1336d0bb92427`.

## Disposition

`accepted-with-limits`: workspace-search completion handling is isolated behind
a Qt-free typed boundary and the package identity is recorded, while native
runtime and release gates remain open.

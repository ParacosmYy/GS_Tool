# Handoff: 2026-08-10-d81-recovery-scan-coordinator

| Field | Value |
|---|---|
| ID | `2026-08-10-d81-recovery-scan-coordinator` |
| Delivery / slice | `D81 / ARCH-56 recovery-scan coordinator boundary` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-10T23:59:00+08:00` |

## User outcome

Recovery inventory result handling now has a focused Qt-free coordinator. The
shell still owns RecoveryService, TaskRunner dispatch, the recovery prompt and
restore/discard/later decisions, session/workspace/tab policy, and close
behavior.

## Scope and boundaries

### In scope

- Qt-free `RecoveryScanCoordinator` for inventory classification and stale
  callback release.
- Typed candidate prompt, session snapshot, startup continuation, and
  notification seams.
- MainWindow callback wiring and removal of three scan-result callbacks.
- Preservation of manual/startup empty behavior, invalid/failure handling,
  candidate dispatch, and close guard state.
- Source, static, package, handoff, and release evidence.

### Out of scope

- No RecoveryService, recovery persistence, RecoveryPromptSurface, restore/
  discard/later decision, document, session, workspace, or close policy change.
- No MainWindow wholesale rewrite or new asynchronous framework.
- No native runtime, screenshot, accessibility, clean-machine, signing,
  installer, updater, legal, support, or release-owner change.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Meitner the 3rd / Luna max | Read-only boundary consultation; `NO_CONCLUSION` after bounded window |
| Independent review | Ohm the 3rd / Luna max | Read-only source review; `NO_CONCLUSION` after bounded window |
| Parent | Architect | Sole writer, integration, source review, and verification |

No child PASS is claimed.

## Changed files and modules

- `src/quillforge/presentation/recovery_scan_coordinator.py` — Qt-free
  inventory result classification and startup continuation.
- `src/quillforge/presentation/main_window.py` — coordinator composition,
  callback wiring, and removal of three scan-result callbacks.
- D81 ADR/reviews/handoff and synchronized delivery records.

## Decisions and constraints

- MainWindow retains RecoveryService, TaskRunner, RecoveryPromptSurface,
  restore/discard/later decisions, session/workspace/tab policy, and close
  behavior.
- The coordinator finishes only the matching RecoveryScanJob and never chooses
  a recovery decision or executes a filesystem operation.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, static, packaging, and release
  handoff checks are the permitted validation boundary.
- This is Python/PyQt6 application/presentation code. Embedded C/C++
  assurance and vendor manufacturer requirements are `N/A`.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| D81 recovery-scan boundary probe | `PASS` | Job identity, result validation, prompt/continuation ownership, and close gates. |
| D81 Qt-free coordinator probe | `PASS` | Bare import does not load PyQt6. |
| D81 RED precondition probe | `PASS` | Confirmed the three callbacks before extraction. |
| Targeted compileall / Ruff / format | `PASS` | Changed presentation source. |
| `scripts\package.ps1` | `PASS` | Root/dist candidate identity recorded below. |
| `scripts\verify_handoff.ps1` | `PASS` | Final D81 records and indexed handoff are synchronized. |
| `scripts\check.ps1` | `PASS` | Final repository checks passed after synchronization. |
| `scripts\verify_release_handoff.ps1` | `EXPECTED NO-GO` | Existing report bindings and external gates remain open. |

## Unrun checks and reason

- Native Qt prompt rendering, callback interleaving, recovery interaction,
  accessibility, DPI, fonts, runtime startup, clean-machine, cross-machine,
  signing, installer, updater, legal, support, and release-owner checks —
  prohibited or outside authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project R&D policy and not created.
- Hardware/firmware evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static source probes do not prove native prompt timing or actual recovery
  interaction on this machine.
- Both delegated review windows returned no conclusion; no child PASS is
  claimed.
- The portable candidate remains unsigned and release remains `NO-GO`.

## Acceptance and evidence IDs

- Acceptance: `D81-AC01`, `S110`.
- Evidence: ADR-0106, D81 boundary/Qt-free probes, parent/independent review
  records, static checks, package identity, and expected release NO-GO.

## Next owner and next action

- Owner: Architect.
- Action: synchronize D81 delivery records, run handoff/repository checks,
  then continue the next bounded MainWindow/application coordinator slice or
  obtain authorized runtime/release evidence.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and root `QuillForge.exe`.
- Version: `0.1.0`.
- SHA-256 / size: `C1CB37ED57C264134C4A7F46785970DF8664B964F2CEF145B8956B0120D86E84` / `38,458,844` bytes.
- Source revision: `tree-sha256:cbcb70c6454abeaacc5636ca1d673ffcccd2dcb6f2a1d85d5c7ffd3a103deedf`.

## Disposition

`accepted-with-limits`: recovery-scan result handling is isolated behind a
Qt-free typed boundary and the package identity is recorded, while native
runtime and release gates remain open.

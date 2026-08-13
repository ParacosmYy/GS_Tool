# Handoff: 2026-08-10-d91-recovery-capture-abort-coordinator

| Field | Value |
|---|---|
| ID | `2026-08-10-d91-recovery-capture-abort-coordinator` |
| Delivery / slice | `D91 / ARCH-66 recovery-capture abort coordinator boundary` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-10T23:59:00+08:00` |

## User outcome

Recovery capture cancellation and failure now share one focused Qt-free
coordinator. MainWindow still owns editor capture stepping, Qt scheduling,
channel construction, RecoveryService dispatch, retry/stale decisions, tab
policy, notifications, persistence, and close behavior.

## Scope and boundaries

### In scope

- Qt-free generic `RecoveryCaptureAbortCoordinator[JobT, OwnerT]`.
- Existing capture identity, worker-started discard, pre-worker release,
  channel/session abort, document lifecycle release, and optional failure
  notification ordering.
- MainWindow composition and wrapper wiring without changing capture scheduling.
- Source, static, package, handoff, and release evidence.

### Out of scope

- No RecoveryService, snapshot store, editor capture implementation, channel
  implementation, backpressure, writer callback, retry scheduler, or close
  policy change.
- No new asynchronous framework, worker, retry, cache, or persistence model.
- No native runtime, screenshot, accessibility, clean-machine, signing,
  installer, updater, legal, support, or release-owner change.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Herschel the 3rd / Luna max | Read-only boundary consultation; `NO_CONCLUSION` after bounded window |
| Independent review | Parfit the 3rd / Luna max | Read-only source review; `NO_CONCLUSION` after bounded window |
| Parent | Architect | Sole writer, integration, source review, and verification |

No child PASS is claimed.

## Changed files and modules

- `src/quillforge/presentation/recovery_capture_abort_coordinator.py` — Qt-free
  capture cleanup orchestration.
- `src/quillforge/presentation/main_window.py` — coordinator composition and
  abort wrapper; Qt scheduling and capture policy remain local.
- D91 ADR/reviews/handoff and synchronized delivery records.

## Decisions and constraints

- MainWindow retains editor session, QTimer scheduling, channel creation,
  RecoveryService, worker dispatch, tab liveness, dirty/content-version and
  stale decisions, notifications, persistence, and close behavior.
- The coordinator preserves identity-guarded producer cleanup and the
  worker-started discard/pre-worker release distinction without owning Qt,
  editor, filesystem, or foreground busy policy.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, static, packaging, and release
  handoff checks are the permitted validation boundary.
- This is Python/PyQt6 application/presentation code. Embedded C/C++
  assurance and vendor manufacturer requirements are `N/A`.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `D91-RECOVERY-CAPTURE-ABORT-QT-FREE-BOUNDARY-PROBE=PASS` | `PASS` | Import boundary, MainWindow wiring, wrapper lifecycle removal, and retained policy. |
| D91 targeted compileall | `PASS` | Changed presentation source and package modules. |
| D91 Ruff / format | `PASS` | `uv run ruff check` and `uv run ruff format --check` on changed source. |
| D91 package identity | `PASS` | Root/dist candidate identity recorded below. |
| D91 JSON/traceability/release/no-process probes | `PASS` | Synchronized records, package manifest, and expected release dossier checks. |
| `scripts\verify_handoff.ps1` | `PASS` | Final D91 records and indexed handoff are synchronized. |
| `scripts\check.ps1` | `PASS` | Repository checks passed. |
| `scripts\verify_release_handoff.ps1` | `EXPECTED NO-GO` | Existing artifact-binding and external release gates remain open. |

## Unrun checks and reason

- Native editor capture/session/channel timing, backpressure, accessibility,
  DPI, fonts, runtime startup, clean-machine, cross-machine, signing,
  installer, updater, legal, support, and release-owner checks — prohibited or
  outside authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project R&D policy and not created.
- Hardware/firmware evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static source probes do not prove native editor capture timing, channel
  backpressure, or writer interleaving.
- Both delegated D91 review windows returned no conclusion; no child PASS is
  claimed.
- The portable candidate remains unsigned and release remains `NO-GO`.

## Acceptance and evidence IDs

- Acceptance: `D91-AC01`, `S120`.
- Evidence: ADR-0116, D91 boundary probe, parent/independent review records,
  static checks, package identity, and expected release NO-GO.

## Next owner and next action

- Owner: Architect.
- Action: synchronize D91 delivery records, run handoff/repository/release
  checks, then continue the next bounded MainWindow/application coordinator
  slice or obtain authorized runtime/release evidence.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and root `QuillForge.exe`.
- Version: `0.1.0`.
- SHA-256 / size: `9408A2EEAD427FA74BBD439948F1E18677C4A6272454A4456B6FEA4C87165A6B` /
  `38,473,963` bytes.
- Source revision: `tree-sha256:ee34efbe0d27cafb1accbb7bd42f8614c39096361de11d9cfb381553791a390a`.

## Disposition

`accepted-with-limits`: recovery capture failure/cancellation cleanup is
isolated behind a Qt-free typed boundary while native runtime and release gates
remain open.

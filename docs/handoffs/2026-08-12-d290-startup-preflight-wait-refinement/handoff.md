# Handoff: 2026-08-12-d290-startup-preflight-wait-refinement

| Field | Value |
|---|---|
| ID | `2026-08-12-d290-startup-preflight-wait-refinement` |
| Delivery / slice | `D290 / ARCH-260 Startup preflight wait refinement` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Shared checkout | `D:\Workplace\Agent_Workplace\QuillForge` |

## User outcome

The no-window file-open/startup preflight no longer busy-spins while waiting
for asynchronous Qt work. It retains the production file-open path and
completion evidence while reducing event-batch churn.

## Scope and boundaries

- Added an unparented, short-lived 2ms `QTimer` wake source to the existing
  wait helper.
- Added the Qt 6 `WaitForMoreEvents` flag to let the calling thread wait when
  the event queue is empty.
- Preserved timeout, pending-work, startup-path, timer-cleanup, report, and
  no-window/no-`exec()` contracts; the report explicitly says
  `timeout_mode=soft`.
- No EXE/Qt launch, native dialog, unit-test asset, installer/updater/registry
  operation, or worktree was used.

## Team roles and ownership

| Role | Owner / agent | Responsibility |
|---|---|---|
| Architect | parent; Luna/max consultation | Wait-loop boundary and architecture review |
| Developer | parent | MainWindow and static-contract implementation |
| QA | parent | Source diagnostics and non-destructive static/build verification |
| Independent reviewer | Luna/max consultation | Requested independent source review; no conclusion returned |

## Changed files and modules

- `src/quillforge/presentation/main_window.py` — bounded wake-up wait.
- `scripts/audit_presentation_contracts.py` — wake/flag/cleanup contract.
- `docs/adr/0326-startup-preflight-wait-refinement.md` and review records.

## Decisions and constraints

- Keep the wait and cleanup ownership in `MainWindow`; do not move startup
  policy into the application entry point.
- Use a Qt wake source instead of a blocking sleep or a nested local event
  loop, so the diagnostic preserves queued worker completion delivery without
  entering `QApplication.exec()`.
- Shared checkout writer: parent only; no worktree or parallel writer was
  used.
- Runtime launch policy: EXE/Qt startup was not allowed; only source,
  offscreen diagnostic, static, package, and archive evidence was authorized.

## Public-source applicability

Python 3.12 monotonic-clock/cleanup semantics and public Qt 6.11 event-loop
flags are applicable. Source: <https://doc.qt.io/qt-6/qeventloop.html>. No
manufacturer requirement applies. Embedded workflow and simplifier: `N/A`
because this is Python/Qt desktop code; no private ByteDance standard,
certification, MISRA, ISO 26262, ASPICE, or firmware claim is made.

## Verification commands and results

| Evidence | Result |
|---|---|
| Baseline | D289 source diagnostic: 8,679 event batches / about 592 ms |
| Repeated source diagnostic | D290 runs: 7, 9, and 7 event batches; final revised run: 11; all exit 0 |
| File-open result | `status=passed`; `startup_paths_total=1`; `startup_paths_open=1`; `pending_work=false` |
| No-window boundary | `window_shown=false`; `event_loop_entered=false` |
| Timeout semantics | `timeout_mode=soft`; deadline checked before/after event processing |
| Static contract | `D290-STARTUP-WAIT-CONTRACT=PASS`; targeted AST/control-flow checks |
| Compile/lint/format | `D290-COMPILEALL=PASS`; `D290-RUFF=PASS`; `D290-FORMAT=PASS` |
| Project checks | `D290-CHECK=PASS`; presentation contract audit passed |
| PE header | `D290-PE-HEADER=PASS`; AMD64 / PE32+ / WINDOWS_GUI |
| Frozen archive | `D290-PE-ARCHIVE=PASS`; 9 required entries present |
| Package identity | `D290-MANIFEST-COPY=PASS`; 38,590,159 bytes; SHA-256 `89A4B36A636866C744309C5FE2A81826501830F10A8B5B98A2F84FA5A3C61AA9` |
| Release verifier | `EXPECTED-NO-GO`; 10 open gates; three artifact-bound startup-report consistency failures remain under no-launch policy |
| Parent review | `PASS` |
| Independent review | Initial `REVISE`; follow-up Luna/max review `NO_CONCLUSION` after two bounded waits and closure |
| Simplification | `PASS` |

## Unrun checks and reason

Native EXE startup, frozen-process timing, native rendering, clean-machine
repeatability, shell drag/drop, signing, installer/update, registry, and
release-owner acceptance remain unrun under the active no-launch policy. The
release verifier is expected to remain `NO-GO` with artifact-bound startup
report gates open.

## Known risks and limits

- The 2ms wake interval is measured on the current source diagnostic only; it
  is not a native startup SLA.
- The timeout is soft: a long-running Qt event handler can finish after the
  deadline because main-thread handlers cannot be interrupted safely.
- The change does not prove a clean Windows machine, frozen Qt startup, or
  native window rendering.
- The first independent review returned `REVISE`; its three findings were
  addressed. The follow-up independent review returned `NO_CONCLUSION` after
  two bounded waits; no independent approval is claimed. Architecture
  consultation also returned no conclusion.

## Acceptance and evidence IDs

- Acceptance: `S330`.
- Architecture slice: `ARCH-260`.
- Evidence: `D290-STARTUP-WAIT-CONTRACT=PASS`,
  `D290-SOURCE-DIAGNOSTIC=PASS`, `D290-CHECK=PASS`, and package identity after
  packaging: `D290-MANIFEST-COPY=PASS`, `D290-PE-HEADER=PASS`,
  `D290-PE-ARCHIVE=PASS`.

## Next owner and next action

- Owner: `architect`.
- Action: rebuild the portable candidate, bind the artifact identity, and
  refresh the release dossier while keeping native startup unrun.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and root `QuillForge.exe`.
- SHA-256 / size: `89A4B36A636866C744309C5FE2A81826501830F10A8B5B98A2F84FA5A3C61AA9` /
  `38,590,159` bytes; root copy matches.
- Source revision: `tree-sha256:d326d606228c01df03006ce38002f4348f9766b46c4e8cd5f040e199a8148bf5`.
- Packaging note: portable candidate rebuilt and archive-bound; no native launch.

## Disposition

`accepted-with-limits`: measured source wait refinement, package identity, and
non-destructive release checks are recorded; native startup and enterprise
release gates remain open.

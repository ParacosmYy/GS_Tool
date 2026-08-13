# Handoff: 2026-08-12-d288-startup-restore-preflight

| Field | Value |
|---|---|
| ID | `2026-08-12-d288-startup-restore-preflight` |
| Delivery / slice | `D288 / ARCH-258 Startup restore preflight` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Shared checkout | `D:\Workplace\Agent_Workplace\QuillForge` |

## User outcome

The no-window startup diagnostic now exercises the production asynchronous
restore chain when no modal recovery decision is required. It proves session
file opening, editor-tab projection, worker drain, and restore-barrier release
without showing a native window or persisting user state.

## Scope and boundaries

- Reused `MainWindow.restore_startup_state()` and its existing coordinators.
- Waited through `QApplication.processEvents()` under a five-second bound.
- Stopped recovery/session-save timers in a `finally` block, including
  synchronous restore-start failures.
- Skipped valid recovery candidates safely instead of opening a modal prompt.
- No EXE/Qt launch, native dialog, unit-test asset, installer/updater/registry
  operation, or worktree was used.

## Team roles and ownership

| Role | Owner / agent | Responsibility |
|---|---|---|
| Architect | parent; Luna/max consultation | Restore lifecycle boundary and architecture review |
| Developer | parent | MainWindow/runtime/diagnostic implementation |
| QA | parent | Source, static, compile, package, PE/archive, and identity checks |
| Independent reviewer | Luna/max consultation | Requested independent source review; no conclusion returned |

## Changed files and modules

- `src/quillforge/presentation/main_window.py` — bounded production restore
  preflight and timer cleanup.
- `src/quillforge/composition.py` — thin runtime diagnostic seam.
- `src/quillforge/app.py` — safe recovery-candidate branch and restore probe.
- `scripts/audit_presentation_contracts.py` — no-window/non-modal contract.
- `README.md`, architecture/roadmap/plan records, ADR, reviews, acceptance,
  delivery register, and handoff index.

## Decisions and constraints

- Keep restore lifecycle and cleanup in `MainWindow`; do not duplicate the
  session/recovery policy in the application entry point.
- Do not open the recovery prompt during a no-window diagnostic.
- Shared checkout writer: parent only; no worktree or parallel writer was
  used.
- Runtime launch policy: EXE/Qt startup was not allowed; only non-destructive
  source, archive, package, and static evidence was authorized.

## Public-source applicability

Public Python 3.12 monotonic-clock/`try`-`finally` behavior and public Qt 6
event-processing/lifecycle references are applicable. No manufacturer
requirement applies. Embedded workflow and simplifier: `N/A` because this is
Python/Qt desktop code. No private ByteDance standard, MISRA, ISO 26262,
ASPICE, or certification claim is made.

## Verification commands and results

| Evidence | Result |
|---|---|
| Startup restore source diagnostic | `D288-SOURCE-DIAGNOSTIC=PASS exit=0`; `restore_stage=executed`, `startup_restore_completed=true`, `pending_work=false`, `tab_count=1` |
| No-window boundary | `window_shown=false`; `event_loop_entered=false` |
| User-state write boundary | session/settings timestamps unchanged after the preflight |
| Presentation/static contract | `D288-STARTUP-RESTORE-CONTRACT=PASS` |
| Compile/lint/format | `D288-COMPILEALL=PASS`; `D288-RUFF=PASS`; `D288-FORMAT=PASS` |
| Parent review | `PASS` |
| Independent review | `NO_CONCLUSION` after bounded waits and closure |
| Simplification | `PASS` |

## Unrun checks and reason

Native EXE/Qt startup, asynchronous restore in a frozen process, modal
recovery decisions, clean-machine behavior, native window lifetime,
cross-machine repeatability, signing, installer/update, registry, and release
owner acceptance remain unrun under the active no-launch policy. Release
verification is expected to remain `NO-GO` with historical artifact-bound
reports and enterprise gates open.

## Known risks and limits

- The no-window restore preflight covers the current no-recovery branch; valid
  recovery candidates are intentionally skipped because their decision is
  user-owned and modal.
- Passing source restore does not prove frozen Qt startup or native rendering.
- The independent review and architecture consultation returned no conclusion;
  no independent approval is claimed.

## Acceptance and evidence IDs

- Acceptance: `S328`.
- Architecture slice: `ARCH-258`.
- Evidence: `D288-STARTUP-RESTORE-CONTRACT=PASS`,
  `D288-SOURCE-DIAGNOSTIC=PASS`, `D288-CHECK=PASS`,
  `D288-MANIFEST-IDENTITY=PASS` after packaging.

## Next owner and next action

- Owner: `architect`.
- Action: rebuild the portable candidate, bind package identity, and run the
  release verifier while keeping native startup unrun.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and root `QuillForge.exe`.
- Version: `0.1.0`.
- SHA-256 / size: `30DD3DE4CA99062D42808BEBD93F1215C7ADC06693500A50400A750F601050E6` / `38590454` bytes; source `tree-sha256:5fb971ae71ffa8d15e26520d5ce2a1478e7018dbe5813c8dbb158e1c5f1ecaa2`.
- Packaging note: portable candidate rebuilt; root/dist copies match; no native launch.

## Disposition

`accepted-with-limits`: source restore evidence is complete; package identity
and release-dossier refresh remain for the next handoff step.

# Handoff: 2026-08-12-d285-startup-preparation

| Field | Value |
|---|---|
| ID | `2026-08-12-d285-startup-preparation` |
| Delivery / slice | `D285 / ARCH-255 Startup preparation preflight` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Shared checkout | `D:\Workplace\Agent_Workplace\QuillForge` |

## User outcome

The no-window startup report now reaches the same plugin activation and command
refresh stages used immediately before the normal window is shown. A failure
there is reportable instead of remaining a silent pre-show exit.

## Scope and boundaries

- Shared `DesktopRuntime` startup stages between normal startup and diagnosis.
- Preserved normal order around session restore and window display.
- Kept session/recovery asynchronous restoration, window display, and explicit
  startup-path opening out of the no-window probe.
- No EXE/Qt launch, native dialog, unit-test asset, installer/updater/registry
  operation, or worktree was used.

## Team roles and ownership

| Role | Owner / agent | Responsibility |
|---|---|---|
| Architect | parent; Hilbert consultation | Lifecycle seam and ordering review |
| Developer | parent | Shared startup stages and diagnostic integration |
| QA | parent | Source, static, compile, package, PE/archive, and identity checks |
| Independent reviewer | Gauss consultation | Requested independent source review; no conclusion returned |

## Changed files and modules

- `src/quillforge/composition.py` — shared startup preparation and command
  refresh stages.
- `src/quillforge/app.py` — no-window probe invokes shared stages.
- `scripts/audit_presentation_contracts.py` — lifecycle ordering and boundary
  contract.
- `README.md` — diagnostic scope guidance.
- D285 ADR, review, plan, roadmap, register, acceptance, and handoff records.

## Decisions and constraints

- Do not start session/recovery asynchronous work in the no-window probe.
- Do not call `show()` or `QApplication.exec()` from the probe.
- Shared checkout writer: parent only; no worktree or parallel writer was used.
- Runtime launch policy: EXE/Qt startup was not allowed; only non-destructive
  source, archive, package, and static evidence was authorized.

## Public-source applicability

Python 3.12 language semantics and public Qt 6 QApplication lifecycle
documentation are applicable references; see ADR-0321. No manufacturer
requirement applies. Embedded workflow and simplifier: `N/A`.

## Verification commands and results

| Evidence | Result |
|---|---|
| Startup-preparation source diagnostic | `D285-SOURCE-DIAGNOSTIC=PASS exit=0 runtime_composition=passed startup_prepared=true window_shown=false event_loop_entered=false` |
| Lifecycle contract | `D285-STARTUP-PREPARATION-CONTRACT=PASS order_preserved=1 no_window=1` |
| Presentation audit | `D285-PRESENTATION-AUDIT=PASS` |
| Compileall | `D285-COMPILEALL=PASS` |
| Ruff/formatting | `D285-RUFF=PASS`; `D285-FORMAT=PASS` |
| Project checks | `D285-CHECK=PASS` |
| PE/archive | `D285-PE-HEADER=PASS machine=AMD64 optional=PE32+ subsystem=WINDOWS_GUI`; `D285-PE-ARCHIVE=PASS required=9 qwindows=present qscintilla=present app_modules=present` |
| Root/dist identity | `D285-MANIFEST-COPY=PASS bytes=38587106 sha=E8D58A883DE40E092F294CEE9DCD379056CD012DCC3CF36CA838F4306578A446` |
| Source revision | `tree-sha256:ecc42fd2fc1718d5847019ce0681d7b5e2a76fe246c963456f9bdc98598b8604` |
| Parent review | `PASS` |
| Independent review | `NO_CONCLUSION` after bounded waits and closure |
| Simplification | `PASS` |

## Unrun checks and reason

Native EXE/Qt startup, asynchronous session/recovery completion, clean-machine
behavior, native window lifetime, cross-machine repeatability, signing,
installer/update, registry, and release-owner acceptance remain unrun under
the active no-launch/non-destructive policy. Release verification is expected
to remain `NO-GO` with historical artifact-bound reports and enterprise gates
open.

## Known risks and limits

- Preparation success does not prove native platform-plugin loadability,
  rendering, window lifetime, DPI, accessibility, or cross-machine behavior.
- Plugin activation failures are isolated by the existing PluginManager
  policy; this probe verifies the host path, not successful behavior of every
  plugin implementation.

## Acceptance and evidence IDs

- Acceptance: `S325`.
- Architecture slice: `ARCH-255`.
- Evidence: `D285-STARTUP-PREPARATION-CONTRACT=PASS`,
  `D285-SOURCE-DIAGNOSTIC=PASS`, `D285-CHECK=PASS`,
  `D285-MANIFEST-IDENTITY=PASS`,
  `D285-ARCHITECTURE-CONSULTATION=NO_CONCLUSION`,
  `D285-INDEPENDENT-REVIEW=NO_CONCLUSION`, and
  `D285-SIMPLIFICATION-ASSESSMENT=PASS`.

## Next owner and next action

- Owner: user / authorized QA.
- Action: if the candidate still shows no window, run the documented
  `--diagnose-startup --report` command and attach the D285 report plus a
  fresh `%LOCALAPPDATA%\QuillForge\startup-error.log` only if the current
  attempt recreates it.

## Artifact information

- Artifact paths: `dist/QuillForge.exe` and `QuillForge.exe`.
- SHA-256 / size:
  `E8D58A883DE40E092F294CEE9DCD379056CD012DCC3CF36CA838F4306578A446` /
  `38,587,106` bytes.
- Source revision:
  `tree-sha256:ecc42fd2fc1718d5847019ce0681d7b5e2a76fe246c963456f9bdc98598b8604`.

## Disposition

`accepted-with-limits`: pre-show plugin and command preparation is now
diagnosable through the shared lifecycle, while native startup and enterprise
release gates remain open.

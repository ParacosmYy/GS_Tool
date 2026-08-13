# Handoff: 2026-08-12-d286-editor-shell-preflight

| Field | Value |
|---|---|
| ID | `2026-08-12-d286-editor-shell-preflight` |
| Delivery / slice | `D286 / ARCH-256 Editor-shell startup preflight` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Shared checkout | `D:\Workplace\Agent_Workplace\QuillForge` |

## User outcome

The no-window startup report now constructs the first empty editor tab through
the production QScintilla/editor/theme/font/tab path. Failures in that path
are reportable before the native window is shown.

## Scope and boundaries

- Reused `ensure_initial_document()` rather than duplicating editor assembly.
- Stopped recovery and session-save timers after the preflight, including on
  construction failure.
- Did not restore session state, open files, show a window, enter the Qt event
  loop, or write a document.
- No EXE/Qt launch, native dialog, unit-test asset, installer/updater/registry
  operation, or worktree was used.

## Team roles and ownership

| Role | Owner / agent | Responsibility |
|---|---|---|
| Architect | parent; Averroes consultation | Editor-shell boundary and side-effect review |
| Developer | parent | Preflight lifecycle implementation |
| QA | parent | Source, static, compile, package, PE/archive, and identity checks |
| Independent reviewer | Sartre consultation | Requested independent source review; no conclusion returned |

## Changed files and modules

- `src/quillforge/presentation/main_window.py` — editor-shell preflight and
  timer cleanup.
- `src/quillforge/composition.py` — runtime editor-shell preflight seam.
- `src/quillforge/app.py` — no-window probe integration.
- `scripts/audit_presentation_contracts.py` — construction and cleanup guard.
- `README.md` — diagnostic scope guidance.
- D286 ADR, review, plan, roadmap, register, acceptance, and handoff records.

## Decisions and constraints

- Keep initial-document policy in normal session/recovery restore; the probe is
  diagnostic-only.
- Keep timer cleanup in `MainWindow`, the owner of those QTimers.
- Shared checkout writer: parent only; no worktree or parallel writer was used.
- Runtime launch policy: EXE/Qt startup was not allowed; only non-destructive
  source, archive, package, and static evidence was authorized.

## Public-source applicability

Python 3.12 language semantics and public Qt 6 QTimer/QApplication lifecycle
documentation are applicable references; see ADR-0322. No manufacturer
requirement applies. Embedded workflow and simplifier: `N/A`.

## Verification commands and results

| Evidence | Result |
|---|---|
| Editor-shell source diagnostic | `D286-SOURCE-DIAGNOSTIC=PASS exit=0 runtime_composition=passed startup_prepared=true editor_shell_prepared=true window_shown=false event_loop_entered=false` |
| Editor-shell contract | `D286-EDITOR-SHELL-CONTRACT=PASS timer_cleanup=1 no_window=1` |
| Presentation audit | `D286-PRESENTATION-AUDIT=PASS` |
| Compileall | `D286-COMPILEALL=PASS` |
| Ruff/formatting | `D286-RUFF=PASS`; `D286-FORMAT=PASS` |
| Project checks | `D286-CHECK=PASS` |
| PE/archive | `D286-PE-HEADER=PASS machine=AMD64 optional=PE32+ subsystem=WINDOWS_GUI`; `D286-PE-ARCHIVE=PASS required=9 qwindows=present qscintilla=present app_modules=present` |
| Root/dist identity | `D286-MANIFEST-COPY=PASS bytes=38585469 sha=24FE0400F08C2C5E10CCBE9CF8A2DF5A92E433A3A19E89CDCB28E73D6C8C875F` |
| Source revision | `tree-sha256:a687a4230d7c39825224e1042f85db553ebb89931ba672aacb6160228c99b056` |
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

- Editor-shell construction success does not prove native platform-plugin
  loadability, rendering, DPI, accessibility, or cross-machine behavior.
- The probe creates only an empty untitled editor and does not validate every
  language lexer or document-open decoding path.

## Acceptance and evidence IDs

- Acceptance: `S326`.
- Architecture slice: `ARCH-256`.
- Evidence: `D286-EDITOR-SHELL-CONTRACT=PASS`,
  `D286-SOURCE-DIAGNOSTIC=PASS`, `D286-CHECK=PASS`,
  `D286-MANIFEST-IDENTITY=PASS`,
  `D286-ARCHITECTURE-CONSULTATION=NO_CONCLUSION`,
  `D286-INDEPENDENT-REVIEW=NO_CONCLUSION`, and
  `D286-SIMPLIFICATION-ASSESSMENT=PASS`.

## Next owner and next action

- Owner: user / authorized QA.
- Action: if the candidate still shows no window, run the documented
  `--diagnose-startup --report` command and attach the D286 report plus a
  fresh `%LOCALAPPDATA%\QuillForge\startup-error.log` only if the current
  attempt recreates it.

## Artifact information

- Artifact paths: `dist/QuillForge.exe` and `QuillForge.exe`.
- SHA-256 / size:
  `24FE0400F08C2C5E10CCBE9CF8A2DF5A92E433A3A19E89CDCB28E73D6C8C875F` /
  `38,585,469` bytes.
- Source revision:
  `tree-sha256:a687a4230d7c39825224e1042f85db553ebb89931ba672aacb6160228c99b056`.

## Disposition

`accepted-with-limits`: first editor-shell construction is now included in the
no-window diagnostic, while native startup and enterprise release gates remain
open.

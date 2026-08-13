# Handoff: 2026-08-12-d284-startup-runtime-composition

| Field | Value |
|---|---|
| ID | `2026-08-12-d284-startup-runtime-composition` |
| Delivery / slice | `D284 / ARCH-254 Startup runtime-composition preflight` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Shared checkout | `D:\Workplace\Agent_Workplace\QuillForge` |

## User outcome

The no-window startup report now checks the complete desktop composition
constructor. If `MainWindow` or a presentation adapter fails before the native
window is shown, the report can identify that failure instead of stopping at
module-import success.

## Scope and boundaries

- Reused the existing `build_desktop_runtime()` composition root.
- Created and released a temporary `QApplication` only inside the diagnostic.
- Did not start plugins, restore session state, show a window, or enter the Qt
  event loop.
- Added a static contract for probe order, cleanup, and the no-window boundary.
- No EXE/Qt launch, native dialog, unit-test asset, installer/updater/registry
  operation, or worktree was used.

## Team roles and ownership

| Role | Owner / agent | Responsibility |
|---|---|---|
| Architect | parent; Chandrasekhar consultation | Composition seam and diagnostic scope |
| Developer | parent | Runtime-composition probe and contract |
| QA | parent | Source, static, compile, package, PE/archive, and identity checks |
| Independent reviewer | Halley consultation | Requested independent source review; no conclusion returned |

## Changed files and modules

- `src/quillforge/app.py` — no-window runtime-composition probe.
- `scripts/audit_presentation_contracts.py` — probe order, cleanup, and
  no-window static contract.
- `README.md` — diagnostic behavior and boundary guidance.
- D284 ADR, review, plan, roadmap, register, acceptance, and handoff records.

## Decisions and constraints

- Reuse `build_desktop_runtime()` as the only composition owner.
- Do not call `DesktopRuntime.start()`, `window.show()`, or
  `QApplication.exec()` from the probe.
- Shared checkout writer: parent only; no worktree or parallel writer was used.
- Runtime launch policy: EXE/Qt startup was not allowed; source, archive, and
  package evidence were the authorized non-destructive checks.

## Public-source applicability

Python 3.12 `try`/`finally` semantics and public Qt 6 `QApplication` lifecycle
documentation are applicable references; see ADR-0320. No manufacturer
requirement applies. Embedded workflow and simplifier: `N/A`.

## Verification commands and results

| Evidence | Result |
|---|---|
| Runtime composition source diagnostic | `D284-SOURCE-DIAGNOSTIC=PASS exit=0 runtime_composition=passed window_shown=false event_loop_entered=false` |
| Runtime composition contract | `D284-RUNTIME-COMPOSITION-CONTRACT=PASS` |
| Presentation audit | `D284-PRESENTATION-AUDIT=PASS` |
| Compileall | `D284-COMPILEALL=PASS` |
| Ruff/formatting | `D284-RUFF=PASS`; `D284-FORMAT=PASS` |
| Project checks | `D284-CHECK=PASS` |
| PE/archive | `D284-PE-HEADER=PASS machine=AMD64 optional=PE32+ subsystem=WINDOWS_GUI`; `D284-PE-ARCHIVE=PASS qwindows=present qscintilla=present app_modules=present` |
| Root/dist identity | `D284-MANIFEST-COPY=PASS bytes=38585720 sha=C60A7112306A29BC400475FD3D6E5A6A209DEA206E681C1BE20064C033FD7749` |
| Source revision | `tree-sha256:62890a432c0be8610c40c428664854c2d52b60ff216839db5edf08e5d3e98283` |
| Parent review | `PASS` |
| Independent review | `NO_CONCLUSION` after bounded waits and closure |
| Simplification | `PASS` |

## Unrun checks and reason

Native EXE/Qt startup, clean-machine behavior, native window lifetime,
cross-machine repeatability, signing, installer/update, registry, and
release-owner acceptance remain unrun under the active no-launch and
non-destructive policy. The release verifier is expected to remain `NO-GO`
with historical artifact-bound reports and enterprise gates open.

## Known risks and limits

- A passing construction probe does not prove native platform-plugin startup,
  window lifetime, rendering, DPI, or cross-machine behavior.
- The probe intentionally does not restore session state or activate plugins;
  those paths remain outside this diagnostic slice.

## Acceptance and evidence IDs

- Acceptance: `S324`.
- Architecture slice: `ARCH-254`.
- Evidence: `D284-RUNTIME-COMPOSITION-CONTRACT=PASS`,
  `D284-SOURCE-DIAGNOSTIC=PASS`, `D284-CHECK=PASS`,
  `D284-MANIFEST-IDENTITY=PASS`,
  `D284-ARCHITECTURE-CONSULTATION=NO_CONCLUSION`,
  `D284-INDEPENDENT-REVIEW=NO_CONCLUSION`, and
  `D284-SIMPLIFICATION-ASSESSMENT=PASS`.

## Next owner and next action

- Owner: user / authorized QA.
- Action: if the candidate still shows no window, run the documented
  `--diagnose-startup --report` command and attach the D284 report plus a fresh
  `%LOCALAPPDATA%\QuillForge\startup-error.log` only if the current attempt
  recreates it.

## Artifact information

- Artifact paths: `dist/QuillForge.exe` and `QuillForge.exe`.
- SHA-256 / size:
  `C60A7112306A29BC400475FD3D6E5A6A209DEA206E681C1BE20064C033FD7749` /
  `38,585,720` bytes.
- Source revision:
  `tree-sha256:62890a432c0be8610c40c428664854c2d52b60ff216839db5edf08e5d3e98283`.

## Disposition

`accepted-with-limits`: the no-window diagnostic now exercises full runtime
composition, while native startup and enterprise release gates remain open.

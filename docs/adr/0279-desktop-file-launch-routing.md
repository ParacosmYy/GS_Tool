# ADR-0279: Route explicit desktop launch paths into the existing open boundary

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D233 / ARCH-215
- Scope: `src/quillforge/application/desktop_launch.py`, `src/quillforge/app.py`, `src/quillforge/composition.py`, `src/quillforge/presentation/main_window.py`

## Context

The windowed entry point passed the complete process argument list to
`QApplication`, but no application-owned component interpreted a bare file
path. Launching `QuillForge.exe <file>` or dropping a file onto the executable
therefore did not reach the existing asynchronous document-open admission
boundary. A folder path had no explicit startup routing either.

## Decision

Add a Qt-free `DesktopLaunchRequest` parser that keeps Qt options separate from
explicit file or directory paths. `app.main` constructs `QApplication` only
with the retained Qt arguments and passes the ordered, de-duplicated paths to
the composition root. `DesktopRuntime` forwards those paths to the public
`MainWindow.open_startup_paths()` lifecycle seam after the window is shown.

`MainWindow` queues the paths behind recovery and session restoration, then
opens one path at a time through the existing
`DocumentOpenAdmissionCoordinator` or
`WorkspaceNavigationAdmissionCoordinator`. Completion schedules the next path
after the current projection returns. Missing paths are reported as warnings;
directories use the existing workspace-open boundary. This supports command
line launch and Windows drag-and-drop without adding a second document service,
workspace service, or OS-association registry owner.

## Boundary and non-goals

- The parser is framework-neutral and owns only argument classification,
  normalization, order, and duplicate suppression.
- `MainWindow` owns startup ordering and presentation notifications; it does
  not parse command-line syntax.
- Existing asynchronous services, operation tracking, session persistence,
  and recovery policy remain unchanged.
- The portable candidate still does not configure Windows file associations;
  installer/association policy remains a separate release decision.

## Review and simplification

Parent review and simplification are `PASS`. The bounded architecture role
(`Pascal the 7th / Luna max`) and independent role (`Descartes the 7th / Luna
max`) returned `NO_CONCLUSION` when their windows closed. The separate pure
parser is justified because it prevents Qt/application argument ownership from
leaking into the presentation shell; no further abstraction or line-count
reduction is warranted.

## Public-source applicability

Python 3.12 public `pathlib`, `collections.abc.Sequence`, and process-argument
contracts are applicable engineering references. PyInstaller public one-file
entry behavior is the packaging reference; no dependency or vendor SDK changed.
Public CloudWeGo material is an engineering reference only; no private
ByteDance standard, certification, or compliance claim is made. Embedded
C/C++, MCU, RTOS, and manufacturer requirements are not applicable.

## Limits

No `QApplication`, GUI, native EXE, drag-and-drop shell, file-association,
clean-machine, or cross-machine launch was performed under the project
no-launch policy. Source, parser, package-archive, and package-identity
evidence do not claim native rendering or enterprise-release readiness.

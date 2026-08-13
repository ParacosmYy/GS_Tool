# D233 parent review — desktop file-launch routing

## Decision

`PASS` with limits.

## Findings

- `DesktopLaunchRequest` keeps bare paths out of `QApplication` while retaining
  Qt value arguments such as `-platform windows`.
- `app.main` remains the diagnostic/desktop dispatcher and now hands only the
  explicit startup-path tuple to the composition root.
- `DesktopRuntime.start()` preserves plugin activation, startup restore,
  menu refresh, and show order, then uses the public
  `MainWindow.open_startup_paths()` seam.
- `MainWindow` queues paths behind recovery/session restore and reuses the
  existing document and workspace admission coordinators. It does not call a
  service directly from the entry point or add a second open implementation.
- Ordered paths are drained one at a time; invalid paths warn and continue,
  while async completion schedules the next path after the current projection.

## Evidence

- `D233-DESKTOP-LAUNCH-PARSER-PROBE=PASS`
- `D233-STARTUP-WIRING-SOURCE-PROBE=PASS`
- `D233-STARTUP-DIAGNOSTIC-SOURCE-PROBE=PASS`
- `D233-COMPILEALL=PASS`
- `D233-RUFF=PASS`
- `D233-PRESENTATION-AUDIT=PASS`
- `D233-PACKAGE-ARCHIVE-PROBE=PASS entries=166 pyz=PYZ.pyz`
- `D233-PYZ-MODULE-PROBE=PASS modules=261`
- `D233-PACKAGE-IDENTITY-PROBE=PASS hash=4DBA48F93ABB527BF83061F68A7A92A2CCB4646CA4FBD4D9CF76314848FB11A5 bytes=38573337`
- `D233-PYINSTALLER-WARNING-SCOPE-PROBE=PASS`

## Simplification assessment

`D233-SIMPLIFICATION-ASSESSMENT=PASS`: the parser is the single Qt-free
argument-classification owner; composition only transfers immutable startup
paths; MainWindow only owns lifecycle ordering and existing admissions. Merging
these responsibilities or bypassing the established coordinators would reduce
traceability and increase coupling, so no further simplification was made.

## Public-source applicability

Python 3.12 standard-library argument/path behavior and PyInstaller one-file
entry packaging are engineering references. No embedded target, vendor SDK,
manufacturer requirement, or certification claim applies.

## Limits

The parent did not launch the EXE, construct `QApplication`, exercise native
drag-and-drop, or validate Windows registry associations. Those runtime and
release gates remain user-owned or open.

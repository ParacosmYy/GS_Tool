# ADR-0285: Frozen startup provenance diagnostic

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D241 / ARCH-222

## Decision

Extend only the existing no-window `--diagnose-startup --report` path. Record
the `quillforge.app` module/package/origin/loader provenance and, for frozen
execution, the selected Qt plugin root, Windows platform plugin presence, and
whether `QT_PLUGIN_PATH` is configured. Reuse the helper-selected platform
plugin path for the existing `qwindows_platform_plugin` check.

The diagnostic does not construct `QApplication`, mutate the environment, or
change the normal desktop startup path. It makes a frozen import/resource
failure observable without treating a diagnostic report as proof of native
GUI startup.

## Public-source applicability

The applicable public engineering reference is PyInstaller's first-party
Run-time Information documentation, which documents frozen detection and
bundle/module path semantics:

- PyInstaller, *Run-time Information*, stable documentation, accessed
  2026-08-12: <https://pyinstaller.org/en/stable/runtime-information.html>

The installed PyInstaller 6.22.0 `pyi_rth_pyqt6.py` runtime hook was inspected
as a tool implementation reference for `QT_PLUGIN_PATH`; it is not a
manufacturer requirement. No private ByteDance standard, certification,
MISRA, ISO 26262, ASPICE, or compliance claim is made. Embedded C/C++, MCU,
BSP/HAL, RTOS, and manufacturer requirements are not applicable.

## Evidence and limits

- `D241-AST=PASS`; `D241-COMPILEALL=PASS`; `D241-RUFF=PASS`;
  `D241-FORMAT=PASS`
- `D241-SOURCE-STARTUP-DIAGNOSTIC=PASS`
- `D241-DESKTOP-LAUNCH-PARSER=PASS`
- `D241-FROZEN-ARCHIVE-ENTRYPOINT=PASS`
- `D241-PACKAGE-IDENTITY=PASS`: root/dist match, 38,575,419 bytes
- Final independent review: `PASS`; parent review and simplification:
  `PASS`.
- Native EXE/Qt startup, frozen diagnostic execution, and clean-machine
  behavior remain unrun under the permanent no-launch policy.

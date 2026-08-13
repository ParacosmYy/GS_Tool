# ADR-0284: Frozen entrypoint path hardening

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D240 / ARCH-221

## Decision

Keep `src/quillforge/__main__.py` compatible with direct source-file
diagnostics, but do not apply source-style `__file__` parent insertion when
the process is frozen. When `__package__` is empty, the entrypoint now checks
`getattr(sys, "frozen", False)` before the existing `sys.path.insert`; the
absolute `from quillforge.app import main` import and public `main(argv)`
contract remain unchanged.

This keeps direct `python src/quillforge/__main__.py` useful while allowing the
PyInstaller importer to resolve the packaged `quillforge` package without
path surgery. No new entrypoint, importer, service, or packaging abstraction
is introduced.

## Public-source applicability

The applicable public engineering reference is PyInstaller's versioned
Run-time Information documentation, which documents `sys.frozen` as the
bundle indicator and the bundled-module `__file__` behavior:

- PyInstaller, *Run-time Information*, stable documentation, accessed
  2026-08-12: <https://pyinstaller.org/en/stable/runtime-information.html>

The installed build tool is PyInstaller 6.22.0. The PyInstaller document is
an engineering reference for the packaging tool, not a manufacturer
requirement. No private ByteDance standard, certification, MISRA, ISO 26262,
ASPICE, or compliance claim is made. Embedded C/C++, MCU, BSP/HAL, RTOS, and
manufacturer requirements are not applicable.

## Evidence and limits

- `D240-FROZEN-ENTRYPOINT-STATIC-CONTRACT=PASS`
- `D240-COMPILEALL=PASS`; `D240-RUFF=PASS`; `D240-FORMAT=PASS`
- `D240-FROZEN-ARCHIVE-ESSENTIALS=PASS`
- `D240-PACKAGE-IDENTITY=PASS`: root/dist match, 38,573,963 bytes
- Parent review and simplification assessment are `PASS`.
- Mendel the 7th and Galileo the 7th architecture windows returned
  `NO_CONCLUSION`; James the 7th independent Luna/max window returned
  `NO_CONCLUSION`. No child PASS is claimed.
- Native EXE/Qt startup, clean-machine startup, and interactive file/folder
  routing remain unrun under the permanent no-launch policy.

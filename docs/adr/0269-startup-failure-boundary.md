# ADR-0269: Windowed startup-failure boundary

- Status: accepted-with-limits
- Date: 2026-08-11
- Delivery: D222 / ARCH-205

## Context

The portable PyInstaller candidate is a windowed application (`console=False`).
When an exception occurs before the Qt shell is shown, Windows provides no
useful console output and the user experiences the failure as an EXE that does
not open. Existing startup evidence is intentionally stale under the active
no-launch policy, so the entry point needs a safe diagnostic boundary before
the next authorized runtime run.

## Decision

Keep startup-failure handling in `quillforge.__main__`, the executable entry
boundary. Catch ordinary startup exceptions around `main()`, write a bounded
human-readable traceback to `%LOCALAPPDATA%/QuillForge/startup-error.log` (or
the existing non-Windows local-data fallback), and show a Windows-native error
message when Qt may not exist yet. Return exit code 1 after reporting.

## Boundaries

1. No application, domain, infrastructure, presentation, settings, session,
   theme, or plugin behavior changes.
2. The handler does not swallow the exception silently; it preserves the
   original exception as the cause of the final exit and treats log/message
   reporting as best-effort.
3. The log is diagnostic output only; it is not a release-startup pass or a
   substitute for authorized GUI/EXE evidence.

## Public-source applicability and review

This is Python 3.12 entry-point code. Python's public
[built-in exceptions documentation](https://docs.python.org/3.12/library/exceptions.html)
and standard-library `pathlib`, `traceback`, `datetime`, and `ctypes` APIs are
the applicable first-party references. Public CloudWeGo material remains an
engineering reference only; no private ByteDance standard, certification, or
compliance claim is made. Embedded C/C++, MCU, RTOS, and manufacturer
requirements are not applicable.

The architecture role `Russell the 6th / Luna max` returned `NO_CONCLUSION`
after bounded waits and closure. The independent role `Hooke the 6th / Luna
max` returned `NO_CONCLUSION` after bounded waits and closure. Parent review is
`PASS`; behavior-preserving simplification is `PASS`.

## Verification and limits

- `D222-STARTUP-REPORT-PROBE=PASS bytes=218`.
- `D222-COMPILEALL=PASS`, `D222-RUFF=PASS`, `D222-FORMAT=PASS`, and
  `D222-PRESENTATION-AUDIT=PASS`.
- `D222-PACKAGE-BUILD-PS51=PASS` and `D222-PACKAGE-BUILD-PS7=PASS`.
- `D222-PACKAGE-IDENTITY-PROBE=PASS` with root/dist SHA
  `158178BF032BA94423D386380465B5B96E037EB38B1B4D80576FF1F4A044F6C8`,
  38,565,695 bytes, and source revision
  `tree-sha256:0b790bfbc52c3288ff300b2179a2b4ae4558219b9125b975003cf4de164e169a`.
- The PyInstaller recursive archive contains the entry point, QuillForge
  modules, QScintilla, `qwindows.dll`, and the authored icon.

No GUI/QApplication, EXE launch, native dialog rendering, screenshot,
accessibility, clean-machine, cross-machine, signing, installer, updater,
legal, support, permission/disk-pressure, or release-owner evidence was run.
No unit-test asset was created or run.

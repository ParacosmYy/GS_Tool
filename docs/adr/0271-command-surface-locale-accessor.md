# ADR-0271: Command-surface locale accessor startup fix

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D225 / ARCH-207

## Context

The exact packaged candidate's startup log reported:
`AttributeError: 'CommandSurface' object has no attribute '_locale'`.
The failure occurred during `MainWindow._create_menus()` while
`CommandSurface.create_menus()` attempted to resolve menu titles. The class
already receives and stores the composition-owned `locale_provider`, but the
projection methods referenced a missing private accessor, so the windowed EXE
terminated before showing its shell.

## Decision

Add one private `_locale() -> Locale` accessor to
`src/quillforge/presentation/command_surface.py`. It delegates directly to the
existing `_locale_provider` callable. Existing calls in menu creation, toolbar
creation, menu refresh, and retranslation now resolve through a defined method;
command registration, menu order, toolbar actions, callbacks, locale catalog,
and application ownership remain unchanged.

## Boundaries

1. The fix is limited to the presentation projection's missing method; no
   fallback locale, duplicated catalog, or new service is introduced.
2. The provider remains composition-owned and is evaluated at the existing
   call sites, preserving dynamic locale refresh behavior.
3. The actual GUI/EXE launch remains user-owned under the permanent project
   no-launch boundary; source diagnostics and packaged archive evidence do not
   claim native startup success.

## Public-source applicability and review

This is Python 3.12/PyQt6 desktop presentation code. Python's public
[`typing.Callable`](https://docs.python.org/3.12/library/typing.html#typing.Callable)
and Qt's public [QMainWindow](https://doc.qt.io/qt-6/qmainwindow.html) APIs
are the applicable first-party references; no dependency was added. Public
CloudWeGo material remains an engineering reference only, and no private
ByteDance standard, certification, or compliance claim is made. Embedded
C/C++, MCU, RTOS, and manufacturer requirements are not applicable.

The architecture role `Volta the 6th / Luna max` returned `NO_CONCLUSION`
after bounded waits and closure. The independent role `Lorentz the 6th /
Luna max` returned `NO_CONCLUSION` after bounded waits and closure. Parent
review is `PASS`; behavior-preserving simplification is `PASS`.

## Verification and limits

- `D225-STARTUP-LOG-ROOT-CAUSE=PASS`: the user-local log identified the
  missing `_locale` method at the first shell construction failure.
- `D225-LOCALE-STARTUP-FIX-PROBE=PASS`, `D225-COMMAND-SURFACE-IMPORT=PASS`,
  and `D225-COMPILEALL=PASS`.
- `D225-SOURCE-DIAGNOSTIC-PROBE=PASS exit=0 failed=0`.
- `D225-PACKAGE-BUILD-PS51=PASS` and `D225-PACKAGE-BUILD-PS7=PASS`.
- `D225-PACKAGE-ARCHIVE-PROBE=PASS` and
  `D225-PACKAGE-IDENTITY-PROBE=PASS`.
- Final PS7 candidate: SHA-256
  `1B4536194F1D161DB33AA8604FA834412DAF57E4598748A09CB02842634EEDE8`,
  38,569,401 bytes, source revision
  `tree-sha256:d7c354212b31f99b1c0b263464bbd325a61546107edb31924aae6c904e688fe6`.
- `D225-CHECK-PS51/PS7=PASS` and `D225-HANDOFF-PS51/PS7=PASS`.

No GUI/QApplication or EXE launch, native menu rendering, accessibility,
clean-machine, cross-machine, signing, installer, updater, legal, support,
permission/disk-pressure, hard-power, or release-owner evidence was run.
No unit-test asset was created or run.

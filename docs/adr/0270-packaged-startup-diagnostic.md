# ADR-0270: No-window packaged startup diagnostic contract

- Status: accepted-with-limits
- Date: 2026-08-11
- Delivery: D223 / ARCH-206

## Context

The portable candidate is a windowed PyInstaller application. D222 makes an
unexpected early exception visible, but a user still needs a deterministic
way to distinguish a missing runtime dependency, a missing Qt plugin, a
missing authored asset, and a composition import failure without opening the
normal editor window. The current no-launch policy also requires that this
diagnostic remain safe to run as a no-window probe.

## Decision

Add the `--diagnose-startup --report <path>` branch to the existing
`quillforge.app.main()` dispatcher before `QApplication` construction. The
branch writes a versioned JSON report and returns `0` only when all applicable
checks pass; malformed arguments, report-write failures, and failed probes
return `2`.

The report records source-versus-packaged execution, executable identity for a
frozen candidate, Python and Qt versions, QScintilla importability,
composition importability, and the frozen Qt Windows platform plugin and
authored application icon. Source execution marks frozen-only checks as
`not_applicable` and verifies the source icon path. The normal GUI path and
its existing `QApplication`/runtime lifecycle remain unchanged.

## Boundaries

1. The diagnostic imports Qt modules and composition code but never constructs
   a `QApplication`, opens a window, invokes a native file dialog, or enters
   the Qt event loop.
2. The report is diagnostic evidence, not a claim that the normal GUI startup
   or native rendering works on every machine.
3. No application, domain, infrastructure, presentation, settings, session,
   theme, plugin policy, or PyInstaller packaging shape is moved across
   layers.
4. The report path is caller-selected; existing file permissions and storage
   policy remain outside this small preflight contract.

## Public-source applicability and review

This is Python 3.12 and PyQt6 desktop entry-point code. The applicable public
first-party references are Python's [`argparse`](https://docs.python.org/3.12/library/argparse.html),
[`importlib`](https://docs.python.org/3.12/library/importlib.html),
[`json`](https://docs.python.org/3.12/library/json.html), and
[`pathlib`](https://docs.python.org/3.12/library/pathlib.html) documentation,
Qt's [`QApplication`](https://doc.qt.io/qt-6/qapplication.html) lifecycle
documentation, and PyInstaller's [runtime information](https://pyinstaller.org/en/stable/runtime-information.html)
documentation. Public CloudWeGo material remains an engineering reference
only; no private ByteDance standard, certification, or compliance claim is
made. Embedded C/C++, MCU, RTOS, and manufacturer requirements are not
applicable.

The architecture role `Peirce the 6th / Luna max` returned `NO_CONCLUSION`
after bounded waits and closure. The independent role `Meitner the 6th /
Luna max` returned `NO_CONCLUSION` after bounded waits and closure. Parent
review is `PASS`; behavior-preserving simplification is `PASS`.

## Verification and limits

- `D223-STATIC-EXIT=0`: Ruff format check, Ruff check, and targeted compile
  all passed.
- `D223-SOURCE-DIAGNOSTIC-PROBE=PASS exit=0 failed=0`.
- `D223-PACKAGE-BUILD-PS51=PASS` and `D223-PACKAGE-BUILD-PS7=PASS`.
- `D223-PACKAGE-ARCHIVE-PROBE=PASS`: the final archive contains the entry
  point, QuillForge modules, QScintilla, `qwindows.dll`, and
  `assets\quillforge.ico`.
- `D224-RUNTIME-ARCHIVE-COVERAGE-PROBE=PASS`: all 136 runtime QuillForge
  modules and required frozen assets are present; the top-level `__main__`
  name is the intentional PyInstaller mapping, and the release-manifest store
  is packaging-script-only.
- `D223-RECORD-JSON-PROBE=PASS`, `D223-CHECK-PS51=PASS`,
  `D223-CHECK-PS7=PASS`, and `D223-VERIFY-HANDOFF-PS51/PS7=PASS`.
- `D223-RELEASE-VERIFY-PS51/PS7=EXIT=1` with the expected three open
  artifact-bound report consistency failures; the release decision remains
  `no-go`.
- Final PS7 candidate identity is
  `02A47AF05D54F19816C0A8DDD96FF8492484A5A8D8667638C8281193D63AA50C`,
  38,566,961 bytes, source revision
  `tree-sha256:68028edd86daca972d87b75262a1e854455f5b64cb3751b685efeba764cdb01a`.

No GUI/QApplication, EXE launch, native dialog, screenshot, accessibility,
clean-machine, cross-machine, signing, installer, updater, legal, support,
permission/disk-pressure, hard-power, or release-owner evidence was run. No
unit-test asset was created or run.

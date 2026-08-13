# ADR-0298: Frozen-startup runtime dependency diagnostic

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D254 / ARCH-232

## Context

The no-window `--diagnose-startup` path already checked Python, QtCore,
QScintilla importability, application/composition imports, the application icon,
and the Windows platform plugin. It did not list the core frozen binaries that
must exist before a normal `QApplication` can start, making a portable EXE
failure harder to distinguish from a missing platform plugin.

## Decision

Extend only the existing frozen diagnostic branch with a
`qt_runtime_dependencies` result containing explicit presence checks for:

- `PyQt6/Qt6/bin/Qt6Core.dll`;
- `PyQt6/Qt6/bin/Qt6Gui.dll`;
- `PyQt6/Qt6/bin/Qt6Widgets.dll`; and
- `PyQt6/Qsci.pyd`.

The result reports `passed` or `failed` with a stable required list and missing
relative paths. Source execution reports `not_applicable`. Normal
`QApplication` construction, Qt plugin environment handling, runtime
composition, diagnostic exit codes, and package contents are otherwise
unchanged.

## Boundaries and alternatives

The helper is diagnostic-only and does not load DLLs, mutate `PATH`, set a Qt
plugin path, launch a process, or attempt recovery. A runtime repair pass would
hide deployment defects and make startup behavior environment-dependent. A
single aggregate boolean would be less actionable than the missing relative
paths.

## Public-source applicability and review

Python 3.12 first-party [`pathlib.Path.is_file`](https://docs.python.org/3.12/library/pathlib.html#pathlib.Path.is_file)
documentation is applicable to the non-destructive frozen-bundle presence
checks. PyInstaller's first-party
[`one-file bundle documentation`](https://pyinstaller.org/en/stable/operating-mode.html#how-the-one-file-program-works)
is applicable to treating the bundled relative paths as diagnostic evidence,
not as a runtime API contract. No manufacturer contract changed. Public
CloudWeGo/ByteDance material remains an engineering reference only; no private
corporate standard, certification, MISRA, ISO 26262, ASPICE, or embedded
C/C++/MCU/RTOS claim is made.

The architecture role `Bacon the 7th / Luna max` and independent reviewer
`Aristotle the 7th / Luna max` returned `NO_CONCLUSION` after bounded waits and
closure. No child approval is claimed. Parent review is `PASS`; the
behavior-preserving simplification assessment is `PASS`.

## Evidence and limits

- `D254-SOURCE-DIAGNOSTIC-EXIT=0` and
  `D254-DIAGNOSTIC-DEPENDENCIES-STATUS=not_applicable`.
- `D254-COMPILEALL=PASS`, `D254-RUFF=PASS`, and `D254-FORMAT=PASS`.
- PS5.1 and PS7 package builds passed. Final root/dist identity is SHA-256
  `3925F642D38536DAA59420FAF4F1CCA1A9D6A26BA4CD89900EE052AE01C68CFA`,
  38,581,732 bytes, source revision
  `tree-sha256:6f73e761de1f8eac8e1d5575a7c7085cc63e92fbaf435a25e730d4700ee93009`.
- `D254-ARCHIVE-RUNTIME-DEPENDENCIES=PASS required=7`, outer archive 166,
  PYZ 261, and 25 non-empty PyInstaller warning lines.

Native EXE/Qt startup, native dialogs, clean-machine, signing, installer,
updater, and release-owner evidence remain unrun under the active
non-destructive policy. Release remains `no-go` with existing report gates
open.

# ADR-0290: Startup path stat routing

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D246 / ARCH-224

## Context

The explicit desktop-launch queue classified each resolved path by calling
`Path.is_dir()` and then `Path.is_file()`. Python's path predicates
intentionally turn some missing/broken-link `OSError` cases into `False`.
When a queued file disappeared between argument parsing and admission, the
shell therefore reported only the generic `Cannot open path` warning and lost
the underlying path error.

## Decision

Keep startup path ownership in `MainWindow._drain_startup_paths()` and replace
the two predicates with one `Path.stat()` call. Route the returned mode through
the standard-library `S_ISDIR` and `S_ISREG` predicates:

- directories continue through `_start_workspace_open()`;
- regular files, including symlinks to regular files, continue through
  `_start_open()`;
- special files retain the existing generic warning;
- stat failures use the existing `OSError` handler, preserving the requested
  path and exception detail.

No new queue, document service, workspace service, async protocol, or error
surface is introduced.

## Boundaries and alternatives

- The one `stat()` call narrows the classification race; it cannot lock a
  filesystem path against a later deletion or replacement.
- `Path.stat()` follows links, matching the prior `is_dir()`/`is_file()`
  behavior for link targets.
- Special device/FIFO/socket paths remain unsupported and are not opened.
- No EXE/Qt launch, registry, installer, updater, or unit-test asset is used.

## Public-source applicability and review

Python 3.12 public first-party documentation for
[pathlib](https://docs.python.org/3.12/library/pathlib.html) and the
[stat module](https://docs.python.org/3.12/library/stat.html) is the
applicable engineering reference for `Path.stat()` and `S_ISDIR/S_ISREG`.
No Qt API or manufacturer contract changed. Public CloudWeGo/ByteDance
material remains an engineering reference only; no private corporate
standard, certification, MISRA, ISO 26262, ASPICE, or manufacturer
requirement is claimed. Embedded C/C++, MCU, RTOS, and related public-source
workflows are not applicable.

The architecture role `Boole the 7th / Luna max` returned `NO_CONCLUSION`
after a bounded wait and closure. The independent reviewer `Maxwell the 7th /
Luna max` likewise returned `NO_CONCLUSION` after a bounded wait and closure.
No child approval is claimed. Parent review is `PASS`; behavior-preserving
simplification is `PASS`.

## Evidence and limits

- `D246-STARTUP-STAT-ROUTING=PASS` and
  `D246-STARTUP-ERROR-CONTEXT=PASS`.
- `D246-SOURCE-GATE=PASS`, compileall, Ruff, and format checks passed.
- `D246-CHECK-PS51=PASS`, `D246-CHECK-PS7=PASS`,
  `D246-HANDOFF-PS51=PASS`, and `D246-HANDOFF-PS7=PASS`.
- PS5.1 package build passed with intermediate SHA
  `F18088F6A2756382F7D25B2FD40283CBE0D48A4F1956FF53D4F19CDA78323E9A`.
- PS7 package build passed with final SHA
  `9BF18A4ABFAD6D4E27304A0AC9CF50E031A02C3514F4D3AD04FA51AFAF6F23C7`,
  38,576,264 bytes; root and `dist` copies match.
- The frozen archive contains `__main__`, `quillforge.app`,
  `quillforge.composition`, PyQt6, and `qwindows.dll`; the warning file has
  25 non-empty lines and remains within the established packaging scope.
- `D246-RELEASE-VERIFY=EXPECTED-NO-GO` with 10 open gates and the three
  expected artifact-bound report consistency failures;
  `D246-FINAL-DOSSIER-BINDING=PASS` and `D246-FINAL-JSON-PARSE=PASS`.

Native EXE/Qt startup, shell association launch, real filesystem races,
clean-machine behavior, signing, installer, updater, support, and release-
owner evidence remain open under the active no-launch/non-destructive policy.

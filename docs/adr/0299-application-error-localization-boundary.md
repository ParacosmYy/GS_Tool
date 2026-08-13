# ADR-0299: Application-error localization boundary

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D255 / ARCH-233

## Context

The shared presentation localizer already handled stable shell notifications,
typed filesystem failures, and several diagnostic prefixes. Application
validation errors could still reach a Chinese user as raw English, especially
when a coordinator interpolated an exception into a status message such as a
workspace, recovery, plugin, or autosave failure.

## Decision

Extend only `presentation.i18n` with a bounded application-error catalog and
known presentation-wrapper handling. The catalog covers user-visible
application messages such as missing document targets, workspace state/path
failures, invalid search input, and plugin approval failures. Wrapper handling
recursively localizes the stable error detail while preserving document names,
paths, plugin IDs, and unknown diagnostic text.

The English locale returns the original message before any mapping. Internal
invariant and infrastructure-only exception literals are not translated by
guessing their context. Normal application policy, exception types, worker
boundaries, and UI ownership remain unchanged.

## Boundaries and alternatives

The change is presentation-only: it does not rewrite exception constructors,
change business validation, or broaden `NotificationSink` contracts. A global
word-for-word translation pass would risk changing paths, plugin-provided
details, or internal diagnostics. Passing every exception through every
coordinator would increase coupling; a bounded shared localizer keeps the
existing projection seams intact.

## Public-source applicability and review

Python 3.12 first-party [`re` documentation`](https://docs.python.org/3.12/library/re.html)
and [`str.startswith` documentation`](https://docs.python.org/3.12/library/stdtypes.html#str.startswith)
are applicable to the bounded wrapper matching and prefix preservation.
No manufacturer requirement changed. Public CloudWeGo/ByteDance material is
an engineering reference only; no private corporate standard, certification,
MISRA, ISO 26262, ASPICE, or embedded C/C++/MCU/RTOS claim is made.

The architecture role `Zeno the 7th / Luna max` and independent reviewer
`Ptolemy the 7th / Luna max` returned `NO_CONCLUSION` after bounded waits and
closure. No child approval is claimed. Parent review is `PASS`; the
behavior-preserving simplification assessment is `PASS`.

## Evidence and limits

- `D255-LOCALIZATION-BOUNDARY=PASS cases=7` covers direct application errors,
  nested coordinator errors, path details, autosave details, and plugin
  failure details.
- `D255-SOURCE-DIAGNOSTIC-EXIT=0` and
  `D255-DIAGNOSTIC-DEPENDENCIES-STATUS=not_applicable`.
- `D255-COMPILEALL=PASS`, `D255-RUFF=PASS`, and `D255-FORMAT=PASS`.
- PS5.1 and PS7 package builds passed. Final root/dist identity is SHA-256
  `C9BB0C6B8271CA3534DA6DA98C34D87C8CCDB6B96E2BB52D75E88BB077CD63B9`,
  38,581,105 bytes, source revision
  `tree-sha256:30a23d226f08c89b797e3ecb4fbc4dfb57941cd8b8077d9bc78f4c771732d07d`.
- `D255-ARCHIVE-RUNTIME-DEPENDENCIES=PASS required=7`, outer archive 166,
  PYZ 261, and 29 PyInstaller warning-file lines.

Native EXE/Qt startup, native dialogs, clean-machine, signing, installer,
updater, and release-owner evidence remain unrun under the active
non-destructive policy. Release remains `no-go` with existing report gates
open.

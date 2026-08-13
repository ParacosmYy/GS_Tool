# ADR-0300: Built-in document-statistics localization

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D256 / ARCH-234

## Context

The built-in `quillforge.document-stats` plugin used the public notification
port correctly, but its command title and two user-visible notifications were
literal English. As a result, a Simplified Chinese shell still exposed
`Document Statistics`, `No active document`, and the dynamic statistics
message in English.

## Decision

Add the built-in command ID to the existing core command catalog and add the
two stable notification forms to the existing presentation localizer. The
statistics form uses a bounded regular expression so line and character
counters, including comma-grouped counters, remain unchanged. The plugin API
and built-in plugin implementation remain untouched.

Core command IDs continue to use the catalog while unknown external plugin
commands continue to use their supplied fallback titles. The English locale
returns the original notification text before mappings are considered.

## Boundaries and alternatives

The change stays in the presentation translation boundary and does not push
Qt or locale policy into the plugin API. Rewriting the built-in plugin to
construct translated strings would duplicate presentation policy and make the
public notification contract less stable. A broad word replacement would
risk changing unknown plugin-provided diagnostics, so the dynamic mapping is
anchored to the exact built-in message shape.

## Public-source applicability and review

Python 3.12 first-party [`re.fullmatch` documentation](https://docs.python.org/3.12/library/re.html#re.Pattern.fullmatch)
is applicable to the anchored counter mapping. The project’s existing
command-catalog contract is the applicable engineering reference for core
command IDs. No manufacturer requirement changed. Public CloudWeGo/ByteDance
material remains an engineering reference only; no private corporate
standard, certification, MISRA, ISO 26262, ASPICE, or embedded
C/C++/MCU/RTOS claim is made.

The architecture role `Einstein the 7th / Luna max` and independent reviewer
`Lovelace the 7th / Luna max` returned `NO_CONCLUSION` after bounded waits and
closure. No child approval is claimed. Parent review is `PASS`; the
behavior-preserving simplification assessment is `PASS`.

## Evidence and limits

- `D256-BUILTIN-PLUGIN-LOCALIZATION=PASS cases=3` covers the empty-document
  notice, ordinary and comma-grouped counters, core command title, and
  en-US preservation.
- `D256-COMPILEALL=PASS`, `D256-RUFF=PASS`, and `D256-FORMAT=PASS`.
- PS5.1 and PS7 package builds passed. Final root/dist identity is SHA-256
  `8E53E9DF668E6651156AEB243BBDDC173A32A7FCD5D679700469B94471AEA713`,
  38,580,380 bytes, source revision
  `tree-sha256:4ce202e572231e7ccc394fe637fcc95e1c0233ad14d54869eb87d499b928fe56`.
- `D256-ARCHIVE-RUNTIME-DEPENDENCIES=PASS required=7`, outer archive 166,
  PYZ 261, and 29 PyInstaller warning-file lines.

Native EXE/Qt startup, native dialogs, clean-machine, signing, installer,
updater, and release-owner evidence remain unrun under the active
non-destructive policy. Release remains `no-go` with existing report gates
open.

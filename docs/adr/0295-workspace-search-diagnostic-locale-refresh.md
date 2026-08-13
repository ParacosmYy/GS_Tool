# ADR-0295: Workspace-search diagnostic locale refresh

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D251 / ARCH-229

## Context

Workspace-search diagnostics were rendered directly from provider reason
strings. The dialog therefore exposed English reasons such as `file read
failed`, and changing locale did not update already-rendered diagnostic rows.

## Decision

Keep the existing application result contract and fix the projection boundary:

- `WorkspaceSearchDialog` retains the latest immutable
  `WorkspaceSearchResult` only for presentation re-projection;
- diagnostic rows pass their reason through the shared `localize_message()`
  mapper while preserving the relative path and the untranslated detail
  suffix;
- `set_locale()` updates existing diagnostic text and tooltips without
  rebuilding the list or changing its expanded/collapsed state;
- the presentation catalog translates only the provider's stable diagnostic
  prefixes and fixed reasons; English remains unchanged.

Search policy, provider behavior, result counts, truncation, cancellation,
diagnostic ordering, and application ownership remain unchanged.

## Boundaries and alternatives

The result reference is transient UI state and is cleared when the workspace
root changes or diagnostics are cleared. No localized reason is written back
to the application result, and no second diagnostic translator is introduced.
Changing provider output would couple infrastructure to locale policy and
would make English compatibility harder to preserve.

## Public-source applicability and review

Python 3.12 first-party documentation for
[`str.startswith`](https://docs.python.org/3.12/library/stdtypes.html#str.startswith)
and [`pathlib`](https://docs.python.org/3.12/library/pathlib.html) is applicable
to the prefix-preserving message mapping and relative-path projection. No Qt
API or manufacturer contract changed. Public CloudWeGo/ByteDance material
remains an engineering reference only; no private corporate standard,
certification, MISRA, ISO 26262, ASPICE, or embedded C/C++/MCU/RTOS claim is
made.

The architecture role `Avicenna the 7th / Luna max` and independent reviewer
`Newton the 7th / Luna max` returned `NO_CONCLUSION` after bounded waits and
closure. No child approval is claimed. Parent review is `PASS`; the
behavior-preserving simplification assessment is `PASS`.

## Evidence and limits

- `D251-DIAGNOSTIC-SOURCE-RETENTION=PASS`,
  `D251-DIAGNOSTIC-LOCALE-REFRESH=PASS`,
  `D251-DIAGNOSTIC-PREFIX-CATALOG=PASS`, and
  `D251-DIAGNOSTIC-EXPANSION-PRESERVATION=PASS`.
- Compileall, Ruff, and format checks passed.
- PS5.1 and PS7 package builds passed. Final root/dist identity is SHA-256
  `31788CA4CD50816F920F2AC9ED310CC67F605B489122918B56EFFE9AB684734E`,
  38,579,914 bytes, source revision
  `tree-sha256:c19a6dba52e3d85f9cbb922d89a8c2983d883bed7289d91ba1ea40da1df543db`.
- The frozen archive contains the entry point, PyQt6 platform plugin, i18n,
  workspace-search dialog modules, and 25 non-empty PyInstaller warning
  lines.

Native EXE/Qt startup, native dialogs, clean-machine, signing, installer,
updater, and release-owner evidence remain unrun under the active
non-destructive policy. Release remains `no-go` with existing report gates
open.

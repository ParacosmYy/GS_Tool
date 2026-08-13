# ADR-0297: Workspace-entry error locale refresh

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D253 / ARCH-231

## Context

The workspace provider correctly classified symbolic links and unsupported or
unreadable entries as disabled `WorkspaceEntry` values, but
`WorkspacePanel._item_for()` placed the raw `error` string directly into the
tooltip. Locale changes also skipped disabled rows, so a Chinese shell could
retain English provider diagnostics after switching language.

## Decision

Keep the provider result contract and correct only the workspace presentation
projection:

- retain the original `WorkspaceEntry.error` in a private Qt item data role;
- pass the error through the shared `localize_message()` mapper when the row is
  created;
- reproject that same source when `set_locale()` refreshes the tree, including
  disabled inaccessible rows;
- translate only stable provider-owned reasons, while preserving unknown
  filesystem/provider text unchanged.

The row remains disabled, its path and kind remain unchanged, inaccessible
entries remain non-activatable, and ordinary file/folder activation signals,
workspace provider behavior, ordering, bounds, and application ownership do
not move.

## Boundaries and alternatives

The raw error stays in the item source role and is never rewritten in the
domain result. A provider-side locale dependency would couple filesystem
infrastructure to presentation policy. A separate workspace error catalog
would duplicate the existing fallback mapper and increase translation drift.

## Public-source applicability and review

Qt 6 first-party documentation for
[`QTreeWidgetItem::setData`](https://doc.qt.io/qt-6/qtreewidgetitem.html#setData)
is applicable to retaining a presentation-only source value in a user-role
item field. Python 3.12 first-party string documentation is applicable to the
shared exact-message fallback. No manufacturer contract changed. Public
CloudWeGo/ByteDance material remains an engineering reference only; no private
corporate standard, certification, MISRA, ISO 26262, ASPICE, or embedded
C/C++/MCU/RTOS claim is made.

The architecture role `Hypatia the 7th / Luna max` and independent reviewer
`Peirce the 7th / Luna max` returned `NO_CONCLUSION` after bounded waits and
closure. No child approval is claimed. Parent review is `PASS`; the
behavior-preserving simplification assessment is `PASS`.

## Evidence and limits

- `D253-WORKSPACE-ERROR-SOURCE-RETENTION=PASS`,
  `D253-WORKSPACE-ERROR-TOOLTIP-LOCALIZATION=PASS`,
  `D253-WORKSPACE-DISABLED-LOCALE-REFRESH=PASS`, and
  `D253-WORKSPACE-UNKNOWN-ERROR-FALLBACK=PASS`.
- Compileall, Ruff, and format checks passed.
- PS5.1 and PS7 package builds passed. Final root/dist identity is SHA-256
  `9C10457042889B196BFD2F0BDAE26EA11FC8C9BA3A8D6CDCCE7AA04DED92033C`,
  38,580,637 bytes, source revision
  `tree-sha256:73dca3a0684d8c1d8b492e4b4e39db3a0ea126177445d3b16d5b2d52fbaa79b6`.
- The frozen archive contains the entry point, PyQt6 Windows platform plugin,
  workspace-panel/i18n modules, and 25 non-empty PyInstaller warning lines.

Native EXE/Qt startup, native dialogs, clean-machine, signing, installer,
updater, and release-owner evidence remain unrun under the active
non-destructive policy. Release remains `no-go` with existing report gates
open.

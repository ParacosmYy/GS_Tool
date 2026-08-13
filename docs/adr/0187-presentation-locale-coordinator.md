# ADR-0187: presentation locale coordinator

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D143 / ARCH-125

## Context

`MainWindow._retranslate_ui()` was a small but cross-surface orchestration
point. It read the current locale, refreshed the application title, and then
updated commands, dialogs, workspace, editor shell, tabs, status, search, and
plugin presentation in one concrete Qt host method. The order was meaningful,
and an optional workspace/search surface had to remain safe when absent.

## Decision

Add a Qt-free `PresentationLocaleCoordinator` with a frozen/slotted
`PresentationLocalePorts` contract. The coordinator owns only the stable locale
snapshot and existing callback order. MainWindow continues to own every Qt
surface, dynamic-surface check, translation key, settings snapshot, and
application policy by injecting callbacks. `_retranslate_ui()` now delegates to
the coordinator; no locale, signal, widget, persistence, or animation behavior
changes.

## Alternatives rejected

- A global locale service or singleton would expand ownership and make tests or
  future surfaces depend on hidden state.
- Passing concrete Qt widgets into a coordinator would violate the existing
  Qt-free presentation contract.
- Reordering or batching refreshes could change dynamic surface behavior, so
  the established sequence is retained explicitly.

## Review and evidence

Meitner the 4th / Luna max was assigned the architecture assessment and
returned no conclusion in the bounded window. Linnaeus the 4th / Luna max was
assigned the independent read-only review and also returned no conclusion after
two short waits. No child PASS is claimed. Parent review is `PASS`;
simplification assessment is `PASS` because one typed callback boundary and a
single delegation are the smallest complete extraction.

Authorized non-destructive evidence:

- `D143-LOCALE-SEQUENCE-PROBE=PASS`
- `D143-LOCALE-OPTIONAL-SURFACE-PROBE=PASS`
- `D143-QT-FREE-LOCALE-PROBE=PASS`
- `D143-MAINWINDOW-WIRING-PROBE=PASS`
- `D143-COMPILEALL=PASS`
- `D143-RUFF=PASS`
- `D143-FORMAT=PASS`
- `D143-CHECK=PASS`
- `D143-VERIFY-HANDOFF=PASS`
- `D143-PACKAGE-IDENTITY-PROBE=PASS`
- expected release `NO-GO` dossier and no-launch/traceability checks.

The packaged candidate is `dist/QuillForge.exe` and the root test copy:

- SHA-256: `B6C23EE8AF4D5FEE054F896A1082231528318EC007664A2939AD4AAEA6A3BD91`
- bytes: `38536972`
- source revision: `tree-sha256:daf21377f0b8f1568b7f4319862bee3862166821c2356129e30b35824ba3fdf5`

Public-source applicability is Python 3.12/PyQt6 presentation composition;
embedded C/C++, MCU, RTOS, and manufacturer requirements do not apply. Public
CloudWeGo material remains an engineering reference only, not a private
ByteDance standard or a certification/compliance claim.

## Limits

Static and inline probes do not prove native Qt rendering, event-loop timing,
font/DPI metrics, accessibility, runtime startup, clean-machine behavior,
cross-machine behavior, filesystem durability, signing, installer/update,
legal clearance, support ownership, or release readiness. Those gates remain
open under the active no-launch/no-release authorization boundary.

# ADR-0247: Command menu contract closure

- Status: accepted-with-limits
- Date: 2026-08-11
- Delivery: D199 / ARCH-185

## Context

`Command` accepted any string as `menu_id`, while the presentation projection
only created the four fixed top-level menus and silently skipped unknown IDs.
A plugin could therefore complete registration successfully while its command
was absent from every menu and had no explicit integration failure.

## Decision

Make the application command boundary own the supported `MenuId` set
(`file`, `edit`, `tools`, and `help`). `CommandRegistry.register` rejects an
unknown menu ID before admission. `CommandSurface` reuses the same ordered
constant to create its localized menu projection, while retaining ownership of
labels, QAction objects, shortcuts, and callbacks in presentation.

## Preserved invariants

- Existing built-in commands, plugin commands using `tools`, menu order,
  shortcuts, callbacks, locale refresh, and command execution remain unchanged.
- No dynamic menu system or silent Tools fallback is introduced; invalid
  integration data fails at the registration boundary with a clear error.
- Application code remains Qt-free and presentation continues to own Qt object
  trees and labels; dependency direction is not reversed.
- Plugin trust, permission, enablement, lifecycle, and execution policy remain
  in their existing owners.

## Review and applicability

The architecture consultation (`Carson the 6th / Luna max`) returned no
conclusion after two bounded waits and was closed; no child PASS is claimed.
The independent review (`Dirac the 6th / Luna max`) likewise returned no
conclusion after two bounded waits and was closed. Parent review is `PASS`, and
the behavior-preserving simplification assessment is `PASS` because one
application-owned constant and one admission guard remove the silent failure
without adding a menu abstraction.

This is a Python 3.12/PyQt6 application-contract change. Embedded C/C++, MCU,
RTOS, and manufacturer requirements are not applicable; the mandatory
embedded enterprise workflow is therefore not applicable to this slice.
Public CloudWeGo material is engineering reference only; no private ByteDance
standard, certification, or compliance claim is made.

Applicable public source: Qt 6 Style Sheets Reference, Qt Project, current
online documentation, especially `QMenu` item/indicator selectors:
https://doc.qt.io/qt-6/stylesheet-reference.html. It supports the adjacent
presentation projection only; it does not define a private enterprise menu
contract.

## Evidence and limits

- `D199-MENU-CONTRACT-PROBE=PASS valid=4 invalid=reject`
- `D199-BOUNDARY-SCOPE-PROBE=PASS`
- `D199-STATIC-AUDIT=PASS`
- `D199-COMPILE-RUFF-FORMAT=PASS`
- `D199-PACKAGE-BUILD-PS51=PASS`
- `D199-PACKAGE-BUILD-PS7=PASS`

No GUI, QApplication, EXE launch, screenshot, or unit-test asset was created
or run. Runtime plugin integration, native menu rendering, accessibility,
clean-machine, cross-machine, signing, installer, updater, legal, support, and
release-owner evidence remain open.

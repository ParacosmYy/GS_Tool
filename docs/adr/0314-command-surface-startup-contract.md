# ADR-0314: Command-surface startup contract

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D278 / ARCH-248

## Context

The local QuillForge startup log recorded a native startup failure while the
shell was being assembled:
`MainWindow._create_menus()` called `CommandSurface.create_menus()`, which
attempted to use a missing `_locale` member. The exact failure was an
`AttributeError` and occurred before the main window could be shown.

The current source already contains the bounded D225 fix: `CommandSurface`
stores the composition-owned locale provider and resolves it through a
private `_locale()` accessor. D278 adds a Qt-free static contract so a future
edit cannot remove that accessor or reorder MainWindow initialization past the
first menu/toolbar use without failing the project audit.

## Decision

Extend `scripts/audit_presentation_contracts.py` with
`_audit_startup_surface_contract()`.

The contract checks two ownership boundaries:

1. `CommandSurface` stores `locale_provider`, defines `_locale()`, delegates
   to the provider, and refreshes projected menu actions.
2. `MainWindow` injects the provider before menu creation, creates menus before
   the toolbar, and creates the locale coordinator after both command-surface
   projections exist.

The audit remains source-only and does not construct Qt objects. Runtime
locale ownership, command registration, menu order, toolbar callbacks, and
the existing D225 fix remain unchanged.

## Boundaries and public-source applicability

This is Python 3.12/PyQt6 desktop presentation code. Python's public
[`str.find`](https://docs.python.org/3.12/library/stdtypes.html#str.find),
[`zip(strict=True)`](https://docs.python.org/3.12/library/functions.html#zip),
and AST/source inspection behavior are applicable to the static audit. Qt's
public [`QMainWindow`](https://doc.qt.io/qt-6/qmainwindow.html) API remains
the applicable UI composition reference. No dependency or Qt API behavior was
changed.

CloudWeGo/ByteDance material is an engineering-style reference only; no
private corporate standard is inferred. No certification, MISRA, ISO 26262,
ASPICE, or compliance claim is made. Embedded C/C++, MCU, RTOS, and
manufacturer requirements are not applicable; the mandatory embedded workflow
was assessed as N/A for this Python/PyQt6 change.

## Review and simplification

The architecture role `Nietzsche the 7th / Luna max` was requested for a
read-only call-chain review. It returned no conclusion after bounded waits and
was closed. The independent review role `Kepler the 7th / Luna max` was also
requested for the audit diff and returned no conclusion after bounded waits
and closure.

Parent review: `PASS`. The change adds one declarative, Qt-free ordering
contract and does not duplicate runtime locale policy. Behavior-preserving
simplification assessment: `PASS`; no simplification was necessary because
the existing provider/accessor seam is already the smallest clear boundary.

## Verification and limits

- `D278-STARTUP-SURFACE-CONTRACT=PASS`: provider/accessor and four startup
  ordering anchors are present.
- `D278-COMMAND-SURFACE-LOCALE-PROBE=PASS locale=zh-CN`.
- Presentation audit, compileall, Ruff, and formatting passed.
- `D278-SOURCE-DIAGNOSTIC=PASS exit=0 status=passed failed=0`.
- `D278-PE-HEADER=PASS machine=AMD64 optional=PE32+ subsystem=WINDOWS_GUI`.
- `D278-PE-ARCHIVE=PASS outer_entries=166 pyz_entries=261 required=8
  embedded_command_surface=True`.
- The rebuilt root and `dist` candidates match at SHA-256
  `7ACD072EA6E0778C468C04958CC4DAC18E7FEB095FFCC14323C69726ACDB52BC`,
  size 38,583,521 bytes, source revision
  `tree-sha256:e62e62c51c7d698128bbe1418934af482ac419f1bbf39b40d488b673885e8d5e`.

No EXE/Qt launch, native menu rendering, native startup, clean-machine,
cross-machine, signing, installer, updater, registry, or accessibility
verification was run under the active no-launch/non-destructive policy. The
old local startup log is root-cause evidence for the pre-D225 candidate; the
current candidate is source- and archive-verified but not native-launch
verified.

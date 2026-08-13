# ADR-0268: i18n literal-key static gate

- Status: accepted-with-limits
- Date: 2026-08-11
- Delivery: D221 / ARCH-204

## Context

QuillForge already owns its English and Simplified Chinese catalogs in
`presentation.i18n`, but a future literal `tr("...")` call could silently
fall back to its key. The existing presentation contract audit did not compare
literal call-site keys with the canonical catalog.

## Decision

Extend `scripts/audit_presentation_contracts.py` with an AST-only check that
reads the top-level `_ENGLISH` catalog, scans presentation Python files for
literal first arguments to `tr()`, and reports any key absent from that
catalog. Dynamic keys remain allowed because their values cannot be proven
statically.

## Boundaries

1. No translation content, locale fallback, runtime lookup, or dynamic-key
   behavior changes.
2. The audit remains a static source gate and does not import or instantiate
   Qt.
3. This does not claim that every dynamic key is valid or that native text
   rendering is correct.

## Public-source applicability and review

This is Python 3.12 AST/static tooling applied to PyQt6 presentation source.
Python's public [AST documentation](https://docs.python.org/3.12/library/ast.html)
is the applicable first-party source for the syntax inspection. Public
CloudWeGo material remains an engineering reference only; no private ByteDance
standard, certification, or compliance claim is made. Embedded C/C++, MCU,
RTOS, and manufacturer requirements are not applicable.

The architecture role `Hegel the 6th / Luna max` returned `NO_CONCLUSION`
after bounded waits and closure. The independent role `Heisenberg the 6th /
Luna max` returned `NO_CONCLUSION` after bounded waits and closure. Parent
review is `PASS`; behavior-preserving simplification is `PASS`.

## Verification and limits

- `D221-I18N-LITERAL-KEY-PROBE=PASS catalog_keys=262 missing=0
  dynamic_keys=allowed`.
- `D221-COMPILEALL=PASS`, `D221-RUFF=PASS`, `D221-FORMAT=PASS`, and
  `D221-PRESENTATION-AUDIT=PASS`.
- `D221-PACKAGE-BUILD-PS51=PASS`, `D221-PACKAGE-BUILD-PS7=PASS`, and
  `D221-PACKAGE-IDENTITY-PROBE=PASS` with SHA
  `812E3DE56A2521FDF1D7524EE0C8CB4D5A7ED143952B993E8D4C70BE853A9606`,
  38,565,743 bytes, and source revision
  `tree-sha256:8cde8ee2725e247666547751f130aaf2a95565666bbafd84bea75c00b35954c8`.

No GUI/QApplication, EXE launch, native rendering, screenshot, accessibility,
clean-machine, cross-machine, signing, installer, updater, legal, support,
permission/disk-pressure, or release-owner evidence was run. No unit-test
asset was created or run.

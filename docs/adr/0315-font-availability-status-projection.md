# ADR-0315: Font availability status projection

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D279 / ARCH-249

## Context

QuillForge already persisted validated interface/editor font family, size, and
style values, but the Settings dialog only displayed the family names. A user
could select a family that is not installed on the current Windows machine
without knowing that Qt would fall back to another font. This made the font
switching feature look unreliable even though the saved value was valid.

## Decision

Keep the supported family lists and persistence contract in the existing
application settings owner. In the presentation-only `SettingsDialog`, query
the current platform catalog through `QFontDatabase.families()` once when the
dialog is created and project a localized status line for the selected
interface and editor families:

- installed;
- not installed, system fallback; or
- availability unavailable if the platform catalog cannot be queried.

The status is derived display text only. `settings_snapshot()` continues to
read the raw combo family values, so no status suffix or localized text can be
persisted into `SettingsSnapshot` or `settings_store.py`. Locale changes and
font selection changes reproject the status line immediately inside the dialog.

## Boundaries and public-source applicability

Qt's public [`QFontDatabase::families`](https://doc.qt.io/qt-6/qfontdatabase.html#families)
API is the applicable first-party reference for installed font-family lookup;
the existing PyQt6 binding is used without adding a dependency. Python 3.12
`frozenset` and `RuntimeError` behavior are standard-library/runtime
references. The query remains in presentation because it requires Qt GUI
runtime state; application/domain settings remain Qt-free.

CloudWeGo/ByteDance material is an engineering-style reference only. No
private corporate standard, certification, MISRA, ISO 26262, ASPICE, or
manufacturer requirement is inferred. Embedded C/C++, MCU, RTOS, and
manufacturer requirements are not applicable; the mandatory embedded workflow
and embedded simplifier were assessed as N/A for this Python/PyQt6 change.

## Review and simplification

The architecture role `Popper the 7th / Luna max` was requested for a
read-only boundary review and returned no conclusion after bounded waits and
closure. The independent review role `Curie the 7th / Luna max` was requested
for the implementation and returned no conclusion after bounded waits and
closure.

Parent review: `PASS`. The existing presentation surface is the correct owner;
no new service or persistence field is needed. Behavior-preserving
simplification assessment: `PASS`; the implementation uses one cached font
catalog and one status projection helper, and no further reduction would make
the fallback/unknown states clearer.

## Verification and limits

- `D279-FONT-STATUS-LOCALIZATION=PASS locales=2 placeholders=4`.
- `D279-FONT-PERSISTENCE-CONTRACT=PASS raw_family_values=2
  status_not_persisted=1`.
- Presentation contract audit, compileall, Ruff, and formatting passed.
- `D279-SOURCE-DIAGNOSTIC=PASS exit=0 status=passed failed=0`.
- `D279-PE-HEADER=PASS machine=AMD64 optional=PE32+ subsystem=WINDOWS_GUI`.
- `D279-PE-ARCHIVE=PASS outer_entries=166 pyz_entries=261 required=8
  embedded_settings_dialog=True`.
- The rebuilt root and `dist` candidates match at SHA-256
  `A4E1AD249D3131407F27CACD621AD24AD4BF4C43A49E371BA30B1749CB4F79C8`,
  size 38,585,345 bytes, source revision
  `tree-sha256:0f9aed11ab80f40aa92f6a63e7e5121bb995dd1106ecf1581f2f0fd3c3f4a51f`.

No EXE/Qt launch, native font enumeration/rendering, DPI, accessibility,
clean-machine, cross-machine, signing, installer, updater, registry, or
release-owner verification was run under the active no-launch/non-destructive
policy. Native behavior remains an explicit follow-up.

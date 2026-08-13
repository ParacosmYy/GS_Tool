# ADR-0304: Font-size unit locale refresh

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D260 / ARCH-238

## Context

The Settings dialog exposed interface and editor font sizes through two
`QSpinBox` controls. Their suffix was hard-coded as ` pt`, while the live
preview already rendered the Chinese unit as `磅`. Changing the language in the
Settings dialog refreshed labels and preview text but left both controls in
English.

## Decision

Add one catalog entry for the font-size suffix (` pt` in en-US and ` 磅` in
zh-CN). `SettingsDialog.set_locale` applies that suffix to both font-size
controls alongside the existing label refresh. The numeric values, ranges,
settings snapshot, persistence schema, and preview data remain unchanged.

The constructor no longer owns a locale-specific literal; its existing final
`set_locale` call initializes both suffixes before the dialog is shown.

## Boundaries and alternatives

The unit belongs to the Settings presentation catalog rather than the domain
font-size integer or persistence layer. Adding separate unit policy to the
settings model would couple storage to display language. Reusing the existing
preview string would create a formatting dependency between unrelated widgets,
so a small dedicated catalog value is clearer.

## Public-source applicability and review

Qt 6 first-party [`QSpinBox::setSuffix` documentation](https://doc.qt.io/qt-6/qspinbox.html#suffix-prop)
is applicable to refreshing the visible unit without changing the numeric
value. The project settings snapshot and locale-refresh contracts are the
applicable engineering references. No manufacturer requirement changed.
Public CloudWeGo/ByteDance material remains an engineering reference only; no
private corporate standard, certification, MISRA, ISO 26262, ASPICE, or
embedded C/C++/MCU/RTOS claim is made.

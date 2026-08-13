# ADR-0313: Settings typography contract audit

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D277 / ARCH-247

## Context

QuillForge exposes the requested language, theme/accent, interface/editor font
family, size, style, and motion preferences. Those values cross persistence,
settings UI, save projection, global QSS, and live editor adapters. A break at
any seam could leave a control visible while silently failing to apply or save
its value.

## Decision

Extend the existing Qt-free presentation contract audit with nine source-boundary
checks covering settings normalization and schema versioning, JSON persistence,
SettingsDialog controls/snapshot/locale projection, save projection ordering,
MainWindow application, global QSS typography, editor projection, editor
adapter setters, and localized size/style catalog entries.

This is a regression gate only. It does not change settings values, validation,
schema compatibility, save order, QSS selectors, editor behavior, or motion
policy.

## Review and boundaries

- Parent review: `PASS`.
- Simplification assessment: `PASS`; one declarative contract table keeps the
  cross-layer seam visible without adding runtime indirection.
- Architecture role `Dirac the 7th / Luna max`: `NO_CONCLUSION` after a bounded
  wait and safe closure.
- Independent review `Mencius the 7th / Luna max`: `NO_CONCLUSION` after a
  bounded wait and safe closure.
- Embedded C/C++, MCU, RTOS, and manufacturer requirements: not applicable.

## Public-source applicability

Python 3.12 first-party dataclasses/typing and the existing settings/theme/editor
port contracts are applicable references. No private ByteDance standard,
certification, MISRA, ISO 26262, ASPICE, or embedded claim is made.

## Evidence and limits

- `D277-TYPOGRAPHY-CONTRACT=PASS routes=9`.
- Presentation audit, compileall, Ruff, formatting, and `scripts/check.ps1`
  passed.
- PE is Windows x64 GUI; Qt/QScintilla/platform/style/icon runtime entries are
  present in the frozen archive.
- Current candidate SHA-256 is
  `166FDFBB9A294793720EB9E9384DB19F9E03B9857A7CA737274C91DF9F587DCD`,
  38,584,188 bytes; source revision is
  `tree-sha256:bb6627070a1e74f44af9aa88242eef79eec3da5229ced50afc121b4ce3b2820a`.
- Native rendering, installed-font metrics, native startup, clean-machine
  behavior, signing, installer/update, registry, and release-owner evidence
  remain unrun.

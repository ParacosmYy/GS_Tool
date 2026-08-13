# ADR-0312: Workspace file-open contract audit

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D276 / ARCH-246

## Context

The reported workspace bug described a state where folders opened but files did
not. The current source already has separate file and directory signals, but a
future refactor could silently disconnect the file branch while leaving folder
navigation green. The existing audit did not verify this complete presentation
to asynchronous-open chain.

## Decision

Extend the Qt-free presentation contract audit with six source-boundary checks:
the workspace tree's file/directory intents, the surface signal routes, the
file activation coordinator's containment/tab/open admission, the MainWindow
bindings, the asynchronous document-open admission, and the native file picker.

This is a regression gate only. It does not change item activation timing,
folder navigation, containment, busy/session gates, tab reuse, document
loading, or filesystem policy.

## Review and boundaries

- Parent review: `PASS`.
- Simplification assessment: `PASS`; one compact contract table keeps all
  boundaries in the existing audit without adding runtime indirection.
- Architecture role `Erdos the 7th / Luna max`: `NO_CONCLUSION` after a bounded
  wait and safe closure.
- Independent review `Ampere the 7th / Luna max`: `NO_CONCLUSION` after a
  bounded wait and safe closure.
- Embedded C/C++, MCU, RTOS, and manufacturer requirements: not applicable.

## Public-source applicability

Python 3.12 first-party `pathlib`/AST source behavior and the project's
presentation/application port contracts are applicable references. No private
ByteDance standard, certification, MISRA, ISO 26262, ASPICE, or embedded claim
is made.

## Evidence and limits

- `D276-FILE-OPEN-CONTRACT=PASS routes=6`.
- Presentation audit, compileall, Ruff, formatting, and `scripts/check.ps1`
  passed.
- PE is Windows x64 GUI; Qt/QScintilla/platform/style/icon runtime entries are
  present in the frozen archive.
- Current candidate SHA-256 is
  `A0547AC429E76CB7C6EE8B3A3BB180B2446DFE6871C9F27FFBEC88A9EEE51268`,
  38,584,607 bytes; source revision is
  `tree-sha256:39e0b45f7001340d67e110e02738adedeb04b532f7514382aa5b1d1f5fcf74a8`.
- Native file activation, native startup, rendering, clean-machine behavior,
  signing, installer/update, registry, and release-owner evidence remain
  unrun.

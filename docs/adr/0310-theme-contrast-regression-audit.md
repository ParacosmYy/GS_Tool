# ADR-0310: Theme contrast regression audit

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D273 / ARCH-244

## Context

QuillForge supports three shell themes and four accent choices. A prior
readability bug could make accent-colored text disappear when a bright color
was used as a background. `theme_tokens.py` already resolves readable
foregrounds, but the existing development audit did not assert that contract
across the full selection matrix.

## Decision

Extend `scripts/audit_presentation_contracts.py` with one Qt-free contrast
check. It resolves the existing `theme_colors` and `editor_color_tokens` for
all 12 theme/accent combinations and checks the three accent foreground pairs,
three shell text pairs, and six editor semantic foreground pairs against the
normal-text contrast floor of 4.5.

The check consumes the existing resolver and does not alter QSS, palette
application, editor behavior, or user settings. The resolver remains the sole
owner of fallback decisions.

## Review and boundaries

- Parent review: `PASS`.
- Architecture role `Rawls the 7th / Luna max`: `NO_CONCLUSION` after bounded
  wait and closure.
- Independent review `McClintock the 7th / Luna max`: `NO_CONCLUSION` after
  bounded wait and closure.
- Simplification assessment: `PASS`; one helper reuses the existing token
  functions and has no runtime state.
- D271 file-open chain audit also passed without requiring a behavior change.
- Embedded C/C++, MCU, RTOS, and manufacturer requirements: not applicable.

## Public-source applicability

Python 3.12 first-party runtime behavior and the project's token/contrast
contract are applicable references. No private ByteDance standard,
certification, MISRA, ISO 26262, ASPICE, or embedded claim is made.

## Evidence and limits

- `D273-CONTRAST-MATRIX=PASS checks=144 failures=0`.
- Presentation audit, compileall, Ruff, formatting, and `scripts/check.ps1`
  passed.
- PE is Windows x64 GUI; Qt/QScintilla/platform/icon runtime entries are in
  the frozen archive.
- Current candidate SHA-256 is
  `605373E0875E59956C2C9EE1DA8B031EECE4C643C866662D793B9016EF1C7987`,
  38,584,016 bytes; source revision is
  `tree-sha256:0cd9983ee2e88ea3b3c316c477d49447a97c594ced87d591b85fefb22655902f`.
- Native EXE/Qt startup, native rendering, clean-machine behavior, signing,
  installer/update, registry, and release-owner evidence remain unrun.


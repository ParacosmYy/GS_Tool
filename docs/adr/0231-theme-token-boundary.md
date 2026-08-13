# ADR-0231: Framework-neutral theme token boundary

- Status: Accepted with limits
- Date: 2026-08-11
- Delivery: D182 / ARCH-169 / UI-94

## Context

`presentation.theme` had become the owner of both pure palette resolution and
the Qt-specific application/editor projection. The single visual source was
valuable, but the two responsibilities had different reasons to change and
made the visual system harder to reuse or inspect without Qt.

## Decision

1. Add `presentation.theme_tokens` as the sole owner of the framework-neutral
   `ThemeColors` and `EditorColorTokens` value objects, bounded theme/accent
   palettes, token resolution, and contrast helpers.
2. Keep `presentation.theme` as the sole owner of Qt palette, generated QSS,
   application icon, and editor-adapter projection.
3. Preserve the existing `theme.py` public imports and private diagnostic names
   as a compatibility façade; no caller is required to migrate in D182.
4. Keep all values immutable and retain the existing fallback and contrast
   behavior for every supported theme/accent combination.

## Boundaries and non-goals

This is a presentation-module ownership split. It does not introduce a second
styling system, change settings persistence, add runtime theme discovery,
change widget behavior, or claim native Qt rendering or accessibility proof.

## Applicability and evidence

The applicable contract is Python 3.12/PyQt6 presentation code in this local
checkout. Embedded C/C++, MCU, RTOS, and manufacturer requirements are not
applicable. Public CloudWeGo/ByteDance-adjacent material, if referenced for
engineering vocabulary, is not an internal standard or compliance evidence.

Static evidence includes a no-Qt-import boundary probe for `theme_tokens`,
12-theme/accent compatibility and contrast probes, QSS wiring checks,
compileall, Ruff, and format checks. Runtime Qt painting, DPI, installed-font,
screen-reader, clean-machine, cross-machine, signing, installer, updater, and
release-owner gates remain open.

## Review disposition

Parent review: PASS. The bounded architecture and independent child windows
returned no conclusion before their time limits; no child PASS is claimed.
The parent found no further safe simplification after the mechanical move and
compatibility façade review.

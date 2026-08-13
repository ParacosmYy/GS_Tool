# D9 / UI-05 parent review

## Scope

- **Delivery:** D9 Modern UI iteration
- **Slice:** UI-05 Modern editor canvas
- **Owner:** Architect (parent)
- **Change type:** QScintilla adapter/theme projection only

## Product and architecture decision

The editor canvas is the product's primary surface, so its colors must be
intentional rather than inherited from platform defaults. `EditorWidget`
remains the only QScintilla owner. `theme.py` owns the visual token map and
receives a small attribute protocol; application services and plugins still
see only `EditorEngine`.

## Implementation record

- The editor uses ink canvas/margins, cyan caret, current-line emphasis, and
  readable line-number foreground/background colors.
- Python lexer styles map comments, numbers, strings, keywords, names,
  operators, decorators, and malformed strings to the shared visual tokens.
- `set_language("python")` reapplies the same theme after replacing the lexer,
  avoiding a regression when a path-backed Python document is opened.
- Non-Python documents retain the existing no-lexer behavior; no language
  expansion is smuggled into this UI slice.

## Verification and limits

- Ruff check/format and `scripts/check.ps1` are the required static gates.
- `.\scripts\package.ps1` completed after UI-05; root/dist are 38,323,223
  bytes with SHA-256
  `B9151D4D3AAA25AA73827ABF033CC3C6F2022174C0A3BFAED1A904E768F3EAF7`.
- No EXE, Qt window, interactive startup, or screenshot is launched because
  the user explicitly prohibited starting the software.
- Font availability, lexer-version differences, DPI, accessibility contrast,
  and non-Python syntax themes remain open for a permitted visual review.

## Disposition

`PROCEED WITH LIMITS`: the adapter boundary and source token map are complete;
runtime visual acceptance remains pending.

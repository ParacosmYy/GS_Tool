# ADR-0287: Readable accent text endpoints

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D243 / UI-25

## Decision

Keep `accent_alt` as a visual accent and filled-control background token, but
do not use it unconditionally as text on general surfaces. The Qt palette link
color, workspace eyebrow, and generic checkbox hover selector now consume one
private `theme.py` resolver. It preserves the accent when it meets the
contrast threshold against the deepest shared surface and falls back to
`text_primary` when the accent is too light.

Filled controls continue to use the existing `on_accent`, `on_accent_pink`,
and `on_accent_gold` tokens. `ThemeColors` remains framework-neutral and no
widget or application service gains a second contrast policy.

## Public-source applicability

The applicable public engineering references are first-party Qt and W3C
contrast guidance:

- Qt, *QPalette Class*, Qt 6.11.1 documentation, accessed 2026-08-12:
  <https://doc.qt.io/qt-6/qpalette.html>
- W3C, *Web Content Accessibility Guidelines (WCAG) 2.2*, Success Criterion
  1.4.3 Contrast (Minimum), accessed 2026-08-12:
  <https://www.w3.org/TR/WCAG22/#contrast-minimum>

These are framework/accessibility references, not manufacturer requirements.
No private ByteDance standard, certification, MISRA, ISO 26262, ASPICE, or
compliance claim is made. Embedded C/C++, MCU, BSP/HAL, RTOS, and manufacturer
requirements are not applicable.

## Evidence and limits

- `D243-COMPILEALL=PASS`; `D243-RUFF=PASS`; `D243-FORMAT=PASS`
- `D243-THEME-ENDPOINT-CONTRAST=PASS min=9.207` across 3 themes × 4 accents
  and all four shared surfaces.
- `D243-DIRECT-ACCENT-TEXT-PROBE=PASS`; `D243-QSS-SEMANTIC-ENDPOINTS=PASS`
- PS5.1 build passed with intermediate SHA
  `B1FCE515A36C83CAEDF23D876CAFF3316B6F255323BEC75A553A142AC9CE9F88`;
  PS7 final build passed with SHA
  `AF63CEBE385F4DDBA1174C4D71847550BB6FB42D0E417856F71E7B4D3E63B6B0`,
  38,573,642 bytes.
- Final source revision:
  `tree-sha256:27d841ef539a0de2e6e9cfc13876cf0a7b1ed4e0c78618705fbac2c61b39ddf6`.
- Archive/resource/warning checks, `scripts/check.ps1`, and handoff checks
  passed in PS5.1 and PS7; final release dossier remains expected no-go.
- Parent review and simplification assessment: `PASS`.
- Fermat the 7th architecture and Linnaeus the 7th independent Luna/max
  windows returned `NO_CONCLUSION` after bounded waits; no child PASS is
  claimed.
- Native EXE/Qt startup, native rendering, clean-machine behavior, and
  screen-reader review remain unrun under the permanent no-launch policy.

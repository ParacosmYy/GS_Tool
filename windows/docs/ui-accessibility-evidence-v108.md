# UI / Accessibility Evidence v108

## Scope

This increment strengthens the secondary copy in the shared login and
registration cards. The explanatory line and alternate-route link now use the
project's stronger readable text token, a local glyph keyline, and a restrained
lime underline. No opaque overlay or authentication-flow change was added.

## Runtime evidence

- Isolated server: `127.0.0.1:5209`.
- Real `320 × 720` login and registration screenshots show the card remains a
  transparent glass surface while the helper line and bottom route link stay
  legible over the laptop/character highlights.
- Computed style reports `blur(11px) saturate(1.06)` for the card, transparent
  card background, `rgb(215, 219, 229)` helper/footnote text, and a lime
  underline on the alternate-route link.
- A real browser click on the username field produced `:focus-within`, the
  expected focus border/shadow, and retained the existing keyboard path.
- Page logs: `v108tab.dev.logs()` returned an empty array. Browser telemetry
  warnings from the isolated harness were not page console output.

## Responsive matrix

| Page | Viewport | Overflow | Named controls | Heading count |
| --- | --- | ---: | ---: | --- |
| Login | 320 × 720 | 0 px | 5 | 1 h1 / 1 h2 |
| Login | 390 × 844 | 0 px | 5 | 1 h1 / 1 h2 |
| Login | 768 × 900 | 0 px | 5 | 1 h1 / 1 h2 |
| Login | 1440 × 900 | 0 px | 5 | 1 h1 / 1 h2 |
| Register | 320 × 720 | 0 px | 5 | 1 h1 / 1 h2 |
| Register | 390 × 844 | 0 px | 5 | 1 h1 / 1 h2 |
| Register | 768 × 900 | 0 px | 5 | 1 h1 / 1 h2 |
| Register | 1440 × 900 | 0 px | 5 | 1 h1 / 1 h2 |

The viewport override was reset after evidence collection. This is
isolated-browser evidence, not a claim about physical-device metrics,
reduced-motion, forced-colors, real credentials, or production deployment.

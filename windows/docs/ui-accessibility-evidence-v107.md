# UI / Accessibility Evidence v107

## Scope

This increment adds a local glyph keyline to the dashboard hero explanation.
The copy crosses the laptop and character highlights on compact screens, so
the change improves scanability without introducing a filled panel or changing
the transparent scene composition.

## Runtime evidence

- Isolated server: `127.0.0.1:5208`.
- The real `320 × 720` screenshot shows the Chinese hero explanation readable
  over the illustrated laptop and character clothing while its background stays
  transparent.
- Computed style for `.hero-lede` is `color: rgb(215, 219, 229)` with a tight
  two-stage dark text shadow and no background or backdrop blur.
- The desktop screenshot keeps the original two-column hero composition; the
  transparent header and metadata row remain unchanged.
- A real browser click on `Week` synchronized the active range and
  `aria-pressed`; deep scroll kept the transparent header and established
  `blur(12px)` content-card reading surface.
- Page logs: `tab5208.dev.logs()` returned an empty array. Browser telemetry
  warnings from the isolated harness were not page console output.

## Responsive matrix

| Viewport | Overflow | Hero copy shadow | Metadata border | Named controls | Heading count |
| --- | ---: | --- | --- | ---: | --- |
| 320 × 720 | 0 px | present | all transparent | 36 | 1 h1 / 8 h2 |
| 390 × 844 | 0 px | present | all transparent | 36 | 1 h1 / 8 h2 |
| 768 × 900 | 0 px | present | all transparent | 40 | 1 h1 / 8 h2 |
| 1024 × 900 | 0 px | present | all transparent | 40 | 1 h1 / 8 h2 |
| 1440 × 900 | 0 px | present | all transparent | 40 | 1 h1 / 8 h2 |

The viewport override was reset after evidence collection. This is
isolated-browser evidence, not a claim about physical-device metrics,
reduced-motion, forced-colors, real Provider keys, or production deployment.

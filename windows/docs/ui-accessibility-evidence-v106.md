# UI / Accessibility Evidence v106

## Scope

This increment removes the residual border inherited by the hero metadata
row. The row now keeps the scene continuous on all four sides; the short
signal trace remains the only intentional visual boundary.

## Runtime evidence

- Isolated server: `127.0.0.1:5206`.
- Desktop and mobile computed styles report `background: transparent`,
  `backdrop-filter: none`, and transparent top/right/bottom/left borders for
  `.hero-topline`.
- A real browser click on `Week` changed the active range and synchronized
  `aria-pressed` without changing the transparent metadata layer.
- Deep-scroll computed styles kept the header transparent and the dashboard
  content card at its established `blur(12px)` reading surface.
- Page logs: `tab5206.dev.logs()` returned an empty array. Browser telemetry
  warnings from the isolated harness were not page console output.

## Responsive matrix

| Viewport | Overflow | Metadata border | Metadata blur | Named controls | Heading count |
| --- | ---: | --- | --- | ---: | --- |
| 320 × 844 | 0 px | all transparent | none | 36 | 1 h1 / 8 h2 |
| 390 × 844 | 0 px | all transparent | none | 36 | 1 h1 / 8 h2 |
| 768 × 900 | 0 px | all transparent | none | 40 | 1 h1 / 8 h2 |
| 1024 × 900 | 0 px | all transparent | none | 40 | 1 h1 / 8 h2 |
| 1440 × 900 | 0 px | all transparent | none | 40 | 1 h1 / 8 h2 |

The viewport override was reset after evidence collection. This is
isolated-browser evidence, not a claim about physical-device metrics,
reduced-motion, forced-colors, real Provider keys, or production deployment.

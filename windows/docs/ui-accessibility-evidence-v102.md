# UI / Accessibility Evidence v102

## Scope

This increment lightens the dashboard period switcher in ordinary color mode.
The control keeps its border, active lime state, keyboard semantics, and
forced-colors boundary, while removing the dark blurred slab that conflicted
with the transparent AI TOKEN brand row.

## Runtime evidence

- Isolated server: `127.0.0.1:5202`.
- Dashboard session: synthetic local account created only for this run.
- Desktop screenshot: the header remains transparent; the period switcher uses
  a low-alpha vertical gradient and `backdrop-filter: none`.
- Deep-scroll screenshot at approximately `scrollY=720`: metric cells and
  content cards remain readable, while the period control no longer creates a
  heavy floating tile over the illustration.
- Period interaction: `Today`, `Week`, `Month`, and `All time` each became the
  active `aria-pressed="true"` state after a real browser click.
- Page logs: `tab5202.dev.logs()` returned an empty array.

## Responsive matrix

| Requested viewport | Page overflow | Visible named controls | Heading count |
| --- | ---: | ---: | --- |
| 320 × 844 | 0 px | 36 / 36 | 1 h1 / 8 h2 |
| 390 × 844 | 0 px | 36 / 36 | 1 h1 / 8 h2 |
| 768 × 900 | 0 px | 40 / 40 | 1 h1 / 8 h2 |
| 1024 × 900 | 0 px | 40 / 40 | 1 h1 / 8 h2 |
| 1440 × 900 | 0 px | 40 / 40 | 1 h1 / 8 h2 |

The browser viewport override was reset after evidence collection. This is
isolated-browser evidence, not a claim about physical-device metrics,
reduced-motion, forced-colors, real Provider keys, or production deployment.

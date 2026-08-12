# UI / Accessibility Evidence v105

## Scope

This increment aligns the automatic-collection connection badge with the
dashboard's lighter transparent surface language. The disconnected, ready,
and error variants retain separate semantic borders and colors; the ordinary
surface no longer uses the older dark blurred capsule.

## Runtime evidence

- Isolated server: `127.0.0.1:5205`.
- Dashboard deep-scroll screenshot shows `未连接` beside the automatic
  collection heading as a low-alpha local boundary, not a dark floating tile.
- Ordinary disconnected computed style: `backdrop-filter: none`, low-alpha
  vertical gradient, and a light inset keyline.
- The source keeps explicit `.is-ready` and `.is-error` rules; no ready/error
  provider response was fabricated for this empty local database run.
- Base URL focus was verified after a real browser locator click: the input
  was active, the form card matched `:focus-within`, and the lime outline was
  present.
- Page logs: `tab5205.dev.logs()` returned an empty array.

## Responsive matrix

| Viewport | Overflow | Badge blur | Card blur | Visible named controls | Heading count |
| --- | ---: | --- | --- | ---: | --- |
| 320 × 844 | 0 px | none | `blur(12px)` | 36 / 37 | 1 h1 / 8 h2 |
| 390 × 844 | 0 px | none | `blur(12px)` | 36 / 37 | 1 h1 / 8 h2 |
| 768 × 900 | 0 px | none | `blur(12px)` | 40 / 41 | 1 h1 / 8 h2 |
| 1024 × 900 | 0 px | none | `blur(12px)` | 40 / 41 | 1 h1 / 8 h2 |
| 1440 × 900 | 0 px | none | `blur(12px)` | 40 / 41 | 1 h1 / 8 h2 |

The browser viewport override was reset after evidence collection. This is
isolated-browser evidence, not a claim about physical-device metrics,
reduced-motion, forced-colors, real Provider keys, or production deployment.

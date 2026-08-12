# UI / Accessibility Evidence v104

## Scope

This increment reduces the visual weight of Dashboard content surfaces. Chart,
automatic collection, guide, records, and manual detail cards now use a lower
alpha deep-blue glass layer and shorter blur, so the illustrated scene remains
continuous after the hero section. Form fields keep their own local reading
surface and focus treatment.

## Runtime evidence

- Isolated server: `127.0.0.1:5204`.
- Desktop and mobile deep-scroll screenshots show the scene continuing around
  and through content cards instead of a single dense dark slab.
- Dashboard card computed style: `blur(12px) saturate(1.06)` with the shared
  `.70/.82` gradient layer in ordinary color mode.
- Period interaction: `Today`, `Week`, `Month`, and `All time` each became the
  active `aria-pressed="true"` state after a real browser click.
- Form focus: the Base URL field remained focused with its lime outline and
  the containing form card retained `:focus-within`.
- Page logs: `tab5204.dev.logs()` returned an empty array.

## Responsive matrix

| Viewport | Overflow | Card blur | Visible named controls | Heading count |
| --- | ---: | --- | ---: | --- |
| 320 × 844 | 0 px | `blur(12px)` | 36 / 37 | 1 h1 / 8 h2 |
| 390 × 844 | 0 px | `blur(12px)` | 36 / 37 | 1 h1 / 8 h2 |
| 768 × 900 | 0 px | `blur(12px)` | 40 / 41 | 1 h1 / 8 h2 |
| 1024 × 900 | 0 px | `blur(12px)` | 40 / 41 | 1 h1 / 8 h2 |
| 1440 × 900 | 0 px | `blur(12px)` | 40 / 41 | 1 h1 / 8 h2 |

The browser viewport override was reset after evidence collection. This is
isolated-browser evidence, not a claim about physical-device metrics,
reduced-motion, forced-colors, real Provider keys, or production deployment.

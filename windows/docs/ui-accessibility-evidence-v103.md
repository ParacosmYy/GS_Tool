# UI / Accessibility Evidence v103

## Scope

This increment lightens the authentication card in ordinary color mode. The
card keeps a stable navy reading surface for credentials, but exposes more of
the illustrated scene through a lower-alpha gradient and a shorter blur.

## Runtime evidence

- Isolated server: `127.0.0.1:5203`.
- Login and registration were checked in a real browser session.
- Desktop and mobile screenshots show the character, code displays, and device
  scene continuing through the authentication surface instead of a single
  opaque black slab.
- Login field focus remained visible: the card retained `:focus-within`, the
  active field retained its lime outline, and the field reading background was
  unchanged.
- Page-level scroll width stayed within the browser client width at every
  requested viewport.

## Responsive matrix

| Route | Viewport | Overflow | Card surface | Named controls | Headings |
| --- | --- | ---: | --- | ---: | --- |
| `/login` | 320 × 844 | 0 px | `blur(11px)` | 7 / 7 | 1 h1 / 1 h2 |
| `/register` | 320 × 844 | 0 px | `blur(11px)` | 7 / 7 | 1 h1 / 1 h2 |
| `/login` | 390 × 844 | 0 px | `blur(11px)` | 7 / 7 | 1 h1 / 1 h2 |
| `/register` | 390 × 844 | 0 px | `blur(11px)` | 7 / 7 | 1 h1 / 1 h2 |
| `/login` | 768 × 900 | 0 px | `blur(11px)` | 7 / 7 | 1 h1 / 1 h2 |
| `/register` | 768 × 900 | 0 px | `blur(11px)` | 7 / 7 | 1 h1 / 1 h2 |
| `/login` | 1024 × 900 | 0 px | `blur(11px)` | 7 / 7 | 1 h1 / 1 h2 |
| `/register` | 1024 × 900 | 0 px | `blur(11px)` | 7 / 7 | 1 h1 / 1 h2 |
| `/login` | 1440 × 900 | 0 px | `blur(11px)` | 7 / 7 | 1 h1 / 1 h2 |
| `/register` | 1440 × 900 | 0 px | `blur(11px)` | 7 / 7 | 1 h1 / 1 h2 |

The browser viewport override was reset after evidence collection. This is
isolated-browser evidence, not a claim about physical-device metrics,
reduced-motion, forced-colors, real Provider keys, or production deployment.

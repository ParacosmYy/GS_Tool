# UI / Accessibility Evidence v109

## Scope

This increment adds the dashboard period switcher to the existing sticky
occlusion candidate set. When the transparent AI TOKEN header covers the
switcher, it fades out and becomes non-interactive; `:focus-within` restores
the control so keyboard users never lose an active range control.

## Runtime evidence

- Isolated server: `127.0.0.1:5210`.
- At a real desktop `scrollY=760` overlap point, `.range-switcher` reports
  `is-under-sticky-header`, `opacity: 0`, and `pointer-events: none`; the
  sticky header remains `0–76px` and the document has no horizontal overflow.
- At `320 × 720` and `1024 × 900`, the same overlap boundary hides the
  switcher. At `390 × 844`, `768 × 900`, and `1440 × 900`, the control remains
  visible when it is physically below the header, so the rule does not hide
  content merely because the page is scrolled.
- A real click on `Today` followed by deep scroll kept the focused button
  visible and interactive through `:focus-within`, with `opacity: 1` and
  `pointer-events: auto`.
- Page logs: `v109fresh.dev.logs()` returned an empty array. Browser telemetry
  warnings from the isolated harness were not page console output.

## Responsive matrix

| Viewport | Overflow | Range overlap at probe | Range opacity | Named controls | Heading count |
| --- | ---: | --- | ---: | ---: | --- |
| 320 × 720 | 0 px | yes | 0 | 36 | 1 h1 / 8 h2 |
| 390 × 844 | 0 px | no | 1 | 36 | 1 h1 / 8 h2 |
| 768 × 900 | 0 px | no | 1 | 40 | 1 h1 / 8 h2 |
| 1024 × 900 | 0 px | yes | 0 | 40 | 1 h1 / 8 h2 |
| 1440 × 900 | 0 px | no | 1 | 40 | 1 h1 / 8 h2 |

The viewport override was reset after evidence collection. This is
isolated-browser evidence, not a claim about physical-device metrics,
reduced-motion, forced-colors, real Provider keys, or production deployment.

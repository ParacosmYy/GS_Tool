# UI Accessibility Evidence — v80

> Owner: Frontend UI Engineer
> Reviewers: UI-1, UI-2, Architect
> Scope: empty analytics signal surface and transparent brand-row verification
> Runtime: isolated local Flask instance on port 5176
> Date: 2026-08-12 (Asia/Shanghai)

## 1. Change intent

An account with no usage records already had an explanatory empty state, but
the chart area still read as a large unoccupied black canvas. v80 gives that
state a quiet instrument treatment:

- the trend empty state receives a low-contrast grid and a zero baseline;
- a short lime scan line communicates that the ledger is waiting for data;
- the model-mix empty state receives restrained concentric guide rings;
- the existing marker, copy, and automatic-collection action remain the
  accessible content layer;
- `prefers-reduced-motion` and `forced-colors` disable or replace decoration.

The change is presentation-only. No DOM, Chart.js data contract, API, auth,
provider, database, or navigation behavior changed.

## 2. Runtime boundary

- CSS: `windows/token_tracker/static/responsive-tuning.css`
- Isolated runtime: `windows/.cache/ui-v2400-runtime-20260812-5176/`
- Test URL: `http://127.0.0.1:5176/dashboard`
- A synthetic local account was created only through the local registration
  flow to observe an empty Dashboard. Credentials, cookies, storage values,
  provider keys, and user data were not read or submitted.

## 3. Real browser evidence

At 1440px, a scroll capture showed the trend card as a defined graph surface:
the grid remains below the empty-state content, the baseline runs across the
lower plot area, and the lime scan remains subordinate to the action button.
The model-mix card uses the same empty-state hierarchy with circular guides.

The header was checked separately because it is the user-facing transparency
contract:

```text
top header background: rgba(0, 0, 0, 0)
top header backdrop-filter: none
scrolled header background: rgba(0, 0, 0, 0)
scrolled header backdrop-filter: none
brand background: rgba(0, 0, 0, 0)
```

On scroll, only the right nav/account reading rail activates. The brand region
remains outside that rail, so the AI TOKEN row continues to show the scene
through it rather than becoming a solid horizontal panel.

## 4. Responsive matrix

All checks used a real browser at 900px viewport height. `overflow` is the
boolean result of `scrollWidth > clientWidth`.

| Viewport | Client / scroll width | Overflow | Trend empty height | Mix empty height | Header top |
|---:|---:|:---:|---:|---:|---|
| 320 | 305 / 305 | false | 205 | 190 | transparent |
| 390 | 375 / 375 | false | 205 | 190 | transparent |
| 768 | 753 / 753 | false | 205 | 190 | transparent |
| 1024 | 1009 / 1009 | false | 205 | 190 | transparent |
| 1440 | 1425 / 1425 | false | 220 | 205 | transparent |

The empty-state background images were present at each width. The 320px and
390px cards stayed within their content columns; no page or chart wrapper
introduced horizontal scrolling.

## 5. Accessibility and interaction checks

- Accessibility DOM snapshot length: 5,978 characters.
- The snapshot retained the skip link, banner, labeled navigation, main
  heading, period controls, chart `role=status` messages, forms, tables, and
  live status regions.
- Clicking `Week` set `aria-pressed="true"`; clicking `Today` restored
  `aria-pressed="true"` for the default range.
- Page console logs: `[]`.
- The CSS-only scan has no focus stop and uses `pointer-events: none`.
- The browser harness emitted a tool-level telemetry batching warning during
  repeated viewport instrumentation; it did not appear in page logs and is
  not an application error.

## 6. Explicit limits

- `prefers-reduced-motion` and `forced-colors` behavior is protected by source
  rules; those OS modes were not claimed as physically simulated here.
- Non-empty Chart.js rendering, real provider calls, physical devices,
  assistive technology, and production deployment were not claimed by this
  local visual pass.
- No unit-test code, mock, fixture, harness, or test-only asset was created or
  changed.

## 7. Acceptance

**Status: PASS for v80 empty analytics presentation scope.** The zero-data
charts now communicate a deliberate waiting state without adding a dark slab,
while the AI TOKEN brand row remains computed-transparent at rest and during
scroll. Remaining release gates stay tracked in `release-readiness.md` and
`final-acceptance-matrix.md`.

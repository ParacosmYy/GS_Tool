# UI Accessibility Evidence — v77

> Owner: Frontend UI Engineer
> Reviewers: UI-1, UI-2, Architect
> Scope: global scene backdrop and AI TOKEN header transparency
> Runtime: isolated local Flask instance on port 5173
> Date: 2026-08-12 (Asia/Shanghai)

## 1. Change intent

The source artwork already contains a dark editorial copy-safe area. The
previous `story-backdrop::before` rule added another black veil, with desktop
left-edge alpha values reaching `.36` and mobile values reaching `.60`.
That combination made the top-left field read as a solid black block behind the
AI TOKEN row.

v77 replaces the black veil with a controlled navy atmosphere. The scene still
protects white copy, but the illustration can now contribute depth through the
brand row and the upper-left login composition. No DOM, API, data, auth, or
provider behavior changed.

## 2. Source and runtime boundary

- CSS: `windows/token_tracker/static/scene-motion.css`
- Artwork inspected: `windows/token_tracker/static/assets/embedded-rust-engineer-bg-v14.png`
- Isolated runtime directory: `windows/.cache/ui-v2100-runtime-20260812-5173/`
- Isolated SQLite file: `windows/.cache/ui-v2100-runtime-20260812-5173/token_tracker-5173.sqlite3`
- Test URL: `http://127.0.0.1:5173/login?ui_v2100=v77-final-1440`
- Browser interaction stayed on the local login page; no credentials, cookies,
  storage values, provider keys, or user data were read or submitted.

## 3. Observed computed-style matrix

All checks used a real browser at 900px viewport height. `overflow` is the
boolean result of `scrollWidth > clientWidth`.

| Viewport | Client / scroll width | Overflow | Header background | Header blur | Backdrop rule |
|---:|---:|:---:|---|---|---|
| 390 | 375 / 375 | false | transparent | none | mobile ≤620 navy rule |
| 320 | 305 / 305 | false | transparent | none | mobile ≤620 navy rule |
| 768 | 753 / 753 | false | transparent | none | tablet ≤900 navy rule |
| 1024 | 1009 / 1009 | false | transparent | none | desktop navy rule |
| 1440 | 1425 / 1425 | false | transparent | none | desktop navy rule |

The final 1440px computed `story-backdrop::before` values were:

```text
radial-gradient(62% 55% at 10% 7%, rgba(65, 125, 196, 0.38), transparent 76%),
linear-gradient(90deg, rgba(4, 18, 38, 0.16) 0%, rgba(4, 18, 38, 0.12) 23%,
rgba(4, 18, 38, 0.07) 47%, rgba(4, 18, 38, 0.02) 70%, transparent 100%),
linear-gradient(rgba(4, 12, 24, 0.02), transparent 36%, rgba(8, 9, 12, 0.16))
```

The final 390px computed rule reduced the black veil to a navy blend with
`.18/.14/.08/.02` horizontal alpha stops. The 768px rule uses
`.16/.12/.07/.02` stops. These values preserve a readable copy zone without
turning the responsive scene into a black slab.

## 4. Accessibility and runtime checks

- Accessibility snapshot length: 1,079 characters.
- Snapshot retained a `banner` containing the `AI TOKEN OBSERVATORY` link and
  a `main` landmark containing the login heading, labeled username/password
  fields, and the primary submit button.
- Browser page logs: `[]`.
- Desktop and compact viewports had no horizontal overflow.
- The header remained `background: rgba(0, 0, 0, 0)`,
  `backdrop-filter: none`, transparent bottom border, and no box shadow across
  the responsive matrix.
- The decorative backdrop remains `aria-hidden` and pointer-inert.

## 5. Explicit limits

- `prefers-reduced-motion`, `forced-colors`, physical devices, assistive
  technology, real provider calls, and production deployment were not claimed
  as verified by this local visual pass.
- The current browser facade did not expose a screenshot capture method in this
  run; computed styles, accessibility snapshot, source artwork inspection, and
  console evidence are recorded instead.
- No unit-test code, mock, fixture, or test-only asset was created or changed.

## 6. Acceptance

**Status: PASS for v77 local visual scope.** The AI TOKEN row is transparent at
all checked widths, while the backdrop beneath it no longer uses a heavy black
veil at any responsive breakpoint. Remaining release gates stay tracked in
`release-readiness.md` and `final-acceptance-matrix.md`.

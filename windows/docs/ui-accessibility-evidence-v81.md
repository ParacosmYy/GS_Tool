# UI Accessibility Evidence — v81

> Owner: Frontend UI Engineer
> Reviewers: UI-1, UI-2, Architect
> Scope: sticky brand readability without a solid header bar
> Runtime: isolated local Flask instance on port 5177
> Date: 2026-08-12 (Asia/Shanghai)

## 1. Change intent

During a long Dashboard scroll, section labels could pass behind the small AI
TOKEN identity lockup. The header itself was already transparent, but the
overlapping text weakened the brand and reading order. v81 adds a local,
pointer-inert brand lens only while the header is scrolled and fades the
one-time `Scroll to explore` cue after it has served its purpose.

- the full header remains `background: transparent` with no global blur;
- the brand lens does not change the brand's layout box or add a focus stop;
- the lens uses only `.035` alpha and `9px` backdrop blur, not a solid bar;
- the scroll cue transitions out of the sticky header reading lane;
- forced-colors hides the decorative lens and keeps system colors in control.

No DOM, API, auth, data, provider, navigation contract, or business state
changed.

## 2. Runtime boundary

- CSS: `windows/token_tracker/static/responsive-tuning.css`
- Isolated runtime: `windows/.cache/ui-v2500-runtime-20260812-5177/`
- Test URL: `http://127.0.0.1:5177/dashboard`
- A synthetic local account was used only through the local registration flow.
  Credentials, cookies, storage values, provider keys, and user data were not
  read or submitted.

## 3. Real browser evidence

The resting 1440px screenshot kept the complete scene visible through the
header. At 1440px scroll depth, the brand had a compact optical boundary while
the long form and activity headings remained readable underneath. At 390px,
the old scroll cue disappeared from the brand lane and the range switcher
remained usable below the sticky row.

### Computed transparency contract

```text
top header background: rgba(0, 0, 0, 0)
top header backdrop-filter: none
top brand background: rgba(0, 0, 0, 0)
top brand lens content: none
scrolled header background: rgba(0, 0, 0, 0)
scrolled header backdrop-filter: none
scrolled brand background: rgba(0, 0, 0, 0)
scrolled brand lens: rgba(7, 14, 24, .035) / blur(9px)
scrolled scroll cue opacity: 0
```

The right navigation rail remains the existing v79 local rail. v81 does not
turn the AI TOKEN row into an opaque horizontal panel.

## 4. Responsive matrix

All top-state checks used a real browser at the listed viewport dimensions.
`overflow` is the boolean result of `scrollWidth > clientWidth`.

| Viewport | Client / scroll width | Overflow | Header top | Nav |
|---:|---:|:---:|---|---|
| 320 | 305 / 305 | false | transparent | hidden |
| 390 | 375 / 375 | false | transparent | hidden |
| 768 | 753 / 753 | false | transparent | visible |
| 1024 | 1009 / 1009 | false | transparent | visible |
| 1440 | 1425 / 1425 | false | transparent | visible |

Scrolled-state checks also retained no overflow at 390px (`375 / 375`) and
1440px (`1425 / 1425`). The brand lens was active only after scroll and the
cue opacity was `0` in both states.

## 5. Accessibility and interaction checks

- Accessibility DOM snapshot length: 5,978 characters.
- The snapshot retained the skip link, banner, labeled navigation, main
  heading, period controls, chart status messages, forms, tables, and live
  status regions.
- Clicking `Week` set `aria-pressed="true"`; clicking `Today` restored the
  default pressed state.
- Page console logs: `[]`.
- The lens and cue transition are decorative; neither creates a focus stop or
  captures pointer input.
- The browser harness emitted external telemetry timeout messages while
  running repeated instrumentation. They were not page console logs or
  application errors; the page log check remained empty.

## 6. Explicit limits

- `prefers-reduced-motion` and `forced-colors` behavior is protected by source
  rules; those OS modes were not physically simulated in this pass.
- Physical devices, assistive technology, real provider calls, non-empty data
  charts, and production deployment were not claimed by this local visual
  check.
- No unit-test code, mock, fixture, harness, or test-only asset was created or
  changed.

## 7. Acceptance

**Status: PASS for v81 sticky brand readability scope.** The AI TOKEN header
remains transparent at rest and during scroll, while the brand identity no
longer competes with passing section copy. Remaining release gates stay in
`release-readiness.md` and `final-acceptance-matrix.md`.

# UI Accessibility Evidence — v79

> Owner: Frontend UI Engineer
> Reviewers: UI-1, UI-2, Architect
> Scope: scroll-safe Dashboard navigation rail
> Runtime: isolated local Flask instance on port 5175
> Date: 2026-08-12 (Asia/Shanghai)

## 1. Change intent

The Dashboard keeps the AI TOKEN brand row transparent by design. During a
scroll, however, the sticky navigation/account controls sat directly over
chart buttons, hero actions, and form content. The old fully transparent state
made those layers visually pass through one another and weakened the reading
order.

v79 adds a local, scroll-only reading rail for the right navigation/account
cluster:

- the header element itself remains `background: transparent` with no global
  backdrop filter;
- the brand area stays outside the rail and continues to show the artwork;
- the rail uses low-alpha deep-blue glass and `pointer-events: none`, so links
  and the account button remain the only interactive layer;
- desktop and tablet use separate start positions because tablet navigation
  compresses toward the brand;
- forced-colors hides the decorative rail and leaves system Canvas/Highlight
  colors in control.

No DOM, navigation contract, API, auth, data, or provider behavior changed.

## 2. Runtime boundary

- CSS: `windows/token_tracker/static/responsive-tuning.css`
- Isolated runtime: `windows/.cache/ui-v2300-runtime-20260812-5175/`
- Test URL: `http://127.0.0.1:5175/dashboard?ui_v2300=top-final-1440`
- A synthetic local account was created only through the local registration
  flow to observe the authenticated Dashboard. No real credentials, cookies,
  storage values, provider keys, or user data were read or submitted.

## 3. Real browser evidence

Screenshots were captured before and after the rail change at desktop scroll
positions, plus a tablet scroll screenshot. Before v79, navigation text and
the underlying call-to-action buttons occupied the same visual plane. After
v79, the right navigation/account cluster has a quiet glass boundary while the
left AI TOKEN brand remains scene-transparent.

### Resting top state

At every checked top position, the computed header remained:

```text
background: rgba(0, 0, 0, 0)
backdrop-filter: none
rail ::before content: none
```

### Scrolled state

At 1440px and 768px after scrolling, the computed header remained transparent
while the rail became active:

```text
rail backdrop-filter: blur(12px) saturate(1.06)
desktop rail: rgba(7, 14, 24, .04 → .12 → .24)
tablet rail: rgba(7, 14, 24, .03 → .09 → .18)
rail pointer-events: none
```

The measured tablet rail began before the compressed nav left edge, closing the
coverage gap found in the first v79 pass. The desktop rail stayed narrower and
near the actual nav/account geometry.

## 4. Responsive matrix

All checks used a real browser at 900px viewport height. `overflow` is the
boolean result of `scrollWidth > clientWidth`.

| Viewport | Client / scroll width | Overflow | Header top | Nav |
|---:|---:|:---:|---|---|
| 320 | 305 / 305 | false | transparent, no rail | hidden |
| 390 | 375 / 375 | false | transparent, no rail | hidden |
| 768 | 753 / 753 | false | transparent, no rail | visible |
| 1024 | 1009 / 1009 | false | transparent, no rail | visible |
| 1440 | 1425 / 1425 | false | transparent, no rail | visible |

Scrolled 768px and 1440px states both retained the rail without changing
document width or the header's computed background.

## 5. Accessibility and runtime checks

- Accessibility snapshot length: 5,962 characters.
- Snapshot retained the skip link, `banner`, labeled `主要导航`, four named
  navigation links, logout button, `main`, one level-one heading, nine total
  headings, period button group, chart status empty states, forms, tables, and
  live status regions.
- Browser page logs: `[]`.
- The rail is decorative, pointer-inert, and does not add DOM focus stops.
- Four period buttons remained present and `Today` retained its pressed state.
- No horizontal overflow occurred at 320/390/768/1024/1440px.

## 6. Explicit limits

- `prefers-reduced-motion`, `forced-colors`, physical devices, assistive
  technology, real provider calls, and production deployment were not claimed
  as verified by this local visual pass; forced-colors behavior is protected by
  source-level rules only.
- No unit-test code, mock, fixture, harness, or test-only asset was created or
  changed.

## 7. Acceptance

**Status: PASS for v79 local Dashboard navigation scope.** The brand remains
transparent at rest and the scrolling nav/account cluster no longer competes
with content beneath it. Remaining release gates stay tracked in
`release-readiness.md` and `final-acceptance-matrix.md`.

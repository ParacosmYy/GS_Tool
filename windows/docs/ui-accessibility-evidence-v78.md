# UI Accessibility Evidence — v78

> Owner: Frontend UI Engineer
> Reviewers: UI-1, UI-2, Architect
> Scope: authentication autofocus visual hierarchy
> Runtime: isolated local Flask instance on port 5174
> Date: 2026-08-12 (Asia/Shanghai)

## 1. Change intent

The login page autofocuses the username field on fine-pointer desktop screens
to preserve a fast keyboard path. Before v78, that first frame could expose a
bright lime `:focus-visible` outline, a lime input halo, and a card-level focus
ring at the same time. The result read as an alarm state before the user had
interacted with the page.

v78 adds an explicit autofocus visual state in `auth-focus.css`:

- the initial field uses a quiet lavender border, background tint, and one-pixel
  keyline;
- the automatic `:focus-visible` outline is suppressed only while the
  `data-auth-autofocus` marker is present;
- the existing full lime keyboard ring returns after the first intentional
  keyboard or pointer event removes the marker;
- forced-colors behavior remains owned by the existing system-color rule.

No authentication, form submission, API, DOM structure, or provider behavior
changed.

## 2. Runtime boundary

- CSS: `windows/token_tracker/static/auth-focus.css`
- Isolated runtime: `windows/.cache/ui-v2200-runtime-20260812-5174/`
- Test URL: `http://127.0.0.1:5174/login?ui_v2200=final`
- The browser only observed the local login page. No credentials, cookies,
  storage values, provider keys, or real user data were read or submitted.

## 3. Real browser evidence

The final browser pass captured desktop and mobile screenshots at 1440×900 and
390×900. The desktop screenshot shows the username field with a calm lavender
edge instead of the previous neon green outline. The mobile screenshot keeps
the form card readable and preserves the image-backed composition.

### Desktop computed state

At 1440px after page load:

```text
active: username
data-auth-autofocus: true
focus-visible: true
outline: none
border-color: rgba(196, 192, 255, 0.72)
background: rgba(196, 192, 255, 0.06)
box-shadow: 0 0 0 1px rgba(196, 192, 255, 0.16)
```

After pressing `Tab` on the username field:

```text
active: password
data-auth-autofocus: absent
focus-visible: true
outline-color: rgb(217, 255, 120)
```

This confirms that the change softens only the automatic first frame and does
not remove intentional keyboard focus feedback.

## 4. Responsive matrix

All checks used a real browser at 900px viewport height. `overflow` is the
boolean result of `scrollWidth > clientWidth`.

| Viewport | Client / scroll width | Overflow | Header background | Autofocus |
|---:|---:|:---:|---|---|
| 320 | 305 / 305 | false | transparent | disabled |
| 390 | 375 / 375 | false | transparent | disabled |
| 768 | 753 / 753 | false | transparent | enabled, quiet |
| 1024 | 1009 / 1009 | false | transparent | enabled, quiet |
| 1440 | 1425 / 1425 | false | transparent | enabled, quiet |

The card retained its existing `blur(12px) saturate(1.12)` glass treatment and
the page retained zero horizontal overflow at every checked width.

## 5. Accessibility and runtime checks

- Accessibility snapshot length: 1,079 characters.
- Snapshot retained the skip link, `banner`, `main`, one level-one heading,
  level-two login heading, labeled username/password textboxes, submit button,
  registration link, and `contentinfo` landmark.
- Browser page logs: `[]`.
- The first automatic focus remains keyboard reachable; the deliberate Tab path
  restores the full visible focus ring.
- The header remains `background: rgba(0, 0, 0, 0)` with no backdrop filter.

## 6. Explicit limits

- `prefers-reduced-motion`, `forced-colors`, physical devices, assistive
  technology, real provider calls, and production deployment were not claimed
  as verified by this local visual pass.
- No unit-test code, mock, fixture, harness, or test-only asset was created or
  changed.

## 7. Acceptance

**Status: PASS for v78 local authentication visual scope.** Automatic focus is
now quiet and intentional keyboard focus remains visible. Remaining release
gates stay tracked in `release-readiness.md` and
`final-acceptance-matrix.md`.

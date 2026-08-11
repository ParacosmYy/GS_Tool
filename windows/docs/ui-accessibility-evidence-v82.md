# UI Accessibility Evidence — v82

> Owner: Frontend UI Engineer
> Reviewers: UI-1, UI-2, Architect
> Scope: quiet pointer follower lifecycle
> Runtime: isolated local Flask instance on port 5178
> Date: 2026-08-12 (Asia/Shanghai)

## 1. Change intent

The decorative pointer aura previously remained at the last pointer position
and kept its animation loop alive while the user was reading a chart, form, or
history surface. v82 gives the follower an explicit lifecycle:

- it is hidden before the first fine-pointer movement;
- it becomes visible while the pointer is moving;
- after 1.8 seconds without movement, `pointer-idle` hides the decorative layer
  and cancels its animation frame;
- the next pointer movement restores the layer and the animation loop;
- the native cursor, hover styles, keyboard focus, DOM, API, and data state are
  unchanged.

The visibility state is presentation-only. The existing pointer nodes remain
pointer-inert and the existing reduced-motion/fine-pointer gates still apply.

## 2. Runtime boundary

- Source: `windows/token_tracker/static/modules/motion.js`,
  `scene-motion.css`, and the base pointer rule in `style.css`
- Isolated runtime: `windows/.cache/ui-v2601-runtime-20260812-5178/`
- Test URL: `http://127.0.0.1:5178/dashboard`
- A synthetic local account was used only through the local registration flow.
  Credentials, cookies, storage values, provider keys, and user data were not
  read or submitted.

## 3. Real browser evidence

The browser was observed at a 1668px document width and 892px viewport height.
The hero scene remained unobstructed after the follower was hidden. The state
sequence was checked with the browser's visible interaction surface:

| State | Body class | Ring opacity | Ring visibility |
|---|---|---:|---|
| Before movement | none | 1 | hidden |
| During movement | `pointer-ready` | 1 | visible |
| After 2.1s idle | `pointer-ready pointer-idle` | 1 | hidden |
| Next movement | `pointer-ready` | 1 | visible |

The page document reported `clientWidth=1668`, `scrollWidth=1668`, and no
horizontal overflow. `Week` set `aria-pressed="true"`; `Today` restored its
default pressed state. Accessibility DOM snapshot length was 5,971
characters, and application page logs were `[]`.

## 4. Accessibility and performance boundaries

- The pointer layer keeps `pointer-events: none`; it cannot capture focus or
  pointer input.
- The native cursor remains available and keyboard focus is not represented by
  the decorative follower.
- `prefers-reduced-motion` and non-fine-pointer devices continue to skip the
  follower setup.
- The idle timer cancels the follower's requestAnimationFrame loop, avoiding a
  permanent 60fps decorative loop while the page is being read.
- The four-breakpoint responsive gate, physical devices, assistive technology,
  forced-colors, reduced-motion simulation, provider calls, and production
  deployment were not claimed by this focused v82 check; prior responsive
  evidence remains the applicable baseline.
- No unit-test code, mock, fixture, harness, or test-only asset was created or
  changed.

## 5. Acceptance

**Status: PASS for v82 pointer follower lifecycle scope.** The follower no
longer leaves a stale visual artifact over content, returns on the next
movement, and stops its animation work while idle. Remaining release gates stay
in `release-readiness.md` and `final-acceptance-matrix.md`.

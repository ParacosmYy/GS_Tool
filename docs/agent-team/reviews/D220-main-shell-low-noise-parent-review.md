# D220 / UI-118 parent review: main-shell low-noise visual hierarchy

## Decision

`PASS` for the bounded centralized-QSS refinement, accepted with explicit
native-rendering and release limits.

## Evidence

- Exactly four existing shell selectors were refined in `presentation.theme`.
- Existing surface and accent tokens form the new flat surface ladder.
- No object name, widget behavior, signal, action, locale, font, motion, or
  state selector changed.
- The targeted matrix covers all 3 themes × 4 accents and retains readable
  primary text and accent-endpoint contrast.

## Simplification assessment

`PASS`: removing decorative gradients and reducing nested shell geometry is
the smallest complete visual change. No new token family, title-bar system,
or widget-local stylesheet is justified.

## Limits

The architecture and independent bounded windows returned `NO_CONCLUSION`.
Native Qt selector parsing/painting, layout metrics, accessibility, runtime,
and external release evidence remain unrun.

# D314 parent review — theme-aware combo-box expanded affordance

## Scope

Reviewed the shared `QComboBox::drop-down` and `QComboBox::down-arrow` QSS,
expanded-state selector order, explicit subcontrol geometry, the language/
theme/accent/font settings roles, the source contract, the 3-theme/4-accent
contrast matrix, and the rebuilt portable candidate.

## Findings

- PASS — the change stays in the centralized presentation stylesheet and
  applies to all existing combo boxes, including the localized and typography
  settings controls; no settings, persistence, item data, keyboard, popup,
  application, or startup behavior changed.
- PASS — `QComboBox::drop-down:on` aligns its open-state surface with the
  already-existing field `:on` and native arrow `:on` states, while hover,
  pressed, and disabled rules remain explicit.
- PASS — `subcontrol-origin: border` and `subcontrol-position: top right`
  make the button geometry intentional without adding an icon asset or a
  widget-local adapter.
- PASS — the static contract and 3-theme/4-accent matrix cover the new open
  state and keep `text_primary` on `pressed` above the 4.5 contrast floor.
- PASS — formatting, compilation, Ruff, presentation audit, source
  diagnostic, package identity, PE header, and frozen archive inventory
  passed.

## Simplification assessment

PASS. The smallest complete implementation is one shared subcontrol geometry
rule, one shared popup-open state, and one focused audit function. Adding
per-role selectors, custom arrow assets, event handlers, or popup adapters
would duplicate presentation ownership and widen behavior without fixing the
identified state gap.

## Limits and applicability

The architecture consultation returned `NO_CONCLUSION` after three bounded
Luna/max waits. The independent review returned `NO_CONCLUSION` after three
bounded Luna/max waits. Native EXE/Qt launch, popup rendering, keyboard
navigation, accessibility, DPI, alternate style engines, clean-machine
behavior, and release gates remain unverified. This is Python/PyQt6 desktop
code; embedded vendor applicability is N/A.


# ADR-0242: Native control affordance cohesion

- Status: accepted-with-limits
- Date: 2026-08-11
- Delivery: D194 / UI-104 / ARCH-180

## Context

The shell already owns palette tokens and stateful QSS for form controls, but
the small ComboBox and SpinBox arrows still came from the Fusion default. That
left the control language dependent on native style details instead of the
selected QuillForge theme. A checked checkbox could also lose its accent focus
boundary because its checked indicator rule was declared after the generic
focus rule.

## Decision

Keep `presentation.theme` as the single Qt stylesheet projection owner. Add
token-backed `QComboBox::down-arrow`, `QAbstractSpinBox::up-arrow`, and
`QAbstractSpinBox::down-arrow` geometry plus open/disabled color states, and
add an explicit checked-checkbox focus selector. Do not introduce image assets,
custom widget classes, a second style system, or behavior changes.

Qt's public [Qt Style Sheets Reference](https://doc.qt.io/qt-6/stylesheet-reference.html)
identifies these arrows and indicators as supported subcontrols, and its
customization guide explains that complex-widget subcontrols should be styled
as a coherent set.

## Preserved invariants

- ComboBox, SpinBox, and CheckBox construction, signals, keyboard behavior,
  values, settings persistence, and locale refresh are unchanged.
- All colors continue to resolve from `ThemeColors`; no raw theme-specific
  colors or widget-local stylesheet fragments were added.
- Hover/pressed/open/disabled visual differences remain on the parent control;
  the explicit arrows provide stable geometry and readable disabled states.
- Presentation remains dependent on existing contracts only; application,
  domain, infrastructure, and plugin ownership do not move.
- The change is reversible by removing one bounded QSS block.

## Review and applicability

The architecture consultation (`Huygens the 6th / Luna max`) returned no
conclusion after two bounded waits and was closed; no child PASS is claimed.
The independent review (`Bernoulli the 6th / Luna max`) likewise returned no
conclusion after two bounded waits and was closed. Parent review is `PASS`, and
the behavior-preserving simplification assessment is `PASS` because the
subcontrol rules are the smallest cohesive projection needed to remove the
mixed default affordances.

This is a Python 3.12/PyQt6 presentation change. Embedded C/C++, MCU, RTOS,
and manufacturer requirements are not applicable; the mandatory embedded
enterprise workflow is therefore recorded as not applicable to this source
slice. Public Qt documentation is the applicable framework source. Public
CloudWeGo material remains engineering reference only; this ADR makes no
private ByteDance standard, certification, or compliance claim.

## Evidence and limits

- `D194-CONTROL-AFFORDANCE-QSS-PROBE=PASS`
- `D194-CONTROL-AFFORDANCE-CONTRAST-PROBE=PASS`
- `D194-COMPILE-RUFF-FORMAT=PASS`
- `D194-PRESENTATION-AUDIT=PASS`
- `D194-SIMPLIFICATION-ASSESSMENT=PASS`
- `D194-PACKAGE-BUILD=PASS`
- `D194-PACKAGE-IDENTITY-PROBE=PASS`

Native QSS parsing/painting, accessibility-tree output, DPI, screenshot
review, GUI/EXE startup, clean-machine, cross-machine, legal, signing,
installer, updater, support, and release-owner evidence remain open. Release
verification remains `no-go` under the current no-launch and external-gate
policy.

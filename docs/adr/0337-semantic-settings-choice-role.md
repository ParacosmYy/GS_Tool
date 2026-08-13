# ADR-0337: Give Settings identity choices one semantic presentation role

## Status

Accepted with limits — D301 / UI-120 / ARCH-271.

## Context

D300 made the theme and accent selectors visually distinct through dedicated
`settingsTheme` and `settingsAccent` QSS selectors. That solved the immediate
hierarchy gap, but concrete widget IDs would require another selector pair for
every future Settings identity choice and would make the visual contract depend
on object naming rather than presentation meaning.

## Decision

Keep the existing `settingsTheme` and `settingsAccent` object names, item/data
values, icons, signals, and snapshot contract. Assign both controls the one
dynamic property `settingsRole="identityChoice"` before item population, and
scope the normal, hover, focus, and open-menu hierarchy through that semantic
QSS selector in centralized `presentation.theme`.

The shared disabled selector remains ID-based for compatibility with the
existing generic disabled-state contract. The audit has one canonical
identity-choice helper: a small state table checks the semantic selector and
visual tokens, while separate checks cover property declaration order and
legacy selector removal. No new QSS engine, widget subclass, settings policy,
or runtime styling owner is introduced.

Qt's `QObject::setProperty` and dynamic-property stylesheet selector guidance
are applicable framework references:

- https://doc.qt.io/qt-6/qobject.html
- https://doc.qt.io/qt-6/stylesheet-syntax.html
- https://doc.qt.io/qt-6/stylesheet-reference.html

They are engineering references for this Python 3.12/PyQt6 desktop UI, not
manufacturer requirements.

## Consequences

- Future identity choices can opt into the same visual role without coupling
  the centralized QSS to another concrete object ID.
- D300's visible surface, accent edge, stronger weight, hover/focus/open states,
  and disabled compatibility remain available.
- Dynamic properties are set before the application stylesheet is projected,
  so this slice does not introduce a runtime repolish or event-routing seam.
- Native Qt painting, property-polish timing across alternate style engines,
  accessibility, DPI/font metrics, clean-machine behavior, and release gates
  remain unverified under the active `software_start_allowed=false` policy.

## Public-source applicability

This slice changes Python/PyQt6 presentation code only. Public embedded-vendor
source applicability is N/A: no embedded C/C++, MCU, BSP/HAL, RTOS, protocol,
manufacturer requirement, MISRA, ISO 26262, ASPICE, certification, or private
ByteDance-standard claim is made.

## Verification boundary

The required evidence is the semantic-property source contract, canonical
QSS/state audit, 3-theme × 4-accent contrast matrix, compile/Ruff/format,
source startup and file-open diagnostics, package identity, PE/archive
inspection, project/handoff checks, review/simplification records, and the
expected release no-go result. Native EXE/Qt startup remains outside the
authorized validation boundary.

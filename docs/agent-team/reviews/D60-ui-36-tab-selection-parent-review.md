# D60 parent review — UI-36 document-tab selection hierarchy

| Field | Value |
|---|---|
| Delivery | `D60 / UI-36` |
| Decision | `accepted-with-limits` |
| Owner | `Architect` |
| Checkout | Current local checkout only |

## User and visual outcome

The active document tab is now visually scannable: selection has a distinct
surface, accent border, and leading accent marker; selected hover has a readable
filled accent state; the tab rail and workspace dock title have clear boundaries.

## Role and review record

| Role | Agent | Result |
|---|---|---|
| Architect | Darwin the 2nd / Luna max | Two bounded waits returned no conclusion; no architecture PASS claimed |
| Independent reviewer | Hubble the 2nd / Luna max | Two bounded waits returned no conclusion; no independent PASS claimed |
| Parent | Architect | Sole writer, integrated, inspected, and statically verified the QSS slice |

No child PASS is claimed.

## Contract and boundary

Only `presentation/theme.py::_stylesheet()` changed. It reuses
`selection`, `accent`, `accent_alt`, `accent_pink`, `on_accent`, and `border`
tokens. It does not change DocumentTabSurface, tab signals, close behavior,
settings, locale, motion, editor colors, warning foreground derivation, or
application policy.

## Simplification assessment

The visual refinement reuses the existing centralized token system and adds no
new theme field, style engine, widget state, or alternate selector owner. The
smallest readable hierarchy is therefore the complete change; no additional
safe simplification or abstraction was identified.

## Public-source applicability and embedded gate

This is Python/PyQt6 presentation code only. Embedded C/C++ assurance and
manufacturer-source applicability are `N/A`; no MISRA, ISO, certification, or
private ByteDance-standard claim is made. Public CloudWeGo material is only an
engineering reference.

## Authorized non-destructive validation

- `UI-36-tab-selection-hierarchy-probe=PASS`.
- `UI-36-tab-selection-contrast-probe=PASS` for 3 themes × 4 accents.
- Targeted and full compileall, Ruff, and format checks — `PASS` for the
  source slice before final documentation/package synchronization.
- Package identity, handoff, repository check, and expected release NO-GO
  evidence are recorded after the final package.
- No QApplication/Qt/EXE launch, screenshots, unit tests, mocks, fixtures,
  harnesses, test-only assets, deployment, or hardware operation were run.

## Limits and disposition

Static QSS and color evidence cannot prove native style specificity, font
metrics, or rendered focus/hover behavior. The slice is accepted with those
limits; runtime visual and release gates remain open, and child reviews are
recorded as no-conclusion.

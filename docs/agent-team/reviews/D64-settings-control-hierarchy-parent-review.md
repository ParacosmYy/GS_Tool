# D64 parent review — settings control hierarchy

| Field | Value |
|---|---|
| Delivery | `D64 / UI-39` |
| Decision | `accepted-with-limits` |
| Owner | `Architect` |
| Checkout | Current local checkout only |

## User and visual outcome

The settings dialog now exposes two clear sections and two semantic control
families: appearance controls use an alternate-accent rail, editor controls use
a pink rail, and group titles remain readable in light and dark themes. This
directly improves the language/theme/font/size configuration surface.

## Role and review record

| Role | Agent | Result |
|---|---|---|
| Architect | James the 2nd / Luna max | Two bounded waits returned no conclusion; no architecture PASS claimed |
| Independent reviewer | Popper the 2nd / Luna max | Two bounded waits returned no conclusion; agent was closed; no independent PASS claimed |
| Parent | Architect | Sole writer, integrated, inspected, caught and corrected a light-theme contrast issue, then statically verified |

No child PASS is claimed.

## Contract and boundary

`SettingsDialog` only adds `settingsAppearanceGroup` and
`settingsEditorGroup` object names. Central QSS scopes the new borders to the
settings dialog and existing control object names. Settings ranges, values,
locale updates, signals, `SettingsSurface`, SettingsService/TaskRunner,
MainWindow policy, and persistence remain unchanged.

## Simplification assessment

The object-name plus QSS approach is the smallest safe refinement: no preview
model, token, widget, signal, or settings coordinator is introduced. No
further behavior-preserving simplification was identified.

## Public-source applicability and embedded gate

This is Python/PyQt6 presentation code only. Embedded C/C++ assurance and
manufacturer-source applicability are `N/A`; no MISRA, ISO, certification, or
private ByteDance-standard claim is made. Public CloudWeGo material is only an
engineering reference.

## Authorized non-destructive validation

- `D64-settings-visual-contract-probe=PASS`.
- Initial contrast probe found Paper/Sand accent-colored titles below 4.5:1;
  the implementation was corrected to use `text_primary` for title text while
  retaining accent borders.
- `D64-settings-contrast-probe=PASS` for 3 themes × 4 accents × 4 states.
- Targeted compileall, Ruff, and format checks — `PASS` before final
  documentation/package synchronization.
- Full compileall, handoff, repository check, package identity, and expected
  release NO-GO evidence are recorded after final synchronization.
- No QApplication/Qt/EXE launch, screenshots, unit tests, mocks, fixtures,
  harnesses, test-only assets, deployment, or hardware operation were run.

## Limits and disposition

Static QSS and token evidence cannot prove native rendering, runtime settings
interaction, or font metrics. The slice is accepted with those limits;
runtime and release gates remain open, and delegated reviews are recorded as
no-conclusion.

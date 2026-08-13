# D55 parent review — UI-35 warning-background foreground contract

| Field | Value |
|---|---|
| Delivery | `D55 / UI-35` |
| Decision | `accepted-with-limits` |
| Owner | Architect |
| Checkout | Current local checkout only |

## User outcome

Warning-background text no longer reuses the decorative 砂金 endpoint or a
second generic foreground. The transient warning message, attention phase,
workspace/search warning feedback, workspace cancel hover/focus, warning
action, and Find status now receive a foreground selected for their actual
warning background.

## Architecture decision

The change remains inside `presentation/theme.py`. `_stylesheet()` derives one
local `warning_foreground` through the existing pure `_best_on_accent()` helper
and uses it for every warning-background text selector. `ThemeColors`,
settings, locale, motion, signals, command routing, and application policy are
unchanged. Gold borders and the filled warning-action hover continue to use
their existing endpoint tokens.

## Role and review record

| Role | Agent | Result |
|---|---|---|
| Architect | Hypatia the 2nd / Luna max | Two bounded waits returned no conclusion; no architecture PASS claimed |
| Independent reviewer | Kuhn the 2nd / Luna max | Two bounded waits returned no conclusion; no independent PASS claimed |
| Follow-up architect | Hegel the 2nd / Luna max | Final scope-extension review returned no conclusion after two bounded waits |
| Follow-up independent reviewer | Kierkegaard the 2nd / Luna max | Final scope-extension review returned no conclusion after two bounded waits |
| Parent | Architect | Integrated, inspected, and verified the smallest source change |

The parent owns the final source and evidence review. Neither child result is
represented as approval.

The final scope-extension review windows also returned no conclusion; no child
PASS is claimed for either follow-up.

## Root cause and simplification assessment

Warning text selectors used either a decorative gold endpoint or a generic
foreground directly even though their background was a separate semantic
token. This made the state fragile to palette changes and matched the reported
砂金 readability regression. The smallest complete fix is one local derivation
and six QSS block substitutions covering seven semantic selectors. Extending
the theme schema, adding a second
contrast engine, or moving warning policy into a widget would add coupling and
was not justified by the single consumer.

## Public-source applicability and embedded gate

This is Python/PyQt6 presentation code only. The embedded C/C++ assurance gate
is `N/A`; no MCU/vendor requirement or certification claim applies. Public
CloudWeGo material is recorded only as transferable engineering reference, not
as a private ByteDance standard.

## Authorized non-destructive validation

- `UI-35-warning-foreground-contrast-probe=PASS`: 12 theme/accent warning
  foreground pairs meet the 4.5:1 static threshold.
- `UI-35-warning-foreground-boundary-probe=PASS`: all warning-background text
  blocks use the derived foreground while the warning-action gold hover
  contract remains present.
- `UI-35-source-static=PASS` — compileall, Ruff, and format check for
  `theme.py`.
- Full compileall, Ruff, format, handoff, repository check, package identity,
  and expected release NO-GO evidence are recorded after the final package.
- UI-35 package identity: root/dist SHA-256
  `3C24352AC8F8432B15F0C9991323D74144755F0EE10CAC12EDAA89B3767C49ED`,
  `38,432,820` bytes, source
  `tree-sha256:92e997bb02b927abba201df1eef75917855b47e5b98b084dac88f85d4a455602`.
- No QApplication/Qt/EXE launch, screenshots, unit tests, mocks, fixtures,
  harnesses, test-only assets, deployment, or hardware operation were run.

## Limits and disposition

Static QSS generation cannot prove native style specificity, font rasterization,
DPI behavior, accessibility output, or perceived runtime appearance. The slice
is accepted with those limits; the release dossier remains NO-GO until its
independent external gates are closed.

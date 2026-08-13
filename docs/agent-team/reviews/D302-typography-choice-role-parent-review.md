# D302 parent review — Typography choice role

## Scope

Reviewed the six Settings typography controls, dynamic-property declaration
order, current value and preview paths, centralized QSS state ownership,
interface/editor tone cues, contrast fallback, disabled precedence, audit
contract, source diagnostics, and package boundary.

## Findings

- PASS — all six controls retain their object names and declare
  `typographyChoice` plus the correct interface/editor tone before values,
  ranges, font previews, or style options are configured.
- PASS — `SettingsSnapshot` still reads `currentText()`, `currentData()`, and
  `value()`; item data, font previews, locale refresh, persistence, preview
  refresh, signals, and keyboard behavior remain in their existing owners.
- PASS — centralized QSS provides one normal/hover/focus/open/disabled role
  contract; interface/editor tone-specific edges are independent selectors,
  not cascade-overwrite behavior.
- PASS — the existing ID-based typography active-state selectors are removed;
  the shared disabled compatibility selector remains last and role-based.
- PASS — the edge fallback uses the existing token layer and passes the 3:1
  non-text floor across 3 themes × 4 accents; text states pass 4.5:1.
- PASS — source startup and README file-open diagnostics pass without showing
  a window or entering the event loop.

## Review correction

The first independent review returned `REVISE`: it identified that the initial
open-state rule collapsed the two tone cues and that the audit did not verify
tone-specific values. The parent added `readable_edge_foreground()`, separate
interface/editor open selectors, and explicit audit assertions; the final
static audit and matrix pass.

## Simplification assessment

PASS. The role/tone metadata removes six duplicated active-state selector
groups and reuses one pure token helper. A custom typography widget, new
settings model, generic QSS parser, or second palette policy would add
coupling without improving behavior.

## Limits

The initial architecture consultation and follow-up architecture consultation
both returned `NO_CONCLUSION` after three bounded waits; the parent retained
the existing presentation/theme-token boundary. The independent follow-up
review returned `NO_CONCLUSION` after three bounded waits; no independent PASS
is claimed. Native Qt/EXE rendering, focus/accessibility, DPI/font fallback,
clean-machine behavior, and real DLL loading remain unverified. Unit tests,
mocks, fixtures, and test harnesses were not created or run under policy.

## Public-source applicability

Qt dynamic-property stylesheet documentation and WCAG 2.2 SC 1.4.11 are
public engineering references. Embedded vendor source applicability is N/A;
no embedded compliance claim is made.

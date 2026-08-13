# D281 parent review — startup diagnostic user guidance

## Scope

Reviewed the README change against `src/quillforge/app.py`, the D280 handoff,
and the active no-launch policy.

## Findings

- PASS — the documented command and `--report` argument match the existing
  startup dispatcher.
- PASS — `settings_preflight` wording matches the actual four metadata fields:
  path, presence, decode validity, and normalized schema version.
- PASS — the README explicitly says the diagnostic does not write settings or
  include locale, theme, accent, font, or motion values.
- PASS — the README does not claim native window creation or release approval;
  it retains the startup-error log path and exit-code contract.
- CORRECTED — independent review initially found the missing argument-parse
  exit branch and ambiguous “only” wording; both were fixed and the bounded
  follow-up review returned PASS.

## Simplification assessment

PASS. The change is limited to one explanatory paragraph and one boundary
paragraph. No new command, wrapper, duplicated runtime logic, or API was
introduced.

## Review applicability

This is documentation-only work. Python standard-library behavior and the
existing source contract are the applicable public references; embedded
enterprise workflow is `N/A`. The initial independent revise and follow-up
pass are recorded separately.

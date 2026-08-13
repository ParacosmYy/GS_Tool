# Handoff: 2026-08-11-d191-dynamic-find-localization

| Field | Value |
|---|---|
| ID | `2026-08-11-d191-dynamic-find-localization` |
| Delivery / slice | `D191 / UI-101 / ARCH-177 Dynamic Find/Replace status localization closure` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-11T23:59:00+08:00` |

## User outcome

The Simplified Chinese Find/Replace surface no longer shows an English plural
tail or an entirely English Replace All limit message. Counts remain visible,
while English keeps its original wording.

## Scope and boundaries

### In scope

- Strict dynamic-message matching in `presentation/i18n.py`.
- Reuse of the existing `find.status.replaced_count` and `find.status.limit`
  English/Simplified Chinese catalog entries.
- Removal of the unsafe generic `Replaced ` prefix fallback.

### Out of scope

- Editor algorithms, Replace All limits, cancellation, rollback, signals,
  status severity, persistence, application coordinators, and locale backend
  changes.
- Native Qt metrics, accessibility-tree output, DPI, GUI/EXE startup,
  screenshots, clean-machine, cross-machine, legal, signing, installer,
  updater, support, and release-owner acceptance.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | `parent` | Integration, final review, verification, and handoff decision |
| Project Manager | `parent` | Plan, dependency, risk, and status record |
| Product | `user outcome` | Complete Chinese Find/Replace feedback |
| Developer | `parent` | Presentation localization boundary |
| QA | `parent` | Read-only source, static, handoff, and package verification |

## Changed files and modules

- `src/quillforge/presentation/i18n.py` — bounded dynamic Find/Replace
  matchers and catalog delegation.

## Decisions and constraints

- Shared checkout writer: `parent`, one bounded presentation slice.
- Runtime launch policy: not allowed; no GUI, EXE, screenshot, native popup,
  or test-only asset was run/created.
- Architecture window: `Linnaeus the 6th / Luna max` — `NO_CONCLUSION` after
  bounded timeout; no child PASS is claimed.
- Independent review: `Ampere the 6th / Luna max` — `NO_CONCLUSION` after
  bounded timeout and closure; no child PASS is claimed.
- Parent review: `PASS`; simplification assessment: `PASS`.
- Python/PyQt6 presentation only; embedded C/C++, MCU, RTOS, and manufacturer
  requirements are not applicable.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| Dynamic matcher locale probe | `PASS` | `D191-I18N-MATCHER-PROBE=PASS`; English identity, Chinese plural/limit, count preservation, and unknown-shape fallback. |
| `uv run python -m compileall -q src` | `PASS` | No GUI startup. |
| `uv run ruff check src scripts` | `PASS` | No lint diagnostics. |
| `uv run ruff format --check src scripts` | `PASS` | All files already formatted. |
| Presentation contract audit | `PASS` | `D191-PRESENTATION-AUDIT=PASS`. |
| `scripts/package.ps1` | `PASS` | Portable candidate rebuilt; EXE not launched. |
| `scripts/verify_release_handoff.ps1` | `EXPECTED NO-GO` | Artifact-bound runtime reports and external release gates remain open. |

## Unrun checks and reason

- Native Qt text metrics, accessibility, DPI, screenshot comparison, and
  GUI/EXE startup — prohibited by the current no-launch policy.
- Clean-machine, cross-machine, signing, installer, updater, legal, support,
  permission/disk-pressure, and release-owner checks — external authorization
  or evidence remains pending.

## Known risks and limits

- A future locale should add catalog-backed message shapes rather than using
  generic prefix translation for count-bearing grammar.
- Native font metrics may wrap longer localized status text at unusual DPI or
  with unusually large selected fonts.
- Existing release status remains `no-go`.

## Acceptance and evidence IDs

- Acceptance: `S243`
- Evidence: `D191-I18N-MATCHER-PROBE=PASS`,
  `D191-COMPILE-RUFF-FORMAT=PASS`, `D191-PRESENTATION-AUDIT=PASS`,
  `D191-SIMPLIFICATION-ASSESSMENT=PASS`, `D191-PACKAGE-IDENTITY-PROBE=PASS`,
  `D191-ARCHITECT-REVIEW=NO_CONCLUSION`,
  `D191-INDEPENDENT-REVIEW=NO_CONCLUSION`.

## Next owner and next action

- Owner: QA / Product
- Action: authorize native text-metric, accessibility, and runtime locale
  review before treating this as runtime accepted.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and `QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size: `48F129224E47D7D97A3A53968288666D3E429BA78AD828AA9B25F797A7F70A8B` /
  `38,561,061` bytes
- Source revision: `tree-sha256:f26cedfdc38cf432f2f39adb19e3f12cc1f2f61e74f3814b626c3fa9b53d19a7`
- Packaging note: portable PyInstaller one-file candidate rebuilt; no
  installer or updater artifact is claimed.

## Disposition

`accepted-with-limits`: the dynamic Find/Replace localization boundary and
static evidence are recorded; native rendering, runtime, and enterprise
release gates remain open.

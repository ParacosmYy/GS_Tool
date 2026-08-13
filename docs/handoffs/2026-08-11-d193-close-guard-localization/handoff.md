# Handoff: 2026-08-11-d193-close-guard-localization

| Field | Value |
|---|---|
| ID | `2026-08-11-d193-close-guard-localization` |
| Delivery / slice | `D193 / UI-103 / ARCH-179 Close-guard pending feedback localization` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-11T23:59:00+08:00` |

## User outcome

When closing is blocked by retained background work, Simplified Chinese now
shows a complete count-bearing message instead of an English sentence. Close
behavior and worker safety remain unchanged.

## Scope and boundaries

### In scope

- `error.wait_pending` English/Simplified Chinese catalog entries.
- Strict count-bearing matching in `presentation.i18n`.
- Existing MessageSurface localization route.

### Out of scope

- CloseGuard, TaskRunner, session save, pending-work count, worker draining,
  shutdown policy, and notification severity.
- Native Qt metrics, accessibility-tree output, DPI, GUI/EXE startup,
  screenshots, clean-machine, cross-machine, legal, signing, installer,
  updater, support, and release-owner acceptance.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | `parent` | Integration, final review, verification, and handoff decision |
| Project Manager | `parent` | Plan, dependency, risk, and status record |
| Product | `user outcome` | Complete Chinese close-block feedback |
| Developer | `parent` | Presentation localization adapter |
| QA | `parent` | Read-only source, static, handoff, and package verification |

## Changed files and modules

- `src/quillforge/presentation/i18n.py` — catalog entry and bounded pending
  close-guard matcher.

## Decisions and constraints

- Shared checkout writer: `parent`, one bounded presentation slice.
- Runtime launch policy: not allowed; no GUI, EXE, screenshot, native popup,
  or test-only asset was run/created.
- Architecture window: `Raman the 6th / Luna max` — `NO_CONCLUSION` after
  bounded timeout; no child PASS is claimed.
- Independent review: `Aquinas the 6th / Luna max` — `NO_CONCLUSION` after
  bounded timeout and closure; no child PASS is claimed.
- Parent review: `PASS`; simplification assessment: `PASS`.
- Python/PyQt6 presentation only; embedded C/C++, MCU, RTOS, and manufacturer
  requirements are not applicable.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| Close-guard locale probe | `PASS` | `D193-CLOSE-GUARD-I18N-PROBE=PASS`; count preservation, English identity, and unknown-shape fallback. |
| `uv run python -m compileall -q src` | `PASS` | No GUI startup. |
| `uv run ruff check src scripts` | `PASS` | No lint diagnostics. |
| `uv run ruff format --check src scripts` | `PASS` | All files already formatted. |
| Presentation contract audit | `PASS` | `D193-PRESENTATION-AUDIT=PASS`. |
| `scripts/package.ps1` | `PASS` | Portable candidate rebuilt; EXE not launched. |
| `scripts/verify_release_handoff.ps1` | `EXPECTED NO-GO` | Artifact-bound runtime reports and external release gates remain open. |

## Unrun checks and reason

- Native Qt text metrics, accessibility, DPI, screenshot comparison, GUI/EXE
  startup, and close interaction — prohibited by the current no-launch policy.
- Clean-machine, cross-machine, signing, installer, updater, legal, support,
  permission/disk-pressure, and release-owner checks — external authorization
  or evidence remains pending.

## Known risks and limits

- A future close-guard wording change must update the strict matcher and catalog
  together; unknown shapes intentionally remain visible in English.
- Native font metrics may wrap longer localized close messages at unusual DPI
  or with unusually large selected fonts.
- Existing release status remains `no-go`.

## Acceptance and evidence IDs

- Acceptance: `S245`
- Evidence: `D193-CLOSE-GUARD-I18N-PROBE=PASS`,
  `D193-COMPILE-RUFF-FORMAT=PASS`, `D193-PRESENTATION-AUDIT=PASS`,
  `D193-SIMPLIFICATION-ASSESSMENT=PASS`, `D193-PACKAGE-IDENTITY-PROBE=PASS`,
  `D193-ARCHITECT-REVIEW=NO_CONCLUSION`,
  `D193-INDEPENDENT-REVIEW=NO_CONCLUSION`.

## Next owner and next action

- Owner: QA / Product
- Action: authorize native text-metric, accessibility, and runtime close-flow
  review before treating this as runtime accepted.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and `QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size: `B8B0B1B7D5E935D367A70949EECD6B56F64E6092009820D2380018C5EED56540` /
  `38,560,614` bytes
- Source revision: `tree-sha256:affcc2b30466bfb5f3075797a578d0ddfbe26a2af5456a63b1f2135e1402d102`
- Packaging note: portable PyInstaller one-file candidate rebuilt; no
  installer or updater artifact is claimed.

## Disposition

`accepted-with-limits`: the close-guard pending feedback localization boundary
and static evidence are recorded; native rendering, runtime, and enterprise
release gates remain open.

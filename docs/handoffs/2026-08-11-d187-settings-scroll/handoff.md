# Handoff: 2026-08-11-d187-settings-scroll

| Field | Value |
|---|---|
| ID | `2026-08-11-d187-settings-scroll` |
| Delivery / slice | `D187 / UI-98 / ARCH-174 Responsive Settings scroll boundary` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-11T23:59:00+08:00` |

## User outcome

The Settings surface keeps its appearance controls, live preview, editor
controls, and guidance reachable in a short window or with larger selected
fonts. The content scrolls inside one named viewport while Save and Cancel
remain visible in the outer action rail.

## Scope and boundaries

### In scope

- `SettingsDialog` composition of the existing settings content.
- Named `settingsContent` and `settingsScroll` presentation identities.
- Resizable vertical scrolling with horizontal overflow disabled.
- Scoped transparent QSS for the Settings viewport and content surface.

### Out of scope

- New settings fields, ranges, schema, persistence, locale, signals, theme
  application, motion policy, editor behavior, or application ownership.
- Native Qt scroll metrics, keyboard traversal, screenshots, GUI/EXE startup,
  clean-machine, cross-machine, or release acceptance.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | `parent` | Integration, final review, verification, and handoff decision |
| Project Manager | `parent` | Plan, dependency, risk, and status record |
| Product | `user outcome` | Comfortable Settings use at short heights and larger fonts |
| Developer 1 | `parent` | SettingsDialog scroll composition |
| Developer 2 | `parent` | Scoped theme projection and package identity |
| QA | `parent` | Read-only source, static, handoff, and package verification |

## Changed files and modules

- `src/quillforge/presentation/settings_dialog.py` — content/viewport/action
  composition.
- `src/quillforge/presentation/theme.py` — scoped Settings scroll QSS.

## Decisions and constraints

- Shared checkout writer: `parent`, one bounded presentation slice.
- Runtime launch policy: not allowed; no GUI, EXE, screenshot, native scroll
  inspection, or test-only asset was run/created.
- Architecture window: Herschel the 6th / Luna max — `NO_CONCLUSION` after
  bounded timeout; no child PASS is claimed.
- Independent review: Jason the 6th / Luna max — `NO_CONCLUSION` after
  bounded timeout and closure; no child PASS is claimed.
- Parent review: `PASS`; simplification assessment: `PASS`.
- Python/PyQt6 presentation only; embedded C/C++, MCU, RTOS, and manufacturer
  requirements are not applicable.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| Settings scroll source probe | `PASS` | `D187-SETTINGS-SCROLL-PROBE=PASS`; content and action containment are explicit. |
| `uv run python -m compileall -q src` | `PASS` | No GUI startup. |
| `uv run ruff check src scripts` | `PASS` | No lint diagnostics. |
| `uv run ruff format --check src scripts` | `PASS` | All files already formatted. |
| `scripts/package.ps1` | `PASS` | Portable candidate rebuilt; EXE not launched. |
| `scripts/check.ps1` | `PASS` | Full repository static/handoff checks pass. |
| `scripts/verify_handoff.ps1` | `PASS` | Handoff identity and index checks pass. |
| `scripts/verify_release_handoff.ps1` | `EXPECTED NO-GO` | Artifact-bound runtime report and external release gates remain open. |

## Unrun checks and reason

- Native scroll viewport painting, size hints, focus traversal, accessibility,
  DPI, screenshot comparison, and GUI/EXE startup — prohibited by the current
  no-launch policy.
- Clean-machine, cross-machine, signing, installer, updater, legal, support,
  and release-owner checks — external authorization/evidence remains pending.

## Known risks and limits

- Native Qt size hints or scrollbar metrics may require a follow-up adjustment
  at narrow DPI or with unusually large system fonts.
- Horizontal overflow is intentionally hidden; the current 540px minimum width
  and existing compact form controls remain the static layout contract.
- Existing release status remains `no-go` with artifact-bound runtime and
  external enterprise gates open.

## Acceptance and evidence IDs

- Acceptance: `S240`
- Evidence: `D187-SETTINGS-SCROLL-PROBE=PASS`,
  `D187-COMPILE-RUFF-FORMAT=PASS`, `D187-SIMPLIFICATION-ASSESSMENT=PASS`,
  `D187-PACKAGE-BUILD=PASS`, `D187-PACKAGE-IDENTITY-PROBE=PASS`,
  `D187-CHECK=PASS`, `D187-VERIFY-HANDOFF=PASS`,
  `D187-RELEASE-DOSSIER-INVARIANT-PROBE=PASS`,
  `D187-ARCHITECT-REVIEW=NO_CONCLUSION`,
  `D187-INDEPENDENT-REVIEW=NO_CONCLUSION`.

## Next owner and next action

- Owner: QA / Product
- Action: authorize native short-height, larger-font, focus-traversal, and
  accessibility review before treating the responsive boundary as runtime
  accepted.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and `QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size: `6DF4C23152B69077AFF13463AA5125B0E82E2CD6707FA1E7541D16DC47D65CFB` /
  `38,557,988` bytes
- Source revision: `tree-sha256:102dddd3df2254b9f9854c5d270f126a441b248762116ca5e2d650b24fce7dcf`
- Packaging note: portable PyInstaller one-file candidate rebuilt; no
  installer or updater artifact is claimed.

## Disposition

`accepted-with-limits`: the Settings scroll boundary and static evidence are
recorded; native rendering, runtime, and enterprise release gates remain open.

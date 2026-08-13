# Handoff: 2026-08-11-d201-disabled-menu-state

| Field | Value |
|---|---|
| ID | `2026-08-11-d201-disabled-menu-state` |
| Delivery / slice | `D201 / UI-106 / ARCH-187 Disabled menu state hierarchy` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-11T23:59:00+08:00` |

## User outcome

Disabled menu actions no longer inherit a bright accent/selection background
with a low-contrast muted label. Selected and checked disabled items now use a
quiet elevated surface with an authored left boundary, so unavailable actions
remain legible and visibly distinct.

## Scope and boundaries

### In scope

- The centralized `QMenu` disabled-state QSS projection in `theme.py`.
- Static selector-order, 12-combination contrast, compile, package, and
  artifact identity evidence.

### Out of scope

- QAction enablement, command registry, menu projection, labels, shortcuts,
  callbacks, locale, settings, motion, GUI/QApplication, native rendering,
  screenshots, runtime accessibility, and external release gates.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | `parent` | Integration, architecture boundary, final review, handoff |
| Project Manager | `parent` | Plan, dependency, risk, and status record |
| Product | `user outcome` | Legible unavailable menu actions |
| Developer | `parent` | Centralized QSS selector change |
| QA | `parent` | Static, contrast, package, and release-boundary checks |

## Changed files and modules

- `src/quillforge/presentation/theme.py` — selected/checked disabled menu
  override.
- Synchronized ADR, review, acceptance, register, roadmap, and handoff files.

## Decisions and constraints

- Shared checkout writer: `parent`; no worktree or second writer was used.
- Keep the fix in the existing presentation token/QSS owner; no new widget,
  dynamic property, palette branch, or application dependency.
- Architecture window: `Ramanujan the 6th / Luna max` — `NO_CONCLUSION` after
  two bounded waits and closure; no child architecture PASS is claimed.
- Independent review: `Harvey the 6th / Luna max` — `NO_CONCLUSION` after two
  bounded waits and closure; no independent PASS is claimed.
- Parent review: `PASS`; simplification assessment: `PASS`.
- Embedded C/C++, MCU, RTOS, and manufacturer requirements are not applicable.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| Generated QSS selector/order probe | `PASS` | `D201-MENU-DISABLED-QSS-PROBE=PASS combinations=12 selectors=2`. |
| Disabled foreground contrast probe | `PASS` | `D201-DISABLED-CONTRAST-PROBE=PASS minimum=text_muted/surface_2>=4.5`. |
| Compile, Ruff, format | `PASS` | `D201-COMPILE-RUFF-FORMAT=PASS`. |
| Windows PowerShell 5.1 package | `PASS` | SHA `B8F5796B1191B7BCF5AE04CAA2512A1FA38A57B66C11A77640866985FC0B8407`, 38,562,134 bytes; source revision matches PS7. |
| PowerShell 7 package | `PASS` | Final candidate SHA `BFAC209D26B4190515957D2C057CF1DBA22B81F87581E08650722644C2A3CAE6`, 38,562,358 bytes; source revision matches PS5. |
| Root/dist/package identity | `PASS` | `D201-PACKAGE-IDENTITY-PROBE=PASS`; root and dist match. |

## Unrun checks and reason

GUI/QApplication, native Qt menu rendering, screenshots, accessibility tree,
DPI, EXE startup, clean-machine, cross-machine, signing, installer/updater,
legal, support, permission/disk-pressure, hard-power, and release-owner checks
were not run under the active no-launch or external-authorization policy. No
unit-test asset was created or run.

## Known risks and limits

- Native Qt selector specificity and platform menu metrics still require an
  authorized runtime visual check.
- The checkout has no Git baseline, and both delegated review windows returned
  `NO_CONCLUSION`; parent review is the only PASS review claim.
- Separate PyInstaller invocations can differ in artifact bytes; each manifest
  binds its own artifact. The final PS7 candidate is the current identity.

## Acceptance and evidence IDs

- Acceptance: `S253`
- Evidence: `D201-MENU-DISABLED-QSS-PROBE=PASS combinations=12 selectors=2`,
  `D201-DISABLED-CONTRAST-PROBE=PASS minimum=text_muted/surface_2>=4.5`,
  `D201-COMPILE-RUFF-FORMAT=PASS`, `D201-PACKAGE-BUILD-PS51=PASS`,
  `D201-PACKAGE-BUILD-PS7=PASS`, `D201-PACKAGE-IDENTITY-PROBE=PASS`,
  `D201-ARCHITECT-REVIEW=NO_CONCLUSION`,
  `D201-INDEPENDENT-REVIEW=NO_CONCLUSION`.

## Next owner and next action

- Owner: QA / Product.
- Action: authorize native menu rendering review when runtime evidence is
  allowed; retain this QSS state as the fallback static contract.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and `QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size: `BFAC209D26B4190515957D2C057CF1DBA22B81F87581E08650722644C2A3CAE6` /
  `38,562,358` bytes
- Source revision: `tree-sha256:cecf78580582c4a2feba27753b93022678d4d581441f31d95717291afc2ece90`
- PS5 package evidence: `B8F5796B1191B7BCF5AE04CAA2512A1FA38A57B66C11A77640866985FC0B8407` / `38,562,134` bytes; the final current candidate is the PS7 package.

## Disposition

`accepted-with-limits`: disabled selected/checked menu readability is closed
as a static token/QSS contract; native runtime and enterprise release gates
remain open.

## Required final marker

`EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded`

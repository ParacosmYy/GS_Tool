# Handoff: 2026-08-11-d211-workspace-disabled-item

| Field | Value |
|---|---|
| ID | `2026-08-11-d211-workspace-disabled-item` |
| Delivery / slice | `D211 / UI-113 / ARCH-196 Workspace disabled-item hierarchy` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-11T23:59:00+08:00` |

## User outcome

Inaccessible workspace entries and the disabled overflow marker now read as
non-actionable rows through a subdued surface, neutral left boundary, and
readable muted text. Selected-disabled rows, file/folder activation, loading,
locale, and icons retain their existing behavior.

## Scope and boundaries

### In scope

- One scoped disabled-row selector in `presentation/theme.py`.
- Source/specificity/contrast probes, static checks, package identity, and
  release-boundary records.

### Out of scope

- Workspace provider/service policy, custom delegates, item data, signals,
  filesystem behavior, GUI/QApplication, EXE startup, screenshots,
  accessibility, DPI, unit-test assets, and release closure.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | `parent` | Boundary decision, integration, final review, handoff |
| Project Manager | `parent` | Plan, dependency, risk, and status record |
| Product | `user outcome` | Make disabled workspace rows visually unmistakable |
| Developer | `parent` | Centralized QSS implementation |
| QA | `parent` | Static, contrast, package, and release-boundary checks |

## Changed files and modules

- `src/quillforge/presentation/theme.py` — scoped disabled workspace row QSS.
- Synchronized ADR, reviews, acceptance, register, roadmap, and handoff files.

## Decisions and constraints

- Shared checkout writer: `parent`; no worktree or second writer was used.
- Keep one source of truth in the central theme stylesheet; no widget-local
  stylesheet or new workspace state owner.
- Architecture window: `Maxwell the 6th / Luna max` — `NO_CONCLUSION` after
  two bounded waits and closure; no child architecture PASS is claimed.
- Independent review: `Kuhn the 6th / Luna max` — `NO_CONCLUSION` after two
  bounded waits and closure; no independent PASS is claimed.
- Parent review: `PASS`; simplification assessment: `PASS`.
- Embedded C/C++, MCU, RTOS, and manufacturer requirements are not applicable.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| Workspace disabled source probe | `PASS` | Existing disabled callers and activation routes remain present. |
| QSS specificity probe | `PASS` | Selected-disabled rule remains more specific. |
| 3-theme × 4-accent contrast probe | `PASS` | `text_muted` on `surface_2`, minimum `5.14`. |
| Compile | `PASS` | `D211-COMPILEALL=PASS`. |
| Ruff | `PASS` | `D211-RUFF=PASS`. |
| Format | `PASS` | `D211-FORMAT=PASS`. |
| Presentation contract audit | `PASS` | `D211-PRESENTATION-AUDIT=PASS`. |
| Windows PowerShell 5.1 package | `PASS` | `D211-PACKAGE-BUILD-PS51=PASS`; artifact identity is bound below. |
| PowerShell 7 package | `PASS` | `D211-PACKAGE-BUILD-PS7=PASS`; final candidate identity is bound below. |

## Unrun checks and reason

GUI/QApplication, native QSS/item painting, screenshots, accessibility tree,
DPI, live workspace refresh, clean-machine, cross-machine, signing,
installer/updater, legal, support, permission/disk-pressure, hard-power, and
release-owner checks were not run under the active no-launch or external-
authorization policy. No unit-test asset was created or run.

## Known risks and limits

- Native QSS specificity and platform style metrics still require an
  authorized runtime visual pass.
- The checkout has no Git baseline, and both delegated review windows returned
  `NO_CONCLUSION`; parent review is the only PASS review claim.
- Separate PyInstaller invocations may differ in artifact bytes; each manifest
  must remain independently bound.

## Acceptance and evidence IDs

- Acceptance: `S262`
- Evidence: `D211-WORKSPACE-DISABLED-SOURCE-PROBE=PASS`,
  `D211-QSS-SPECIFICITY-SOURCE-PROBE=PASS`,
  `D211-DISABLED-CONTRAST-PROBE=PASS combinations=12 min=5.14`,
  `D211-COMPILEALL=PASS`, `D211-RUFF=PASS`, `D211-FORMAT=PASS`,
  `D211-PRESENTATION-AUDIT=PASS`, `D211-PACKAGE-BUILD-PS51=PASS`,
  `D211-PACKAGE-BUILD-PS7=PASS`, `D211-PACKAGE-IDENTITY-PROBE=PASS`,
  `D211-SIMPLIFICATION-ASSESSMENT=PASS`,
  `D211-ARCHITECT-REVIEW=NO_CONCLUSION`,
  `D211-INDEPENDENT-REVIEW=NO_CONCLUSION`.

## Next owner and next action

- Owner: Architect / QA.
- Action: continue the user-feature and visual-state audit while keeping the
  workspace provider, file activation, and release gates explicit.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and `QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size: `698F468119A535A3099D760BB671126446A6A636233DBF96DBE300C88CFA463D` /
  `38,563,787` bytes
- Source revision: `tree-sha256:a77db57b0259ec654c5e0774352b070291ab02105b26e077d76e90992afbf741`
- Root/dist identity: both paths match the final PS7 candidate.

## Disposition

`accepted-with-limits`: disabled workspace rows are now visually explicit;
native rendering and enterprise release gates remain open.

## Required final marker

`EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded`

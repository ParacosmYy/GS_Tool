# Handoff: 2026-08-11-d212-localized-untitled-tab

| Field | Value |
|---|---|
| ID | `2026-08-11-d212-localized-untitled-tab` |
| Delivery / slice | `D212 / UI-114 / ARCH-197 Localized untitled tab title` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-11T23:59:00+08:00` |

## User outcome

Pathless documents now use the active English or Simplified Chinese label,
and already-open tabs reproject that label when the locale changes. Saved
documents retain their file basenames and dirty tabs retain the leading `*`.

## Scope and boundaries

### In scope

- `document.untitled` in the existing two-locale catalog.
- Locale-aware `_tab_title` projection and existing-tab refresh callback.
- Source probes, static checks, dual-shell package identity, and release-boundary
  records.

### Out of scope

- Native tab rendering, font fallback, accessibility, DPI, GUI/QApplication,
  EXE startup, screenshots, unit-test assets, file renaming, document service
  behavior, and release closure.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | `parent` | Boundary decision, integration, final review, handoff |
| Project Manager | `parent` | Plan, dependency, risk, and status record |
| Product | `user outcome` | Eliminate mixed-language untitled tab labels |
| Developer | `parent` | Existing i18n and presentation-locale boundary |
| QA | `parent` | Static, contract, package, and release-boundary checks |

## Changed files and modules

- `src/quillforge/presentation/i18n.py` — localized untitled label.
- `src/quillforge/presentation/main_window.py` — locale-aware title projection
  and existing-tab refresh wiring.
- `src/quillforge/presentation/presentation_locale_coordinator.py` — typed
  title-refresh port and deterministic refresh ordering.
- Synchronized ADR, reviews, acceptance, register, roadmap, and handoff files.

## Decisions and constraints

- Shared checkout writer: `parent`; no worktree or second writer was used.
- Keep title projection in the existing presentation boundary; do not add a
  title service or mutate document state during locale changes.
- Architecture window: `Faraday the 6th / Luna max` — `NO_CONCLUSION` after
  two bounded waits and closure; no child architecture PASS is claimed.
- Independent review: `Kepler the 6th / Luna max` — `NO_CONCLUSION` after two
  bounded waits and closure; no independent PASS is claimed.
- Parent review: `PASS`; simplification assessment: `PASS`.
- Embedded C/C++, MCU, RTOS, and manufacturer requirements are not applicable.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| Localized tab-title source probe | `PASS` | Both locale keys, locale-aware branch, and MainWindow wiring present. |
| Locale refresh-order probe | `PASS` | Existing titles refresh after editor shell locale and before icons. |
| Fixed-title regression probe | `PASS` | No fixed `"Untitled"` literal remains in `main_window.py`. |
| i18n key probe | `PASS` | English and Simplified Chinese catalogs are covered. |
| Compile | `PASS` | `D212-COMPILEALL=PASS`. |
| Ruff | `PASS` | `D212-RUFF=PASS`. |
| Format | `PASS` | `D212-FORMAT=PASS`. |
| Presentation contract audit | `PASS` | `D212-PRESENTATION-AUDIT=PASS`. |
| Windows PowerShell 5.1 package | `PASS` | `D212-PACKAGE-BUILD-PS51=PASS`. |
| PowerShell 7 package | `PASS` | `D212-PACKAGE-BUILD-PS7=PASS`. |
| Final package identity | `PASS` | `D212-PACKAGE-IDENTITY-PROBE=PASS`. |

## Unrun checks and reason

GUI/QApplication, native QTabWidget painting, screenshots, accessibility
tree, font fallback, DPI, runtime locale/open/save/recovery interaction,
clean-machine, cross-machine, signing, installer/updater, legal, support,
permission/disk-pressure, hard-power, and release-owner checks were not run
under the active no-launch or external-authorization policy. No unit-test
asset was created or run.

## Known risks and limits

- Native tab metrics and font fallback still require an authorized runtime
  visual pass.
- The checkout has no Git baseline, and both delegated review windows returned
  `NO_CONCLUSION`; parent review is the only PASS review claim.
- Separate PyInstaller invocations may differ in artifact bytes; each
  manifest remains independently bound.

## Acceptance and evidence IDs

- Acceptance: `S263`
- Evidence: `D212-LOCALIZED-TAB-TITLE-SOURCE-PROBE=PASS`,
  `D212-LOCALE-REFRESH-ORDER-PROBE=PASS`,
  `D212-FIXED-UNTITLED-REGRESSION-PROBE=PASS`,
  `D212-I18N-KEY-PROBE=PASS locales=2`, `D212-COMPILEALL=PASS`,
  `D212-RUFF=PASS`, `D212-FORMAT=PASS`, `D212-PRESENTATION-AUDIT=PASS`,
  `D212-PACKAGE-BUILD-PS51=PASS`, `D212-PACKAGE-BUILD-PS7=PASS`,
  `D212-PACKAGE-IDENTITY-PROBE=PASS`,
  `D212-SIMPLIFICATION-ASSESSMENT=PASS`,
  `D212-ARCHITECT-REVIEW=NO_CONCLUSION`,
  `D212-INDEPENDENT-REVIEW=NO_CONCLUSION`.

## Next owner and next action

- Owner: Architect / QA.
- Action: continue the user-feature and visual-state audit while keeping the
  locale refresh, document tab, file activation, and release gates explicit.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and `QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size: `4DA722606FD21A4C86B769BE0668F71EA4D31866CD21E6337FE183440883687C` /
  `38,560,934` bytes
- Source revision: `tree-sha256:e222e7076ea5a3b6bddb0c3fc2e824953de60cefdcb7222472f96e7468edaaf1`
- Root/dist identity: both paths match the final PS7 candidate.

## Disposition

`accepted-with-limits`: localized pathless tab labels are projected through
the existing locale boundary; native rendering and enterprise release gates
remain open.

## Required final marker

`EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded`

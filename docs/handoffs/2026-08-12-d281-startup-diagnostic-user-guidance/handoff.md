# Handoff: 2026-08-12-d281-startup-diagnostic-user-guidance

| Field | Value |
|---|---|
| ID | `2026-08-12-d281-startup-diagnostic-user-guidance` |
| Delivery / slice | `D281 / ARCH-251 Startup diagnostic user guidance` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Shared checkout | `D:\Workplace\Agent_Workplace\QuillForge` |

## User outcome

README now explains how to collect and interpret the existing no-window
startup report when the portable executable shows no window, including the
D280 settings preflight and the normal-startup error-log location.

## Scope and boundaries

- Documentation-only change in `README.md`.
- Describes existing command syntax, fields, exit codes, privacy boundary,
  and native-startup limitation.
- No runtime, settings schema, package, installer, updater, registry, or
  user-data behavior changed.
- No EXE/Qt launch, native dialog, unit-test asset, or worktree was used.

## Team roles and ownership

| Role | Owner / agent | Responsibility |
|---|---|---|
| Architect | parent; Lagrange consultation | Documentation boundary and ownership review |
| Developer | parent | README guidance update |
| QA | parent | README assertion, source diagnostic, compile, and project checks |
| Independent reviewer | Bernoulli consultation | Initial revise; bounded follow-up PASS |

## Changed files and modules

- `README.md` — startup preflight interpretation and no-launch boundary.
- D281 ADR, review, and handoff records.

## Decisions and constraints

- Keep the source implementation in `quillforge.app`; README only describes
  the existing contract.
- Do not promise that a passing no-window report proves native Windows startup.
- The current portable artifact remains the D280 candidate because this slice
  does not change packaged source or binary content.

## Public-source applicability

Python 3.12 `argparse`/`pathlib` documentation and the existing source
contract are applicable references. No manufacturer requirement applies; this
slice contains no embedded C/C++, MCU, BSP/HAL, RTOS, ISR/DMA, driver,
bootloader, or firmware change. Embedded workflow and simplifier: `N/A`.

## Verification commands and results

| Evidence | Result |
|---|---|
| README contract | `D281-README-PREFLIGHT=PASS` |
| Source startup diagnostic | `D281-SOURCE-DIAGNOSTIC=PASS settings_preflight=passed schema=3` |
| Compileall | `D281-COMPILEALL=PASS` |
| Ruff/formatting | `D281-RUFF=PASS`; `D281-FORMAT=PASS` |
| Project checks | `D281-CHECK=PASS` |
| Parent review | `PASS` |
| Independent review | `REVISE` initially; follow-up `PASS` after README correction |
| Simplification | `PASS` |
| Candidate identity | unchanged from D280: `8E2D076F9AA685F4836525438163612B1A192C6517B5D92E0485E10947B7196F`, `38,582,863` bytes |

## Unrun checks and reason

Native EXE/Qt startup, clean-machine behavior, signing, installer/update,
registry, cross-machine repeatability, and release-owner acceptance remain
unrun under the active no-launch/non-destructive policy. Release verification
remains expected `NO-GO` with the existing artifact-bound mechanical failures.

## Known risks and limits

The guidance improves issue collection but cannot repair an unobserved native
startup failure by itself. The report may pass while a later Qt/window/runtime
stage still fails; the startup-error log remains the authoritative exception
record when normal entry reaches its guard.

## Acceptance and evidence IDs

- Acceptance: `S321`.
- Architecture slice: `ARCH-251`.
- Evidence: `D281-README-PREFLIGHT=PASS`, `D281-CHECK=PASS`,
  `D281-ARCHITECTURE-CONSULTATION=NO_CONCLUSION`,
  `D281-INDEPENDENT-REVIEW=PASS_AFTER_CORRECTION`, and
  `D281-SIMPLIFICATION-ASSESSMENT=PASS`.

## Next owner and next action

- Owner: user / authorized QA.
- Action: run the documented preflight if native startup still fails and
  attach both the JSON report and `%LOCALAPPDATA%\QuillForge\startup-error.log`
  when present.

## Artifact information

- Artifact paths: `dist/QuillForge.exe` and `QuillForge.exe`.
- Candidate identity: D280 unchanged, SHA-256
  `8E2D076F9AA685F4836525438163612B1A192C6517B5D92E0485E10947B7196F`,
  `38,582,863` bytes.

## Disposition

`accepted-with-limits`: startup diagnostics are now discoverable and
interpretable; native startup and enterprise release gates remain open.

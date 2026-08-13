# Handoff: 2026-08-11-d199-command-menu-contract

| Field | Value |
|---|---|
| ID | `2026-08-11-d199-command-menu-contract` |
| Delivery / slice | `D199 / ARCH-185 Command menu contract closure` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-11T23:59:00+08:00` |

## User outcome

Command registrations can no longer disappear silently because of an
unsupported menu ID. The application command boundary now rejects invalid
menu integration data before admission, while the existing four-menu
projection and command behavior remain unchanged.

## Scope and boundaries

### In scope

- The supported `MenuId` contract in `application.commands`.
- Registry admission validation and presentation menu projection reuse.
- Static dependency, compile, package, and provenance evidence.

### Out of scope

- Dynamic menus, Tools fallback, command execution, callbacks, shortcuts,
  locale behavior, plugin trust, enablement, permissions, GUI/QApplication,
  EXE launch, screenshots, and runtime release evidence.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | `parent` | Integration, boundary decision, final review, and handoff |
| Project Manager | `parent` | Plan, dependency, risk, and status record |
| Product | `user outcome` | Prevent invisible plugin commands |
| Developer | `parent` | Application command contract and presentation reuse |
| QA | `parent` | Contract, static, package, and handoff probes |

## Changed files and modules

- `src/quillforge/application/commands.py` — typed supported menu IDs and
  registration guard.
- `src/quillforge/presentation/command_surface.py` — menu projection reuses
  the application-owned ordered contract.
- Synchronized ADR, review, acceptance, roadmap, register, and handoff files.

## Decisions and constraints

- Shared checkout writer: `parent`; no worktree or second writer was used.
- The explicit contract is `file`, `edit`, `tools`, and `help`; invalid values
  raise at registration rather than being dropped by presentation.
- Architecture window: `Carson the 6th / Luna max` — `NO_CONCLUSION` after two
  bounded waits and closure; no child PASS is claimed.
- Independent review: `Dirac the 6th / Luna max` — `NO_CONCLUSION` after two
  bounded waits and closure; no child PASS is claimed.
- Parent review: `PASS`; simplification assessment: `PASS`.
- Embedded C/C++, MCU, RTOS, and manufacturer requirements are not applicable.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| Valid/invalid registry boundary probe | `PASS` | `D199-MENU-CONTRACT-PROBE=PASS valid=4 invalid=reject`. |
| Dependency boundary probe | `PASS` | `D199-BOUNDARY-SCOPE-PROBE=PASS`; application remains Qt-free. |
| Static source audit | `PASS` | `D199-STATIC-AUDIT=PASS`. |
| Compile, Ruff, format | `PASS` | `D199-COMPILE-RUFF-FORMAT=PASS`. |
| Windows PowerShell 5.1 package | `PASS` | `D199-PACKAGE-BUILD-PS51=PASS`; EXE not launched. |
| PowerShell 7 package | `PASS` | `D199-PACKAGE-BUILD-PS7=PASS`; EXE not launched. |

## Unrun checks and reason

GUI/QApplication, native menu rendering, accessibility, DPI, EXE startup,
runtime plugin integration, clean-machine, cross-machine, signing,
installer/updater, legal, support, and release-owner checks were not run
because the active policy prohibits software launch and external release
actions. No unit-test asset was created or run.

## Known risks and limits

- Rejecting unknown menu IDs is an intentional compatibility tightening for
  integrations that depended on silent loss; it surfaces the error earlier.
- The checkout has no Git baseline, and both delegated review windows returned
  `NO_CONCLUSION`; parent review is the only PASS review claim.
- Native rendering, runtime accessibility, plugin integration, and enterprise
  release gates remain open.

## Acceptance and evidence IDs

- Acceptance: `S251`
- Evidence: `D199-MENU-CONTRACT-PROBE=PASS valid=4 invalid=reject`,
  `D199-BOUNDARY-SCOPE-PROBE=PASS`, `D199-STATIC-AUDIT=PASS`,
  `D199-COMPILE-RUFF-FORMAT=PASS`, `D199-PACKAGE-BUILD-PS51=PASS`,
  `D199-PACKAGE-BUILD-PS7=PASS`, `D199-ARCHITECT-REVIEW=NO_CONCLUSION`,
  `D199-INDEPENDENT-REVIEW=NO_CONCLUSION`.

## Next owner and next action

- Owner: `architect` / QA.
- Action: use the closed `MenuId` contract for future command integrations;
  keep dynamic menu requirements as a separately scoped architecture change.

## Artifact information

- Historical D199 package: `dist/QuillForge.exe` and `QuillForge.exe`.
- SHA-256 / size: `5BAD7755BD227CF3C79F06920B2CF23348ED2449F7BFF65121919DA1773A746C` /
  `38,561,446` bytes.
- Source revision: `tree-sha256:ed2fe679927e6956e8b58b8419df5c13c42fd935dcea9ee3ea8cb4a2ed951078`.
- The later D200 packaging determinism slice supersedes this as the current
  candidate and records the cross-shell-stable source revision.

## Disposition

`accepted-with-limits`: the invisible-command admission gap is closed at the
application boundary; runtime and enterprise release gates remain open.

## Required final marker

`EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded`

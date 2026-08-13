# Handoff: 2026-08-11-d219-application-error-taxonomy

| Field | Value |
|---|---|
| ID | `2026-08-11-d219-application-error-taxonomy` |
| Delivery / slice | `D219 / ARCH-203 Remaining application error taxonomy` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Shared checkout | `D:\Workplace\Agent_Workplace\QuillForge` |

## User outcome

The remaining Qt-free application boundaries now expose stable application
error categories. Existing callers can still catch `ValueError` and
`RuntimeError`, while command, plugin, session, recovery, release, and event
policy no longer leak generic exception ownership.

## Scope and boundaries

- Reused `ApplicationValidationError` for application-owned validation.
- Reused `ApplicationStateError` for application-owned state failures.
- Covered `commands`, `events`, `plugin_enablement`, `plugin_execution`,
  `plugin_governance`, `plugin_host`, `ports`, `recovery`, and
  `release_metadata`.
- Preserved exact messages, built-in compatibility, validation order,
  dataclass/protocol shapes, and policy behavior.
- Left dedicated recovery-channel exceptions and all other layers unchanged.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | `parent` | Boundary decision, integration, final review, handoff |
| Project Manager | `parent` | Plan, dependency, risk, and status record |
| Product | `architecture outcome` | Stable application failure contract |
| Developer | `parent` | Focused category migration |
| QA | `parent` | Compatibility, static, package, and release-boundary checks |

## Changed files and modules

- `src/quillforge/application/commands.py`
- `src/quillforge/application/events.py`
- `src/quillforge/application/plugin_enablement.py`
- `src/quillforge/application/plugin_execution.py`
- `src/quillforge/application/plugin_governance.py`
- `src/quillforge/application/plugin_host.py`
- `src/quillforge/application/ports.py`
- `src/quillforge/application/recovery.py`
- `src/quillforge/application/release_metadata.py`
- Synchronized ADR, reviews, acceptance, register, roadmap, architecture,
  task, release, index, and handoff files.

## Decisions and constraints

- Shared checkout writer: `parent`; no worktree or second writer was used.
- Architecture role: `Sagan the 6th / Luna max` — `NO_CONCLUSION` after two
  bounded waits and closure.
- Independent role: `Pasteur the 6th / Luna max` — `NO_CONCLUSION` after two
  bounded waits and closure.
- Parent review: `PASS`; simplification assessment: `PASS`.
- Embedded C/C++, MCU, RTOS, and manufacturer requirements are not applicable.

## Verification commands and results

| Evidence | Result | Notes |
|---|---|---|
| Application taxonomy probe | `PASS` | `D219-APPLICATION-ERROR-TAXONOMY-PROBE=PASS files=9 direct_generic_raises=0`. |
| Compile | `PASS` | `D219-COMPILEALL=PASS`. |
| Ruff | `PASS` | `D219-RUFF=PASS`. |
| Format | `PASS` | `D219-FORMAT=PASS`. |
| Presentation audit | `PASS` | `D219-PRESENTATION-AUDIT=PASS`. |
| PS5.1 package | `PASS` | `D219-PACKAGE-BUILD-PS51=PASS`. |
| PS7 package | `PASS` | `D219-PACKAGE-BUILD-PS7=PASS`. |
| Package identity | `PASS` | `D219-PACKAGE-IDENTITY-PROBE=PASS`; root/dist SHA `4CB787965CD11F45D996705C7731EA5D9D613DEEFBC3A188AC2289682B3F2A5B`, 38,564,673 bytes. |

## Public-source applicability

Python 3.12 application-layer code applies. Python's public [built-in
exceptions documentation](https://docs.python.org/3.12/library/exceptions.html)
is the applicable first-party source for preserving built-in inheritance.
Public CloudWeGo material remains an engineering reference only; no private
ByteDance standard, certification, or compliance claim is made. Embedded
C/C++, MCU, RTOS, and manufacturer requirements are not applicable.

## Unrun checks and reason

- GUI/QApplication and EXE startup, event-thread timing, runtime filesystem
  and plugin-host exchange, screenshot, accessibility, font/DPI,
  clean-machine, cross-machine, signing, installer/update, legal, support,
  permission/disk-pressure, hard-power, and release-owner checks were not run
  under the active no-launch or external-authorization policy.
- Unit tests, mocks, fixtures, harnesses, and test-only assets were not created
  or run under the project policy.
- `scripts/verify_release_handoff.ps1` is expected to remain `no-go` with the
  existing 10 open gates and three historical artifact-binding failures.

## Known risks and limits

- The checkout has no Git baseline; delegated architecture and independent
  windows did not conclude.
- Exception category behavior is statically and offscreen-probe validated;
  native callback timing and runtime interleavings are not proven.
- Separate PyInstaller invocations can differ in bytes; the current PS7
  manifest is bound to its final candidate.

## Acceptance and evidence IDs

- Acceptance: `S270`
- Evidence: `D219-APPLICATION-ERROR-TAXONOMY-PROBE=PASS files=9 direct_generic_raises=0`,
  `D219-ARCHITECT-REVIEW=NO_CONCLUSION`,
  `D219-INDEPENDENT-REVIEW=NO_CONCLUSION`,
  `D219-SIMPLIFICATION-ASSESSMENT=PASS`.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and `QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size: `4CB787965CD11F45D996705C7731EA5D9D613DEEFBC3A188AC2289682B3F2A5B` /
  `38,564,673` bytes
- Source revision: `tree-sha256:4dd7533b7f24966c051c9c199a44d1e8f7a7a4ae1acf4d967f04f81398fd259f`
- Root/dist identity: both paths match the final PS7 candidate.

## Next owner and next action

- Owner: Architect / QA.
- Action: continue the remaining visual-state, runtime, and release-gate
  audit; do not mark the broader product goal complete.

## Disposition

`accepted-with-limits`: remaining application-owned error categories now use a
stable, built-in-compatible contract; runtime and enterprise release gates
remain open.

## Required final marker

`EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded`

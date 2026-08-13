# Handoff: 2026-08-11-d218-workspace-search-error-taxonomy

| Field | Value |
|---|---|
| ID | `2026-08-11-d218-workspace-search-error-taxonomy` |
| Delivery / slice | `D218 / ARCH-202 Workspace-search application error taxonomy` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Shared checkout | `D:\Workplace\Agent_Workplace\QuillForge` |

## User outcome

Workspace search now exposes stable application-owned validation and type
categories. Existing callers can still catch the original built-in
`ValueError` and `TypeError`, while the application boundary no longer leaks
generic exception ownership.

## Scope and boundaries

- Added `ApplicationTypeError` beside the existing application error types.
- Migrated the 39 validation/provider-result branches in
  `application.workspace_search` to the matching category.
- Preserved exact messages, validation order, dataclass shape, search policy,
  provider protocol, directory capability, and Qt-free imports.
- Left the two `Path.relative_to()` built-in `ValueError` catches intact.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | `parent` | Boundary decision, integration, final review, handoff |
| Project Manager | `parent` | Plan, dependency, risk, and status record |
| Product | `architecture outcome` | Stable workspace-search failure contract |
| Developer | `parent` | Focused error-category migration |
| QA | `parent` | Compatibility, static, package, and release-boundary checks |

## Changed files and modules

- `src/quillforge/application/errors.py` — added `ApplicationTypeError`.
- `src/quillforge/application/workspace_search.py` — category mapping for
  policy, value-object, request, and provider-result validation.
- Synchronized ADR, reviews, acceptance, register, roadmap, architecture,
  task, release, index, and handoff files.

## Decisions and constraints

- Shared checkout writer: `parent`; no worktree or second writer was used.
- Architecture role: `Pascal the 6th / Luna max` — `NO_CONCLUSION` after two
  bounded waits and closure.
- Independent role: `James the 6th / Luna max` — `NO_CONCLUSION` after two
  bounded waits and closure.
- Parent review: `PASS`; simplification assessment: `PASS`.
- Embedded C/C++, MCU, RTOS, and manufacturer requirements are not applicable.

## Verification commands and results

| Evidence | Result | Notes |
|---|---|---|
| Workspace-search taxonomy probe | `PASS` | `D218-WORKSPACE-SEARCH-TAXONOMY-PROBE=PASS branches=39`. |
| Compile | `PASS` | `D218-COMPILEALL=PASS`. |
| Ruff | `PASS` | `D218-RUFF=PASS`. |
| Format | `PASS` | `D218-FORMAT=PASS`. |
| Presentation audit | `PASS` | `D218-PRESENTATION-AUDIT=PASS`. |
| PS5.1 package | `PASS` | `D218-PACKAGE-BUILD-PS51=PASS`. |
| PS7 package | `PASS` | `D218-PACKAGE-BUILD-PS7=PASS`. |
| Package identity | `PASS` | `D218-PACKAGE-IDENTITY-PROBE=PASS`. |

## Public-source applicability

Python 3.12 application-layer code applies. Python's public [built-in
exceptions documentation](https://docs.python.org/3.12/library/exceptions.html)
is the applicable first-party source for preserving built-in inheritance.
Public CloudWeGo material remains an engineering reference only; no private
ByteDance standard, certification, or compliance claim is made. Embedded
C/C++, MCU, RTOS, and manufacturer requirements are not applicable.

## Unrun checks and reason

- GUI/QApplication and EXE startup, runtime filesystem traversal, provider
  timing, cancellation interleavings, screenshot, accessibility, font/DPI,
  clean-machine, cross-machine, signing, installer/update, legal, support,
  permission/disk-pressure, hard-power, and release-owner checks were not run
  under the active no-launch or external-authorization policy.
- Unit tests, mocks, fixtures, harnesses, and test-only assets were not created
  or run under the project policy.
- `scripts/verify_release_handoff.ps1` is expected to remain `no-go` with the
  existing 10 open gates and three stale artifact-binding failures.

## Known risks and limits

- The checkout has no Git baseline; delegated architecture and independent
  windows did not conclude.
- Runtime provider behavior and concurrency/cancellation ordering are not
  proven by this taxonomy-only change.
- Separate PyInstaller invocations can differ in bytes; each manifest is bound
  to its own candidate.

## Acceptance and evidence IDs

- Acceptance: `S269`
- Evidence: `D218-WORKSPACE-SEARCH-TAXONOMY-PROBE=PASS branches=39`,
  `D218-ARCHITECT-REVIEW=NO_CONCLUSION`,
  `D218-INDEPENDENT-REVIEW=NO_CONCLUSION`,
  `D218-SIMPLIFICATION-ASSESSMENT=PASS`.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and `QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size: `C3D27E840CEFC1677426BEFCA6B80A297F23CE60644D9DB143928AD566770A12` /
  `38,564,742` bytes
- Source revision: `tree-sha256:2d80e370c3eeaa631f05802b5243dc63e29a751931ccecf63f2de3fbb73bda25`
- Root/dist identity: both paths match the final PS7 candidate.

## Next owner and next action

- Owner: Architect / QA.
- Action: continue the remaining application taxonomy, visual-state, runtime,
  and release-gate audit; do not mark the broader product goal complete.

## Disposition

`accepted-with-limits`: workspace-search now has an explicit application error
taxonomy with built-in compatibility; runtime and enterprise release gates
remain open.

## Required final marker

`EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded`

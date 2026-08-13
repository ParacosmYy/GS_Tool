# Handoff: 2026-08-11-d203-shared-directory-capability

| Field | Value |
|---|---|
| ID | `2026-08-11-d203-shared-directory-capability` |
| Delivery / slice | `D203 / ARCH-189 Shared directory capability for workspace search` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-11T23:59:00+08:00` |

## User outcome

Workspace navigation and Find in Files now share one explicit directory
capability contract. Filesystem directory classification stays in adapters,
while application services retain search limits, cancellation, containment,
and user-facing error policy; this makes the enterprise boundary easier to
extend without duplicating filesystem rules.

## Scope and boundaries

### In scope

- `application.ports.DirectoryCapability`.
- `WorkspaceProvider` and `WorkspaceSearchProvider` contract reuse.
- Search-service and file-search-adapter directory validation.
- Static contract, compile, lint, format, package, and artifact evidence.

### Out of scope

- Search limits, cancellation, traversal, symlink/reparse handling, and
  result validation changes.
- Workspace navigation policy changes beyond the D202 shared capability.
- UI signals, asynchronous dispatch, native dialogs, GUI/QApplication, EXE
  startup, live filesystem-race checks, unit-test assets, and release closure.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | `parent` | Integration, boundary decision, final review, handoff |
| Project Manager | `parent` | Plan, dependency, risk, and status record |
| Product | `user outcome` | Consistent, replaceable workspace/search boundary |
| Developer | `parent` | Shared Port and adapter implementation |
| QA | `parent` | Static, package, and release-boundary checks |

## Changed files and modules

- `src/quillforge/application/ports.py` — shared directory capability.
- `src/quillforge/application/workspace_search.py` — provider delegation.
- `src/quillforge/infrastructure/workspace_search_provider.py` — adapter
  implementation and reuse.
- Synchronized ADR, reviews, acceptance, register, roadmap, and handoff files.

## Decisions and constraints

- Shared checkout writer: `parent`; no worktree or second writer was used.
- Keep search limits, cancellation, containment, traversal, diagnostics, and
  error wording in their current owners; only directory classification is
  shared.
- Architecture window: `Nash the 6th / Luna max` — `NO_CONCLUSION` after two
  bounded waits and closure; no child architecture PASS is claimed.
- Independent review: `Turing the 6th / Luna max` — `NO_CONCLUSION` after two
  bounded waits and closure; no independent PASS is claimed.
- Parent review: `PASS`; simplification assessment: `PASS`.
- Embedded C/C++, MCU, RTOS, and manufacturer requirements are not applicable.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| Shared directory capability probe | `PASS` | `D203-DIRECTORY-CAPABILITY-PROBE=PASS shared=1 workspace=1 search=1 application_is_dir=0`. |
| Compile | `PASS` | `D203-COMPILEALL=PASS`. |
| Ruff | `PASS` | `D203-RUFF=PASS`. |
| Format | `PASS` | `D203-FORMAT=PASS`. |
| Presentation contract audit | `PASS` | `D203-PRESENTATION-AUDIT=PASS`. |
| Windows PowerShell 5.1 package | `PASS` | SHA `013BDC16E005152F4C23084ACFDEB5B9B4D08DE2F1D1FA53582364205BC4F4B4`, 38,562,945 bytes; source revision matches PS7. |
| PowerShell 7 package | `PASS` | Final candidate SHA `E656ECA9589F0D3022D15D8D65B33A6A0B0CECC8BAEBD72E4E893184133718DD`, 38,562,245 bytes; source revision matches PS5. |
| Root/dist/package identity | `PASS` | `D203-PACKAGE-IDENTITY-PROBE=PASS`; root and dist match. |

## Unrun checks and reason

GUI/QApplication, native file dialog and filesystem rendering, screenshots,
accessibility tree, DPI, EXE startup, live provider races, search cancellation
timing, clean-machine, cross-machine, signing, installer/updater, legal,
support, permission/disk-pressure, hard-power, and release-owner checks were
not run under the active no-launch or external-authorization policy. No
unit-test asset was created or run.

## Known risks and limits

- The shared capability preserves the current adapters' `Path.is_dir()`
  behavior, but live filesystem races and provider-specific runtime behavior
  still need authorized evidence.
- The checkout has no Git baseline, and both delegated review windows returned
  `NO_CONCLUSION`; parent review is the only PASS review claim.
- Separate PyInstaller invocations can differ in artifact bytes; each
  manifest binds its own artifact. The final PS7 candidate is the current
  identity.

## Acceptance and evidence IDs

- Acceptance: `S255`
- Evidence: `D203-DIRECTORY-CAPABILITY-PROBE=PASS shared=1 workspace=1 search=1 application_is_dir=0`,
  `D203-COMPILEALL=PASS`, `D203-RUFF=PASS`, `D203-FORMAT=PASS`,
  `D203-PRESENTATION-AUDIT=PASS`, `D203-PACKAGE-BUILD-PS51=PASS`,
  `D203-PACKAGE-BUILD-PS7=PASS`, `D203-PACKAGE-IDENTITY-PROBE=PASS`,
  `D203-ARCHITECT-REVIEW=NO_CONCLUSION`,
  `D203-INDEPENDENT-REVIEW=NO_CONCLUSION`.

## Next owner and next action

- Owner: Architect / QA.
- Action: continue only after identifying another concrete boundary or
  user-visible defect; retain `DirectoryCapability` as the canonical local
  directory predicate seam.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and `QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size: `E656ECA9589F0D3022D15D8D65B33A6A0B0CECC8BAEBD72E4E893184133718DD` /
  `38,562,245` bytes
- Source revision: `tree-sha256:fbc2b87fd8f1980d9738d8f8eaeeba6f9e1b1b0643d06abcc537ff9b00127d24`
- PS5 package evidence: `013BDC16E005152F4C23084ACFDEB5B9B4D08DE2F1D1FA53582364205BC4F4B4` /
  `38,562,945` bytes; final current candidate is the PS7 package.

## Disposition

`accepted-with-limits`: the workspace/search directory predicate is closed as
a shared typed capability contract; live filesystem and enterprise release
gates remain open.

## Required final marker

`EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded`

# Handoff: 2026-08-11-d202-workspace-provider-directory

| Field | Value |
|---|---|
| ID | `2026-08-11-d202-workspace-provider-directory` |
| Delivery / slice | `D202 / ARCH-188 Workspace provider directory capability` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-11T23:59:00+08:00` |

## User outcome

Workspace open and directory navigation keep their existing behavior while
the application layer no longer reaches directly into a concrete filesystem
predicate. The directory capability is now explicit at the existing provider
boundary, making the file/workspace architecture easier to replace and audit.

## Scope and boundaries

### In scope

- `application.ports.WorkspaceProvider.is_directory`.
- `WorkspaceService` delegation for root and child-directory validation.
- `FileWorkspaceProvider` filesystem implementation.
- Static contract, compile, lint, format, package, and artifact evidence.

### Out of scope

- Workspace search provider validation.
- Document path normalization and document persistence.
- Directory enumeration policy, symlink traversal policy, UI signals,
  asynchronous dispatch, native dialogs, GUI/QApplication, and EXE startup.
- Unit-test assets and external release-gate closure.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | `parent` | Integration, boundary decision, final review, handoff |
| Project Manager | `parent` | Plan, dependency, risk, and status record |
| Product | `user outcome` | Replaceable, auditable workspace boundary |
| Developer | `parent` | Port and adapter implementation |
| QA | `parent` | Static, package, and release-boundary checks |

## Changed files and modules

- `src/quillforge/application/ports.py` — typed directory capability.
- `src/quillforge/application/workspace.py` — delegated directory checks.
- `src/quillforge/infrastructure/workspace_provider.py` — filesystem adapter.
- Synchronized ADR, reviews, acceptance, register, roadmap, and handoff files.

## Decisions and constraints

- Shared checkout writer: `parent`; no worktree or second writer was used.
- Keep containment, normalization, limits, and error wording in their current
  owners; only the concrete filesystem predicate crosses the Port.
- Architecture window: `Singer the 6th / Luna max` — `NO_CONCLUSION` after
  two bounded waits and closure; no child architecture PASS is claimed.
- Independent review: `Bacon the 6th / Luna max` — `NO_CONCLUSION` after two
  bounded waits and closure; no independent PASS is claimed.
- Parent review: `PASS`; simplification assessment: `PASS`.
- Embedded C/C++, MCU, RTOS, and manufacturer requirements are not applicable.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| Port/adapter/application predicate probe | `PASS` | `D202-WORKSPACE-PREDICATE-PROBE=PASS port=1 adapter=1 app_direct_is_dir=0`. |
| Compile | `PASS` | `D202-COMPILEALL=PASS`. |
| Ruff | `PASS` | `D202-RUFF=PASS`. |
| Format | `PASS` | `D202-FORMAT=PASS`. |
| Presentation contract audit | `PASS` | `D202-PRESENTATION-AUDIT=PASS`. |
| Windows PowerShell 5.1 package | `PASS` | SHA `E439859B078DEFBA15879C7E06115BA424C465E596A8398F85EA4E10F421F93F`, 38,561,098 bytes; source revision matches PS7. |
| PowerShell 7 package | `PASS` | Final candidate SHA `EC1350EB7C09DCCEE1C1EEB7A9480A46218FEEF59C24112A799A3DCC88F1A2BB`, 38,560,175 bytes; source revision matches PS5. |
| Root/dist/package identity | `PASS` | `D202-PACKAGE-IDENTITY-PROBE=PASS`; root and dist match. |

## Unrun checks and reason

GUI/QApplication, native file dialog and filesystem rendering, screenshots,
accessibility tree, DPI, EXE startup, live provider races, clean-machine,
cross-machine, signing, installer/updater, legal, support, permission/disk-
pressure, hard-power, and release-owner checks were not run under the active
no-launch or external-authorization policy. No unit-test asset was created or
run.

## Known risks and limits

- The new capability preserves the current adapter's `Path.is_dir()` result,
  but live filesystem races and provider-specific error behavior still need
  authorized runtime evidence.
- The checkout has no Git baseline, and both delegated review windows returned
  `NO_CONCLUSION`; parent review is the only PASS review claim.
- Separate PyInstaller invocations can differ in artifact bytes; each
  manifest binds its own artifact. The final PS7 candidate is the current
  identity.

## Acceptance and evidence IDs

- Acceptance: `S254`
- Evidence: `D202-WORKSPACE-PREDICATE-PROBE=PASS port=1 adapter=1 app_direct_is_dir=0`,
  `D202-COMPILEALL=PASS`, `D202-RUFF=PASS`, `D202-FORMAT=PASS`,
  `D202-PRESENTATION-AUDIT=PASS`, `D202-PACKAGE-BUILD-PS51=PASS`,
  `D202-PACKAGE-BUILD-PS7=PASS`, `D202-PACKAGE-IDENTITY-PROBE=PASS`,
  `D202-ARCHITECT-REVIEW=NO_CONCLUSION`,
  `D202-INDEPENDENT-REVIEW=NO_CONCLUSION`.

## Next owner and next action

- Owner: Architect / QA.
- Action: continue the next distinct application/infrastructure boundary
  only after identifying a concrete direct-filesystem or policy leak; retain
  this provider capability as the canonical workspace directory seam.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and `QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size: `EC1350EB7C09DCCEE1C1EEB7A9480A46218FEEF59C24112A799A3DCC88F1A2BB` /
  `38,560,175` bytes
- Source revision: `tree-sha256:b184cf1d8ca3b2d3a317f809d4d98f0a9f0c2ff974c630b2ddc649135b636096`
- PS5 package evidence: `E439859B078DEFBA15879C7E06115BA424C465E596A8398F85EA4E10F421F93F` /
  `38,561,098` bytes; final current candidate is the PS7 package.

## Disposition

`accepted-with-limits`: workspace directory classification is closed as a
typed application/infrastructure contract; live filesystem and enterprise
release gates remain open.

## Required final marker

`EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded`

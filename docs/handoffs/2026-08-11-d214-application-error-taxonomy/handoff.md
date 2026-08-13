# Handoff: 2026-08-11-d214-application-error-taxonomy

| Field | Value |
|---|---|
| ID | `2026-08-11-d214-application-error-taxonomy` |
| Delivery / slice | `D214 / ARCH-199 Application error taxonomy` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-11T23:59:00+08:00` |

## User outcome

Document and workspace use cases now expose stable application-owned
validation/state categories while preserving the existing built-in exception
compatibility, messages, file behavior, workspace containment, and UI error
projection. This advances the enterprise architecture migration Phase 3
without moving domain conflicts or infrastructure details across boundaries.

## Scope and boundaries

### In scope

- `src/quillforge/application/errors.py` taxonomy.
- Focused `DocumentService` and `WorkspaceService` category migration.
- Static compatibility probes, dual-shell package identity, and release-boundary
  records.

### Out of scope

- Catch-all adapter wrapping, result-union redesign, domain conflict migration,
  GUI/QApplication, EXE startup, runtime filesystem interaction, unit-test
  assets, and release closure.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | `parent` | Boundary decision, integration, final review, handoff |
| Project Manager | `parent` | Plan, dependency, risk, and status record |
| Product | `architecture outcome` | Make application failure ownership explicit |
| Developer | `parent` | Qt-free taxonomy and focused service migration |
| QA | `parent` | Compatibility, static, package, and release-boundary checks |

## Changed files and modules

- `src/quillforge/application/errors.py` — stable application error classes.
- `src/quillforge/application/documents.py` — missing-target category.
- `src/quillforge/application/workspace.py` — validation/state categories.
- Synchronized ADR, reviews, acceptance, register, roadmap, and handoff files.

## Decisions and constraints

- Shared checkout writer: `parent`; no worktree or second writer was used.
- Preserve all existing messages and built-in catch compatibility.
- Architecture window: `Arendt the 6th / Luna max` — `NO_CONCLUSION` after two
  bounded waits and closure; no child architecture PASS is claimed.
- Independent review: `Fermat the 6th` — bounded static source review `PASS`.
  A separate `Aristotle the 6th / Luna max` window returned `NO_CONCLUSION`
  after two bounded waits and closure; the timeout remains a limitation.
- Parent review: `PASS`; simplification assessment: `PASS`.
- Embedded C/C++, MCU, RTOS, and manufacturer requirements are not applicable.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| Application taxonomy source probe | `PASS` | Categories and focused migrations are present. |
| Exception compatibility probe | `PASS` | New types remain catchable as ValueError/RuntimeError. |
| Compile | `PASS` | `D214-COMPILEALL=PASS`. |
| Ruff | `PASS` | `D214-RUFF=PASS`. |
| Format | `PASS` | `D214-FORMAT=PASS`. |
| Presentation contract audit | `PASS` | `D214-PRESENTATION-AUDIT=PASS`. |
| Windows PowerShell 5.1 package | `PASS` | `D214-PACKAGE-BUILD-PS51=PASS`. |
| PowerShell 7 package | `PASS` | `D214-PACKAGE-BUILD-PS7=PASS`. |
| Final package identity | `PASS` | `D214-PACKAGE-IDENTITY-PROBE=PASS`. |

## Unrun checks and reason

GUI/QApplication, EXE launch, runtime filesystem/provider interaction,
screenshots, accessibility, clean-machine, cross-machine, signing,
installer/updater, legal, support, permission/disk-pressure, hard-power, and
release-owner checks were not run under the active no-launch or
external-authorization policy. No unit-test asset was created or run.

## Known risks and limits

- Broader application services still contain generic validation exceptions;
  this delivery intentionally closes only the document/workspace slice.
- The checkout has no Git baseline. Independent PASS evidence is limited to
  Fermat's bounded static source review; the separate Aristotle window did not
  conclude, and runtime behavior remains unverified.
- Separate PyInstaller invocations may differ in artifact bytes; each
  manifest remains independently bound.

## Acceptance and evidence IDs

- Acceptance: `S265`
- Evidence: `D214-APPLICATION-ERROR-TAXONOMY-SOURCE-PROBE=PASS`,
  `D214-EXCEPTION-COMPATIBILITY-PROBE=PASS`, `D214-COMPILEALL=PASS`,
  `D214-RUFF=PASS`, `D214-FORMAT=PASS`, `D214-PRESENTATION-AUDIT=PASS`,
  `D214-PACKAGE-BUILD-PS51=PASS`, `D214-PACKAGE-BUILD-PS7=PASS`,
  `D214-PACKAGE-IDENTITY-PROBE=PASS`,
  `D214-SIMPLIFICATION-ASSESSMENT=PASS`, `D214-ARCHITECT-REVIEW=NO_CONCLUSION`,
  `D214-INDEPENDENT-REVIEW=PASS`,
  `D214-INDEPENDENT-SECOND-WINDOW=NO_CONCLUSION`.

## Next owner and next action

- Owner: Architect / QA.
- Action: continue Phase 3 error-contract coverage and the remaining
  user-feature, visual-state, and release-gate audit without widening this
  slice retroactively.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and `QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size: `22F957EDADEB98500D023605C7C621DC6E08C74E5DE9D6DC0ABF14FF70FD039F` /
  `38,564,414` bytes
- Source revision: `tree-sha256:0d41b3f355f000e86e877b5212f329eccb2e43954f4612ab0e0ba1a227701d2d`
- Root/dist identity: both paths match the final PS7 candidate.

## Disposition

`accepted-with-limits`: the document/workspace application error categories
are explicit and backward-compatible; broader taxonomy coverage, runtime, and
enterprise release gates remain open.

## Required final marker

`EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded`
